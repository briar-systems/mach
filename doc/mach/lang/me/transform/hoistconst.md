# mach.lang.me.transform.hoistconst

## fun run_in

```mach
pub fun run_in(m: *me_ir.Module, tgt: *lang_target.Target, workspace: *scratch.Workspace) res[bool, fail.Fail];
```

the constants a loop rebuilds on every iteration, materialized once in its
preheader instead (#3807): a float other than +0.0, which is a constant-pool
load or a register built from its bits, and an integer the instruction set
cannot build in one instruction, by its own rule. each loop, innermost first,
gives the constants its body reads a `const` in its own preheader, so an
inner loop's constant is built once per entry and lives only across that
loop. only the operands whose lowering reads any value are rewritten: an
operand a later pass or the lowering needs as a literal (a shift count, a
divisor, a power-of-two multiplier, a low-bit mask, an index) keeps it, and a
debug binding keeps the constant it names. the allocator rematerializes a
`const` rather than spill it, so the hoist never costs the loop a reload. an
instruction set without the rule hoists nothing

