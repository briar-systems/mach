[mach](../README.md) / [manifest](README.md) / **setting**

# mach.manifest.setting

`--set`: a manifest key written over every table of its section for one invocation. the key
and its value are read by the rows and decoders that read the key in a manifest, so a value
`--set` gives is held to every rule the key keeps in mach.toml

<sub>source: [`src/manifest/setting.mach`](../../../src/manifest/setting.mach) · imported by 2 modules</sub>

<sub>`A` is `std.allocator` · `intern` is [`mach.base.intern`](../base/intern.md) · `base_source` is [`mach.base.source`](../base/source.md) · `fail` is [`mach.base.fail`](../base/fail.md)</sub>

| Type | | |
| :--- | :--- | :--- |
| [`Assignment`](#assignment) | `rec` | one `--set <key>=<value>`, split at its first `=` |

| On `Assignment` | |
| :--- | :--- |
| [`apply`](#apply) | write each assignment over every table of its key's section in `m`, in order, so a later assignment of a key wins |

---

## Assignment

```mach
pub rec Assignment {
    key:   str;
    value: str;
}
```

one `--set <key>=<value>`, split at its first `=`

| Field | Type | |
| :--- | :--- | :--- |
| `key` | `str` | the manifest key, as mach.toml writes it |
| `value` | `str` | the value, as written |

### apply

```mach
pub fun apply(alloc: *A.Allocator, itn: *intern.Interner, sources: *base_source.SourceMap, m: *Manifest, list: *Assignment,
    n: usize) err[fail.Fail]
```

write each assignment over every table of its key's section in `m`, in order, so a later
assignment of a key wins

| Parameter | |
| :--- | :--- |
| `alloc` | owns a refusal's text |
| `itn` | receives the strings |
| `sources` | receives each assignment's text, which a refusal points into |
| `m` | the manifest, its native target included |
| `list` | the assignments |
| `n` | length of `list` |
| **returns** | ok, or the refusal of the first key `--set` does not write or value its key does not accept |

