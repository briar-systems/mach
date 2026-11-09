# mach.lang.fe.expect

`#[expect("<key>", ...)]`: a declaration's acknowledgement of named warnings

type checking validates each `#[expect]` where it stands; this module reads the
keys back out of a parsed module into the expectations a diagnostic store
matches warnings against, so a warning replayed from the cache is matched
exactly as a fresh one is

## fun key_of

```mach
pub fun key_of(s: *session.Session, ctx: *comptime_scope.ComptimeCtx, eid: ast_id.ExprId) opt[str];
```

the key an argument evaluates to; none when the argument is not a constant string

## fun arg_span

```mach
pub fun arg_span(a: *ast.Ast, eid: ast_id.ExprId, fallback: lang_source.Span) lang_source.Span;
```

the span an argument occupies, where a refused or unfulfilled key is reported

## rec Index

```mach
pub rec Index;
```

where a module's `#[expect]` declarations stand in its syntax, read once per parse so a
refresh of the diagnostics walks none of the module's declarations

syntax: the parsed module the sites were read from, nil before the first read
incarnation: that parse's incarnation
sites: each declaration that carries `#[expect]`, in source order
steps: the `$if` arms each site stands in, outermost first, as the sites' ranges name them
visited: the declarations every read has walked, over the module's parses

## fun index_dnit

```mach
pub fun index_dnit(idx: *Index);
```

## fun site_count

```mach
pub fun site_count(idx: *Index) usize;
```

how many declarations in the module the index holds carry `#[expect]`

## fun index

```mach
pub fun index(s: *session.Session, a: *ast.Ast, idx: *Index) err[fail.Fail];
```

bring the index up to the parsed module `a`, reading its sites when it was read from another
parse; a failed read leaves it unread, so the next one walks the module again

## fun collect

```mach
pub fun collect(s: *session.Session, a: *ast.Ast, idx: *Index, ctx: *comptime_scope.ComptimeCtx, checked: bool,
out: *Vector[diagnostic.Expectation]) err[fail.Fail];
```

every `#[expect]` key at the sites the index read from `a`, one expectation each

s:       the session whose interner holds its keys
a:       the parsed module the index was brought up to
idx:     the module's index
ctx:     the comptime scope whose gate decisions say which `$if` branch is taken, over the load's
         bindings that hold its keys
checked: the module was type checked this build, or its warnings replayed
out:     the list the expectations are appended to

