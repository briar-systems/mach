# mach.lang.target.abi.riscv

## val LP64

```mach
pub val LP64: abi.AbiVTable = abi.AbiVTable;
```

the psABI passes a `_Float16` as the float it is: in an f register when one
is free and the ABI has any, NaN-boxed to FLEN, else by the integer rule.
that holds with or without Zfh

## val LP64F

```mach
pub val LP64F: abi.AbiVTable = abi.AbiVTable;
```

## val LP64D

```mach
pub val LP64D: abi.AbiVTable = abi.AbiVTable;
```

## val ILP32

```mach
pub val ILP32: abi.AbiVTable = abi.AbiVTable;
```

## val ILP32F

```mach
pub val ILP32F: abi.AbiVTable = abi.AbiVTable;
```

## val ILP32D

```mach
pub val ILP32D: abi.AbiVTable = abi.AbiVTable;
```

