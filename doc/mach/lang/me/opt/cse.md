# mach.lang.me.opt.cse

common subexpression elimination within a block. an instruction with no
effect that computes what an earlier one of the same block computed, by its
opcode, type and operands, is replaced by that one. a load of an address an
earlier load of the block read is that load while nothing between them may
write memory, and the two agree on the secrecy of what they read. float
constants compare by bit pattern, so distinct zeros and nans stay apart

## val PASS

```mach
pub val PASS: pass.Pass = pass.Pass;
```

the pass the pipeline schedules

