# mach.lang.target.isa.riscv.register

## fun select_features

```mach
pub fun select_features(model: *isa.MachineModel, bits: u32);
```

narrows the registered template to one selection: the float and multiply facts,
the register classes and the cross-bank widths follow the selected extensions

## fun register_riscv64

```mach
pub fun register_riscv64(reg: *isa.IsaRegistry) err[fail.Fail];
```

## fun register_riscv32

```mach
pub fun register_riscv32(reg: *isa.IsaRegistry) err[fail.Fail];
```

