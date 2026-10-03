# mach.cli.cmd.dep

## fun run

```mach
pub fun run(argv: **u8, inv: *args.ParsedInvocation) i64;
```

`mach dep`: route to one action: list, add, remove, update, pull, verify or outdated, each a
command over the dependency manager (mach.lang.package) that renders what it reports

argv: the full process arguments
inv: the parsed invocation for this command
ret: exit.OK, exit.USER for a malformed invocation, or the code `exit.of` maps the
      action's printed failure to

## fun pull_project

```mach
pub fun pull_project(root: str, quiet: bool, environ: **u8) i64;
```

`mach dep pull`: realize the dependency closure of a project (package_closure.pull)

root: the project root directory
quiet: suppress progress lines
environ: the environment git runs in
ret: exit.OK when realized, otherwise the code `exit.of` maps the printed failure to

