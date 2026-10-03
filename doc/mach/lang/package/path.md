# mach.lang.package.path

path: the member that realizes a dependency declared by a local `path`. it has no
releases and no pin: every pull syncs dep/<id> with its source, copying what the source's
git keeps when the source is in a work tree, and leaving behind the build output and the
realized dependencies of every project inside it

## fun verify

```mach
pub fun verify(s: *session.Session, alias: str, dep_dir: str) res[intern.StrId, fail.Fail];
```

what `mach dep verify` checks of a path dependency's copy: no repository metadata and no
symlink anywhere in it

ret: the realized content an exact selector is checked against

## fun realize

```mach
pub fun realize(s: *session.Session, root: str, id: str, src_dir: str, rep: *report.Report) res[bool, fail.Fail];
```

sync dep/<id> with a path dependency's source

ret: true when the copy was refreshed, false when it already matched its source and was reused

