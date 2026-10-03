# mach.lang.be.codegen.regalloc.coalesce

copy elimination before the scan and the hints the scan follows: dead
moves are swept, single-assignment copy webs and dying copies are merged,
and copies, ties and sinks become register preferences

## fun instr_is_reg_copy

```mach
pub fun instr_is_reg_copy(tgt: *lang_target.Binding, mi: *lang_mir.MirInstr) bool;
```

## fun sweep_dead_movs

```mach
pub fun sweep_dead_movs(tgt: *lang_target.Binding, ctx: *regalloc_context.Context) err[fail.Fail];
```

## fun drop_self_copies

```mach
pub fun drop_self_copies(tgt: *lang_target.Binding, ctx: *regalloc_context.Context, kind: lang_mir.MirOperandKind) err[fail.Fail];
```

a copy of a register onto itself goes, operands of `kind` alone: virtual
ones before allocation, physical ones after the rewrite. a declassifying one
leaves its marker

## fun copies

```mach
pub fun copies(tgt: *lang_target.Binding, ctx: *regalloc_context.Context) err[fail.Fail];
```

## fun dying_copies

```mach
pub fun dying_copies(tgt: *lang_target.Binding, ctx: *regalloc_context.Context) err[fail.Fail];
```

a copy `d = s` whose source dies at it: d and s become one value when no
position holds both, so every definition of either writes one register and
the copy is a self-copy that vanishes. unlike copies, either side
may have many definitions, which is the shape of a loop-carried variable
and the value its back edge copies into it. the test is exact liveness:
two values that never meet may share a register soundly. a position where
one side's last read meets the other's definition is shared only when the
instruction reads before it writes, the rule the scan's hand-off applies,
so a merge never makes a two-address op clobber its own operand. one web's
hull must lie inside the other's, so the merged value spans nothing new,
and a value copied from or into a physical register keeps its own hint:
without either, merges lengthen the hot loops. the constraints a register
must meet (calls, argument windows, definitions, pins and the reload
guarantee) apply to the merged value as the rounds place it. the copies are
taken hottest first, so a loop's copy is not lost to a colder one that
would merge a value it meets

## fun build_hints

```mach
pub fun build_hints(tgt: *lang_target.Binding, ctx: *regalloc_context.Context);
```

