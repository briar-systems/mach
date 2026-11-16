[mach](../README.md) / [package](README.md) / **pull**

# mach.package.pull

pull: `mach dep pull`, which realizes a project's dependency closure and refuses one that
pins a package at more than one commit

<sub>source: [`src/package/pull.mach`](../../../src/package/pull.mach) · imported by 1 module</sub>

<sub>`session` is [`mach.session`](../session.md) · `A` is `std.allocator` · `report` is [`mach.package.report`](report.md) · `fail` is [`mach.base.fail`](../base/fail.md)</sub>

| Functions | |
| :--- | :--- |
| [`project_closure`](#project_closure) | realize the dependency closure of a project: acquire every declared dependency transitively, check each realized manifest's project id against its declared name, and report realized directories under dep/ that are no longer in the closure, then refuse a package pinned at different commits within it, the closure realized all the same |

---

## Functions

### project_closure

```mach
pub fun project_closure(s: *session.Session, a: *A.Allocator, root: str, quiet: bool, rep: *report.Report) err[fail.Fail]
```

realize the dependency closure of a project: acquire every declared dependency
transitively, check each realized manifest's project id against its declared name, and
report realized directories under dep/ that are no longer in the closure, then refuse a
package pinned at different commits within it, the closure realized all the same

| Parameter | |
| :--- | :--- |
| `s` | the session; a failure may borrow from it, so the caller shows it before ending it |
| `a` | the storage the operation works in |
| `root` | the project root directory |
| `quiet` | report no progress lines |
| `rep` | receives progress, notes, and the effects a failure left |
| **returns** | err naming the failure |

