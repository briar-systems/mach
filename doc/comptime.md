# Compile-Time Features

Mach's compile-time system provides type introspection, conditional compilation, and symbol attributes through the `$` prefix.


## Type Introspection

| Intrinsic | Description |
|-----------|-------------|
| `$size_of(Type)` | Size in bytes of the type |
| `$align_of(Type)` | Alignment in bytes of the type |
| `$offset_of(Type, field)` | Byte offset of a field within a record |
| `$type_of(expr)` | Type of an expression |

```mach
rec Foo {
    a: u32;
    b: u64;
}

val size:   u64 = $size_of(Foo);        # 16
val align:  u64 = $align_of(Foo);       # 8
val offset: u64 = $offset_of(Foo, b);   # 8
```


## Conditional Compilation

`$if` / `or` selectively includes code based on compile-time conditions. Code that does not meet the condition is completely omitted from the binary.

```mach
$if ($mach.build.target.os.id == $mach.os.linux.id) {
    # linux-specific code
} or ($mach.build.target.os.id == $mach.os.darwin.id) {
    # macOS-specific code
} or {
    # fallback
}
```

All compile-time identifiers must use the `$` prefix, even inside `$if` blocks.

Conditionally excluded `use` statements are not processed at all -- the imported module is not parsed or analyzed.


## Compiler-Provided Symbols

| Symbol | Description |
|--------|-------------|
| `$mach.compiler.version` | Compiler version string |
| `$mach.compiler.name` | Compiler name (`"mach"`) |
| `$mach.build.target.os` | Target OS name |
| `$mach.build.target.os.id` | Target OS numeric ID |
| `$mach.build.target.arch` | Target architecture name |
| `$mach.build.target.arch.id` | Target architecture numeric ID |
| `$mach.build.target.pointer_width` | Pointer width in bytes |
| `$mach.os.<name>.id` | Numeric ID for an OS (e.g., `$mach.os.linux.id`) |
| `$mach.arch.<name>.id` | Numeric ID for an architecture (e.g., `$mach.arch.x86_64.id`) |


## Symbol Attributes

The `$symbol.attribute = value` syntax sets attributes on the following declaration.

Currently the only supported attribute is `symbol`, which overrides the linker symbol name:

```mach
$main.symbol = "main";
fun main(argc: i64, argv: &&u8) i64 {
    ret 0;
}
```

This removes name mangling from the function, giving it the exact linker symbol `main`. This is required for the runtime entry point.
