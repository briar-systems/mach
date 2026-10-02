# mach.lang.me.pass.widediv

division and remainder wider than the target's ALU have no instruction on
any target, so each becomes a call to a helper the compiler provides with
the program: `__mach_udiv<bits>`, `__mach_urem<bits>`, `__mach_sdiv<bits>`
and `__mach_srem<bits>`, one per operation (a caller that wants only a
remainder does not compute a quotient). the unsigned helpers are the
restoring shift-subtract division over the wide type's own IR, which
legalize realizes in lanes like any other wide arithmetic; the signed ones
take magnitudes, call the unsigned helper and fix the sign. each module that
needs a helper synthesizes it weak, so every copy coalesces at link, as a
generic instantiation does. a divisor of zero yields an all-ones quotient
and the dividend as remainder, the RISC-V convention, rather than a trap:
a helper cannot raise the target's divide fault (#3511, ruling Q5)

## fun run

```mach
pub fun run(m: *me_ir.Module, tgt: *lang_target.Target, itn: *intern.Interner) res[bool, fail.Fail];
```

