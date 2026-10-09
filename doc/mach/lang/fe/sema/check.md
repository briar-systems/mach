# mach.lang.fe.sema.check

## fun check_coercible

```mach
pub fun check_coercible(sc: *sema_context.SemaContext, expected: type.TypeId, eid: ast_id.ExprId, actual: type.TypeId, span: lang_source.Span) bool;
```

## fun check_call

```mach
pub fun check_call(sc: *sema_context.SemaContext, callee_sig: type.TypeId, args_start: u32, args_len: u32, span: lang_source.Span, pack_to_c_tail: bool) bool;
```

## fun check_comptime_call

```mach
pub fun check_comptime_call(sc: *sema_context.SemaContext, eid: ast_id.ExprId, callee_sig: type.TypeId, callee: ast_id.ExprId, f: *ast_decl.DeclFun, args_start: u32, args_len: u32, span: lang_source.Span) bool;
```

## fun check_comptime_arg_fits

```mach
pub fun check_comptime_arg_fits(sc: *sema_context.SemaContext, value: comptime.CTValue, param_ty: type.TypeId, span: lang_source.Span) bool;
```

a comptime argument's value is held to its parameter's type the way a runtime
argument is, so one its type cannot represent is refused as an overflow

## fun imported_module_const_sym

```mach
pub fun imported_module_const_sym(sc: *sema_context.SemaContext, sym: *resolve.Symbol) bool;
```

## fun check_index

```mach
pub fun check_index(sc: *sema_context.SemaContext, object: type.TypeId, index: type.TypeId, span: lang_source.Span) opt[type.TypeId];
```

## fun check_const_index

```mach
pub fun check_const_index(sc: *sema_context.SemaContext, object: type.TypeId, idx: ast_id.ExprId, span: lang_source.Span) bool;
```

## fun check_range_count

```mach
pub fun check_range_count(sc: *sema_context.SemaContext, count: ast_id.ExprId) opt[u32];
```

the count of a range `x[i, n]`: a comptime constant, at least one. absent,
reported at the count, when it is not

## fun check_const_range

```mach
pub fun check_const_range(sc: *sema_context.SemaContext, object: type.TypeId, start: ast_id.ExprId, count: u32, span: lang_source.Span) bool;
```

a constant start must keep the whole range inside an array or a vector:
`start + count` may reach the length and not pass it. a pointer has no length

## fun const_index_value

```mach
pub fun const_index_value(sc: *sema_context.SemaContext, eid: ast_id.ExprId) opt[comptime.CTValue];
```

## fun check_shift_count

```mach
pub fun check_shift_count(sc: *sema_context.SemaContext, span: lang_source.Span, lt: type.TypeId, count: ast_id.ExprId) bool;
```

a shift whose count is a comptime constant must keep the count below the
left operand's width: at or above it the runtime answer is the saturated
value (0, or the sign fill), which a constant program never means

## fun check_secret_address

```mach
pub fun check_secret_address(sc: *sema_context.SemaContext, ty: type.TypeId, span: lang_source.Span) bool;
```

## fun check_member

```mach
pub fun check_member(sc: *sema_context.SemaContext, object: type.TypeId, name: intern.StrId, guarded: bool, span: lang_source.Span) opt[type.TypeId];
```

## fun check_cast

```mach
pub fun check_cast(sc: *sema_context.SemaContext, from: type.TypeId, to: type.TypeId, span: lang_source.Span) opt[type.TypeId];
```

## fun check_reinterpret

```mach
pub fun check_reinterpret(sc: *sema_context.SemaContext, from: type.TypeId, to: type.TypeId, span: lang_source.Span) opt[type.TypeId];
```

## fun check_return

```mach
pub fun check_return(sc: *sema_context.SemaContext, fn_ret: type.TypeId, value: type.TypeId, span: lang_source.Span) bool;
```

## fun check_condition

```mach
pub fun check_condition(sc: *sema_context.SemaContext, cond: type.TypeId, span: lang_source.Span) bool;
```

## fun byte_size

```mach
pub fun byte_size(sc: *sema_context.SemaContext, t: type.TypeId) u32;
```

## fun type_to_str

```mach
pub fun type_to_str(sc: *sema_context.SemaContext, t: type.TypeId) res[str, fail.Fail];
```

## fun type_str_free

```mach
pub fun type_str_free(sc: *sema_context.SemaContext, owned: str);
```

## fun check_float_capability

```mach
pub fun check_float_capability(sc: *sema_context.SemaContext, tid: type.TypeId, span: lang_source.Span);
```

## fun report_no_field

```mach
pub fun report_no_field(sc: *sema_context.SemaContext, span: lang_source.Span, name: intern.StrId, record: type.TypeId);
```

## fun report_named

```mach
pub fun report_named(sc: *sema_context.SemaContext, k: diagnostic_kind.Kind, span: lang_source.Span, prefix: str, name: intern.StrId, suffix: str);
```

## fun report_missing_type_args

```mach
pub fun report_missing_type_args(sc: *sema_context.SemaContext, span: lang_source.Span, name: intern.StrId, expected: u32);
```

## fun report_typed

```mach
pub fun report_typed(sc: *sema_context.SemaContext, k: diagnostic_kind.Kind, span: lang_source.Span, prefix: str, t: type.TypeId, suffix: str);
```

## fun report_typed2

```mach
pub fun report_typed2(sc: *sema_context.SemaContext, k: diagnostic_kind.Kind, span: lang_source.Span, prefix: str, a: type.TypeId, mid: str, b: type.TypeId, suffix: str);
```

## fun expr_span_of

```mach
pub fun expr_span_of(sc: *sema_context.SemaContext, eid: ast_id.ExprId) res[lang_source.Span, fail.Fail];
```

## fun report_out_of_range

```mach
pub fun report_out_of_range(sc: *sema_context.SemaContext, span: lang_source.Span, cr: coerce.CoerceResult);
```

## fun report_default_out_of_range

```mach
pub fun report_default_out_of_range(sc: *sema_context.SemaContext, span: lang_source.Span, cr: coerce.CoerceResult);
```

the range refusal of a literal no context typed, which took the i64 default

