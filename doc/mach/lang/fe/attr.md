# mach.lang.fe.attr

the attribute arguments that take a string, and the string each one is

an argument is a constant expression evaluated in its module's comptime
context: a literal, a `val`, a gated `val`, or an imported constant. the load
evaluates each once, after its walk has bound every constant, and records the
string in its own bindings; every later stage reads it there, so resolution,
type checking, lowering and the link all read one value

## fun string_id

```mach
pub fun string_id(c: *comptime.ComptimeCtx, a: *ast.Ast, dec: *ast_decl.Decorator, ord: u32) intern.StrId;
```

the string argument `ord` of `dec` evaluated to in the scope `c`; STR_NIL when it is not a constant string

## fun string_of

```mach
pub fun string_of(itn: *intern.Interner, c: *comptime.ComptimeCtx, a: *ast.Ast, dec: *ast_decl.Decorator, ord: u32) opt[str];
```

the text of `string_id`

## fun record_strings

```mach
pub fun record_strings[T](itn: *intern.Interner, into: *comptime.ComptimeCtx, a: *ast.Ast, source: str,
c: *comptime.ComptimeCtx, cap_ctx: *T, caps: comptime.PhaseCapabilities[T], reached: *bool) err[fail.Fail];
```

records into `into` the string every string argument of a module's attributes evaluates to, with
the phase's own evaluator over the scope `c`. an argument that is not a constant string is left
out, and so is a declaration `reached` does not mark, where it marks any

