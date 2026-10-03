# mach.lang.package.list

list: what `mach dep list` shows: each root declaration with its winning selection, its
pin and its state, and under it every requirement of the identity that selection overrides

## fun lines

```mach
pub fun lines(op: *package_closure.Operation, out: *Vector[str]) err[fail.Fail];
```

the lines `mach dep list` shows, appended to `out`

op: the operation
out: receives one line per root declaration and one per requirement it overrides
ret: err naming the failure

