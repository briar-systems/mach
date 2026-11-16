[mach](../README.md) / [package](README.md) / **verify**

# mach.package.verify

verify: `mach dep verify`, which checks a realized closure without changing it. the
closure is located as a build locates it, and every edge is held to its pin and selectors
through the source's checkouts table, to the identity and flat layout rules, and to the
release rule; a root override of a requirer's selector is reported, not refused

<sub>source: [`src/package/verify.mach`](../../../src/package/verify.mach) · imported by 1 module</sub>

<sub>`intern` is [`mach.base.intern`](../base/intern.md) · `session` is [`mach.session`](../session.md) · `fail` is [`mach.base.fail`](../base/fail.md) · `A` is `std.allocator` · `report` is [`mach.package.report`](report.md)</sub>

| Type | | |
| :--- | :--- | :--- |
| [`RootOverride`](#rootoverride) | `rec` | a requirer's selector that the root's own declaration replaced |

| On `RootOverride` | |
| :--- | :--- |
| [`root_override_text`](#root_override_text) | the note `mach dep verify` prints for one override |
| [`dependencies`](#dependencies) | check a project's realized dependency closure without changing it |

| Functions | |
| :--- | :--- |
| [`project_closure`](#project_closure) | `mach dep verify`: check that every dependency of a project is realized and consistent without changing anything |

---

## RootOverride

```mach
pub rec RootOverride {
    name:      intern.StrId;
    chain:     intern.StrId;
    requested: intern.StrId;
    declared:  intern.StrId;
}
```

a requirer's selector that the root's own declaration replaced. a root `ref` or `path`
wins over every requirer with no check that the dependency supports it; a root
`version` intersects with the requirers' ranges instead, so it is never one

| Field | Type | |
| :--- | :--- | :--- |
| `name` | `intern.StrId` | the dependency key both declarations use |
| `chain` | `intern.StrId` | the requiring chain, ending at the dependency |
| `requested` | `intern.StrId` | the requirer's selector, as `ref = "..."`, `version = "..."` or `path = "..."` |
| `declared` | `intern.StrId` | the root's selector, in the same form |

### root_override_text

```mach
pub fun root_override_text(s: *session.Session, o: *RootOverride) res[str, fail.Fail]
```

the note `mach dep verify` prints for one override

### dependencies

```mach
pub fun dependencies(s: *session.Session, project_root: str, release_rule: bool,
    overrides: *Vector[RootOverride]) err[fail.Fail]
```

check a project's realized dependency closure without changing it

| Parameter | |
| :--- | :--- |
| `s` | the session |
| `project_root` | the root project's directory |
| `release_rule` | also hold the root to the release rule: every dependency selected by `version` or an exact `tag/`, as a release about to be tagged must be |
| `overrides` | when not nil, receives every requirer selector a root override replaced |
| **returns** | err naming the first mismatch |

## Functions

### project_closure

```mach
pub fun project_closure(s: *session.Session, a: *A.Allocator, root: str, release_rule: bool, rep: *report.Report) err[fail.Fail]
```

`mach dep verify`: check that every dependency of a project is realized and consistent
without changing anything. a project root that is not a repository root is verified from
the realized checkouts, and a realized directory under dep/ outside the closure is refused,
where pull notes and retains it; each root override of a requirer's selector is reported

| Parameter | |
| :--- | :--- |
| `s` | the session; a failure may borrow from it, so the caller shows it before ending it |
| `a` | the storage the operation works in |
| `root` | the project root directory |
| `release_rule` | also refuse a root dependency selected by a branch, a commit or a path |
| `rep` | receives each override note and every refusal past the first |
| **returns** | err naming the failure |

