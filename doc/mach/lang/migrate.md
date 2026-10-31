# mach.lang.migrate

a project written for the previous major, rewritten into this one's spelling,
which `mach migrate` applies

every rule is data its owner already holds: a manifest key a major removed or
retired is a row of the manifest schema with a rewrite, a template variable's
previous name is a row of the template's renamed table, a comptime path's is a
row of the comptime path registry's and a decorator's is a row of the
decorator registry's. so a name a later major retires is migrated by the row
that retires it, and this module names none of them

## fun manifest

```mach
pub fun manifest(a: *A.Allocator, text: str) res[str, fail.Fail];
```

a manifest's text in this major's spelling: each key the schema retires with
a rewrite rewritten, and each template variable a previous major named
renamed wherever a string writes it. a text that is not TOML is refused, and
one already in this spelling comes back unchanged

a: owns the returned text
text: the manifest's text
ret: the rewritten text, owned by `a`

## fun source

```mach
pub fun source(a: *A.Allocator, text: str) res[str, fail.Fail];
```

a source's text in this major's spelling: each comptime path whose leading
segments a previous major named and each decorator it named renamed. only
tokens are read, so a comment or a string that spells one is left as written

a: owns the returned text
text: the source's text
ret: the rewritten text, owned by `a`, or the lexer's refusal

