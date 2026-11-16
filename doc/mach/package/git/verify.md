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
| [`requirer_selector_satisfied`](#requirer_selector_satisfied), [`realized`](#realized) | |

---

## Functions

### requirer_selector_satisfied

```mach
pub fun requirer_selector_satisfied(s: *mach.session.Session, gi: *mach.package.git.Inspector, alias: str, chain: str, project_root: str,
    dir_dep: str, d: *mach.manifest.model.DepDef, content: mach.base.intern.StrId, root_own: bool) err[mach.base.fail.Fail]
```

### realized

```mach
pub fun realized(s: *mach.session.Session, gi: *mach.package.git.Inspector, alias: str, parent_root: str,
    dep_dir: str, dir_dep: str, d: *mach.manifest.model.DepDef) res[mach.base.intern.StrId, mach.base.fail.Fail]
```

