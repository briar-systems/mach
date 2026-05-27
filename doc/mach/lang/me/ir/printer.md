# mach.lang.me.ir.printer

Textual IR dump for debugging and golden tests. Produces a
human-readable rendering of a [`Module`](../ir.md#module) in a
fixed format; the output is **not** a reload-able language —
[`me.lower`](../lower.md) is the only way to materialise IR.

Source is `new/lang/me/ir/printer.mach` (currently empty).

## Format

```
module "<fqn>" {
  global @<name>: <ty> = <value>
  ...

  fn @<name>(<params>): <ret> {
    block bb0:
      %0 = add i32 %a, %b
      %1 = mul i32 %0, 2
      cbr %1, bb1, bb2
    block bb1:
      ...
  }
}
```

- Block labels are `bb<id>` (the [`BlockId`](id.md#blockid)).
- Instruction results are `%<instr_id>`.
- Function parameters are `%<param_name>` (printed by `intern.lookup`
  on the parameter's bound name).
- Globals are `@<name>`.
- Phis render as `phi <ty> [bb0 %0, bb1 %3, ...]`.
- Flags ([`INSTR_FLAG_*`](instruction.md#constants)) render as suffixes:
  `add.nsw`, `load.volatile`.

## Functions

### `print_module`

```mach
pub fun print_module(
    out:     *io.Writer,
    m:       *ir.Module,
    interner: *intern.Interner,
) Result[bool, str]
```

Writes the full module to `out`. The interner is needed to recover
the original strings backing names and FQNs.

### `print_function`, `print_block`, `print_instruction`

```mach
pub fun print_function(out: *io.Writer, m: *ir.Module, fn: *ir.Function, interner: *intern.Interner) Result[bool, str]
pub fun print_block(out: *io.Writer, m: *ir.Module, fn: *ir.Function, blk: *ir.Block, interner: *intern.Interner) Result[bool, str]
pub fun print_instruction(out: *io.Writer, m: *ir.Module, fn: *ir.Function, instr: *instr.Instruction, interner: *intern.Interner) Result[bool, str]
```

Lower-level entry points the verifier and debug logging use. Each
takes the owning [`Module`](../ir.md#module) (for type-table lookups)
and [`Function`](../ir.md#function) (for the instruction pool) plus
the interner for name recovery.

## Dependencies

`std.types.bool`, `std.types.size`, `std.types.result`,
`std.io`,
[`mach.lang.intern`](../../intern.md),
[`mach.lang.me.ir`](../ir.md),
[`mach.lang.me.ir.instruction`](instruction.md),
[`mach.lang.me.ir.value`](value.md),
[`mach.lang.me.ir.type`](type.md).
