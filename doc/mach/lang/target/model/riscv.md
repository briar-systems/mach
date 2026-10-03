# mach.lang.target.model.riscv

## val GPR_COUNT

```mach
pub val GPR_COUNT: i32 = 32
```

## val FPR_COUNT

```mach
pub val FPR_COUNT: i32 = 30
```

## val HAS_FLOAT

```mach
pub val HAS_FLOAT: bool = true
```

## val MACHINE64

```mach
pub val MACHINE64: target_model.Machine = target_model.Machine;
```

## val MACHINE32

```mach
pub val MACHINE32: target_model.Machine = target_model.Machine;
```

## fun features_select

```mach
pub fun features_select(model: *target_model.Machine, bits: u64);
```

narrows the registered template to one selection: the float and multiply facts,
the register classes and the cross-bank widths follow the selected extensions

