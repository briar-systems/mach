[mach](../../README.md) / [package](../README.md) / [git](README.md) / **member**

# mach.package.git.member

member: git as a member of the source and pin contracts, its tables assembled from the
parts beside it. the closure engine opens a source's tables here and works through them,
never naming git itself

Source: [`src/package/git/member.mach`](../../../../src/package/git/member.mach)

Uses types from [`mach.base.fail`](../../base/fail.md), [`mach.package.pin`](../pin.md), [`mach.package.report`](../report.md), [`mach.package.source`](../source.md), [`mach.session`](../../session.md).

| Functions | |
| :--- | :--- |
| [`checkouts`](#checkouts) | the checkouts table of git: dep/\<id> is a submodule |
| [`releases`](#releases) | the releases table of git: a url's releases are its `v`-prefixed semver tags |
| [`pin`](#pin) | the pin table of git: a pin is the gitlink the enclosing repository stages at dep/\<id> |

---

## Functions

### checkouts

```mach
pub fun checkouts(s: *mach.session.Session, rep: *mach.package.report.Report) res[mach.package.source.Checkouts, mach.base.fail.Fail]
```

the checkouts table of git: dep/\<id> is a submodule

| Parameter | |
| :--- | :--- |
| `s` | the session; the table's state is allocated from it |
| `rep` | receives the notes naming what a failure left behind |
| **returns** | the table, released with its own `close` |

### releases

```mach
pub fun releases(s: *mach.session.Session, offline: bool, local: str) res[mach.package.source.Releases, mach.base.fail.Fail]
```

the releases table of git: a url's releases are its `v`-prefixed semver tags

| Parameter | |
| :--- | :--- |
| `s` | the session; the table's state is allocated from it |
| `offline` | read releases only from local checkouts the table is told of, reaching no network |
| `local` | a repository every url's releases are read from offline, or "" for none |
| **returns** | the table, released with its own `close`; err when git cannot be run |

### pin

```mach
pub fun pin(s: *mach.session.Session) mach.package.pin.Pin
```

the pin table of git: a pin is the gitlink the enclosing repository stages at dep/\<id>

