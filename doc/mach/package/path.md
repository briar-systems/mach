[mach](../README.md) / [package](README.md) / **path**

# mach.package.path

path: the member that realizes a dependency declared by a local `path`. it has no
releases and no pin: every pull syncs dep/&lt;id> with its source, copying what the source's
git keeps when the source is in a work tree, and leaving behind the build output and the
realized dependencies of every project inside it

<sub>source: [`src/package/path.mach`](../../../src/package/path.mach) · imported by 3 modules</sub>

<sub>`session` is [`mach.session`](../session.md) · `intern` is [`mach.base.intern`](../base/intern.md) · `fail` is [`mach.base.fail`](../base/fail.md) · `report` is [`mach.package.report`](report.md)</sub>

| Functions | |
| :--- | :--- |
| [`verify`](#verify) | what `mach dep verify` checks of a path dependency's copy: no repository metadata and no symlink anywhere in it |
| [`realize`](#realize) | sync dep/&lt;id> with a path dependency's source |

---

## Functions

### verify

```mach
pub fun verify(s: *session.Session, alias: str, dep_dir: str) res[intern.StrId, fail.Fail]
```

what `mach dep verify` checks of a path dependency's copy: no repository metadata and no
symlink anywhere in it

| Parameter | |
| :--- | :--- |
| **returns** | the realized content an exact selector is checked against |

### realize

```mach
pub fun realize(s: *session.Session, root: str, id: str, src_dir: str, rep: *report.Report) res[bool, fail.Fail]
```

sync dep/&lt;id> with a path dependency's source

| Parameter | |
| :--- | :--- |
| **returns** | true when the copy was refreshed, false when it already matched its source and was reused |

