# Mach Language Documentation

## Getting Started

- [Getting Started](getting-started.md) -- prerequisites, building, creating a project

## Language Reference

- [Types](types.md) -- primitives, records, unions, arrays, pointers, generics
- [Declarations](declarations.md) -- functions, records, unions, variables, type aliases, external bindings
- [Expressions](expressions.md) -- operators, literals, casts, field access, indexing
- [Control Flow](control-flow.md) -- if/or, for loops, ret/brk/cnt, fin (defer)
- [Modules](modules.md) -- imports, module resolution, project structure
- [Memory](memory.md) -- pointers, address-of, dereference, mutability
- [Generics](generics.md) -- generic types and functions, monomorphization
- [Compile-Time](comptime.md) -- $if, $size_of, symbol attributes, compiler constants
- [Variadic Functions](variadic.md) -- va_list, va_start, va_arg, va_end
- [Testing](testing.md) -- test blocks, test runner, assertions
- [Inline Assembly](asm.md) -- asm blocks

## Tooling

- [Project Configuration](config.md) -- mach.toml reference
- [Dependencies](dependencies.md) -- adding, removing, managing dependencies
- [Cheatsheet](cheatsheet.md) -- quick reference card

## Compiler Internals

- [MASM Backend](masm/README.md) -- intermediate representation and code generation

## Proposals

- [Type Conversions](proposals/type_conversions.md) -- implicit widening proposal
