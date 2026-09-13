# mach.lang.target.of.spv

## rec SpvSummary

```mach
pub rec SpvSummary;
```

## fun validate

```mach
pub fun validate(buf: *u8, len: usize) res[SpvSummary, fail.Fail];
```

## fun register

```mach
pub fun register(reg: *of.OfRegistry) err[fail.Fail];
```

