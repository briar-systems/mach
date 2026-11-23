[mach](../README.md) / [front](README.md) / **path**

# mach.front.path

dotted paths read from the source, with the trivia between their segments skipped

a path keeps the comments and whitespace it was written with, so its segments, its last
segment and its prefix are found by walking its span, and `intern_path` interns it with its
segments joined by single dots.

Source: [`src/front/path.mach`](../../../src/front/path.mach)

Uses types from [`mach.base.intern`](../base/intern.md), [`mach.base.source`](../base/source.md).

| Functions | |
| :--- | :--- |
| [`next`](#next) | paths are parser accepted identifier and dot sequences with original trivia intact |
| [`leaf`](#leaf) | the last segment of a dotted path |
| [`prefix`](#prefix) | the path up to the end of its second to last segment, trivia included |
| [`intern_path`](#intern_path) | intern a dotted path with its trivia dropped and its segments joined by single dots |
| [`path_text`](#path_text) | the text `intern_path` interns, from `a` and terminated, without interning it |

---

## Functions

### next

```mach
pub fun next(source: str, path: mach.base.source.Span, cursor: *usize) mach.base.source.Span
```

paths are parser accepted identifier and dot sequences with original trivia intact

### leaf

```mach
pub fun leaf(source: str, path: mach.base.source.Span) mach.base.source.Span
```

the last segment of a dotted path

| Parameter | |
| :--- | :--- |
| `source` | text the path is read from |
| `path` | span of the whole path |
| **returns** | span of the last segment, empty at the path's start when it has none |

### prefix

```mach
pub fun prefix(source: str, path: mach.base.source.Span) mach.base.source.Span
```

the path up to the end of its second to last segment, trivia included

| Parameter | |
| :--- | :--- |
| `source` | text the path is read from |
| `path` | span of the whole path |
| **returns** | span from the path's start to the end of the second to last segment, empty for a one segment path |

### intern_path

```mach
pub fun intern_path(itn: *mach.base.intern.Interner, source: str, path: mach.base.source.Span) res[mach.base.intern.StrId, std.allocator.Error]
```

intern a dotted path with its trivia dropped and its segments joined by single dots

| Parameter | |
| :--- | :--- |
| `itn` | interner to add the text to, whose allocator holds the scratch copy |
| `source` | text the path is read from |
| `path` | span of the whole path |
| **returns** | the interned id, or the allocator's error |

### path_text

```mach
pub fun path_text(a: *std.allocator.Allocator, source: str, path: mach.base.source.Span) res[str, std.allocator.Error]
```

the text `intern_path` interns, from `a` and terminated, without interning it

