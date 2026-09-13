# mach.lang.me.ir.effect

## fun is_volatile

```mach
pub fun is_volatile(inst: *instruction.Instruction) bool;
```

## fun discardable

```mach
pub fun discardable(inst: *instruction.Instruction) bool;
```

## fun cse_safe

```mach
pub fun cse_safe(inst: *instruction.Instruction) bool;
```

## fun movable

```mach
pub fun movable(inst: *instruction.Instruction) bool;
```

## fun speculatable

```mach
pub fun speculatable(inst: *instruction.Instruction) bool;
```

## fun function_has_volatile

```mach
pub fun function_has_volatile(fn: *ir.Function) bool;
```

