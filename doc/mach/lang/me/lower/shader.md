# mach.lang.me.lower.shader

a shader declaration's decorators as the attributes of the target family that reads them

a stage, its entry name and its workgroup size attach to the function, and an interface
variable's storage class, descriptor set and binding, location, built-in and memory
qualifiers to the global, each the number spir-v gives it, read from masc's grammar by
its spir-v name. only a target whose family's schema is spir-v's takes them, so a machine
target compiles a staged function as any other

## fun attaching

```mach
pub fun attaching(lc: *lower_context.LowerContext) bool;
```

whether the target reads shader attributes

## fun initialized

```mach
pub fun initialized(lc: *lower_context.LowerContext, d: *ast_decl.Decl) bool;
```

whether a global of the declaration `d` holds the initial value it is written with: the
host, the pipeline or the previous stage supplies an interface variable's

## fun specialized

```mach
pub fun specialized(lc: *lower_context.LowerContext, d: *ast_decl.Decl) bool;
```

whether the declaration `d` is a `#[gpu.spec]` var on a target that states specialization
constants, which defines no global there: each read of it is the constant

## rec SpecVar

```mach
pub rec SpecVar;
```

a `#[gpu.spec]` var as its specialization constant: the id the host sets it by and the
default its initializer gives

## fun spec_var

```mach
pub fun spec_var(lc: *lower_context.LowerContext, sym: *resolve.Symbol) res[opt[SpecVar], fail.Fail];
```

the `#[gpu.spec]` var `sym` names on a target that states specialization constants, read in
the scope it is written in, none for any other binding or target

## fun global_attributes

```mach
pub fun global_attributes(lc: *lower_context.LowerContext, d: *ast_decl.Decl, gix: u32) err[fail.Fail];
```

the attributes of global `gix` its declaration `d` states, read in the scope `d` is
written in. a global that already carries attributes keeps them

## fun extern_global_attributes

```mach
pub fun extern_global_attributes(lc: *lower_context.LowerContext, sym: *resolve.Symbol, gix: u32) err[fail.Fail];
```

the attributes of the global `gix` declared here for another module's declaration of
`sym`, read in the scope it is written in

## fun function_attributes

```mach
pub fun function_attributes(lc: *lower_context.LowerContext, d: *ast_decl.Decl, fix: u32, bare: intern.StrId) err[fail.Fail];
```

the entry point function `fix` is: its stage, the bare source name a pipeline finds it
by, and a compute stage's workgroup size, one in each dimension it does not state

