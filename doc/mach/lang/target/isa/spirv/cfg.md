# mach.lang.target.isa.spirv.cfg

## val NO_BLOCK

```mach
pub val NO_BLOCK: u32 = 0xFFFFFFFF
```

## val CK_PLAIN

```mach
pub val CK_PLAIN:     u8 = 0
```

## val CK_SELECTION

```mach
pub val CK_SELECTION: u8 = 1
```

## val CK_LOOP

```mach
pub val CK_LOOP:      u8 = 2
```

## val ROLE_REAL

```mach
pub val ROLE_REAL:        u8 = 0
```

## val ROLE_CONTINUE

```mach
pub val ROLE_CONTINUE:    u8 = 1
```

## val ROLE_UNREACHABLE

```mach
pub val ROLE_UNREACHABLE: u8 = 2
```

## val ROLE_FORWARD

```mach
pub val ROLE_FORWARD:     u8 = 4
```

## val ROLE_SPLIT

```mach
pub val ROLE_SPLIT:       u8 = 3
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

## val CFG_LOOP_MERGE

```mach
pub val CFG_LOOP_MERGE:    u8 = 4
```

## val CFG_MERGE_CLAIMED

```mach
pub val CFG_MERGE_CLAIMED: u8 = 5
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

## fun analyze

```mach
pub fun analyze(mf: *mir.MirFunction, alloc: *A.Allocator) res[Structure, fail.Fail];
```

## fun dnit

```mach
pub fun dnit(st: *Structure);
```

## fun status_message

```mach
pub fun status_message(status: u8) str;
```

## fun dominates

```mach
pub fun dominates(st: *Structure, a: u32, b: u32) bool;
```

