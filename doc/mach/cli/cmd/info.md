# mach.cli.cmd.info

## fun run

```mach
pub fun run(argv: **u8, inv: *args.ParsedInvocation) i64;
```

`mach info`: print "mach <version>" and the host target this binary resolves for the
machine it runs on. `--version` prints the version alone; the verb `targets` prints
every target the registries compose end-to-end, one full tuple per line; combining
the two, or any other verb, is an error

argv: the full process arguments
inv: the parsed invocation for this command
ret: 0 success, 1 usage error, 2 target registration or host resolution failure

## fun render_host

```mach
pub fun render_host(a: *A.Allocator, reg: *target.TargetRegistry) res[str, fail.Fail];
```

the `mach info` page: the version and the resolved host target, one dimension
per line, every value taken from the target the host request resolves to

## fun render_targets

```mach
pub fun render_targets(a: *A.Allocator, reg: *target.TargetRegistry) res[str, fail.Fail];
```

the `mach info targets` listing: one line per accepted cell, the `<os>-<isa>`
name then `key=value` for every dimension, each column padded to the widest
value it holds over the whole listing

