# mach.lang.target.isa.spirv.emit.capability

the version, capabilities and extensions a module requires, checked against
the target's environment and written in the module's preamble

## fun select_version

```mach
pub fun select_version(e: *emitter.Emit) err[fail.Fail];
```

the environment pins the version before anything is emitted, since which forms a
module may use depends on it

## fun check_environment

```mach
pub fun check_environment(e: *emitter.Emit) err[fail.Fail];
```

## fun require

```mach
pub fun require(e: *emitter.Emit, mf: *lang_mir.MirFunction, capability: u32, requires: u64, what: str) err[fail.Fail];
```

the one path a requirement takes, whatever raised it: an instruction row, a literal's
value, or an operand's type (such as a 64-bit atomic's Int64Atomics). `what` names the
use in a refusal. the extensions must be selected and the capability's SPIR-V version
reached, and the capability is then declared, with the SPIR-V extension defining it

## fun require_made

```mach
pub fun require_made(e: *emitter.Emit, mf: *lang_mir.MirFunction, capability: u32, requires: u64, made: res[str, fail.Fail]) err[fail.Fail];
```

`require` naming the use by a made text, or the failure that refused to make it

## fun require_at_made

```mach
pub fun require_at_made(e: *emitter.Emit, loc: lang_source.Location, stages: u8, capability: u32, requires: u64, made: res[str, fail.Fail]) err[fail.Fail];
```

## fun require_at

```mach
pub fun require_at(e: *emitter.Emit, loc: lang_source.Location, stages: u8, capability: u32, requires: u64, what: str) err[fail.Fail];
```

`stages` are the stages the use is reached from, which a capability guaranteed only in
compute holds to its `graphics` extension

## fun builtin_decl_text

```mach
pub fun builtin_decl_text(e: *emitter.Emit, gi: u32) res[str, fail.Fail];
```

"the `#[builtin(\"subgroup_size\")]` variable", for a requirement the declaration raises

## fun builtin_use_text

```mach
pub fun builtin_use_text(e: *emitter.Emit, fx: u32, gi: u32) res[str, fail.Fail];
```

the same, used by the stage `fx`

## fun preamble_emit

```mach
pub fun preamble_emit(e: *emitter.Emit);
```

the capabilities in the table's order, then each SPIR-V extension one of them needs, once

