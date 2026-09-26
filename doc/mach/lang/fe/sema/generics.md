# mach.lang.fe.sema.generics

## fun instantiate

```mach
pub fun instantiate(
sc: *sema.SemaContext,
sym: *resolve.Symbol,
args: *type.TypeId,
arg_count: u32,
span: token.Span) res[type.TypeId, fail.Fail];
```

## fun check_arity

```mach
pub fun check_arity(
sc: *sema.SemaContext,
sym: *resolve.Symbol,
arg_count: u32,
span: token.Span) bool;
```

## fun check_annotation_uni_secrecy

```mach
pub fun check_annotation_uni_secrecy(sc: *sema.SemaContext, ast_tid: id.TypeId);
```

## fun check_type_uni_secrecy

```mach
pub fun check_type_uni_secrecy(sc: *sema.SemaContext, tid: type.TypeId, who: *type.GenericOwner,
args: *type.TypeId, arg_len: u32, span: token.Span);
```

## fun substitute

```mach
pub fun substitute(s: *session.Session, body_type: type.TypeId, who: *type.GenericOwner, args: *type.TypeId, arg_count: u32) res[type.TypeId, fail.Fail];
```

replace `who`'s parameters in `body_type` with `args`, position for position;
a parameter of any other declaration (an enclosing generic's, in an open
instance) stays, so a substituted type reads as the identity under a second pass

## fun ensure_fields

```mach
pub fun ensure_fields(s: *session.Session, tid: type.TypeId) err[fail.Fail];
```

## fun contains_secret_deep

```mach
pub fun contains_secret_deep(s: *session.Session, tid: type.TypeId) res[bool, fail.Fail];
```

## fun contains_float_deep

```mach
pub fun contains_float_deep(s: *session.Session, tid: type.TypeId) res[bool, fail.Fail];
```

## fun type_extent

```mach
pub fun type_extent(s: *session.Session, m: layout.Machine, tid: type.TypeId) res[layout.Extent, fail.Fail];
```

## fun type_field_offset

```mach
pub fun type_field_offset(s: *session.Session, m: layout.Machine, tid: type.TypeId, field_ix: u32) res[layout.Extent, fail.Fail];
```

the byte offset of a record field or tag payload from the same checked layout that sizes the type;
`size` carries the offset when `ok`

## fun secrecy_cast_allowed

```mach
pub fun secrecy_cast_allowed(s: *session.Session, m: layout.Machine, from: type.TypeId, to: type.TypeId) res[bool, fail.Fail];
```

whether a `::` or `:~` from `from` to `to` keeps every byte's secrecy class

## fun secrecy_overlay_agrees

```mach
pub fun secrecy_overlay_agrees(s: *session.Session, m: layout.Machine, a: type.TypeId, b: type.TypeId) res[bool, fail.Fail];
```

whether two union variants overlaying one storage agree on every byte they share

## fun module_reaches_secret

```mach
pub fun module_reaches_secret(s: *session.Session, resolved: *type.TypeId, len: u32) res[bool, fail.Fail];
```

## fun contains_tag_value

```mach
pub fun contains_tag_value(s: *session.Session, tid: type.TypeId) res[bool, fail.Fail];
```

