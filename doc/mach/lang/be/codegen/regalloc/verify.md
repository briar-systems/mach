# mach.lang.be.codegen.regalloc.verify

the allocation's own contracts, checked after the scan and the rewrite: no
two values on one register meet, every assigned register is a usable one of
its value's bank, and every rewritten operand names a register of its bank

## fun rewritten_operands

```mach
pub fun rewritten_operands(tgt: *lang_target.Binding, f: *lang_mir.MirFunction) err[fail.Fail];
```

## fun interference

```mach
pub fun interference(ctx: *regalloc_context.Context) err[fail.Fail];
```

two values assigned one register never meet: the allocation's own
contract, checked after every scan so a wrong assignment is a refusal and
not a miscompile. intervals are sorted by start and each is compared with
the earlier ones on its register whose hull still reaches it

## fun allocation

```mach
pub fun allocation(tgt: *lang_target.Binding, ctx: *regalloc_context.Context) err[fail.Fail];
```

