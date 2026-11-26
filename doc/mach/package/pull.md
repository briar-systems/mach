[mach](../README.md) / [package](README.md) / **pull**

# mach.package.pull

pull: `mach dep pull`, which realizes a project's dependency closure and refuses one that
pins a package at more than one commit

Source: [`src/package/pull.mach`](../../../src/package/pull.mach)

Uses types from [`mach.base.fail`](../base/fail.md), [`mach.package.report`](report.md), [`mach.session`](../session.md).

| Functions | |
| :--- | :--- |
| [`project_closure`](#project_closure) | realize the dependency closure of a project: acquire every declared dependency transitively, check each realized manifest's project id against its declared name, and report realized directories under dep/ that are no longer in the closure, then refuse a package pinned at different commits within it, the closure realized all the same |

---

## Functions

### project_closure

```mach
pub fun project_closure(s: *mach.session.Session, a: *std.allocator.Allocator, root: str, quiet: bool, rep: *mach.package.report.Report) err[mach.base.fail.Fail]
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
| `quiet` | report no results |
| `rep` | receives results, notes, and the effects a failure left |
| **returns** | err naming the failure |

