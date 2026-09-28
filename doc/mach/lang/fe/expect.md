# mach.lang.fe.expect

`#[expect("<key>", ...)]`: a declaration's acknowledgement of named warnings

type checking validates each `#[expect]` where it stands; this module reads the
keys back out of a parsed module into the expectations a diagnostic store
matches warnings against, so a warning replayed from the cache is matched
exactly as a fresh one is

## val DIRECTIVE

```mach
pub val DIRECTIVE: str = "expect"
```

## fun placement_allowed

```mach
pub fun placement_allowed(k: decl.DeclKind) bool;
```

the declaration kinds an `#[expect]` may stand on; a module has no
declaration of its own, so a module-wide silence is a profile's `allow`

## fun is_expect

```mach
pub fun is_expect(source: str, dec: *decl.Decorator) bool;
```

## fun key_of

```mach
pub fun key_of(a: *ast.Ast, source: str, itn: *intern.Interner, eid: id.ExprId) opt[str];
```

the key an argument spells, decoded; none when the argument is not a string literal

## fun arg_span

```mach
pub fun arg_span(a: *ast.Ast, eid: id.ExprId, fallback: token.Span) token.Span;
```

the span an argument occupies, where a refused or unfulfilled key is reported

## fun collect

```mach
pub fun collect(a: *ast.Ast, source: str, itn: *intern.Interner, ctx: *comptime.ComptimeCtx, checked: bool,
out: *Vector[diagnostic.Expectation]) err[fail.Fail];
```

every `#[expect]` key in the module, one expectation each

a:       the parsed module
source:  its text
itn:     the interner its string literals decode into
ctx:     the comptime context whose gate decisions say which `$if` branch is taken
checked: the module was type checked this build, or its warnings replayed
out:     the list the expectations are appended to

