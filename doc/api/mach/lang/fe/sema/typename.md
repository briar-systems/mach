# mach.lang.fe.sema.typename

## fun type_to_str

```mach
pub fun type_to_str(s: *session.Session, t: type.TypeId) res[str, fail.Fail];
```

## fun type_str_free

```mach
pub fun type_str_free(s: *session.Session, owned: str);
```

## fun text_join

```mach
pub fun text_join(s: *session.Session, acc: str, piece: str, piece_owned: bool) str;
```

## fun text_stable

```mach
pub fun text_stable(s: *session.Session, owned: str, fallback: str) str;
```

## fun instance_spelling

```mach
pub fun instance_spelling(s: *session.Session, name: intern.StrId, args: *type.TypeId, arg_len: u32) res[str, fail.Fail];
```

## fun name_str

```mach
pub fun name_str(s: *session.Session, name: intern.StrId) str;
```

