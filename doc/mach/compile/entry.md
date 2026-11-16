[mach](../README.md) / [compile](README.md) / **entry**

# mach.compile.entry

an entry build: one artifact of one entry file built with no mach.toml, the command line
stating the whole project. the project it states is made as the manifest's model and held to
the checks every manifest passes, so the build that follows cannot tell the two apart

Source: [`src/compile/entry.mach`](../../../src/compile/entry.mach)

Uses types from [`mach.base.fail`](../base/fail.md), [`mach.manifest.model`](../manifest/model.md), [`mach.session`](../session.md).

| Type | | |
| :--- | :--- | :--- |
| [`Narrowing`](#narrowing) | `rec` | the flags that narrow an entry build's target, each as written and nil when absent |
| [`EntryRequest`](#entryrequest) | `rec` | what the command line states of an entry build |

| On `Narrowing` | |
| :--- | :--- |
| [`narrowed`](#narrowed) | whether any narrowing flag is given |

| On `EntryRequest` | |
| :--- | :--- |
| [`entry_manifest`](#entry_manifest) | the project an entry build states, made as its manifest |

---

## Narrowing

```mach
pub rec Narrowing {
    env:        str;
    abi:        str;
    isa:        str;
    exts:       *str;
    ext_count:  usize;
    code_model: str;
}
```

the flags that narrow an entry build's target, each as written and nil when absent

| Field | Type | |
| :--- | :--- | :--- |
| `env` | `str` | `--env` |
| `abi` | `str` | `--abi` |
| `isa` | `str` | `--isa` |
| `exts` | `*str` | every `--ext`, in argv order |
| `ext_count` | `usize` | length of `exts` |
| `code_model` | `str` | `--code-model` |

### narrowed

```mach
pub fun narrowed(n: *Narrowing) bool
```

whether any narrowing flag is given

## EntryRequest

```mach
pub rec EntryRequest {
    entry:        str;
    kind:         str;
    linkage:      str;
    sources:      *str;
    source_count: usize;
    target:       str;
    narrowing:    Narrowing;
    optimize:     bool;
    debug:        bool;
}
```

what the command line states of an entry build

| Field | Type | |
| :--- | :--- | :--- |
| `entry` | `str` | the entry file, a canonical path under the current directory |
| `kind` | `str` | the artifact's product, mink's spelling |
| `linkage` | `str` | the `--linkage` spelling, nil for the target's default |
| `sources` | `*str` | each `-s` value, `name=dir` as written |
| `source_count` | `usize` | length of `sources` |
| `target` | `str` | the `-t` spelling, nil for the host's system and architecture |
| `narrowing` | `Narrowing` | the flags that narrow the target |
| `optimize` | `bool` | the build runs the optimization pass set |
| `debug` | `bool` | the build emits debug information |

### entry_manifest

```mach
pub fun entry_manifest(s: *mach.session.Session, r: *EntryRequest) res[mach.manifest.model.Manifest, mach.base.fail.Fail]
```

the project an entry build states, made as its manifest. the project root is the current
directory, so `-o` and `-w` keep the rules they keep in a project

| Parameter | |
| :--- | :--- |
| `s` | the session whose build allocator owns the manifest and whose interner holds its names |
| `r` | what the command line states |
| **returns** | the manifest, owned by the session's build allocator; err as the command line names no buildable project |

