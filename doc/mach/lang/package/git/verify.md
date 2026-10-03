# mach.lang.package.git.verify

verify: what `mach dep verify` holds a git dependency to. the enclosing repository must
stage its gitlink and register it in .gitmodules with the declared source; the checkout
must be a repository root at that gitlink, match its committed tree, keep every symlink
inside itself, and satisfy every exact selector and version range the closure declares

## fun requirer_selector_satisfied

```mach
pub fun requirer_selector_satisfied(s: *session.Session, gi: *git.Inspector, alias: str, chain: str, project_root: str,
dir_dep: str, d: *manifest.DepDef, content: intern.StrId, root_own: bool) err[fail.Fail];
```

## fun realized

```mach
pub fun realized(s: *session.Session, gi: *git.Inspector, alias: str, parent_root: str,
dep_dir: str, dir_dep: str, d: *manifest.DepDef) res[intern.StrId, fail.Fail];
```

