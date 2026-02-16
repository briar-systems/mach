# Variadic Functions

Mach supports variadic functions following the SysV x86_64 ABI. Variadic functions accept a variable number of arguments after their named parameters.


## Declaration

The `...` marker as the last parameter makes a function variadic:

```mach
fun sum(count: i64, ...) i64 {
    var ap: va_list;
    va_start(?ap);

    var total: i64 = 0;
    var i: i64 = 0;
    for (i < count) {
        total = total + va_arg[i64](?ap);
        i = i + 1;
    }

    va_end(?ap);
    ret total;
}
```

Named parameters before `...` are accessed normally. The variadic arguments are accessed through the `va_list` type and its associated operations.


## va_list

`va_list` is a built-in type (24 bytes, 8-byte aligned) that tracks the current position in the variadic argument list. It follows the SysV x86_64 ABI layout internally.

Declare a `va_list` as a local variable:

```mach
var ap: va_list;
```


## va_start

Initializes a `va_list` to point at the first variadic argument. Takes a pointer to the `va_list`:

```mach
va_start(?ap);
```

Must be called before any `va_arg` calls. Only valid inside a variadic function.


## va_arg

Extracts the next argument from the `va_list`. The type is specified as a generic parameter:

```mach
val n: i64 = va_arg[i64](?ap);
val s: str = va_arg[str](?ap);
val p: *u8 = va_arg[*u8](?ap);
```

Each call advances the `va_list` to the next argument. The caller is responsible for passing the correct type -- there is no runtime type checking.


## va_end

Marks the `va_list` as no longer in use. Takes a pointer to the `va_list`:

```mach
va_end(?ap);
```

On SysV x86_64 this is a no-op, but should be called for correctness.


## Printf

The standard library provides formatted output using variadic functions:

```mach
use std.runtime;
use print: std.print;

$main.symbol = "main";
fun main(argc: i64, argv: &&u8) i64 {
    print.printf("answer: %d\n", 42::i64);
    print.printlnf("%s = %x", "value", 255::u64);
    ret 0;
}
```

Format specifiers:

| Specifier | Type | Description |
|-----------|------|-------------|
| `%d` | `i64` | Signed decimal |
| `%u` | `u64` | Unsigned decimal |
| `%x` | `u64` | Hex lowercase |
| `%X` | `u64` | Hex uppercase |
| `%s` | `str` | String |
| `%c` | `u8` | Character |
| `%p` | `ptr` | Pointer |
| `%%` | -- | Literal `%` |


## Differences from C

| C | Mach |
|---|------|
| `va_list ap;` | `var ap: va_list;` |
| `va_start(ap, last_named);` | `va_start(?ap);` |
| `va_arg(ap, int)` | `va_arg[i64](?ap)` |
| `va_end(ap);` | `va_end(?ap);` |

Mach uses `?` (address-of) to pass the `va_list` by pointer. The type argument to `va_arg` uses square brackets (generic syntax) instead of C's macro-based parentheses.
