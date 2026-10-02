# mach.lang.be.codegen.structure

## val NO_BLOCK

```mach
pub val NO_BLOCK: u32 = 0xFFFFFFFF
```

## val CK_PLAIN

```mach
pub val CK_PLAIN:     u8 = 0
```

the construct a node heads: a two-way selection, a loop, or a switch, which
branches to one of any number of successors and merges like a selection

## val CK_SELECTION

```mach
pub val CK_SELECTION: u8 = 1
```

## val CK_LOOP

```mach
pub val CK_LOOP:      u8 = 2
```

## val CK_SWITCH

```mach
pub val CK_SWITCH:    u8 = 3
```

## val ROLE_REAL

```mach
pub val ROLE_REAL:        u8 = 0
```

what a node executes. a real node is its MIR block, whose terminator's block
operands are its successors in operand order. a split node carries the
terminator of the loop header it was split from, below the header's merge.
continue and forward nodes branch to their one successor. an exit node stores
its `case` into the selector of its `dispatch` node and branches to it, and a
dispatch node branches to the successor its stored case names. nothing reaches
an unreachable node

## val ROLE_CONTINUE

```mach
pub val ROLE_CONTINUE:    u8 = 1
```

## val ROLE_UNREACHABLE

```mach
pub val ROLE_UNREACHABLE: u8 = 2
```

## val ROLE_SPLIT

```mach
pub val ROLE_SPLIT:       u8 = 3
```

## val ROLE_FORWARD

```mach
pub val ROLE_FORWARD:     u8 = 4
```

## val ROLE_EXIT

```mach
pub val ROLE_EXIT:        u8 = 5
```

## val ROLE_DISPATCH

```mach
pub val ROLE_DISPATCH:    u8 = 6
```

## val CFG_OK

```mach
pub val CFG_OK:            u8 = 0
```

## val CFG_IRREDUCIBLE

```mach
pub val CFG_IRREDUCIBLE:   u8 = 1
```

## val CFG_UNREACHABLE

```mach
pub val CFG_UNREACHABLE:   u8 = 2
```

## val CFG_MULTIWAY

```mach
pub val CFG_MULTIWAY:      u8 = 3
```

## val CFG_CROSSING_EDGE

```mach
pub val CFG_CROSSING_EDGE: u8 = 6
```

## val CFG_NO_TERMINATOR

```mach
pub val CFG_NO_TERMINATOR: u8 = 7
```

## rec Node

```mach
pub rec Node;
```

## rec Structure

```mach
pub rec Structure;
```

## fun succ

```mach
pub fun succ(st: *Structure, x: u32, k: u32) u32;
```

successor k of node x, for k below its nsucc

## fun analyze

```mach
pub fun analyze(mf: *codegen_mir.MirFunction, alloc: *std_allocator.Allocator) res[Structure, fail.Fail];
```

## fun dnit

```mach
pub fun dnit(st: *Structure);
```

## fun analyze_module

```mach
pub fun analyze_module(m: *codegen_mir.MirModule, alloc: *std_allocator.Allocator) res[*Structure, fail.Fail];
```

the structure of every function of a MIR module, in function order. a
function the target cannot structure carries its refusal in its status, which
the emitter reports where it emits that function

## fun dnit_module

```mach
pub fun dnit_module(shapes: *Structure, n: u32, alloc: *std_allocator.Allocator);
```

releases the first n structures of an analyze_module result and the array

## fun refusal_text

```mach
pub fun refusal_text(alloc: *std_allocator.Allocator, name: str, status: u8, target: str) res[str, std_format.FormatError];
```

the refusal a function with this status is rejected by, prefixed with its name

## fun dominates

```mach
pub fun dominates(st: *Structure, a: u32, b: u32) bool;
```

