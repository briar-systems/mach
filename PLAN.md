# Plan: Per-Module .o Emission + Internal Linker

## Context

The self-hosted Mach compiler currently lowers all modules into a single monolithic MASM context, runs isel once, and emits a single ET_EXEC binary directly — no intermediate .o files, no linker step. This works but has several problems:

- No build artifacts for debugging/inspection
- Single RWX PT_LOAD segment (security issue, breaks hardened kernels)
- No .bss support in the executable emitter
- No library (.a) output mode
- No path toward incremental compilation

This plan refactors to: **per-module MASM contexts → per-module .o files → internal link → proper multi-segment ET_EXEC**.

## Output Directory Structure

```
out/
  linux/
    bin/mach          # final executable (or .a for library mode)
    obj/
      main.o
      mach.cli.args.o
      mach.compiler.ast.o
      ...             # one .o per module
```

The `obj/` dir is derived as `<dir_out>/<artifacts>/obj/`. No mach.toml changes needed.

---

## Implementation Phases

### Phase 1: Implement `masm.merge()` + Add `ModuleMasm` Record

**Files:** `src/compiler/masm/masm.mach`, `src/compiler/masm/lower.mach`

**1a. Implement `masm.merge()` at `masm.mach:259`** (currently returns "not yet implemented")

Algorithm:
1. For each section in `src`, find matching section in `dest` by name (or create it)
2. Record `base_offset = dest_section.size`
3. Pad dest section to src section's alignment
4. Append src section's data bytes to dest section
5. Copy src section's relocations to dest, adjusting each `reloc.offset += base_offset`
6. For each symbol in src:
   - If symbol's section matches current section: `sym.offset += base_offset`, point `sym.section` at dest's section, add to dest's symbol array
7. Merge debug_lines (adjust `code_offset += text_base_offset`) and debug_files

**1b. Add `ModuleMasm` record to `lower.mach`** (near top, after imports)

```mach
pub rec ModuleMasm {
    m:          *masm.Masm;
    mod_name:   str;
    file_path:  str;
}
```

---

### Phase 2: Per-Module Lowering (`lower_all_modules_split`)

**File:** `src/compiler/masm/lower.mach`

Add a new function alongside the existing `lower_all_modules`:

```mach
pub fun lower_all_modules_split(
    root: *ast.Node, symbols: *symbol.Table, types: *ty.TypeContext,
    alloc: *allocator.Allocator, tgt: target.Target,
    loaded_mods: **sema.SemaModule, mod_count: i32,
    method_insts: *sema.MethodInstantiation,
    func_insts: *sema.FunctionInstantiation,
    main_file_path: str, main_source: str,
    out_modules: *ModuleMasm, out_count: *i32, max_modules: i32
) Result[bool, str]
```

**Algorithm:**

1. **Lower main module:**
   - Create fresh `masm.Masm`, pre-create `.text`/`.rodata`/`.data` sections
   - Create `LowerContext` pointing at this Masm + main symbol table
   - Call `lower_module(ctx, root)` — collects IR functions into `ctx.ir_funcs`
   - Do NOT run isel yet (generic instantiations may add more IR functions)
   - Store ctx.ir_funcs/count on the ModuleMasm (or separate tracking array)
   - Set `out_modules[0] = { m, "main", main_file_path }`

2. **Lower each imported module** (same loop as lines 6140-6167):
   - Create fresh `masm.Masm` with sections
   - Reset LowerContext: new Masm, swapped symbol table, swapped source/file_path
   - Call `lower_module(ctx, smod.ast)`
   - Store as `out_modules[idx]`

