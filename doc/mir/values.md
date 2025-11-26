# Values & Operands

This document describes SSA values and operands in MIR.

## SSA Values

Values are the fundamental data carriers in MIR's SSA form. Each value represents the immutable result of a single instruction.

### Properties

- **Unique Definition** - Each value is assigned exactly once
- **Typed** - Every value has an associated type
- **Named** - Values use `%` prefix followed by an ID (e.g., `%1`, `%result`)
- **Immutable** - Once assigned, a value cannot be changed

### Value Lifetime

A value's lifetime spans from:
- **Definition** - The instruction that produces it
- **Last Use** - The final instruction that consumes it

This clear lifetime makes register allocation straightforward.

### Value Creation

Values are created by instructions:

| Instruction Type | Creates Value? |
|------------------|----------------|
| Arithmetic (`add`, `mul`, etc.) | Yes |
| Comparison (`eq`, `lt`, etc.) | Yes |
| Load (`load`) | Yes |
| Constant (`const`) | Yes |
| Store (`store`) | No (void) |
| Branch (`br`, `brcond`) | No (terminator) |
| Return (`ret`) | No (terminator) |

### Example

```mir
%1 = const 42       # %1 is a value
%2 = add %1, %1     # %2 is a value, uses %1 twice
%3 = mul %2, %1     # %3 is a value, uses %2 and %1
store %addr, %3     # no value created, uses %3
```

## Operands

Operands are the inputs to MIR instructions. They can reference:
- **SSA Values** - Results from other instructions
- **Constants** - Immediate integer values
- **Globals** - References to global variables/functions
- **Blocks** - Basic block labels for branches

### Operand Types

#### Value Operand
References another SSA value by ID:
```mir
%result = add %a, %b  # %a and %b are value operands
```

#### Immediate Operand
Inline constant value:
```mir
%result = const 42    # 42 is an immediate operand
%sum = add %x, 10     # 10 could be immediate (if supported)
```

#### Global Operand
References a global symbol:
```mir
%addr = mov @global_var  # @global_var is a global operand
%result = call @function # @function is a global operand
```

#### Block Operand
References a basic block for control flow:
```mir
br block_1               # block_1 is a block operand
brcond %c, block_2, block_3  # block_2 and block_3 are block operands
```

## Register Allocation

During code generation, SSA values are mapped to physical registers or stack slots.

### Register Classes

Values are allocated to different register classes based on type:

| Type Category | x86_64 Registers |
|---------------|------------------|
| Integers, Pointers | RAX, RBX, RCX, RDX, RSI, RDI, R8-R15 |
| Floats | XMM0-XMM15 |

### Allocation Strategy

The current implementation uses a simple strategy:
1. Function parameters use ABI-designated registers
2. Temporary values use available registers
3. Spilled values use stack slots

See [ABI](abi.md) for parameter register assignments.

### Example

```mir
function add(i64, i64) -> i64 {
  block_0:
    %1 = param 0    # Allocated to RDI (first param)
    %2 = param 1    # Allocated to RSI (second param)
    %3 = add %1, %2 # Allocated to RAX (return register)
    ret %3
}
```

## Operand Representation

Operands are represented as tagged unions in the implementation:

```c
typedef enum MIROperandKind {
    MIR_OPERAND_VALUE,   // SSA value reference
    MIR_OPERAND_IMM,     // Immediate constant
    MIR_OPERAND_GLOBAL,  // Global symbol
    MIR_OPERAND_BLOCK    // Block label
} MIROperandKind;

typedef struct MIROperand {
    MIROperandKind kind;
    union {
        uint32_t value_id;    // For VALUE
        int64_t imm_value;    // For IMM
        char *global_name;    // For GLOBAL
        uint32_t block_id;    // For BLOCK
    };
} MIROperand;
```

### Helper Functions

```c
// Create operands
MIROperand mir_operand_value(uint32_t id);
MIROperand mir_operand_imm(int64_t value);
MIROperand mir_operand_global(const char *name);
MIROperand mir_operand_block(uint32_t block_id);
```

## Implementation

Values and operands are defined in:
- `boot/include/compiler/mir/value.h`
- `boot/src/compiler/mir/value.c`
- `boot/include/compiler/mir/operand.h`
- `boot/src/compiler/mir/operand.c`

## See Also

- [Opcodes](opcodes.md) - Operations that produce/consume values
- [Functions](functions.md) - Where values are used
- [Codegen](codegen.md) - Register allocation details
