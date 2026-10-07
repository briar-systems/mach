# mach.lang.target.isa.spirv.emit.copy

call planning: the specialised copy of each function per pointer binding at its call sites

## fun plan_copies

```mach
pub fun plan_copies(e: *emitter.Emit) err[fail.Fail];
```

the copies this module emits: each seed's own, then every one a planned copy's
calls ask for, keyed by how each call binds the callee's pointer parameters. the
call graph is acyclic by now, so the set is finite

## fun addr_init

```mach
pub fun addr_init(e: *emitter.Emit) emitter.Addr;
```

## fun mach_callee

```mach
pub fun mach_callee(e: *emitter.Emit, fx: u32, mi: *lang_mir.MirInstr) u32;
```

the emitted function a call's body is, FN_NONE for an `op`, an import or a callee with no body here

## fun param_is_handle

```mach
pub fun param_is_handle(e: *emitter.Emit, fx: u32, k: u32) bool;
```

## fun find_copy

```mach
pub fun find_copy(e: *emitter.Emit, fx: u32, at: u32) u32;
```

## fun copy_is_primary

```mach
pub fun copy_is_primary(e: *emitter.Emit, cx: u32) bool;
```

whether every pointer parameter of the copy is a whole object, the shape its source declares

## fun binding_steps

```mach
pub fun binding_steps(e: *emitter.Emit, p: u32, out_n: *u32) u32;
```

where a binding's steps begin and how many there are

## fun binding_arity

```mach
pub fun binding_arity(e: *emitter.Emit, p: u32) u32;
```

how many SPIR-V parameters a binding takes: its root when passed, then its runtime indices

## fun call_key

```mach
pub fun call_key(e: *emitter.Emit, cx: u32, mf: *lang_mir.MirFunction, mi: *lang_mir.MirInstr, gx: u32, ad: *emitter.Addr,
prov: *u8, nv: u32) res[bool, fail.Fail];
```

appends the key a call gives its callee `gx` to `keys`; false when a pointer argument
addresses nothing one path can name, a call the emitter refuses

## fun trace_address

```mach
pub fun trace_address(e: *emitter.Emit, cx: u32, mf: *lang_mir.MirFunction, op: *lang_mir.MirOperand, ad: *emitter.Addr) res[bool, fail.Fail];
```

follows a pointer operand of copy `cx` back to its root: a global, a local
allocation or a parameter's object, through the chains and copies that
derive it; false when no single definition names it

## fun address_def

```mach
pub fun address_def(mf: *lang_mir.MirFunction, v: u32) *lang_mir.MirInstr;
```

the one instruction that defines `v`, nil when none or several do

