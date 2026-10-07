# mach.lang.target.isa.spirv.emit.global

the module's global table: which IR globals the module holds and how each is reached

## fun build_gl_table

```mach
pub fun build_gl_table(e: *emitter.Emit) err[fail.Fail];
```

## val ACCESS_WRITE

```mach
pub val ACCESS_WRITE: u8 = 1
```

## fun written

```mach
pub fun written(e: *emitter.Emit, ix: u32) bool;
```

## fun ir_of

```mach
pub fun ir_of(e: *emitter.Emit, ix: u32) *me_ir.Global;
```

## fun types_of

```mach
pub fun types_of(e: *emitter.Emit, ix: u32) *ir_type.IrTypeTable;
```

## fun debug_types_of

```mach
pub fun debug_types_of(e: *emitter.Emit, ix: u32) *ir_debug.Table;
```

## fun lookup

```mach
pub fun lookup(e: *emitter.Emit, name: intern.StrId) u32;
```

## fun mod_prefix

```mach
pub fun mod_prefix(e: *emitter.Emit, ix: u32) res[str, fail.Fail];
```

", declared in `module`" for a variable outside the root module, empty in it

## fun spelling_of

```mach
pub fun spelling_of(e: *emitter.Emit, ix: u32) str;
```

## fun global_var_id

```mach
pub fun global_var_id(e: *emitter.Emit, name: intern.StrId) u32;
```

## val GL_NONE

```mach
pub val GL_NONE: u32 = mir_unit.UNIT_NONE
```

