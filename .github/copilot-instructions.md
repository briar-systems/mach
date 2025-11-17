# Style
- Maintain existing coding style and conventions.
- Keep comments lowercase and concise.
- Avoid overcommenting trivial code or changes. Focus comments on complex logic.
- If the bootstrap compiler is modified, once all changes are complete, format the C code using `clang-format` if available on the host system.

# Quality
- Reference language documentation in `doc` for proper usage of language features.
- Maintain clarity and simplicity in all code changes.
- Avoid introducing compatibility shims. Prefer full implementations.
- Fully remove any dead code or lost features if affected by a change. This includes replaced functionality.

# Documentation
- If a change requires documentation, update any existing documentation files in `doc` that pertain to the change.
- Maintain parity with the existing documentation style.

# Mach language cheatsheet

## Comments
`#` for single-line comments only (no block comments). Extends to end of line.

## Keywords and declarations
- `use [alias:] project.module;` - import module, one file = one module
- `pub` - export symbol from module
- `ext "ABI:link_name" binding: type;` - external FFI symbol
- `def Alias: BaseType;` - type alias
- `val name: type = expr;` - immutable binding
- `var name: type = expr;` - mutable binding, can omit initializer
- `fun name(param: Type) RetType { body }` - function
- `fun(this: *T) method() { body }` - method with receiver (pointer)
- `rec Name { field: Type; }` - record type (struct)
- `uni Name[T] { variant: Type; }` - union type (tagged union), supports generics
- `ret expr;` - return from function
- `asm { ... }` - inline assembly

## Types
Primitives: `u8` `u16` `u32` `u64` `i8` `i16` `i32` `i64` `f32` `f64` `ptr` (untyped pointer)
Compounds: `*T` (pointer) `[N]T` (fixed array) `[]T` (slice)
Anonymous: `rec{x: i32;}` and `uni{some: T; none: ptr;}` valid in type position
Generics: `Name[TypeParam]` for both records and unions

## Operators and expressions
- `obj.field` - field access (records) or namespace access (modules). Uses `.` for both values and pointers
- `arr[idx]` - indexing arrays, slices, pointers
- `func(a, b)` - function call
- `obj.method()` - method calls auto-convert between value/pointer as needed for receiver type
- `func[i32]()` - generic call with explicit type arguments
- `Type{field: value}` or `Generic[T]{field: value}` - literal construction
- `value::TargetType` - cast (sizes must match, bit reinterpret (`i64` to `f64` bad etc.))
- `?expr` - address-of (operand must be lvalue, returns pointer)
- `@ptr` - dereference pointer
- Standard arithmetic: `+` `-` `*` `/` `%`
- Comparison: `==` `!=` `<` `>` `<=` `>=`
- Logical: `&&` `||` `!`
- Bitwise: `&` `|` `^` `~` `<<` `>>`

## Control flow
```mach
if (condition) { body }
or (condition) { body }
or { fallback }

for (condition) { body }  // conditional loop
for { body }              // infinite loop
cnt;                      // continue
brk;                      // break
```

## Type system semantics
- Literal coercion only at declaration point. Explicit casts required elsewhere.
- Aliases with identical underlying types are interchangeable without casts.
- All sizes in casts must match exactly (no implicit widening/narrowing).

## Module system
- Modules map 1:1 with `.mach` files under `[project].src` in `mach.toml`
- Module path follows directory structure: `src/driver/pipeline.mach` → `project.driver.pipeline`
- Dependencies declared in `mach.toml` under `[deps.name]` with `source` and `version`

## Entry point convention
Standard library runtime expects: `fun main(args: []str) i64` with `$main.symbol = "main";`
Comptime directives: `$directive.key = value;` evaluated at compile time.
