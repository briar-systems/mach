# mach.lang.target.abi.sysv

## fun callee_saved

```mach
pub fun callee_saved(out: *isa.Register) i32;
```

## fun va_model

```mach
pub fun va_model() abi.VaModel;
```

## fun register

```mach
pub fun register(reg: *abi.AbiRegistry) err[fail.Fail];
```

