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
| [`leaf`](#leaf), [`prefix`](#prefix), [`intern_path`](#intern_path) | |

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

### prefix

```mach
pub fun prefix(source: str, path: mach.base.source.Span) mach.base.source.Span
```

### intern_path

```mach
pub fun intern_path(itn: *mach.base.intern.Interner, source: str, path: mach.base.source.Span) res[mach.base.intern.StrId, std.allocator.Error]
```

