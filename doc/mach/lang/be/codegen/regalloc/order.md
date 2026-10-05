# mach.lang.be.codegen.regalloc.order

the scan order: the blocks are visited in the order the scan walks without
moving them in the block array, through a flow graph built in that order

## fun build

```mach
pub fun build(ctx: *regalloc_context.Context) err[fail.Fail];
```

the scan order is the reverse postorder from the entry, then the blocks it
does not reach in their own order

