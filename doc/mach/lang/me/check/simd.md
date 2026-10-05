# mach.lang.me.check.simd

the vector checks: an operation whose shape the target does not declare, and
an operation that falls back to scalar code

## fun declared

```mach
pub fun declared(s: *check_scope.Scope) err[fail.Fail];
```

a vector operator whose shape the target's catalog does not name is refused in
every simd mode: the outcome is a declaration, never a fallback. the refusal
is a located error on the store, so the tally and exit status agree

## fun fallback

```mach
pub fun fallback(s: *check_scope.Scope) err[fail.Fail];
```

every operation that falls back to scalar code is named at its own site: a
warning by default, and under `simd = "require"` an error, the same sites
with the same text. each names the operation, its lanes and the target, and
the extension whose declared packed row would pack it, read from the catalog

