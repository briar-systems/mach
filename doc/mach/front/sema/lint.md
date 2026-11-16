[mach](../../README.md) / [front](../README.md) / [sema](README.md) / **lint**

# mach.front.sema.lint

the lints over a typed module: uses of deprecated declarations, and testing
declarations reached from code outside a test

<sub>source: [`src/front/sema/lint.mach`](../../../../src/front/sema/lint.mach) · imported by 1 module</sub>

<sub>`sema_context` is [`mach.front.sema.context`](context.md) · `fail` is [`mach.base.fail`](../../base/fail.md)</sub>

| Functions | |
| :--- | :--- |
| [`warn_deprecated_uses`](#warn_deprecated_uses) | |
| [`check_testing_uses`](#check_testing_uses) | a reference to a `#[testing]` declaration is legal only inside a test body or another `#[testing]` declaration: every resolved name and type use is checked against the source ranges of those declarations, and an unmarked `fwd` may not re-export one |

---

## Functions

### warn_deprecated_uses

```mach
pub fun warn_deprecated_uses(sc: *sema_context.SemaContext) err[fail.Fail]
```

### check_testing_uses

```mach
pub fun check_testing_uses(sc: *sema_context.SemaContext) err[fail.Fail]
```

a reference to a `#[testing]` declaration is legal only inside a test body or another
`#[testing]` declaration: every resolved name and type use is checked against the source
ranges of those declarations, and an unmarked `fwd` may not re-export one

