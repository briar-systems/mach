# mach.lang.target.isa.riscv.register

## fun select_features

```mach
pub fun select_features(model: *target_model.Machine, bits: u64);
```

narrows the registered template to one selection: the float and multiply facts,
the register classes and the cross-bank widths follow the selected extensions

## val MODEL64

```mach
pub val MODEL64: target_model.Machine = target_model.Machine;
```

## val MODEL32

```mach
pub val MODEL32: target_model.Machine = target_model.Machine;
```

## val MACHINE64

```mach
pub val MACHINE64: isa.RegMachine = isa.RegMachine;
```

## val MACHINE32

```mach
pub val MACHINE32: isa.RegMachine = isa.RegMachine;
```

## val RELOC

```mach
pub val RELOC: target_of.RelocationCapabilities = target_of.RelocationCapabilities;
```

## val VTABLE64

```mach
pub val VTABLE64: isa.IsaVTable = isa.IsaVTable;
```

## val VTABLE32

```mach
pub val VTABLE32: isa.IsaVTable = isa.IsaVTable;
```

