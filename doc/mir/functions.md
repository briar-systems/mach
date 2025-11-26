# Functions & Basic Blocks

This document describes how functions and control flow are represented in MIR.

## Functions

A function in MIR consists of:
- **Name** - Global function identifier
- **Parameters** - Input values with types
- **Return Type** - Output type (or void)
- **Body** - Sequence of basic blocks
- **Export Status** - Whether the function is visible to the linker

### Function Structure

```mir
function <name>(<param_types>) -> <return_type> {
  block_0:
    <instructions>
    <terminator>
    
  block_1:
    <instructions>
    <terminator>
}
```

### Example

```mir
function add(i64, i64) -> i64 {
  block_0:
    %1 = param 0
    %2 = param 1
    %3 = add %1, %2
    ret %3
}
```

## Basic Blocks

A basic block is a sequence of instructions with:
- **Single entry point** - Control flow enters only at the beginning
- **Single exit point** - Block ends with a terminator instruction
- **Sequential execution** - Instructions execute in order
- **No internal branches** - Control flow changes only at block boundaries

### Block Properties

- Blocks are numbered sequentially: `block_0`, `block_1`, etc.
- First block (`block_0`) is the function entry point
- Every block must end with a terminator instruction
- A block can have multiple predecessors (incoming edges)
- A block can have at most two successors (outgoing edges)

### Terminators

All basic blocks must end with one of these instructions:

| Terminator | Description | Successors |
|------------|-------------|------------|
| `ret` | Return from function | 0 (exits function) |
| `br` | Unconditional branch | 1 |
| `brcond` | Conditional branch | 2 |

## Control Flow

### Linear Flow

Simple sequential execution:

```mir
function simple() {
  block_0:
    %1 = const 42
    %2 = add %1, %1
    ret %2
}
```

### Conditional Flow

Using `if/else` style branching:

```mir
function max(i64, i64) -> i64 {
  block_0:
    %a = param 0
    %b = param 1
    %cond = lt %a, %b      # %a < %b?
    brcond %cond, block_then, block_else
    
  block_then:
    ret %b                 # return b
    
  block_else:
    ret %a                 # return a
}
```

### Loops

Loops are represented as backedges in the control flow graph:

```mir
function count_to_n(i64) {
  block_0:
    %n = param 0
    %i = const 0
    br block_loop
    
  block_loop:
    %cond = lt %i, %n
    brcond %cond, block_body, block_end
    
  block_body:
    %next = add %i, 1
    # Note: In true SSA, we'd need a phi node here
    # Mach's lowering handles this via value mapping
    br block_loop
    
  block_end:
    ret
}
```

## Function Calls

Functions are called using the `call` opcode:

```mir
function main() {
  block_0:
    %a = const 10
    %b = const 20
    %sum = call @add(%a, %b)
    ret %sum
}
```

### Calling Convention

See [ABI](abi.md) for details on parameter passing and register usage.

## Export Status

Functions can be:
- **Exported** (`is_exported = true`) - Visible to linker, can be called externally
- **Internal** (`is_exported = false`) - Local to the module

The lowering phase sets this based on the `pub` keyword in Mach source.

## Implementation

Functions are implemented in `boot/src/compiler/mir/function.c` and `boot/include/compiler/mir/function.h`.

Key structures:
```c
typedef struct MIRFunction {
    char *name;
    Type *return_type;
    Type **param_types;
    int param_count;
    MIRBlock *first_block;
    bool is_exported;
    struct MIRFunction *next;
} MIRFunction;
```

Basic blocks are implemented in `boot/src/compiler/mir/block.c`:
```c
typedef struct MIRBlock {
    uint32_t id;
    MIRInst *first_inst;
    MIRInst *last_inst;
    struct MIRBlock *next;
} MIRBlock;
```

## See Also

- [Opcodes](opcodes.md) - Instruction reference
- [Values & Operands](values.md) - SSA values
- [ABI](abi.md) - Calling conventions
