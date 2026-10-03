# mach.lang.fe.sema.lint

the lints over a typed module: uses of deprecated declarations, and testing
declarations reached from code outside a test

## fun warn_deprecated_uses

```mach
pub fun warn_deprecated_uses(sc: *sema_context.SemaContext) err[fail.Fail];
```

one warning per source site after every body and instantiation has been inferred: named
declaration uses through their resolved symbol, and tag case uses (construction, `sel`,
payload place) through the case table of the place's tag

## fun check_testing_uses

```mach
pub fun check_testing_uses(sc: *sema_context.SemaContext) err[fail.Fail];
```

a reference to a `#[testing]` declaration is legal only inside a test body or another
`#[testing]` declaration: every resolved name and type use is checked against the source
ranges of those declarations, and an unmarked `fwd` may not re-export one

