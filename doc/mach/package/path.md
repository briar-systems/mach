[mach](../README.md) / [package](README.md) / **path**

# mach.package.path

path: the member that realizes a dependency declared by a local `path`. it has no
releases and no pin: every pull syncs dep/\<id> with its source, copying what the source's
git keeps when the source is in a work tree, and leaving behind the build output and the
realized dependencies of every project inside it

Source: [`src/package/path.mach`](../../../src/package/path.mach)

Uses types from [`mach.base.fail`](../base/fail.md), [`mach.base.intern`](../base/intern.md), [`mach.package.report`](report.md), [`mach.session`](../session.md).

| Functions | |
| :--- | :--- |
| [`verify`](#verify) | what `mach dep verify` checks of a path dependency's copy: no repository metadata and no symlink anywhere in it |
| [`realize`](#realize) | sync dep/\<id> with a path dependency's source |

| Constants | |
| :--- | :--- |
| [`COPY_REFRESHED`](#constants) | what syncing a path dependency's copy did: made it where none existed, refreshed what differed from its source, or reused it as it stood |
| [`COPY_REUSED`](#constants), [`COPY_MADE`](#constants) | |

---

## Functions

### verify

```mach
pub fun verify(s: *mach.session.Session, alias: str, dep_dir: str) res[mach.base.intern.StrId, mach.base.fail.Fail]
```

what `mach dep verify` checks of a path dependency's copy: no repository metadata and no
symlink anywhere in it

| Parameter | |
| :--- | :--- |
| **returns** | the realized content an exact selector is checked against |

### realize

```mach
pub fun realize(s: *mach.session.Session, root: str, id: str, src_dir: str, rep: *mach.package.report.Report) res[u8, mach.base.fail.Fail]
```

sync dep/\<id> with a path dependency's source

| Parameter | |
| :--- | :--- |
| **returns** | COPY_MADE, COPY_REFRESHED or COPY_REUSED, what the sync did to the copy |

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `COPY_REFRESHED` | `u8` | `1` | what syncing a path dependency's copy did: made it where none existed, refreshed what differed from its source, or reused it as it stood |
| `COPY_REUSED` | `u8` | `2` |  |
| `COPY_MADE` | `u8` | `3` |  |

