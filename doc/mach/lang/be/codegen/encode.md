# mach.lang.be.codegen.encode

Machine-code encoder. Walks a fully-resolved MIR (post-isel,
post-regalloc, post-frame) and produces section bytes plus the list
of relocations that the object writer needs to patch in. The
per-instruction encoding tables live in each per-arch ISA impl
([`isa.x64`](../../target/isa/x64.md),
[`isa.arm64`](../../target/isa/arm64.md)); this module is the
generic walker that consumes them.

Source is `new/lang/be/codegen/encode.mach` (currently empty).

## Types

### `EncoderOutput`

```mach
pub rec EncoderOutput {
    text:        *u8;
    text_len:    u32;
    relocations: *PendingReloc;
    reloc_count: u32;
    symbols:     *SymbolMark;
    symbol_count: u32;
}
```

The encoded form of one function (or a whole module's text section
when concatenated). Sections other than `.text` (rodata, data, bss)
are populated by [`obj`](../obj.md) directly from
[`ir.Global`](../../me/ir.md#global).

### `PendingReloc`

```mach
pub rec PendingReloc {
    offset:      u32;
    kind:        intern.StrId;
    sym:         intern.StrId;
    addend:      i32;
}
```

| Field  | Type                                          | Description                                                |
|--------|-----------------------------------------------|------------------------------------------------------------|
| offset | `u32`                                         | Byte offset into the section where the patch applies.      |
| kind   | [`intern.StrId`](../../intern.md#strid)       | Relocation kind name — looked up in the active [`OfVTable.relocation_kinds`](../../target/of.md#ofvtable). |
| sym    | [`intern.StrId`](../../intern.md#strid)       | Target symbol name.                                        |
| addend | `i32`                                         | Constant added to the resolved symbol address.             |

### `SymbolMark`

```mach
pub rec SymbolMark {
    name:    intern.StrId;
    offset:  u32;
    flags:   u32;
}
```

Per-function entry point (and any local labels exposed by inline
asm). Consumed by [`obj`](../obj.md) when populating the symbol
table.

## Functions

### `run`

```mach
pub fun run(
    tgt:  *target.Target,
    mir:  *mir.MirModule,
) Result[EncoderOutput, str]
```

Walks each function's blocks in layout order, emitting bytes per
the active ISA's encoder table. Branch displacements are resolved
in a second pass once every block offset is known:

1. **Pass 1.** Emit each instruction's bytes; for branches with
   not-yet-known targets, emit zero placeholders and record a
   pending fixup.
2. **Pass 2.** Patch every fixup. Cross-function symbol references
   (calls, global address references) become `PendingReloc` entries
   for the object writer.

| Param | Type                                              | Description                                          |
|-------|---------------------------------------------------|------------------------------------------------------|
| tgt   | [`*target.Target`](../../target.md#target)        | Active target — selects the encoder table.           |
| mir   | [`*mir.MirModule`](mir.md#mirmodule-mirfunction-mirblock-mirinstr) | MIR module to encode.               |

Returns the per-module [`EncoderOutput`](#encoderoutput).

## Dependencies

`std.types.bool`, `std.types.size`, `std.types.result`,
`std.allocator`,
[`mach.lang.intern`](../../intern.md),
[`mach.lang.target`](../../target.md),
[`mach.lang.be.codegen.mir`](mir.md).
