# mach.lang.fe.sema.embed

## val ESCAPED_EMBED_MSG

```mach
pub val ESCAPED_EMBED_MSG: str = "`embed` path escapes the project root
```

an embed outside the project root is refused, and its bytes are never read:
the driver skips the path when it collects embed inputs, and this is the
diagnostic that names why (the 4.30 warning was removed in 5.0.0)

## fun resolve_one

```mach
pub fun resolve_one(sc: *context.SemaContext, did: id.DeclId, d: *decl.Decl,
dec: *decl.Decorator) err[fail.Fail];
```

