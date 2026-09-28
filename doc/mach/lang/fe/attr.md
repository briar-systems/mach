# mach.lang.fe.attr

the attribute arguments that take a string, and the string each one is

an argument is a constant expression evaluated in its module's comptime
context: a literal, a `val`, a gated `val`, or an imported constant. the load
evaluates each once, after its walk has bound every constant, and records the
string in the session; every later reader takes it from there, so resolution,
type checking, lowering and the link all read one value (#4022)

## fun takes_string

```mach
pub fun takes_string(source: str, dec: *decl.Decorator, ord: u32) bool;
```

whether argument `ord` of `dec` takes a string

## fun string_id

```mach
pub fun string_id(s: *session.Session, mid: session.ModuleId, a: *ast.Ast, dec: *decl.Decorator, ord: u32) opt[intern.StrId];
```

the string argument `ord` of `dec` evaluated to; none when it is not a constant string

## fun string_of

```mach
pub fun string_of(s: *session.Session, mid: session.ModuleId, a: *ast.Ast, dec: *decl.Decorator, ord: u32) opt[str];
```

the text of `string_id`

## fun record_strings

```mach
pub fun record_strings[T](s: *session.Session, mid: session.ModuleId, a: *ast.Ast, source: str,
c: *comptime.ComptimeCtx, cap_ctx: *T, caps: comptime.PhaseCapabilities[T], walked: bool) err[fail.Fail];
```

records the string every string argument of a module's attributes evaluates
to, with the phase's own evaluator over the module's comptime context. an
argument that is not a constant string is left out; a declaration in an `$if`
arm the load did not walk is skipped when `walked` asks for it

## fun forget_strings

```mach
pub fun forget_strings(s: *session.Session, mid: session.ModuleId, expr_count: usize);
```

drops what `record_strings` recorded for a module whose tree has `expr_count` expressions

