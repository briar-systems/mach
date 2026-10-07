# mach.lang.fe.sema.publish

builds a module's interface once sema has typed it: every module-level function and
binding as its declaration says, every module constant evaluated to its deep value, and the
public surface its resolution published

## fun interface_of

```mach
pub fun interface_of(sc: *sema_context.SemaContext) res[interface.Interface, fail.Fail];
```

the interface takes over the module's store of constant values, which its typing built in

