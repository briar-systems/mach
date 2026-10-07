# mach.lang.me.check.scope

what every module check reads, and the shape each check takes

## rec Scope

```mach
pub rec Scope;
```

what a check reads, every check the same record

## def Check

```mach
pub def Check: fun(*Scope) err[fail.Fail]
```

a check over `scope.module`, failed once it has refused

## fun init

```mach
pub fun init(m: *me_ir.Module, tgt: *resolved.Target, itn: *intern.Interner, diags: *diagnostic.DiagnosticStore, alloc: *A.Allocator) Scope;
```

a scope over `m` for `tgt`, with `simd = "require"` off

