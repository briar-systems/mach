[mach](../README.md) / [manifest](README.md) / **refusal**

# mach.manifest.refusal

how the manifest refuses: the text of a refusal, made through the caller's
allocator, where in the file it points, and the other places it names. a
refused allocation while a refusal is made is the failure instead, never an
empty or placeholder text

<sub>source: [`src/manifest/refusal.mach`](../../../src/manifest/refusal.mach) · imported by 15 modules</sub>

<sub>`base_source` is [`mach.base.source`](../base/source.md) · `A` is `std.allocator` · `fail` is [`mach.base.fail`](../base/fail.md) · `diagnostic_kind` is [`mach.base.diagnostic.kind`](../base/diagnostic/kind.md) · `toml_value` is `toml.value`</sub>

| Type | | |
| :--- | :--- | :--- |
| [`Sites`](#sites) | `rec` | the places a refusal names, gathered in order: it points at the first and names the rest as related |

| On `Sites` | |
| :--- | :--- |
| [`sites_init`](#sites_init) · [`sites_add`](#sites_add) | |
| [`sites_fail`](#sites_fail) | `f` pointing at the first place gathered, naming the others as related; the gathered places are released |

| Functions | |
| :--- | :--- |
| [`sfmt`](#sfmt) | `fmt` formatted with `va` into `alloc`, which owns the text |
| [`refuse`](#refuse) | the input is refused under `k` with the text `fmt` formats from `va`, owned by `alloc`; a refused allocation is the failure instead |
| [`point_at`](#point_at) | the same failure pointing at `sp` in the manifest being parsed; the zero span of a value the parser did not write points nowhere |
| [`point_at_file`](#point_at_file) | the same failure pointing at the manifest being parsed as a whole, at its start, for what is missing from the file itself |
| [`fail_at`](#fail_at) | a failure pointing at `sp`, as `point_at` places it |
| [`span_site`](#span_site) | where `sp` is written in the manifest being parsed, as a range whose file `parse` names once the manifest parses; the zero place for a value the parser did not write |
| [`element_sites`](#element_sites) | where each element of `arr` is written, in order, as `span_site` places it; nil for an empty array |
| [`at_sites`](#at_sites) | `f` pointing at the first spanned place of the `n` at `sites`, naming the spanned ones after it as related |

---

## Sites

```mach
pub rec Sites {
    list:    Vector[base_source.Location];
    refused: opt[A.Error];
}
```

the places a refusal names, gathered in order: it points at the first and
names the rest as related

| Field | Type | |
| :--- | :--- | :--- |
| `list` | `Vector[base_source.Location]` | the places, an unspanned one skipped |
| `refused` | `opt[A.Error]` | the allocator refused to hold one, which the refusal becomes |

### sites_init

```mach
pub fun sites_init(alloc: *A.Allocator) Sites
```

### sites_add

```mach
pub fun sites_add(s: *Sites, p: base_source.Location)
```

### sites_fail

```mach
pub fun sites_fail(alloc: *A.Allocator, s: *Sites, f: fail.Fail) fail.Fail
```

`f` pointing at the first place gathered, naming the others as related; the
gathered places are released

## Functions

### sfmt

```mach
pub fun sfmt(alloc: *A.Allocator, fmt: str, va: ...) res[str, fail.Fail]
```

`fmt` formatted with `va` into `alloc`, which owns the text

| Parameter | |
| :--- | :--- |
| **returns** | the text; err when the allocator refused it |

### refuse

```mach
pub fun refuse(alloc: *A.Allocator, k: diagnostic_kind.Kind, fmt: str, va: ...) fail.Fail
```

the input is refused under `k` with the text `fmt` formats from `va`, owned
by `alloc`; a refused allocation is the failure instead

### point_at

```mach
pub fun point_at(f: fail.Fail, sp: toml_value.Span) fail.Fail
```

the same failure pointing at `sp` in the manifest being parsed; the zero span
of a value the parser did not write points nowhere

### point_at_file

```mach
pub fun point_at_file(f: fail.Fail) fail.Fail
```

the same failure pointing at the manifest being parsed as a whole, at its start, for what is
missing from the file itself

### fail_at

```mach
pub fun fail_at(f: fail.Fail, sp: toml_value.Span) err[fail.Fail]
```

a failure pointing at `sp`, as `point_at` places it

### span_site

```mach
pub fun span_site(sp: toml_value.Span) base_source.Location
```

where `sp` is written in the manifest being parsed, as a range whose file
`parse` names once the manifest parses; the zero place for a value the
parser did not write

### element_sites

```mach
pub fun element_sites(alloc: *A.Allocator, arr: *toml_value.Array) res[*base_source.Location, fail.Fail]
```

where each element of `arr` is written, in order, as `span_site` places it;
nil for an empty array. freed with `model.sites_free`

### at_sites

```mach
pub fun at_sites(alloc: *A.Allocator, f: fail.Fail, sites: *base_source.Location, n: usize) fail.Fail
```

`f` pointing at the first spanned place of the `n` at `sites`, naming the
spanned ones after it as related

