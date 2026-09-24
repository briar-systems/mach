# mach.lang.fe.sema.recipe

## fun anonymous

```mach
pub fun anonymous(s: *session.Session, defs: *isa.TargetDefs, kind: type.TypeKind,
fields: *type.FieldEntry, count: u32) res[intern.StrId, fail.Fail];
```

## fun identity

```mach
pub fun identity(s: *session.Session, defs: *isa.TargetDefs, tid: type.TypeId) res[u64, fail.Fail];
```

the first 64 bits of sha-256 over a domain tag and the type's canonical recipe; the recipe
names nominal types by owning module and name, so every unit of one program derives the same value

