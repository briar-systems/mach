[mach](../README.md) / [front](README.md) / **path**

# mach.front.path

dotted paths read from the source, with the trivia between their segments skipped

a path keeps the comments and whitespace it was written with, so its segments, its last
segment and its prefix are found by walking its span, and `intern_path` interns it with its
segments joined by single dots.

<sub>source: [`src/front/path.mach`](../../../src/front/path.mach) · imported by 4 modules</sub>

<sub>`base_source` is [`mach.base.source`](../base/source.md) · `intern` is [`mach.base.intern`](../base/intern.md) · `A` is `std.allocator`</sub>

| Functions | |
| :--- | :--- |
| [`next`](#next) | paths are parser accepted identifier and dot sequences with original trivia intact |
| [`leaf`](#leaf) · [`prefix`](#prefix) · [`intern_path`](#intern_path) | |

---

## Functions

### next

```mach
pub fun next(source: str, path: base_source.Span, cursor: *usize) base_source.Span
```

paths are parser accepted identifier and dot sequences with original trivia intact

### leaf

```mach
pub fun leaf(source: str, path: base_source.Span) base_source.Span
```

### prefix

```mach
pub fun prefix(source: str, path: base_source.Span) base_source.Span
```

### intern_path

```mach
pub fun intern_path(itn: *intern.Interner, source: str, path: base_source.Span) res[intern.StrId, A.Error]
```

