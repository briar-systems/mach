# mach.lang.be.codegen.regalloc.order

the scan order: the blocks are permuted into the order the scan walks and
put back in their original order once the rewrite is done

## fun build

```mach
pub fun build(ctx: *regalloc_context.Context) err[fail.Fail];
```

the scan order is the reverse postorder from the entry, then the blocks it
does not reach in their own order

## fun apply

```mach
pub fun apply(ctx: *regalloc_context.Context) err[fail.Fail];
```

## fun restore

```mach
pub fun restore(ctx: *regalloc_context.Context) err[fail.Fail];
```

