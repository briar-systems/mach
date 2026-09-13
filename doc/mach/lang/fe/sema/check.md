# mach.lang.fe.sema.check

## fun check_assignable

```mach
pub fun check_assignable(sc: *context.SemaContext, expected: type.TypeId, actual: type.TypeId, span: token.Span) bool;
```

## fun check_coercible

```mach
pub fun check_coercible(sc: *context.SemaContext, expected: type.TypeId, eid: id.ExprId, actual: type.TypeId, span: token.Span) bool;
```

## fun check_call

```mach
pub fun check_call(sc: *context.SemaContext, callee_sig: type.TypeId, args_start: u32, args_len: u32, span: token.Span, pack_to_c_tail: bool) bool;
```

## fun check_comptime_call

```mach
pub fun check_comptime_call(sc: *context.SemaContext, callee_sig: type.TypeId, callee: id.ExprId, f: *decl.DeclFun, args_start: u32, args_len: u32, span: token.Span) bool;
```

## fun imported_module_const_sym

```mach
pub fun imported_module_const_sym(sc: *context.SemaContext, sym: *resolve.Symbol) bool;
```

## fun check_index

```mach
pub fun check_index(sc: *context.SemaContext, object: type.TypeId, index: type.TypeId, span: token.Span) opt[type.TypeId];
```

## fun check_const_index

```mach
pub fun check_const_index(sc: *context.SemaContext, object: type.TypeId, idx: id.ExprId, span: token.Span) bool;
```

## fun const_index_value

```mach
pub fun const_index_value(sc: *context.SemaContext, eid: id.ExprId) opt[comptime.CTValue];
```

## fun check_secret_address

```mach
pub fun check_secret_address(sc: *context.SemaContext, ty: type.TypeId, span: token.Span) bool;
```

## fun check_member

```mach
pub fun check_member(sc: *context.SemaContext, object: type.TypeId, name: intern.StrId, guarded: bool, span: token.Span) opt[type.TypeId];
```

## fun check_cast

```mach
pub fun check_cast(sc: *context.SemaContext, from: type.TypeId, to: type.TypeId, span: token.Span) opt[type.TypeId];
```

## fun check_reinterpret

```mach
pub fun check_reinterpret(sc: *context.SemaContext, from: type.TypeId, to: type.TypeId, span: token.Span) opt[type.TypeId];
```

## fun check_return

```mach
pub fun check_return(sc: *context.SemaContext, fn_ret: type.TypeId, value: type.TypeId, span: token.Span) bool;
```

## fun check_condition

```mach
pub fun check_condition(sc: *context.SemaContext, cond: type.TypeId, span: token.Span) bool;
```

## fun is_numeric

```mach
pub fun is_numeric(sc: *context.SemaContext, t: type.TypeId) bool;
```

## fun is_integer

```mach
pub fun is_integer(sc: *context.SemaContext, t: type.TypeId) bool;
```

## fun byte_size

```mach
pub fun byte_size(sc: *context.SemaContext, t: type.TypeId) u32;
```

## fun type_to_str

```mach
pub fun type_to_str(sc: *context.SemaContext, t: type.TypeId) res[str, fail.Fail];
```

## fun type_str_free

```mach
pub fun type_str_free(sc: *context.SemaContext, owned: str);
```

## fun check_float_capability

```mach
pub fun check_float_capability(sc: *context.SemaContext, tid: type.TypeId, span: token.Span);
```

## fun report_no_field

```mach
pub fun report_no_field(sc: *context.SemaContext, span: token.Span, name: intern.StrId, record: type.TypeId);
```

## fun report_named

```mach
pub fun report_named(sc: *context.SemaContext, span: token.Span, prefix: str, name: intern.StrId, suffix: str, fallback: str);
```

## fun report_missing_type_args

```mach
pub fun report_missing_type_args(sc: *context.SemaContext, span: token.Span, name: intern.StrId, expected: u32);
```

## fun report_typed

```mach
pub fun report_typed(sc: *context.SemaContext, span: token.Span, prefix: str, t: type.TypeId, suffix: str, fallback: str);
```

## fun report_typed2

```mach
pub fun report_typed2(sc: *context.SemaContext, span: token.Span, prefix: str, a: type.TypeId, mid: str, b: type.TypeId, suffix: str, fallback: str);
```

## fun report_out_of_range

```mach
pub fun report_out_of_range(sc: *context.SemaContext, span: token.Span, cr: coerce.CoerceResult);
```

## fun report_instantiation_limit

```mach
pub fun report_instantiation_limit(sc: *context.SemaContext, span: token.Span, outcome: u8,
bare: intern.StrId, args: *type.TypeId, arg_len: u32);
```

