# mach.lang.fe.sema.interface

what a module publishes for the lowering of every module that imports it, in place of
its syntax tree: the facts of each module-level function and binding, the deep value of
each module constant, and the public surface a `fwd` re-exports. it owns everything it
holds and points into no syntax tree, scope or other product

## val NONE

```mach
pub val NONE: u32 = 0xFFFFFFFF
```

## def DeclarationFlags

```mach
pub def DeclarationFlags: u8
```

## val DECLARATION_EXT

```mach
pub val DECLARATION_EXT:  DeclarationFlags = 1
```

## val DECLARATION_BODY

```mach
pub val DECLARATION_BODY: DeclarationFlags = 2
```

## val DECLARATION_PACK

```mach
pub val DECLARATION_PACK: DeclarationFlags = 4
```

## rec Declaration

```mach
pub rec Declaration;
```

one module-level function, `val` or `var`

decl: the declaration in its module, which keys the session's tables of it
generics: how many type parameters a function declares
params_start: where a function's parameters start in `params`
op_set: the instruction set and operation an `#[op]` names, STR_NIL without one
value: the node in `values` of a `val`'s constant value, or of the compile-time value
              a `var`'s initializer gives it, NONE when it has none
gated: the value is declared under a comptime condition

## rec Param

```mach
pub rec Param;
```

## rec Exported

```mach
pub rec Exported;
```

one symbol on the module's public surface, as an importer of it names it

origin: the module that declares it, which may be one this module re-exports
module: the module a `pub use` names, for a module symbol

## rec Interface

```mach
pub rec Interface;
```

## rec Reader

```mach
pub rec Reader;
```

how a lowering reads the interface another module published

## fun init

```mach
pub fun init(alloc: *A.Allocator) Interface;
```

## fun dnit

```mach
pub fun dnit(iface: *Interface);
```

## fun declaration_add

```mach
pub fun declaration_add(iface: *Interface, d: Declaration, params: *Param, params_len: u32) err[fail.Fail];
```

adds `d` with the parameters `params[0 .. params_len]`; a second declaration of one kind and
canonical name leaves that name ambiguous, which a lookup by name refuses

## fun declaration_at

```mach
pub fun declaration_at(iface: *Interface, decl: ast_id.DeclId) opt[*Declaration];
```

the declaration `decl` of the module, none for one the interface does not describe

## fun declaration_for

```mach
pub fun declaration_for(iface: *Interface, kind: resolve.SymKind, canon: intern.StrId) res[*Declaration, fail.Fail];
```

the one declaration of `kind` the module defines under `canon`

## fun param_at

```mach
pub fun param_at(iface: *Interface, d: *Declaration, ord: u32) opt[Param];
```

## fun declaration_has_comptime_param

```mach
pub fun declaration_has_comptime_param(iface: *Interface, d: *Declaration) bool;
```

## fun declaration_has_instances

```mach
pub fun declaration_has_instances(iface: *Interface, d: *Declaration) bool;
```

a generic, comptime-parameter or pack function has instances in place of one definition

## fun template_of

```mach
pub fun template_of(iface: *Interface) opt[*template.Template];
```

the template part, none for a module with no generic, comptime-parameter or pack function

## fun exported_add

```mach
pub fun exported_add(iface: *Interface, e: Exported) err[fail.Fail];
```

## fun exported_count

```mach
pub fun exported_count(iface: *Interface) u32;
```

## fun exported_at

```mach
pub fun exported_at(iface: *Interface, ord: u32) *Exported;
```

