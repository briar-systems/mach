# mach.front.sema.publish

builds a module's interface once sema has typed it: every module-level function and
binding as its declaration says, every module constant evaluated to its deep value, and the
public surface its resolution published

## fun interface_of

```mach
pub fun interface_of(sc: *sema_context.SemaContext) res[interface.Interface, fail.Fail];
```

the interface takes over the module's store of constant values, which its typing built in

## fun refusals_report

```mach
pub fun refusals_report(sc: *sema_context.SemaContext, iface: *interface.Interface);
```

reports the refusal each module `val` and `var` the interface publishes holds, as lowering
the global reports it: a refused member of an aggregate or address value under any kind, and
a refused value of its own only as lowering's own fold meets it again, the evaluator's
rejection as it is and an initializer that needs a value no constant gives as no constant.
a `nil` initializer lowers as null and folds nothing

