# Overview

Mach is a small, explicit systems language. It compiles ahead of time, has no garbage collector, infers no types and hides no control flow. These pages are short and example first.

## Start here

| If you want to | Read |
|---|---|
| Get the compiler | [Install](install.md) |
| Write and run a first program | [Hello world](hello-world.md) |
| See how a project is laid out | [Project layout](project-layout.md) |
| Try the language without installing | The [playground](https://machlang.org/playground/) |

## The language

| Page | Covers |
|---|---|
| [Types](types.md) | Integers, floats, pointers, arrays, vectors and function types |
| [Values and expressions](values.md) | `val` and `var`, literals, operators and casts |
| [Statements](statements.md) | `if` and `or`, `for`, `ret`, `brk`, `cnt` and `fin` |
| [Functions](functions.md) | Functions, generics, comptime parameters and variadic packs |
| [Records, unions and tags](records-unions.md) | `rec`, `uni`, `tag` and the `sel` guard |
| [Modules](modules.md) | `use`, `fwd`, `pub` and how files become modules |
| [Comptime](comptime.md) | The `$` channel: build facts, `$if`, `$each` and `$error` |
| [Intrinsics](intrinsics.md) | `$size_of`, reflection and the other compile-time functions |
| [Decorators](decorators.md) | Every `#[...]` and what it applies to |

## Systems work

| Page | Covers |
|---|---|
| [Foreign functions](ext-fun.md) | Calling C with `ext fun`, and linking |
| [Secrecy](secrecy.md) | `^T` and constant-time code |
| [Inline assembly](asm.md) | `asm` blocks |
| [GPU shaders](shaders.md) | Building SPIR-V modules for Vulkan |

## Tooling

| Page | Covers |
|---|---|
| [CLI](cli.md) | Every `mach` command |
| [Manifest](manifest.md) | `mach.toml` |
| [Dependencies](dependencies.md) | Declaring, pinning and updating dependencies |
| [Testing](testing.md) | `test` blocks and `mach test` |

## The full reference

These pages teach the language. The [language reference](../language/README.md) in the compiler's repository states every rule, with grammar, and is the authority where the two differ.
