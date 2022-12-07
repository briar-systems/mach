# Mach Keywords

### Decleration
```
use     # import a module
fun     # function decleration
var     # variable decleration (mut)
val     # value decleration (const)
sig     # signature decleration (type)
```

### Flow Control
```
if      # if statement
or      # or statement (serves as both else and else if)
for     # conditional loop (serves as both for and while)
brk     # break statement
cnt     # continue statement
def     # defer statement (NOT "define")
ret     # return statement
```

### Builtin Type Keywords

```
nil     # null value
true    # boolean true
false   # boolean false
str     # string type (alias for [_]u8)

i8      # signed 8 bit integer
i16     # signed 16 bit integer
i32     # signed 32 bit integer
i64     # signed 64 bit integer

u1      # unsigned 1 bit integer
u8      # unsigned 8 bit integer
u16     # unsigned 16 bit integer
u32     # unsigned 32 bit integer
u64     # unsigned 64 bit integer

f32     # 32 bit floating point number
f64     # 64 bit floating point number
```

## Detailed Keyword Descriptions

Mach's keywords were specifically chosen to minimize both the number of reserved keywords as well as the number of characters per key word. This is to make the language as easy to read as possible. Mach has under 20 non-type keywords, and nearly all of them are 3 or fewer characters long, with the preference being exactly 3 characters for symmetry.

### Decleration Keyword Usage


### `use`

Imports a module by relative path

```
use "std"
```

### `fun`

Declares a function

```
fun main(argc: i32, argv: [_]str) i32 { ... }
```

### `var`

Declares a mutable variable

```
var x: i32 = 0
```

### `val`

Declares an immutable variable

```
val x: i32 = 0
```

### `sig`

Declares a type signature

```
sig vec2 {
    x: i32
    y: i32
}
```

### Flow Control Keyword Usage

### `if`

Declares an if statement

```
if x == 0 { ... }
```

### `or`

Declares an else or else if statement

```
if x == 0 { ... }
or x == 1 { ... }
or        { ... }
```

### `for`

Declares a conditional loop (serves as both for and while)

```
for { ... }                     # no condition (runs forever)
for i < 10 { ... }              # condition (runs until false)
for i < 10; i = i + 1 { ... }   # condition and per-iteration statement
```

### `brk`

Breaks out of a loop

```
for i < 10 {
    if i == 5 { brk }   # skip all iterations after 5
}
```

### `cnt`

Continues to the next iteration of a loop

```
for i < 10 {
    if i == 5 { cnt }   # skip the iteration where i == 5
}
```

### `def`

Declares a statement that is to be run at the end of the current scope. This is useful for things like closing files or freeing memory and is inspired by the `defer` keyword in Go.

```
def { ... }
```

### `ret`

Returns a value from a function.

```
ret 0
```
