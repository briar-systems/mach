# mach.lang.be.linker.dynamic

## rec DynState

```mach
pub rec DynState;
```

## fun init_dynstate

```mach
pub fun init_dynstate(dyn: *DynState);
```

## fun free_dynstate

```mach
pub fun free_dynstate(alloc: *A.Allocator, dyn: *DynState);
```

## fun reserve_call_stubs

```mach
pub fun reserve_call_stubs(s: *session.Session, tgt: *target.Target, dyn: *DynState,
merged: *MergedSection, groups: *SectionGroups) err[fail.Fail];
```

reserves the format's call-stub table at the end of the merged code, before
anything is given an address, so a call site and its stub are never separated
by the image's data however large it is (#3888); the addresses are read back
by place_call_stubs once the layout is final

## fun place_call_stubs

```mach
pub fun place_call_stubs(dyn: *DynState, merged: *MergedSection);
```

## fun build_dynamic_info

```mach
pub fun build_dynamic_info(s: *session.Session, tgt: *target.Target, dyn: *DynState,
modules: *of.ObjectImage, module_count: u32,
sym_locs: *map.Map[intern.StrId, SymbolLoc],
dynlibs: *of.DynLib, dynlib_count: u32,
sec_base: *u32, atoms: *AtomPlan) err[fail.Fail];
```

## fun has_unresolved_declared_imports

```mach
pub fun has_unresolved_declared_imports(
modules: *of.ObjectImage, module_count: u32,
sym_locs: *map.Map[intern.StrId, SymbolLoc],
sec_base: *u32, atoms: *AtomPlan) bool;
```

whether a declared import the image still references resolves nowhere in it

## fun has_unresolved_attributed_imports

```mach
pub fun has_unresolved_attributed_imports(s: *session.Session, modules: *of.ObjectImage,
module_count: u32, dynlibs: *of.DynLib, dynlib_count: u32,
sym_locs: *map.Map[intern.StrId, SymbolLoc], prefix: str,
sec_base: *u32, atoms: *AtomPlan) res[bool, fail.Fail];
```

## rec ImportName

```mach
pub rec ImportName;
```

## fun canonical_import_name

```mach
pub fun canonical_import_name(s: *session.Session, prefix: str,
name: intern.StrId) res[ImportName, fail.Fail];
```

## fun synthesize_local_imports

```mach
pub fun synthesize_local_imports(s: *session.Session, prefix: str,
modules: *of.ObjectImage, module_count: u32,
mode: LinkMode, sec_base: *u32, atoms: *AtomPlan,
out: *of.ObjectImage) res[bool, fail.Fail];
```

## fun dynstate_import_index

```mach
pub fun dynstate_import_index(dyn: *DynState, name: intern.StrId) res[u32, fail.Fail];
```

## fun dynstate_import_is_func

```mach
pub fun dynstate_import_is_func(dyn: *DynState, idx: u32) bool;
```

## fun dynstate_add_fixup

```mach
pub fun dynstate_add_fixup(s: *session.Session, dyn: *DynState, seg_index: u32, seg_offset: u32,
import_index: u32, patch_vaddr: u64, kind: of.RelocKind, addend: i64) err[fail.Fail];
```

## fun dynstate_add_import_addr_fixup

```mach
pub fun dynstate_add_import_addr_fixup(s: *session.Session, dyn: *DynState,
seg_index: u32, seg_offset: u32,
import_index: u32, patch_vaddr: u64,
kind: of.RelocKind, addend: i64) err[fail.Fail];
```

## fun dynstate_add_base_reloc

```mach
pub fun dynstate_add_base_reloc(s: *session.Session, dyn: *DynState, seg_index: u32,
seg_offset: u32, target: u64) err[fail.Fail];
```

## fun target_requires_pie

```mach
pub fun target_requires_pie(tgt: *target.Target) bool;
```

## fun loaderless_request_message

```mach
pub fun loaderless_request_message(s: *session.Session, tgt: *target.Target, mode: LinkMode,
pie: bool, dynamic: bool) str;
```

the refusal for a link that needs a loader on an os that maps none, empty
when the link needs none

## fun pinned_attribution_message

```mach
pub fun pinned_attribution_message(s: *session.Session, sym_name: intern.StrId,
lib: intern.StrId) str;
```

