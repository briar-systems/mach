[mach](../../README.md) / [front](../README.md) / [sema](README.md) / **publish**

# mach.front.sema.publish

builds a module's interface once sema has typed it: every module-level function and
binding as its declaration says, every module constant evaluated to its deep value, and the
public surface its resolution published

<sub>source: [`src/front/sema/publish.mach`](../../../../src/front/sema/publish.mach) · imported by 1 module</sub>

<sub>`sema_context` is [`mach.front.sema.context`](context.md) · `interface` is [`mach.front.sema.interface`](interface.md) · `fail` is [`mach.base.fail`](../../base/fail.md)</sub>

| Functions | |
| :--- | :--- |
| [`interface_of`](#interface_of) | the interface takes over the module's store of constant values, which its typing built in |
| [`refusals_report`](#refusals_report) | reports the refusal each module `val` and `var` the interface publishes holds, as lowering the global reports it: a refused member of an aggregate or address value under any kind, and a refused value of its own only as lowering's own fold meets it again, the evaluator's rejection as it is and an initializer that needs a value no constant gives as no constant |

---

## Functions

### interface_of

```mach
pub fun interface_of(sc: *sema_context.SemaContext) res[interface.Interface, fail.Fail]
```

the interface takes over the module's store of constant values, which its typing built in

### refusals_report

```mach
pub fun refusals_report(sc: *sema_context.SemaContext, iface: *interface.Interface)
```

reports the refusal each module `val` and `var` the interface publishes holds, as lowering
the global reports it: a refused member of an aggregate or address value under any kind, and
a refused value of its own only as lowering's own fold meets it again, the evaluator's
rejection as it is and an initializer that needs a value no constant gives as no constant.
a `nil` initializer lowers as null and folds nothing

