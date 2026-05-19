# mach.lang.fe.ast.module

The top-level AST node for one parsed source file.

## Types

### `Module`

```mach
pub rec Module {
    span:        token.Span;
    decls_start: u32;
    decls_len:   u32;
}
```

| Field        | Type                                  | Description                                              |
|--------------|---------------------------------------|----------------------------------------------------------|
| span         | [`token.Span`](../token.md#span)      | Byte range covering the whole module.                    |
| decls_start  | `u32`                                 | Start index into [`Ast.decl_ids`](../ast.md#ast). |
| decls_len    | `u32`                                 | Number of declarations at module scope.                  |

## Dependencies

`std.types.size`, [`mach.lang.fe.token`](../token.md).
