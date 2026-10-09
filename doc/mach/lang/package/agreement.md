# mach.lang.package.agreement

agreement: one package pinned at one commit across a whole dependency closure. every
project of the closure records the commits of the dependencies it declares, so two
projects, one reached through a path dependency, can pin the same package differently
and a build would mix two versions of it with nothing saying so. identity is the
dependency key, which the closure already holds equal to the dependency's project id.
pins are compared among declarations that select the same source and ref: declarations
that select differently are the root's override or a conflict, which the closure walk
already notes or refuses, and a version range is reconciled by resolution, so a
requirer's pin of it is only a seed

## fun disagreements

```mach
pub fun disagreements(op: *package_closure.Operation, reqs: *Vector[package_closure.Request], out: *Vector[str]) err[fail.Fail];
```

one line per pin that differs from the first one seen for its identity, in closure order

op: the operation
reqs: the realized closure
out: receives the lines, owned by `op.a`; free with `free_lines`
ret: err naming the first failure to read a manifest or a pin

## fun free_lines

```mach
pub fun free_lines(a: *A.Allocator, lines: *Vector[str]);
```

