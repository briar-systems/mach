[mach](../../README.md) / [front](../README.md) / [comptime](README.md) / **path**

# mach.front.comptime.path

the comptime path registry: one row per `$root.segment...` path the compiler
answers, matched once against a written path's segments

a row names the value its path reads; the evaluator reads it by that id and
never by a segment's spelling. a path that matches no row is refused with the
message of the most specific family it falls in, or, when a previous major
named its leading segments, with the name this one gives them, which
`mach migrate` writes. a new path is a row here and its read

Source: [`src/front/comptime/path.mach`](../../../../src/front/comptime/path.mach)

Uses types from [`mach.base.fail`](../../base/fail.md), [`mach.base.intern`](../../base/intern.md), [`mach.base.source`](../../base/source.md), [`mach.front.ast`](../ast.md), [`mach.front.ast.id`](../ast/id.md), [`mach.front.comptime.failure`](failure.md), [`mach.front.comptime.scope`](scope.md), [`mach.front.comptime.value`](value.md).

| Type | | |
| :--- | :--- | :--- |
| [`Read`](#read) | `def` | what a path reads; `Row.arg` refines the version components, 0 major, 1 minor and 2 patch |
| [`Row`](#row) | `rec` | one path |
| [`Renamed`](#renamed) | `rec` | leading segments a previous major named, and the name this one gives them |

| On `Read` | |
| :--- | :--- |
| [`READ_SYS_TAG`](#read), [`READ_ARCH_TAG`](#read), [`READ_ABI_TAG`](#read), [`READ_MODE_TAG`](#read), [`READ_BUILD_CT_MUL`](#read), [`READ_REFUSED`](#read), [`READ_RESERVED`](#read) | |

| On `Row` | |
| :--- | :--- |
| [`at`](#at) | the row at `i` in declaration order, nil past the end |
| [`lookup`](#lookup) | the row a written path matches, called or not; nil when none does |

| On `Renamed` | |
| :--- | :--- |
| [`renamed`](#renamed-1) | the renamed row whose segments lead the written path, nil when none does |
| [`renamed_depth`](#renamed_depth) | how many segments a renamed row's `from` spans |

| Functions | |
| :--- | :--- |
| [`unknown`](#unknown) | the refusal of a path no row matches; nil when its root names no family |
| [`eval_path`](#eval_path), [`resolve_comptime_path`](#resolve_comptime_path) | |
| [`sys_tag_for`](#sys_tag_for) | the tag of `$mach.sys.<name>`: the position of mink's system of that name |
| [`arch_tag_for`](#arch_tag_for) | the tag of `$mach.arch.<name>`: the position of mink's architecture of that name |
| [`abi_tag_for`](#abi_tag_for) | the tag of `$mach.abi.<name>`: the spelling's place among mink's conventions |

| Constants | |
| :--- | :--- |
| [`COMPTIME_BARE_IDENT_MSG`](#constants), [`MODE_DEBUG`](#constants), [`MODE_RELEASE`](#constants) | |
| [`MODE_COUNT`](#constants) | the `$mach.mode.*` tags, each at its mode's value |

---

## Read

```mach
pub def Read: u8
```

what a path reads; `Row.arg` refines the version components, 0 major, 1 minor and 2 patch

| Name | Value | |
| :--- | :--- | :--- |
| `READ_SYS_TAG` | `4` |  |
| `READ_ARCH_TAG` | `5` |  |
| `READ_ABI_TAG` | `6` |  |
| `READ_MODE_TAG` | `7` |  |
| `READ_BUILD_CT_MUL` | `22` |  |
| `READ_REFUSED` | `30` |  |
| `READ_RESERVED` | `31` |  |

## Row

```mach
pub rec Row {
    path: str;
    read: Read;
    arg:  u32;
    call: bool;
    note: str;
}
```

one path

| Field | Type | |
| :--- | :--- | :--- |
| `path` | `str` | its segments after the `$`, `.`-separated, where `*` is any one segment |
| `arg` | `u32` | what `read` needs beyond the path, a version component |
| `call` | `bool` | the path is called with arguments, `$mach.build.ct_mul(op, width)` |
| `note` | `str` | the refusal READ_REFUSED and READ_RESERVED report |

### at

```mach
pub fun at(i: usize) *Row
```

the row at `i` in declaration order, nil past the end

### lookup

```mach
pub fun lookup(source: str, stack: *mach.base.source.Span, depth: u32, call: bool) *Row
```

the row a written path matches, called or not; nil when none does. `stack`
holds its `depth` segments innermost first, the root at `depth - 1`

## Renamed

```mach
pub rec Renamed {
    from: str;
    to:   str;
    note: str;
}
```

leading segments a previous major named, and the name this one gives them

| Field | Type | |
| :--- | :--- | :--- |
| `from` | `str` | the segments as the previous major wrote them, `.`-separated |
| `to` | `str` | the segments that replace them |
| `note` | `str` | the refusal of a path that writes them |

### renamed

```mach
pub fun renamed(source: str, stack: *mach.base.source.Span, depth: u32) *Renamed
```

the renamed row whose segments lead the written path, nil when none does.
`stack` holds its `depth` segments innermost first, the root at `depth - 1`

### renamed_depth

```mach
pub fun renamed_depth(r: *Renamed) u32
```

how many segments a renamed row's `from` spans

## Functions

### unknown

```mach
pub fun unknown(source: str, stack: *mach.base.source.Span, depth: u32) str
```

the refusal of a path no row matches; nil when its root names no family

### eval_path

```mach
pub fun eval_path(
    c: *mach.front.comptime.scope.ComptimeCtx,
    a: *mach.front.ast.Ast,
    source: str,
    e: mach.front.ast.id.ExprId,
    interner: *mach.base.intern.Interner) res[mach.front.comptime.value.CTValue, mach.front.comptime.failure.EvalFail]
```

### resolve_comptime_path

```mach
pub fun resolve_comptime_path(c: *mach.front.comptime.scope.ComptimeCtx, source: str, file: mach.base.source.FileId, stack: *mach.base.source.Span, depth: u32, interner: *mach.base.intern.Interner) res[mach.front.comptime.value.CTValue, mach.front.comptime.failure.EvalFail]
```

### sys_tag_for

```mach
pub fun sys_tag_for(name: View) res[u32, mach.base.fail.Fail]
```

the tag of `$mach.sys.<name>`: the position of mink's system of that name

### arch_tag_for

```mach
pub fun arch_tag_for(name: View) res[u32, mach.base.fail.Fail]
```

the tag of `$mach.arch.<name>`: the position of mink's architecture of that name

### abi_tag_for

```mach
pub fun abi_tag_for(name: View) res[u32, mach.base.fail.Fail]
```

the tag of `$mach.abi.<name>`: the spelling's place among mink's conventions

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `COMPTIME_BARE_IDENT_MSG` | `str` | `` "comptime parameters are referenced without `$`. a comptime path starts at `$mach`, `$project` or `$bin`" `` |  |
| `MODE_DEBUG` | `u32` | `0` |  |
| `MODE_RELEASE` | `u32` | `1` |  |
| `MODE_COUNT` | `usize` | `2` | the `$mach.mode.*` tags, each at its mode's value |

