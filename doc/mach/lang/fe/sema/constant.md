# mach.lang.fe.sema.constant

the module constants sema binds: every module `val` whose initializer evaluates at compile
time, bound once in sema's own bindings, which its result publishes to lowering and to every
importer. a constant is bound where it is first read while declarations are typed, and every
other one once they all are, before any body or instance is

## val CONSTANT_PENDING

```mach
pub val CONSTANT_PENDING: u8 = 0
```

pending: not bound yet, and tried again on the next read; none: no constant, for good

## fun named

```mach
pub fun named(sc: *sema_context.SemaContext, eid: ast_id.ExprId) res[opt[comptime.CTValue], comptime.EvalFail];
```

the value of the module constant the bare name `eid` names: one of this module's, bound on
first read, or one an imported module published

## fun bind

```mach
pub fun bind(sc: *sema_context.SemaContext, did: ast_id.DeclId) err[fail.Fail];
```

binds the module `val` `did` declares when its initializer is a compile-time constant

## fun settle

```mach
pub fun settle(sc: *sema_context.SemaContext);
```

every constant still unbound once all are tried is none, so no later read, inside a body or
an instance, evaluates one again

