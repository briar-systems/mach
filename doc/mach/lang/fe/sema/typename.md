# mach.lang.fe.sema.typename

## fun type_to_str

```mach
pub fun type_to_str(s: *session.Session, t: type.TypeId) res[str, fail.Fail];
```

## fun type_str_free

```mach
pub fun type_str_free(s: *session.Session, owned: str);
```

## fun instance_spelling

```mach
pub fun instance_spelling(s: *session.Session, name: intern.StrId, args: *type.TypeId, arg_len: u32) res[str, fail.Fail];
```

## fun name_str

```mach
pub fun name_str(s: *session.Session, name: intern.StrId) str;
```

the spelling of a type's name; an anonymous type, whose name is absent or
empty, is spelled `<anon>`

