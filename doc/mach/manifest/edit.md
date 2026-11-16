[mach](../README.md) / [manifest](README.md) / **edit**

# mach.manifest.edit

editing a manifest's text in place: adding and removing a `[dep.<name>]`
table, leaving every other byte as it was written

<sub>source: [`src/manifest/edit.mach`](../../../src/manifest/edit.mach) · imported by 2 modules</sub>

<sub>`A` is `std.allocator` · `fail` is [`mach.base.fail`](../base/fail.md)</sub>

| Type | | |
| :--- | :--- | :--- |
| [`DepSource`](#depsource) | `def` | the source key a `mach dep add` writes |
| [`DepTableSpec`](#deptablespec) | `rec` | a `[dep.<name>]` table to append to a manifest's text |

| On `DepSource` | |
| :--- | :--- |
| [`DEP_SOURCE_GIT`](#depsource) | a `git = "<url>"` dependency, with an optional `ref` |
| [`DEP_SOURCE_PATH`](#depsource) | a `path = "<dir>"` dependency |

| On `DepTableSpec` | |
| :--- | :--- |
| [`dep_table_add`](#dep_table_add) | append a `[dep.<name>]` table to manifest text, leaving every existing byte in place |

| Functions | |
| :--- | :--- |
| [`dep_table_remove`](#dep_table_remove) | cut a `[dep.<name>]` table out of manifest text, from its header line to the next header, leaving everything else byte for byte |

---

## DepSource

```mach
pub def DepSource: u8
```

the source key a `mach dep add` writes

| Name | Value | |
| :--- | :--- | :--- |
| `DEP_SOURCE_GIT` | `1` | a `git = "<url>"` dependency, with an optional `ref` |
| `DEP_SOURCE_PATH` | `2` | a `path = "<dir>"` dependency |

## DepTableSpec

```mach
pub rec DepTableSpec {
    name:    str;
    source:  DepSource;
    value:   str;
    ref:     str;
    version: str;
}
```

a `[dep.<name>]` table to append to a manifest's text

| Field | Type | |
| :--- | :--- | :--- |
| `name` | `str` | the dependency name; must satisfy `is_valid_id` |
| `source` | `DepSource` | which key `value` is written under |
| `value` | `str` | the git URL or the path; escaped as a TOML basic string |
| `ref` | `str` | the `ref` value; written only for a git source and only when non-empty |
| `version` | `str` | the `version` range; written only for a git source, only when set, and never with a ref |

### dep_table_add

```mach
pub fun dep_table_add(alloc: *A.Allocator, source_text: str, spec: *DepTableSpec) res[str, fail.Fail]
```

append a `[dep.<name>]` table to manifest text, leaving every existing byte in
place. the block uses the file's line ending and is separated by one blank
line (two when the text lacks a trailing newline)

| Parameter | |
| :--- | :--- |
| `alloc` | owns the returned text |
| `source_text` | the current `mach.toml` |
| `spec` | the table to add |
| **returns** | the new text; err when the name is not an identifier, the source is not git or path, the text is not valid TOML, the dependency is already declared in any form, or the result does not reparse with the table present |

## Functions

### dep_table_remove

```mach
pub fun dep_table_remove(alloc: *A.Allocator, source_text: str, name: str) res[str, fail.Fail]
```

cut a `[dep.<name>]` table out of manifest text, from its header line to the
next header, leaving everything else byte for byte

| Parameter | |
| :--- | :--- |
| `alloc` | owns the returned text |
| `source_text` | the current `mach.toml` |
| `name` | the dependency name |
| **returns** | the new text; err when the name is not an identifier, the text is not valid TOML, the dependency is absent, is declared but not as a table, is declared more than once, or its header is not spelled exactly `[dep.<name>]` at the start of a line |