3. **Assign generic instantiations to caller modules:**
   - `origin_module` on instantiation records IS the caller (set to `sema.current_module` at call site)
   - For each `MethodInstantiation`/`FunctionInstantiation`:
     - Find caller's ModuleMasm index (match `inst.origin_module` pointer against loaded_mods array; nil = main module = index 0)
     - Switch LowerContext to that module's Masm
     - Set symbol table from the origin_module (caller has import visibility to the defining module's symbols)
     - Call `lower_function(ctx, inst.func_ast)` with type bindings/overrides
     - Add resulting IRFunction to that module's ir_func collection

4. **Run isel per-module:**
   - For each `out_modules[i]`, call `isel.run_isel(module.m, module_ir_funcs, module_ir_count, alloc)`
   - Each module's .text section gets populated with its own machine code

5. Set `*out_count` and return.

**Tracking per-module IR functions:**
Need a way to track which IR functions belong to which module. Options:
- (A) Separate ir_funcs array per module — cleanest, allocate `[256]*ir.IRFunction` per module
- (B) Use a single ir_funcs array and track start/count indices per module

Go with (A): add `ir_funcs: [512]*ir.IRFunction` and `ir_func_count: i32` to `ModuleMasm`. Reset the LowerContext's ir_funcs pointer for each module.

**Rewrite `lower_all_modules` to delegate:**
After `lower_all_modules_split` is working, rewrite the existing `lower_all_modules` to:
1. Call `lower_all_modules_split` to get per-module MASMs
2. Create output Masm, call `masm.merge()` for each module
3. Return the merged Masm

This preserves backward compatibility (testing.mach calls lower_all_modules).

---

### Phase 3: Internal Linker

**New file:** `src/compiler/masm/link.mach`

```mach
pub rec LinkInput {
    m:        *masm.Masm;
    mod_name: str;
}

pub fun link(inputs: *LinkInput, input_count: i32,
    alloc: *allocator.Allocator, tgt: target.Target) Result[*masm.Masm, str]

pub fun check_unresolved(m: *masm.Masm) Result[bool, str]
```

**`link()` algorithm:**

1. Create output `Masm` with `.text`, `.rodata`, `.data`, `.bss` sections
2. For each input, call `masm.merge(output, input.m)` — appends sections, symbols, relocations with proper offset adjustment
3. Call `check_unresolved(output)` — iterate all relocations, verify each `symbol_name` exists in merged symbol table
4. Return merged Masm

**`check_unresolved()` algorithm:**
- For each section in m, for each relocation, lookup `reloc.symbol_name` via `masm.get_symbol(m, name)`
- If not found and not a known weak symbol: return `Err` with the unresolved name
- Return `Ok(true)` if all resolved

This is intentionally thin — the heavy lifting is in `masm.merge()`. The linker is essentially merge + validation.

---

### Phase 4: Multi-Segment ELF + .bss Support

**File:** `src/compiler/masm/of/elf.mach`

**4a. Add .bss to `emit_executable()` (line 1363+):**

After finding .text/.rodata/.data sections (lines 1378-1436), also find .bss:
```mach
var bss_sec: *section.Section = nil;
# ... search m.sections ...
var bss_size: u64 = 0;
if (bss_sec != nil) { bss_size = bss_sec.size; }
```

**4b. Rewrite layout computation** to use page-aligned segments:

```
text_file_off   = ELF_PAGE_SIZE                              # 0x1000
text_vaddr      = ELF_BASE_VADDR + text_file_off             # 0x401000

rodata_file_off = page_align(text_file_off + text_size)
rodata_vaddr    = ELF_BASE_VADDR + rodata_file_off

data_file_off   = page_align(rodata_file_off + rodata_size)
data_vaddr      = ELF_BASE_VADDR + data_file_off

bss_vaddr       = data_vaddr + data_size                     # contiguous after .data
```

Helper: `fun page_align(v: u64) u64 { ret (v + ELF_PAGE_SIZE - 1) & ~(ELF_PAGE_SIZE - 1); }`

**4c. Change from 1 to 3 PT_LOAD program headers:**

| Segment | Sections | File size | Mem size | Flags |
|---------|----------|-----------|----------|-------|
| LOAD 1  | .text    | text_size | text_size | PF_R \| PF_X |
| LOAD 2  | .rodata  | rodata_size | rodata_size | PF_R |
| LOAD 3  | .data + .bss | data_size | data_size + bss_size | PF_R \| PF_W |

Skip segment if size is 0 (compute `num_phdrs` dynamically).

**4d. Update `apply_relocations` signature** to include `bss_vaddr`:

```mach
fun apply_relocations(sec: *section.Section, sec_vaddr: u64, m: *masm.Masm,
    text_vaddr: u64, rodata_vaddr: u64, data_vaddr: u64, bss_vaddr: u64)
```

Add `.bss` section name matching when computing symbol addresses:
```mach
if (s.section.name.equals(".bss")) {
    sym_addr = bss_vaddr + s.offset;
}
```

**4e. Write .bss section header** with `SHT_NOBITS` (no file bytes, only memsz).

**4f. Pad between segments** to page boundaries in the file output.

---

### Phase 5: Build Command Integration

**File:** `src/commands/build.mach`

**5a. Rewrite `compile_source` to use the new pipeline:**

```mach
fun compile_source(...) i64 {
    # ... parse, sema (unchanged) ...

    # lower per-module
    var module_masms: [256]lower.ModuleMasm;
    var module_count: i32 = 0;
    val lower_res = lower.lower_all_modules_split(
        root, symbols, types, alloc, tgt,
        ?loaded_mods[0], load_count,
        sem.method_insts, sem.func_insts,
        file_path, source,
        ?module_masms[0], ?module_count, 256
    );

    # emit per-module .o files
    val obj_dir: str = build_obj_dir(out_dir, artifacts, alloc);
    ensure_directory(obj_dir, alloc);
    var i: i32 = 0;
    for (i < module_count) {
        val obj_path: str = make_obj_path(obj_dir, module_masms[i].mod_name, alloc);
        emit.emit_object(module_masms[i].m, obj_path, alloc);
        i = i + 1;
    }

    # link in-memory
    var link_inputs: [256]link.LinkInput;
    i = 0;
    for (i < module_count) {
        link_inputs[i].m = module_masms[i].m;
        link_inputs[i].mod_name = module_masms[i].mod_name;
        i = i + 1;
    }
    val linked = link.link(?link_inputs[0], module_count, alloc, tgt);

    # emit executable
    val emit_res = emit.emit_executable(linked.unwrap_ok(), output_path, alloc);

    # ... cleanup ...
}
```

**5b. Add helper functions:**

```mach
fun build_obj_dir(out_dir: str, artifacts: str, alloc: *allocator.Allocator) str
fun make_obj_path(obj_dir: str, mod_name: str, alloc: *allocator.Allocator) str
fun ensure_directory(path: str, alloc: *allocator.Allocator) bool
```

`make_obj_path`: converts `"mach.compiler.ast"` → `"<obj_dir>/mach.compiler.ast.o"`

**5c. Add library mode to `handle()`:**

Remove the error at line 827-832 for non-executable targets. Add:
```mach
if (target_cfg.mode.tag == ctarget.TMODE_LIBRARY) {
    ret compile_library(source, file_path, module_path, lib_output, alloc, tgt, ...);
}
```

---

### Phase 6: Static Archive (.a) Builder

**New file:** `src/compiler/masm/archive.mach`

AR format is simple:
```
"!<arch>\n"                    (8-byte global header)
[member_header + data] ...     (per .o file)
```

Member header (60 bytes): name/16, date/12, uid/6, gid/6, mode/8, size/10, fmag/2

```mach
pub rec ArMember {
    name: str;
    data: *u8;
    size: usize;
}

pub fun emit_archive(members: *ArMember, member_count: i32,
    output_path: str, alloc: *allocator.Allocator) Result[bool, str]
```

**Algorithm:**
1. Write `"!<arch>\n"` global header
2. For each member: format ar header (name padded to 16 chars with `/` terminator and spaces), write data, pad to 2-byte alignment
3. Write to file

To get .o bytes in memory: add `emit_object_to_buffer()` variant to `elf.mach` that writes to a `spec.Writer` instead of a file path. The archive builder calls this for each per-module Masm.

**`compile_library` in `build.mach`:**
1. `lower_all_modules_split` → per-module MASMs
2. For each module: `elf.emit_object_to_buffer(m, writer, alloc)` → get bytes
3. Build `ArMember` array
4. `archive.emit_archive(members, count, lib_path, alloc)`
5. Also write individual .o files to `obj/` dir (same as executable mode)

---

## Files Summary

| File | Action | Description |
|------|--------|-------------|
| `src/compiler/masm/masm.mach` | Modify | Implement `merge()` (lines 259-272) |
| `src/compiler/masm/lower.mach` | Modify | Add `ModuleMasm` record, add `lower_all_modules_split()`, rewrite `lower_all_modules()` to delegate |
| `src/compiler/masm/link.mach` | **New** | Internal linker: `link()`, `check_unresolved()` |
| `src/compiler/masm/of/elf.mach` | Modify | Multi-segment PT_LOAD, .bss support, `emit_object_to_buffer()`, update `apply_relocations()` |
| `src/compiler/masm/archive.mach` | **New** | Static .a archive builder |
| `src/commands/build.mach` | Modify | New pipeline using split+link, .o emission, library mode, obj dir helpers |
| `src/compiler/masm/emit.mach` | Modify | Add archive dispatch (minor) |

No changes needed to: `sema.mach` (origin_module already tracks caller), `testing.mach` (uses old lower_all_modules which delegates internally), `isel.mach` (already works per-module), `config.mach` (existing fields suffice).

---

## Verification

After each phase:
1. `make test` — all 490 tests must pass
2. Build chain: `make clean && make` — cmach→imach→mach must succeed
3. `./out/linux/bin/imach build .` — must produce working `mach` binary
4. `./out/linux/bin/mach build .` — must produce working `mach` binary (self-hosting)

After Phase 5:
- Verify `out/linux/obj/` contains .o files
- `objdump -d out/linux/obj/main.o` — verify valid ELF relocatable
- `readelf -S out/linux/obj/main.o` — verify sections and symbols
- `readelf -l out/linux/bin/mach` — verify 3 PT_LOAD segments with correct permissions (R+X, R, R+W)
- `readelf -S out/linux/bin/mach` — verify .bss section present

After Phase 6:
- Set a target to `mode = "library"` in a test mach.toml
- Verify .a file is valid: `ar t output.a` should list members
