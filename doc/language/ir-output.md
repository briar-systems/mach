# IR output

`mach build --emit-ir` writes the middle-end IR of each module beside its
object, as `<out>/ir/<artifact>/<module>.ir`. The flag takes an optional form:

```
--emit-ir            the ir-debug dump (the default)
--emit-ir=debug      the same dump, named
--emit-ir=listing    a readable listing
```

`--emit-asm` is the same idea one stage later, and takes no form.

## `debug` — the ir-debug dump

The dump is the compiler's own debugging view. Every instruction carries its
whole record, so no two distinct instructions can print the same text:

```
%5 = mul !0 %p0 {kind=1 ty=!0 secret=false}, %p0 {kind=1 ty=!0 secret=false} ; state{ty=!0, aux=!nil, kind=2, operands=2, flags=0, loc=1:82, secret=false, pure=false, ...}
```

Types are type-table rows (`!0`), positions are `file:byte-offset`, and every
flag prints whether it is set or not. That completeness is the point: a
compiler change that alters an instruction alters its text.

**The dump's text is not a contract.** It tracks the IR's internal shape and
changes with it. Do not parse it.

## `listing` — the readable form

The listing prints the instruction and its source position and nothing else:

```
ir-listing stage="post-codegen" target="linux-x86_64" isa="x86_64" os="linux" abi="sysv64" of="elf"
module demo.main {
  fun @demo.main.square(%p0: i64) i64 [inline] {    ; src/main.mach
    bb0:
      %4 = mul.pure i64 %p0, %p0                    ; 2:30
      ret %4                                        ; 2:26
  }
}
```

- **Types are spelled the way the language spells them**: `i64`, `*i64`,
  `[4]i64`, `i32x4`, `rec{i64, f32}`, `fun(i64) i64`. A type that nests deeper
  than the spelling allows, which a self-referential record does, falls back to
  its type-table row (`!7`).
- **Positions are `line:column`**, one-based, resolved against the source at
  print time. The IR itself still holds a byte offset; nothing about what it
  stores changed.
- **A function's header names the file its body lives in**, and a bare
  `line:column` is measured against that file. An instruction from another
  file — what an inlined callee leaves behind — prints `path:line:column`
  instead, so every line stands on its own.
- **An instruction with no position prints no comment at all.**
- **Flags that change what an instruction means still print**, as suffixes on
  the opcode: `.nsw`, `.nuw`, `.exact`, `.volatile`, `.secret`, `.pure`. A
  secret operand keeps its `~`. Everything else — the type-table rows, the
  operand records, the raw flag words, the metadata ids — is elided.
- If the source an offset indexes is no longer loaded, the position prints as
  `file#<id>@<offset>` rather than a line and column it cannot compute.

**The listing is the form tooling reads.** It is what a consumer such as
Compiler Explorer maps back to editor lines.

## See also

- [manifest.md](manifest.md) — the `mach.toml` manifest reference
- `mach help build` — the full command-line reference
