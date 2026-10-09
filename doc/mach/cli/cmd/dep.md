# mach.cli.cmd.dep

## val COMMAND

```mach
pub val COMMAND: args.CommandSpec = args.CommandSpec;
```

`mach dep`

## fun run

```mach
pub fun run(cx: *args.Call) res[i64, fail.Fail];
```

`mach dep`: route to one action: list, add, remove, update, pull, verify or outdated, each a
command over the dependency manager (mach.lang.package) that renders what it reports

cx: the call, its action read and its operands counted
ret: exit.OK, the code `exit.of` maps the action's rendered failure to, or the failure that
     kept the action from starting

## fun pull_project

```mach
pub fun pull_project(a: *A.Allocator, root: str, quiet: bool, environ: exec.Environment) res[i64, fail.Fail];
```

`mach dep pull`: realize the dependency closure of a project (package_pull.project_closure)

a: owns what the pull allocates
root: the project root directory
quiet: suppress progress lines
environ: the environment git runs in
ret: exit.OK when realized, the code `exit.of` maps the rendered failure to, or the failure
         that kept the pull from starting

