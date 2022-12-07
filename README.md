MACH
===

Mach is a low level, compiled, statically typed programming langauge.

# Key Features

- No magic. No side effects. No bullshit. Code written in Mach follows the WYSIWYG principle.
- Small, clearly defined, and well documented feature set.
- Familiar and easy to read syntax. You do not need to take a class to use mach.
- Barebones standard library with enough to do the basics, but not enough to overload the language.
- Compatibility with C at the ABI level.

# Getting Started

We encourage you to not even install Mach until you have read the [language overview](doc/language/README.md). The docs are written more like a pamphlet then a bible, and assumes that you are familiar with basic programming concepts from other languages.

The reason for this is that Mach may not be for you. If the language does not include some features you hope to use, includes things you despise, or if you just don't like the syntax, then you should look elsewhere. If you read the overview (or maybe even the full documentation) and have decided that you like the language, then you will have just learned the basics and should be capable of diving in.

The [documentation](doc/README.md) includes instructions for how to [install the Mach compiler](doc/language/installation.md) on your system.

If you are new to programming in general, then Mach may not be for you. There are lots of other languages that are better suited for beginners (mostly because of the level of documentation), and we encourage you to look into those instead. Some good "first" programming languages are:
- [Python](https://www.python.org/)
- [Lua](https://www.lua.org/)
- [Javascript](https://www.javascript.com/)

# Simple Examples

## Hello World

```mach
use "std/sys"

fun main() i32 {
    sys.print("Hello, World!")

    ret 0
}
```

## Fibonacci

```mach
use "std/sys"

fun fib(n: i32) i32 {
    if n < 2 {
        ret n
    }

    ret fib(n - 1) + fib(n - 2)
}

fun main() i32 {
    var max: i32 = 10
    sys.print(fib(i))

    ret 0
}
```

# Credit

The inspiration for Mach comes from too many languages to count. Almost every modern language has problems that Mach attempts to elegantly resolve (most often by the process of reductive simplification).

Direct inspiration for the compiler, however, comes from a few more specific sources, the most notable of which are listed below:

- [Golang](https://golang.org/)
- [Vlang](https://vlang.org/)
- [Zig](https://ziglang.org/)
- [Lua](https://www.lua.org/)
- [Python](https://www.python.org/)

The original compiler would not have been written without the ability to reference the source code of these languages. 

Mach, at its core, stands on the shoulders of countles giants that have contributed to the development of these languages either directly or by proxy. It is out of respect for their work that Mach will always be fully open source. Thank you all. 

# License

Mach is released under the [Unlicense](https://unlicense.org/).

Take it. Use it. Break it. Fix it. Love it. Hate it. Improve it. It's yours.
