# mach.lang.fe.sema.constant

the module constants sema binds: every module `val` whose initializer evaluates at compile
time, bound once in sema's own bindings, which its result publishes to lowering and to every
importer. a constant is bound where it is first read while declarations are typed, and every
other one once they all are, before any body or instance is. the deep value of a constant,
its array, record or case literal evaluated member by member, is built in the module's store
of constant values, which its interface publishes

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

## fun declared_at_top

```mach
pub fun declared_at_top(sc: *sema_context.SemaContext, did: ast_id.DeclId) bool;
```

whether `did` is one of the module's own declarations rather than one in a gated arm

## fun settle

```mach
pub fun settle(sc: *sema_context.SemaContext);
```

every constant still unbound once all are tried is none, so no later read, inside a body or
an instance, evaluates one again

## fun own_value

```mach
pub fun own_value(sc: *sema_context.SemaContext, sym: *resolve.Symbol) res[u32, fail.Fail];
```

the node of the value the module `val` `sym` declares, built in the module's store on first
ask: the constant sema bound, or an array, record or case literal evaluated member by member;
NONE for any other initializer

## fun array_value

```mach
pub fun array_value(sc: *sema_context.SemaContext, sym: *resolve.Symbol) res[opt[comptime.CTValue], fail.Fail];
```

the value of the array `sym` names that a `$each` reads: a module constant's, this module's
built once and another module's as it published it, or the value of a `val` local to a body,
built in the walk that reads it since each instance of the body may give it another; none for
a binding with no array, record or case literal value

## fun store_of

```mach
pub fun store_of(sc: *sema_context.SemaContext, module_index: u32) res[*comptime_deep.Store, comptime.EvalFail];
```

the store of constant values of the module with stable id `module`: this module's, or the one
another module's interface publishes

## fun is_aggregate

```mach
pub fun is_aggregate(sc: *sema_context.SemaContext, eid: ast_id.ExprId) bool;
```

an array literal, or a record or case literal spelled with its head type

## fun type_at

```mach
pub fun type_at(sc: *sema_context.SemaContext, eid: ast_id.ExprId) type.TypeId;
```

the type the walk gave an expression

