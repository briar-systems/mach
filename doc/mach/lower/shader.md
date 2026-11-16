[mach](../README.md) / [lower](README.md) / **shader**

# mach.lower.shader

a shader declaration's decorators as the attributes of the target family that reads them

a stage, its entry name and its workgroup size attach to the function, and an interface
variable's storage class, descriptor set and binding, location, built-in and memory
qualifiers to the global, each the number spir-v gives it, read from masc's grammar by
its spir-v name. only a target whose family's schema is spir-v's takes them, so a machine
target compiles a staged function as any other

<sub>source: [`src/lower/shader.mach`](../../../src/lower/shader.mach) · imported by 2 modules</sub>

<sub>`comptime_value` is [`mach.front.comptime.value`](../front/comptime/value.md) · `lower_context` is [`mach.lower.context`](context.md) · `resolve` is [`mach.front.resolve`](../front/resolve.md) · `fail` is [`mach.base.fail`](../base/fail.md) · `ast_decl` is [`mach.front.ast.decl`](../front/ast/decl.md) · `intern` is [`mach.base.intern`](../base/intern.md)</sub>

| Type | | |
| :--- | :--- | :--- |
| [`SpecVar`](#specvar) | `rec` | a `#[gpu.spec]` var as its specialization constant: the id the host sets it by and the default its initializer gives |

| On `SpecVar` | |
| :--- | :--- |
| [`spec_var`](#spec_var) | the `#[gpu.spec]` var `sym` names on a target that states specialization constants, read in the scope it is written in, none for any other binding or target |

| Functions | |
| :--- | :--- |
| [`attaching`](#attaching) | whether the target reads shader attributes |
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
    default: comptime_value.CTValue;
}
```

a `#[gpu.spec]` var as its specialization constant: the id the host sets it by and the
default its initializer gives

### spec_var

```mach
pub fun spec_var(lc: *lower_context.LowerContext, sym: *resolve.Symbol) res[opt[SpecVar], fail.Fail]
```

the `#[gpu.spec]` var `sym` names on a target that states specialization constants, read in
the scope it is written in, none for any other binding or target

## Functions

### attaching

```mach
pub fun attaching(lc: *lower_context.LowerContext) bool
```

whether the target reads shader attributes

### initialized

```mach
pub fun initialized(lc: *lower_context.LowerContext, d: *ast_decl.Decl) bool
```

whether a global of the declaration `d` holds the initial value it is written with: the
host, the pipeline or the previous stage supplies an interface variable's

### specialized

```mach
pub fun specialized(lc: *lower_context.LowerContext, d: *ast_decl.Decl) bool
```

whether the declaration `d` is a `#[gpu.spec]` var on a target that states specialization
constants, which defines no global there: each read of it is the constant

### global_attributes

```mach
pub fun global_attributes(lc: *lower_context.LowerContext, d: *ast_decl.Decl, gix: u32) err[fail.Fail]
```

the attributes of global `gix` its declaration `d` states, read in the scope `d` is
written in. a global that already carries attributes keeps them

### extern_global_attributes

```mach
pub fun extern_global_attributes(lc: *lower_context.LowerContext, sym: *resolve.Symbol, gix: u32) err[fail.Fail]
```

the attributes of the global `gix` declared here for another module's declaration of
`sym`, read in the scope it is written in

### function_attributes

```mach
pub fun function_attributes(lc: *lower_context.LowerContext, d: *ast_decl.Decl, fix: u32, bare: intern.StrId) err[fail.Fail]
```

the entry point function `fix` is: its stage, the bare source name a pipeline finds it
by, and a compute stage's workgroup size, one in each dimension it does not state

