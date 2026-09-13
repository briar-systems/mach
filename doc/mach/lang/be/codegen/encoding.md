# mach.lang.be.codegen.encoding

## rec EncoderOutput

```mach
pub rec EncoderOutput;
```

## def LineRow

```mach
pub def LineRow:  debug_input.LineRow
```

## def InlinePc

```mach
pub def InlinePc: debug_input.InlinePc
```

## def VarLoc

```mach
pub def VarLoc:   debug_input.VarLoc
```

## val VARLOC_NONE

```mach
pub val VARLOC_NONE:  u8 = 0
```

## val VARLOC_REG

```mach
pub val VARLOC_REG:   u8 = 1
```

## val VARLOC_FRAME

```mach
pub val VARLOC_FRAME: u8 = 2
```

## val VARLOC_CMP

```mach
pub val VARLOC_CMP:   u8 = 3
```

## val VARLOC_IMM

```mach
pub val VARLOC_IMM:   u8 = 4
```

## val CMPREL_EQ

```mach
pub val CMPREL_EQ: u8 = 0
```

## val CMPREL_NE

```mach
pub val CMPREL_NE: u8 = 1
```

## val CMPREL_LT

```mach
pub val CMPREL_LT: u8 = 2
```

## val CMPREL_LE

```mach
pub val CMPREL_LE: u8 = 3
```

## rec PendingReloc

```mach
pub rec PendingReloc;
```

## rec SymbolMark

```mach
pub rec SymbolMark;
```

## val CONST_F32

```mach
pub val CONST_F32: u8 = 1
```

## val CONST_F64

```mach
pub val CONST_F64: u8 = 2
```

## val CONST_VEC

```mach
pub val CONST_VEC: u8 = 3
```

## rec ConstEntry

```mach
pub rec ConstEntry;
```

