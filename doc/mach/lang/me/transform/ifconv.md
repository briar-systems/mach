# mach.lang.me.transform.ifconv

if-conversion of small diamonds (#3346)

a block ending in a conditional branch whose arms are blocks entered only
from it and leaving straight to one join (a diamond), or whose one arm does
and whose other edge is the join itself (a triangle), is flattened: the arm
instructions move into the block ahead of its branch, each join phi takes
one `select` on the branch's condition in place of its arm incomings, and
the block branches to the join unconditionally. the emptied arms, which
nothing enters any more, are pruned.

an arm qualifies when it has no phis and every instruction is a scalar,
speculatable computation. a load, store or call is never speculatable, so no
read of memory the branch guards moves, such as a tag payload read under its
tag test; a declassify stays behind its branch, since running it on the
untaken path would disclose there too; a shift by a variable count stays,
since the branch may be what proves the count in range; and an aggregate
operand or result is refused, since reading one reads its memory. every phi the conversion
selects is an integer or pointer scalar the target holds in one register.

the conversion trades a branch the predictor may miss for work done on both
paths, so it is taken only while the speculated instructions and the selects
stay within SPECULATION_BUDGET cost units.

a dbg_value in an arm binds its variable to a value the untaken path never
computed, so it loses its location there; the join's own bindings, now of
the selects, restore the variable after the flattened arms.

## fun run_in

```mach
pub fun run_in(m: *me_ir.Module, tgt: *lang_target.Target, workspace: *scratch.Workspace) res[bool, fail.Fail];
```

