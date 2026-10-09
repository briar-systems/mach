# mach.lang.package.verify

verify: `mach dep verify`, which checks a realized closure without changing it. the
closure is located as a build locates it, and every edge is held to its pin and selectors
through the source's checkouts table, to the identity and flat layout rules, and to the
release rule; a root override of a requirer's selector is reported, not refused

## rec RootOverride

```mach
pub rec RootOverride;
```

a requirer's selector that the root's own declaration replaced. a root `ref` or `path`
wins over every requirer with no check that the dependency supports it; a root
`version` intersects with the requirers' ranges instead, so it is never one

name: the dependency key both declarations use
chain: the requiring chain, ending at the dependency
requested: the requirer's selector, as `ref = "..."`, `version = "..."` or `path = "..."`
declared: the root's selector, in the same form

## fun root_override_text

```mach
pub fun root_override_text(s: *session.Session, o: *RootOverride) res[str, fail.Fail];
```

the note `mach dep verify` prints for one override

## fun dependencies

```mach
pub fun dependencies(s: *session.Session, project_root: str, release_rule: bool,
overrides: *Vector[RootOverride]) err[fail.Fail];
```

check a project's realized dependency closure without changing it

s: the session
project_root: the root project's directory
release_rule: also hold the root to the release rule: every dependency selected by
              `version` or an exact `tag/`, as a release about to be tagged must be
overrides: when not nil, receives every requirer selector a root override replaced
ret: err naming the first mismatch

## fun project_closure

```mach
pub fun project_closure(s: *session.Session, a: *A.Allocator, root: str, release_rule: bool, rep: *report.Report) err[fail.Fail];
```

`mach dep verify`: check that every dependency of a project is realized and consistent
without changing anything. a project root that is not a repository root is verified from
the realized checkouts, and a realized directory under dep/ outside the closure is refused,
where pull notes and retains it; each root override of a requirer's selector is reported

s: the session; a failure may borrow from it, so the caller shows it before ending it
a: the storage the operation works in
root: the project root directory
release_rule: also refuse a root dependency selected by a branch, a commit or a path
rep: receives each override note and every refusal past the first
ret: err naming the failure

