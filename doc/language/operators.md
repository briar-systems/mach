# Mach Operators

Mach has a variety of operators that can be used to perform operations on values.


### Arithmetic Operators

Arithmetic operators are used to perform mathematical operations on numeric values.

| operator | description    |
| -------- | -------------- |
| `+`      | addition       |
| `-`      | subtraction    |
| `*`      | multiplication |
| `/`      | division       |
| `%`      | modulo         |


### Comparison Operators

Comparison operators are used to compare values.

| operator | description           |
| -------- | --------------------- |
| `==`     | equal                 |
| `!=`     | not equal             |
| `<`      | less than             |
| `<=`     | less than or equal    |
| `>`      | greater than          |
| `>=`     | greater than or equal |


### Logical Operators

Logical operators are used to perform logical operations on two values.

| operator | description |
| -------- | ----------- |
| `&&`     | logical and |
| `\|\|`   | logical or  |
| `!`      | logical not |


### Bitwise Operators

Bitwise operators are used to perform bitwise operations on integer values.

| operator | description |
| -------- | ----------- |
| `&`      | bitwise and |
| `\|`     | bitwise or  |
| `^`      | bitwise xor |
| `~`      | bitwise not |
| `<<`     | left shift  |
| `>>`     | right shift |


### Assignment Operators

Assignment operators are used to assign values to variables.
Mach only supports one.

| operator | description  |
| -------- | ------------ |
| `=`      | assign value |


### Reference Operators

Reference operators are used in conjunction with [reference types](types.md#reference-type) and [pointer types](types.md#pointer-type) to manage references and pointers.

| operator | description          |
| -------- | -------------------- |
| `?`      | reference operator   |
| `@`      | dereference operator |

> NOTE: here is an easy way to remember which operator is which:
> - `?foo` can be read as "where is `foo`"
> - `@foo` can be read as "what is at `foo`"
