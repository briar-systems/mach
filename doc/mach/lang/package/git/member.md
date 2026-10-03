# mach.lang.package.git.member

member: git as a member of the source and pin contracts, its tables assembled from the
parts beside it. the closure engine opens a source's tables here and works through them,
never naming git itself

## fun checkouts

```mach
pub fun checkouts(s: *session.Session, rep: *report.Report) res[package_source.Checkouts, fail.Fail];
```

the checkouts table of git: dep/<id> is a submodule

s: the session; the table's state is allocated from it
rep: receives the notes naming what a failure left behind
ret: the table, released with its own `close`

## fun releases

```mach
pub fun releases(s: *session.Session, offline: bool, local: str) res[package_source.Releases, fail.Fail];
```

the releases table of git: a url's releases are its `v`-prefixed semver tags

s: the session; the table's state is allocated from it
offline: read releases only from local checkouts the table is told of, reaching no network
local: a repository every url's releases are read from offline, or "" for none
ret: the table, released with its own `close`; err when git cannot be run

## fun pin

```mach
pub fun pin(s: *session.Session) package_pin.Pin;
```

the pin table of git: a pin is the gitlink the enclosing repository stages at dep/<id>

