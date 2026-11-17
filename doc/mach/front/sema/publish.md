[mach](../../README.md) / [front](../README.md) / [sema](README.md) / **publish**

# mach.front.sema.publish

builds a module's interface once sema has typed it: every module-level function and
binding as its declaration says, every module constant evaluated to its deep value, and the
public surface its resolution published

Source: [`src/front/sema/publish.mach`](../../../../src/front/sema/publish.mach)

Uses types from [`mach.base.fail`](../../base/fail.md), [`mach.front.sema.context`](context.md), [`mach.front.sema.interface`](interface.md).

| Functions | |
| :--- | :--- |
| [`interface_of`](#interface_of) | the interface takes over the module's store of constant values, which its typing built in |
| [`refusals_report`](#refusals_report) | reports the refusal each module `val` and `var` the interface publishes holds: an initializer, or a member of an aggregate or address value, that has no compile-time value as no constant, and any other refusal, a fault in a value it has, under its own kind |

---

## Functions

### interface_of

```mach
pub fun interface_of(sc: *mach.front.sema.context.SemaContext) res[mach.front.sema.interface.Interface, mach.base.fail.Fail]
```

the interface takes over the module's store of constant values, which its typing built in

### refusals_report

```mach
pub fun refusals_report(sc: *mach.front.sema.context.SemaContext, iface: *mach.front.sema.interface.Interface)
```

reports the refusal each module `val` and `var` the interface publishes holds: an initializer,
or a member of an aggregate or address value, that has no compile-time value as no constant,
and any other refusal, a fault in a value it has, under its own kind. a value of its own is
reported only as lowering's own fold meets it again. a `nil` initializer lowers as null and
folds nothing

