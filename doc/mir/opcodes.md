# MIR Opcodes Reference

This document provides a complete reference for all MIR operations.

## Opcode Categories

- [Constants](#constants)
- [Arithmetic](#arithmetic)
- [Bitwise](#bitwise)
- [Comparison](#comparison)
- [Memory](#memory)
- [Control Flow](#control-flow)
- [Type Conversions](#type-conversions)
- [Function Calls](#function-calls)

## Constants

### `const`

Creates a constant integer value.

**Syntax**: `%result = const <value>`

**Operands**:
- Immediate integer value

**Result**: Integer constant

**Example**:
```mir
%1 = const 42      # i64 constant
%2 = const 0       # i64 constant
```

---

## Arithmetic

### `add`

Adds two values.

**Syntax**: `%result = add %a, %b`

**Operands**:
- Two integer or floating-point values

**Result**: Sum of operands (same type as inputs)

**Example**:
```mir
%sum = add %a, %b      # integer addition
%fsum = add %x, %y     # floating-point addition (ADDSD)
```

### `sub`

Subtracts the second value from the first.

**Syntax**: `%result = sub %a, %b`

**Result**: Difference (`%a - %b`)

**Example**:
```mir
%diff = sub %a, %b     # %a - %b
```

### `mul`

Multiplies two values.

**Syntax**: `%result = mul %a, %b`

**Result**: Product of operands

**Example**:
```mir
%prod = mul %a, %b
```

### `div`

Divides the first value by the second.

**Syntax**: `%result = div %a, %b`

**Result**: Quotient (`%a / %b`)

**Note**: Integer division is signed. For floating-point, uses DIVSD instruction.

**Example**:
```mir
%quot = div %a, %b
```

---

## Bitwise

### `and`

Bitwise AND of two integer values.

**Syntax**: `%result = and %a, %b`

**Result**: Bitwise AND

### `or`

Bitwise OR of two integer values.

**Syntax**: `%result = or %a, %b`

**Result**: Bitwise OR

### `xor`

Bitwise XOR of two integer values.

**Syntax**: `%result = xor %a, %b`

**Result**: Bitwise XOR

---

## Comparison

All comparison operations return an integer value (0 for false, non-zero for true).

### `eq` - Equal

**Syntax**: `%result = eq %a, %b`

**Result**: 1 if `%a == %b`, 0 otherwise

### `ne` - Not Equal

**Syntax**: `%result = ne %a, %b`

**Result**: 1 if `%a != %b`, 0 otherwise

### `lt` - Less Than (Signed)

**Syntax**: `%result = lt %a, %b`

**Result**: 1 if `%a < %b`, 0 otherwise

### `le` - Less or Equal (Signed)

**Syntax**: `%result = le %a, %b`

**Result**: 1 if `%a <= %b`, 0 otherwise

### `gt` - Greater Than (Signed)

**Syntax**: `%result = gt %a, %b`

**Result**: 1 if `%a > %b`, 0 otherwise

### `ge` - Greater or Equal (Signed)

**Syntax**: `%result = ge %a, %b`

**Result**: 1 if `%a >= %b`, 0 otherwise

### `ult`, `ule`, `ugt`, `uge` - Unsigned Comparisons

Same as above but for unsigned comparison.

---

## Memory

### `load`

Loads a value from memory.

**Syntax**: `%result = load %ptr`

**Operands**:
- Pointer to memory location

**Result**: Value loaded from memory

**Type**: Must specify type on instruction for correct operation (integer vs. float)

**Example**:
```mir
%val = load %ptr       # load i64 from address
%fval = load %fptr     # load f64 from address (MOVSD)
```

### `store`

Stores a value to memory.

**Syntax**: `store %ptr, %value`

**Operands**:
- Pointer to memory location
- Value to store

**Result**: None (void operation)

**Example**:
```mir
store %ptr, %val      # store i64 to address
store %fptr, %fval    # store f64 to address (MOVSD)
```

### `mov`

Moves or copies a value. Used for:
- Register-to-register moves
- Loading global addresses
- Bitcast operations

**Syntax**: `%result = mov %source`

**Operands**:
- Value or global reference

**Example**:
```mir
%copy = mov %original
%addr = mov @global_var  # load address of global
```

---

## Control Flow

### `br` - Unconditional Branch

Jumps to a basic block.

**Syntax**: `br block_<n>`

**Operands**:
- Target block label

**Result**: None (terminator)

**Example**:
```mir
br block_1  # jump to block_1
```

### `brcond` - Conditional Branch

Branches based on a condition value.

**Syntax**: `brcond %cond, block_true, block_false`

**Operands**:
- Condition value (non-zero = true)
- True target block
- False target block

**Result**: None (terminator)

**Example**:
```mir
%cond = eq %a, %b
brcond %cond, block_then, block_else
```

### `ret` - Return

Returns from the current function.

**Syntax**: 
- `ret %value` - Return with value
- `ret` - Return void

**Operands**:
- Optional return value

**Result**: None (terminator)

**Example**:
```mir
ret %result  # return value
ret          # return void
```

---

## Type Conversions

### `zext` - Zero Extend

Zero-extends a smaller integer to a larger integer.

**Syntax**: `%result = zext %value`

**Example**:
```mir
%wide = zext %byte  # u8 -> u64
```

### `sext` - Sign Extend

Sign-extends a smaller signed integer to a larger signed integer.

**Syntax**: `%result = sext %value`

**Example**:
```mir
%wide = sext %short  # i16 -> i64
```

### `trunc` - Truncate

Truncates a larger integer to a smaller integer.

**Syntax**: `%result = trunc %value`

**Example**:
```mir
%byte = trunc %wide  # i64 -> i8
```

### `fptosi` - Float to Signed Integer

Converts floating-point to signed integer (truncating).

**Syntax**: `%result = fptosi %float`

**Codegen**: Uses CVTTSD2SI instruction on x86_64

**Note**: Reserved for standard library use. User code uses `::` bitcast.

**Example**:
```mir
%int = fptosi %float  # f64 -> i64
```

### `sitofp` - Signed Integer to Float

Converts signed integer to floating-point.

**Syntax**: `%result = sitofp %int`

**Codegen**: Uses CVTSI2SD instruction on x86_64

**Note**: Reserved for standard library use. User code uses `::` bitcast.

**Example**:
```mir
%float = sitofp %int  # i64 -> f64
```

---

## Function Calls

### `call`

Calls a function.

**Syntax**: `%result = call @function_name(%arg1, %arg2, ...)`

**Operands**:
- Function name (global reference)
- Argument values

**Result**: Return value of function (or void)

**Example**:
```mir
%sum = call @add(%a, %b)
call @print(%msg)  # void return
```

### `syscall`

Invokes a system call (Linux x86_64).

**Syntax**: `syscall(%syscall_num, %arg1, %arg2, ...)`

**Operands**:
- Syscall number
- Up to 6 arguments

**Result**: Syscall return value

**Example**:
```mir
syscall(60, 0)  # exit(0)
```

---

## Special Operations

### `cast`

Raw bit reinterpretation between types of the same size.

**Syntax**: `%result = cast %value`

**Note**: Currently implemented as `mov` in lowering. Corresponds to `::` operator in Mach.

**Example**:
```mir
%bits = cast %float  # f64 -> i64 (bit reinterpretation)
```

## See Also

- [Overview](overview.md) - MIR architecture
- [Types](types.md) - Type system details
- [Codegen](codegen.md) - How opcodes map to machine code
