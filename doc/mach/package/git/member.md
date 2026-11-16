[mach](../../README.md) / [package](../README.md) / [git](README.md) / **member**

# mach.package.git.member

member: git as a member of the source and pin contracts, its tables assembled from the
parts beside it. the closure engine opens a source's tables here and works through them,
never naming git itself

<sub>source: [`src/package/git/member.mach`](../../../../src/package/git/member.mach) · imported by 4 modules</sub>

<sub>`session` is [`mach.session`](../../session.md) · `report` is [`mach.package.report`](../report.md) · `package_source` is [`mach.package.source`](../source.md) · `fail` is [`mach.base.fail`](../../base/fail.md) · `package_pin` is [`mach.package.pin`](../pin.md)</sub>

| Functions | |
| :--- | :--- |
| [`checkouts`](#checkouts) | the checkouts table of git: dep/&lt;id> is a submodule |
| [`releases`](#releases) | the releases table of git: a url's releases are its `v`-prefixed semver tags |
| [`pin`](#pin) | the pin table of git: a pin is the gitlink the enclosing repository stages at dep/&lt;id> |

---

## Functions

### checkouts

```mach
pub fun checkouts(s: *session.Session, rep: *report.Report) res[package_source.Checkouts, fail.Fail]
```

the checkouts table of git: dep/&lt;id> is a submodule

| Parameter | |
| :--- | :--- |
| `s` | the session; the table's state is allocated from it |
| `rep` | receives the notes naming what a failure left behind |
| **returns** | the table, released with its own `close` |

### releases

```mach
pub fun releases(s: *session.Session, offline: bool, local: str) res[package_source.Releases, fail.Fail]
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
pub fun pin(s: *session.Session) package_pin.Pin
```

the pin table of git: a pin is the gitlink the enclosing repository stages at dep/&lt;id>

