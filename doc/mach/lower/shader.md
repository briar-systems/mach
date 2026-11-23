[mach](../README.md) / [lower](README.md) / **shader**

# mach.lower.shader

a shader declaration's decorators as the attributes of the target family that reads them

a stage, its entry name and its workgroup size attach to the function, and an interface
variable's storage class, descriptor set and binding, input attachment index, location,
built-in and memory qualifiers to the global, each the number spir-v gives it, read from
masc's grammar by its spir-v name. only a target whose family's schema is spir-v's takes
them, so a machine target compiles a staged function as any other

Source: [`src/lower/shader.mach`](../../../src/lower/shader.mach)

Uses types from [`mach.base.fail`](../base/fail.md), [`mach.base.intern`](../base/intern.md), [`mach.front.ast.decl`](../front/ast/decl.md), [`mach.front.comptime.value`](../front/comptime/value.md), [`mach.front.resolve`](../front/resolve.md), [`mach.lower.context`](context.md).

| Type | | |
| :--- | :--- | :--- |
| [`SpecVar`](#specvar) | `rec` | a `#[gpu.spec]` var as its specialization constant: the id the host sets it by and the default its initializer gives |

| On `SpecVar` | |
| :--- | :--- |
| [`spec_var`](#spec_var) | the `#[gpu.spec]` var `sym` names on a target that states specialization constants, read in the scope it is written in, none for any other binding or target |

| Functions | |
| :--- | :--- |
| [`attaching`](#attaching) | whether the target reads shader attributes |
| [`storage_class`](#storage_class) | the value of the storage class the spir-v grammar names `name` |
| [`initialized`](#initialized) | whether a global of the declaration `d` holds the initial value it is written with: the host, the pipeline or the previous stage supplies an interface variable's |
| [`specialized`](#specialized) | whether the declaration `d` is a `#[gpu.spec]` var on a target that states specialization constants, which defines no global there: each read of it is the constant |
| [`global_attributes`](#global_attributes) | the attributes of global `gix` its declaration `d` states, read in the scope `d` is written in |
| [`extern_global_attributes`](#extern_global_attributes) | the attributes of the global `gix` declared here for another module's declaration of `sym`, read in the scope it is written in |
| [`function_attributes`](#function_attributes) | the entry point function `fix` is: its stage, the bare source name a pipeline finds it by, and a compute stage's workgroup size, one in each dimension it does not state |

---

## SpecVar

```mach
pub rec SpecVar {
    id:      u32;
    default: mach.front.comptime.value.CTValue;
}
```

a `#[gpu.spec]` var as its specialization constant: the id the host sets it by and the
default its initializer gives

### spec_var

```mach
pub fun spec_var(lc: *mach.lower.context.LowerContext, sym: *mach.front.resolve.Symbol) res[opt[SpecVar], mach.base.fail.Fail]
```

the `#[gpu.spec]` var `sym` names on a target that states specialization constants, read in
the scope it is written in, none for any other binding or target

## Functions

### attaching

```mach
pub fun attaching(lc: *mach.lower.context.LowerContext) bool
```

whether the target reads shader attributes

### storage_class

```mach
pub fun storage_class(name: str) opt[u64]
```

the value of the storage class the spir-v grammar names `name`

### initialized

```mach
pub fun initialized(lc: *mach.lower.context.LowerContext, d: *mach.front.ast.decl.Decl) bool
```

whether a global of the declaration `d` holds the initial value it is written with: the
host, the pipeline or the previous stage supplies an interface variable's

### specialized

```mach
pub fun specialized(lc: *mach.lower.context.LowerContext, d: *mach.front.ast.decl.Decl) bool
```

whether the declaration `d` is a `#[gpu.spec]` var on a target that states specialization
constants, which defines no global there: each read of it is the constant

### global_attributes

```mach
pub fun global_attributes(lc: *mach.lower.context.LowerContext, d: *mach.front.ast.decl.Decl, gix: u32) err[mach.base.fail.Fail]
```

the attributes of global `gix` its declaration `d` states, read in the scope `d` is
written in. a global that already carries attributes keeps them

### extern_global_attributes

```mach
pub fun extern_global_attributes(lc: *mach.lower.context.LowerContext, sym: *mach.front.resolve.Symbol, gix: u32) err[mach.base.fail.Fail]
```

the attributes of the global `gix` declared here for another module's declaration of
`sym`, read in the scope it is written in

### function_attributes

```mach
pub fun function_attributes(lc: *mach.lower.context.LowerContext, d: *mach.front.ast.decl.Decl, fix: u32, bare: mach.base.intern.StrId) err[mach.base.fail.Fail]
```

the entry point function `fix` is: its stage, the bare source name a pipeline finds it
by, and a compute stage's workgroup size, one in each dimension it does not state

