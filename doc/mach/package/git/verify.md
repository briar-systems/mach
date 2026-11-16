[mach](../../README.md) / [package](../README.md) / [git](README.md) / **verify**

# mach.package.git.verify

verify: what `mach dep verify` holds a git dependency to. the enclosing repository must
stage its gitlink and register it in .gitmodules with the declared source; the checkout
must be a repository root at that gitlink, match its committed tree, keep every symlink
inside itself, and satisfy every exact selector and version range the closure declares

<sub>source: [`src/package/git/verify.mach`](../../../../src/package/git/verify.mach) · imported by 1 module</sub>

<sub>`session` is [`mach.session`](../../session.md) · `git` is [`mach.package.git`](../git.md) · `manifest_model` is [`mach.manifest.model`](../../manifest/model.md) · `intern` is [`mach.base.intern`](../../base/intern.md) · `fail` is [`mach.base.fail`](../../base/fail.md)</sub>

| Functions | |
| :--- | :--- |
| [`requirer_selector_satisfied`](#requirer_selector_satisfied) · [`realized`](#realized) | |

---

## Functions

### requirer_selector_satisfied

```mach
pub fun requirer_selector_satisfied(s: *session.Session, gi: *git.Inspector, alias: str, chain: str, project_root: str,
    dir_dep: str, d: *manifest_model.DepDef, content: intern.StrId, root_own: bool) err[fail.Fail]
```

### realized

```mach
pub fun realized(s: *session.Session, gi: *git.Inspector, alias: str, parent_root: str,
    dep_dir: str, dir_dep: str, d: *manifest_model.DepDef) res[intern.StrId, fail.Fail]
```

