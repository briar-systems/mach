# mach.lang.be.codegen.isel

Instruction selection. Maps generic IR / MIR opcodes onto concrete
per-arch instruction forms by consulting the active ISA's selection
table. The table itself lives in each per-arch ISA impl
([`isa.x64`](../../target/isa/x64.md),
[`isa.arm64`](../../target/isa/arm64.md)); this module is the
generic dispatcher.

Source is `new/lang/be/codegen/isel.mach` (currently empty).

## Functions

### `run`

```mach
pub fun run(
    tgt:  *target.Target,
    mir:  *mir.MirModule,
) Result[bool, str]
```

Per-function walk:

1. Read the ISA's selection table from
   [`target.arch`](../../target/isa.md#isavtable).
2. For each [`MirInstr`](mir.md#mirmodule-mirfunction-mirblock-mirinstr),
   look up the generic opcode in the table. The matcher considers
   operand kinds and types — the same generic `ADD` may select
   `ADD_RR` (reg/reg), `ADD_RI` (reg/imm), or a load-add fused form
   depending on the operand shape.
3. Rewrite the `MirInstr.opcode` and operand layout to the selected
   form.

| Param | Type                                              | Description                                                |
|-------|---------------------------------------------------|------------------------------------------------------------|
| tgt   | [`*target.Target`](../../target.md#target)        | Active target — picks the ISA table.                       |
| mir   | [`*mir.MirModule`](mir.md#mirmodule-mirfunction-mirblock-mirinstr) | MIR module to mutate.                       |

Returns `ok(true)` after every function has been selected.

## Selection table shape

Each ISA impl exposes a table of selection rules:

```mach
rec SelRule {
    generic:    u32;     # generic opcode id (matches MIR opcodes pre-isel)
    operand_pattern: u32;# bitmask over MirOperandKind for each operand
    target_opcode: u32;  # concrete opcode id for the encoder
    cost:       u8;
}
```

Multiple rules may match; the lowest-cost wins. The table is
sorted by `(generic, -cost)` so the dispatcher does a linear scan
in the matching slice.

Peephole patterns (multi-instruction combine) are out of scope for
the rewrite; isel is single-instruction-pattern only.

## Dependencies

`std.types.bool`, `std.types.size`, `std.types.result`,
[`mach.lang.target`](../../target.md),
[`mach.lang.be.codegen.mir`](mir.md).
