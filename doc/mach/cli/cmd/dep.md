# mach.cli.cmd.dep

## fun run

```mach
pub fun run(argv: **u8, inv: *args.ParsedInvocation) i64;
```

`mach dep`: route to one action: list, add, remove, update, pull, verify, or the
deprecated sync, which runs pull after a note

argv: the full process arguments
inv: the parsed invocation for this command
ret: 0 success, 1 missing or unknown action, unknown flag, or a failed action, 2 setup failure

## fun pull_project

```mach
pub fun pull_project(root: str, quiet: bool) i64;
```

realize the dependency closure of a project: clone or link every declared dependency
transitively, check each realized manifest's project id against its declared name, and
report realized directories under dep/ that are no longer in the closure

root: the project root directory
quiet: suppress progress lines
ret: 0 realized, 1 a manifest, resolution, identity, or checkout error (printed), 2 the
       allocator or session could not be initialised

## fun verify_project

```mach
pub fun verify_project(root: str) i64;
```

check that every dependency of a project is realized and consistent without changing
anything; prints "ok" on success. a project root that is not a repository root is noted
and verified from the realized checkouts. a realized directory under dep/ outside the
closure is an error here, where pull reports and retains it

root: the project root directory
ret: 0 verified, 1 a mismatch or error (printed), 2 the allocator or session could not be
      initialised

