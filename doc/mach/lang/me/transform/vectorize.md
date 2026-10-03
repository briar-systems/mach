# mach.lang.me.transform.vectorize

## val PASS

```mach
pub val PASS: pass.Pass = pass.Pass;
```

## fun run_in

```mach
pub fun run_in(ctx: *pass.Context) res[bool, fail.Fail];
```

`ctx.interner` names the constant globals a splat of a constant reads from;
nil keeps every splat on the stack

