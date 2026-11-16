[mach](../README.md) / [front](README.md) / **expect**

# mach.front.expect

`#[expect("<key>", ...)]`: a declaration's acknowledgement of named warnings

type checking validates each `#[expect]` where it stands; this module reads the
keys back out of a parsed module into the expectations a diagnostic store
matches warnings against, so a warning replayed from the cache is matched
exactly as a fresh one is

<sub>source: [`src/front/expect.mach`](../../../src/front/expect.mach) · imported by 4 modules</sub>

<sub>`ast` is [`mach.front.ast`](ast.md) · `Site` is private · `Step` is private · `session` is [`mach.session`](../session.md) · `fail` is [`mach.base.fail`](../base/fail.md) · `comptime_scope` is [`mach.front.comptime.scope`](comptime/scope.md) · `diagnostic` is [`mach.base.diagnostic`](../base/diagnostic.md) · `ast_id` is [`mach.front.ast.id`](ast/id.md) · `base_source` is [`mach.base.source`](../base/source.md)</sub>

| Type | | |
| :--- | :--- | :--- |
| [`Index`](#index) | `rec` | where a module's `#[expect]` declarations stand in its syntax, read once per parse so a refresh of the diagnostics walks none of the module's declarations |

| On `Index` | |
| :--- | :--- |
| [`index_dnit`](#index_dnit) | |
| [`site_count`](#site_count) | how many declarations in the module the index holds carry `#[expect]` |
| [`index`](#index-1) | bring the index up to the parsed module `a`, reading its sites when it was read from another parse; a failed read leaves it unread, so the next one walks the module again |
| [`collect`](#collect) | every `#[expect]` key at the sites the index read from `a`, one expectation each |

| Functions | |
| :--- | :--- |
| [`key_of`](#key_of) | the key an argument evaluates to; none when the argument is not a constant string |
| [`arg_span`](#arg_span) | the span an argument occupies, where a refused or unfulfilled key is reported |

---

## Index

```mach
pub rec Index {
    syntax:      *ast.Ast;
    incarnation: u64;
    sites:       Vector[Site];
    steps:       Vector[Step];
    visited:     u64;
}
```

where a module's `#[expect]` declarations stand in its syntax, read once per parse so a
refresh of the diagnostics walks none of the module's declarations

| Field | Type | |
| :--- | :--- | :--- |
| `syntax` | `*ast.Ast` | the parsed module the sites were read from, nil before the first read |
| `incarnation` | `u64` | that parse's incarnation |
| `sites` | `Vector[Site]` | each declaration that carries `#[expect]`, in source order |
| `steps` | `Vector[Step]` | the `$if` arms each site stands in, outermost first, as the sites' ranges name them |
| `visited` | `u64` | the declarations every read has walked, over the module's parses |

### index_dnit

```mach
pub fun index_dnit(idx: *Index)
```

### site_count

```mach
pub fun site_count(idx: *Index) usize
```

how many declarations in the module the index holds carry `#[expect]`

### index

```mach
pub fun index(s: *session.Session, a: *ast.Ast, idx: *Index) err[fail.Fail]
```

bring the index up to the parsed module `a`, reading its sites when it was read from another
parse; a failed read leaves it unread, so the next one walks the module again

### collect

```mach
pub fun collect(s: *session.Session, a: *ast.Ast, idx: *Index, ctx: *comptime_scope.ComptimeCtx, checked: bool,
    out: *Vector[diagnostic.Expectation]) err[fail.Fail]
```

every `#[expect]` key at the sites the index read from `a`, one expectation each

s:       the session whose interner holds its keys
a:       the parsed module the index was brought up to
idx:     the module's index
ctx:     the comptime scope whose gate decisions say which `$if` branch is taken, over the load's
         bindings that hold its keys
checked: the module was type checked this build, or its warnings replayed
out:     the list the expectations are appended to

## Functions

### key_of

```mach
pub fun key_of(s: *session.Session, ctx: *comptime_scope.ComptimeCtx, eid: ast_id.ExprId) opt[str]
```

the key an argument evaluates to; none when the argument is not a constant string

### arg_span

```mach
pub fun arg_span(a: *ast.Ast, eid: ast_id.ExprId, fallback: base_source.Span) base_source.Span
```

the span an argument occupies, where a refused or unfulfilled key is reported

