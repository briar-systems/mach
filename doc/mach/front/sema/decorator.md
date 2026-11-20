[mach](../../README.md) / [front](../README.md) / [sema](README.md) / **decorator**

# mach.front.sema.decorator

checking each decorator against its declaration: its places, arguments and rule

Source: [`src/front/sema/decorator.mach`](../../../../src/front/sema/decorator.mach)

Uses types from [`mach.base.fail`](../../base/fail.md), [`mach.base.intern`](../../base/intern.md), [`mach.front.ast`](../ast.md), [`mach.front.ast.decl`](../ast/decl.md), [`mach.front.ast.id`](../ast/id.md), [`mach.front.comptime.scope`](../comptime/scope.md), [`mach.front.sema.context`](context.md), [`mach.session`](../../session.md).

| Functions | |
| :--- | :--- |
| [`infer_decorator_args`](#infer_decorator_args) | types every expression argument of `d`'s decorators, leaving out the arguments a `handle` decorator takes as types and every argument of an unknown decorator |
| [`validate_decorators`](#validate_decorators) | checks each decorator of `d` against its registry row and then against the rule its id adds, with the decorators of a tag's cases, a `def` that states no type, and the symbols `d` defines |
| [`states_function`](#states_function) | whether `d` is a function that states a signature for a symbol: an `ext fun`, or a definition no template makes instances of |
| [`stated_symbol`](#stated_symbol) | the symbol the function `d` of the syntax `a`, whose source is `source`, states: the name its `symbol` or `weak` decorator gives, else an `ext fun`'s own name |
| [`stated_label`](#stated_label) | the label of the place the function `d` states its symbol |
| [`signature_refusal`](#signature_refusal) | the refusal of the function `second` of the syntax `sa` stating the symbol `symbol` with another signature than the function `first` of `fa` states it with, each spelled as its source writes it, released with `str_free` |

---

## Functions

### infer_decorator_args

```mach
pub fun infer_decorator_args(sc: *mach.front.sema.context.SemaContext, d: *mach.front.ast.decl.Decl)
```

types every expression argument of `d`'s decorators, leaving out the arguments a
`handle` decorator takes as types and every argument of an unknown decorator

| Parameter | |
| :--- | :--- |
| `sc` | the sema context of the module being typed |
| `d` | the declaration whose decorators are typed |

### validate_decorators

```mach
pub fun validate_decorators(sc: *mach.front.sema.context.SemaContext, did: mach.front.ast.id.DeclId, d: *mach.front.ast.decl.Decl)
```

checks each decorator of `d` against its registry row and then against the rule its
id adds, with the decorators of a tag's cases, a `def` that states no type, and the
symbols `d` defines. problems are reported to `sc`

| Parameter | |
| :--- | :--- |
| `sc` | the sema context of the module being typed |
| `did` | the declaration's id |
| `d` | the declaration whose decorators are checked |

### states_function

```mach
pub fun states_function(a: *mach.front.ast.Ast, d: *mach.front.ast.decl.Decl) bool
```

whether `d` is a function that states a signature for a symbol: an `ext fun`, or a
definition no template makes instances of

### stated_symbol

```mach
pub fun stated_symbol(s: *mach.session.Session, c: *mach.front.comptime.scope.ComptimeCtx, a: *mach.front.ast.Ast, source: str, d: *mach.front.ast.decl.Decl) res[mach.base.intern.StrId, mach.base.fail.Fail]
```

the symbol the function `d` of the syntax `a`, whose source is `source`, states: the name its
`symbol` or `weak` decorator gives, else an `ext fun`'s own name. nil for a definition the
compiler names, which no other declaration can name

### stated_label

```mach
pub fun stated_label(d: *mach.front.ast.decl.Decl) str
```

the label of the place the function `d` states its symbol

### signature_refusal

```mach
pub fun signature_refusal(s: *mach.session.Session, symbol: mach.base.intern.StrId, fa: *mach.front.ast.Ast, first: *mach.front.ast.decl.Decl, sa: *mach.front.ast.Ast, second: *mach.front.ast.decl.Decl) res[str, mach.base.fail.Fail]
```

the refusal of the function `second` of the syntax `sa` stating the symbol `symbol` with another
signature than the function `first` of `fa` states it with, each spelled as its source writes
it, released with `str_free`

