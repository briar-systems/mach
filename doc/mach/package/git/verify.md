[mach](../../README.md) / [package](../README.md) / [git](README.md) / **verify**

# mach.package.git.verify

verify: what `mach dep verify` holds a git dependency to. the enclosing repository must
stage its gitlink and register it in .gitmodules with the declared source; the checkout
must be a repository root at that gitlink, match its committed tree, keep every symlink
inside itself, and satisfy every exact selector and version range the closure declares

Source: [`src/package/git/verify.mach`](../../../../src/package/git/verify.mach)

Uses types from [`mach.base.fail`](../../base/fail.md), [`mach.base.intern`](../../base/intern.md), [`mach.manifest.model`](../../manifest/model.md), [`mach.package.git`](../git.md), [`mach.session`](../../session.md).

| Functions | |
| :--- | :--- |
| [`requirer_selector_satisfied`](#requirer_selector_satisfied) | check a requirer's git selector, a version range or an exact ref, against the commit the dependency is realized at |
| [`realized`](#realized) | what `mach dep verify` checks of one realized git dependency |

---

## Functions

### requirer_selector_satisfied

```mach
pub fun requirer_selector_satisfied(s: *mach.session.Session, gi: *mach.package.git.Inspector, alias: str, chain: str, project_root: str,
    dir_dep: str, d: *mach.manifest.model.DepDef, content: mach.base.intern.StrId, root_own: bool) err[mach.base.fail.Fail]
```

check a requirer's git selector, a version range or an exact ref, against the commit the
dependency is realized at

| Parameter | |
| :--- | :--- |
| `s` | the session |
| `gi` | the inspector whose git runs the checks, started here when still pending |
| `alias` | the dependency identity |
| `chain` | the requiring chain, as a refusal names it |
| `project_root` | the root project |
| `dir_dep` | the dependency root's name under the project, dep |
| `d` | the requirer's declaration, where a path declaration always holds |
| `content` | the realized content, `git/<commit>` for a git checkout |
| `root_own` | the root declares the identity itself, which shapes the remedy a refusal names |
| **returns** | err naming the selector the pinned commit does not satisfy |

### realized

```mach
pub fun realized(s: *mach.session.Session, gi: *mach.package.git.Inspector, alias: str, parent_root: str,
    dep_dir: str, dir_dep: str, d: *mach.manifest.model.DepDef, source: str) res[mach.base.intern.StrId, mach.base.fail.Fail]
```

what `mach dep verify` checks of one realized git dependency

| Parameter | |
| :--- | :--- |
| `s` | the session |
| `gi` | the inspector, started here when still pending |
| `alias` | the dependency identity |
| `parent_root` | the root of the project that declares it |
| `dep_dir` | the dependency's checkout directory |
| `dir_dep` | the dependency root's name under the parent, dep |
| `d` | the declaration |
| `source` | the source its `git` names, as the closure resolved it (package.git_source) |
| **returns** | the realized content `git/<commit>`, interned, which the closure's selectors are checked against, err naming the first mismatch |

