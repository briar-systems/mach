# mach.lang.me.check.oblivious

## fun run

```mach
pub fun run(s: *check_scope.Scope) err[fail.Fail];
```

a function that computes on a secret without declaring `#[oblivious]` is
refused at the first such operation, every function named

