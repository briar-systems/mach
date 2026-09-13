# mach.lang.target.testing

## fun debug_descriptor

```mach
pub fun debug_descriptor() res[of.DebugVTable, fail.Fail];
```

## fun carriers_convention

```mach
pub fun carriers_convention() abi.AbiVTable;
```

a register-machine scaffold convention: carriers, no banks, no hooks; only for tests that never classify

