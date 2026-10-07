# mach.lang.target.isa.spirv.emit.emitter

the state one module's emission threads through every phase, the refusals
it reports, and the word writers every phase uses

## rec Emit

```mach
pub rec Emit;
```

## rec VMaps

```mach
pub rec VMaps;
```

## rec FnEnt

```mach
pub rec FnEnt;
```

## rec GlobalEnt

```mach
pub rec GlobalEnt;
```

## rec Copy

```mach
pub rec Copy;
```

one emitted OpFunction of a source function. logical addressing admits only a
memory object declaration as a call's pointer argument, and no StorageBuffer or
other block pointer at all, so a pointer parameter bound to a subobject or to an
interface variable is split into the root it points into and the path from there:
the copy names a global root itself, takes a Function-storage root whole, and
takes the path's runtime indices as integers. its key is one binding per source
parameter, spelled in words:
  BIND_VALUE                                  not a pointer parameter
  BIND_WHOLE                                  a whole Function-storage object
  BIND_GLOBAL, global, n, step * n            a path into a global, named in the copy
  BIND_LOCAL, module, ir type, n, step * n    a path into a Function-storage root of
                                              that type, passed first
  BIND_PHYSICAL                               a physical pointer, passed as the value it is
where a step is a member ordinal, which OpAccessChain takes only as a constant, or
STEP_RUNTIME for an array element, whose index (constant or not) is passed after the root

## val BIND_VALUE

```mach
pub val BIND_VALUE: u32 = 0
```

## val BIND_WHOLE

```mach
pub val BIND_WHOLE: u32 = 1
```

## val BIND_GLOBAL

```mach
pub val BIND_GLOBAL: u32 = 2
```

## val BIND_LOCAL

```mach
pub val BIND_LOCAL: u32 = 3
```

## val BIND_PHYSICAL

```mach
pub val BIND_PHYSICAL: u32 = 4
```

## val STEP_RUNTIME

```mach
pub val STEP_RUNTIME: u32 = 0xFFFFFFFF
```

## val COPY_NONE

```mach
pub val COPY_NONE: u32 = 0xFFFFFFFF
```

## rec PhysKey

```mach
pub rec PhysKey;
```

an explicit-layout type built for physical memory: a struct of `n` members at their
offsets, or an array of `n` elements of `ids[0]` at the stride `offs[0]`

## rec PhysRec

```mach
pub rec PhysRec;
```

a record a cycle of pointers reaches, by its key: the pointer to it, reserved before the
record is declared so a member of its cycle can name it, and its explicit-layout struct

## val PROV_LOGICAL

```mach
pub val PROV_LOGICAL: u8 = 1
```

the provenance of a pointer register: a logical pointer names memory the module declares,
a physical one an address in a buffer the host passes

## val PROV_PHYSICAL

```mach
pub val PROV_PHYSICAL: u8 = 2
```

## val ROLE_PHYSICAL

```mach
pub val ROLE_PHYSICAL: u8 = 0xFE
```

the role whose layout rules physical memory follows, beside the interface roles

## val ROOT_GLOBAL

```mach
pub val ROOT_GLOBAL: u8 = 1
```

## val ROOT_ALLOCA

```mach
pub val ROOT_ALLOCA: u8 = 2
```

## val ROOT_PARAM

```mach
pub val ROOT_PARAM: u8 = 3
```

## rec AddrStep

```mach
pub rec AddrStep;
```

one step of an address: a member ordinal, or an element index that a chain of this
function read (its result vreg and the index's position) or that this copy receives
(the parameter and the index's slot among that parameter's runtime indices)

## rec Addr

```mach
pub rec Addr;
```

where a pointer argument points: its root and the steps from there

## fun dnit

```mach
pub fun dnit(e: *Emit);
```

## fun build_fn_table

```mach
pub fun build_fn_table(e: *Emit) err[fail.Fail];
```

## fun mark_emitted

```mach
pub fun mark_emitted(e: *Emit) err[fail.Fail];
```

## fun fn_is_seed

```mach
pub fun fn_is_seed(e: *Emit, ix: u32) bool;
```

a function the module holds for its own sake: a stage, or with no stage any
function of the root module, exported for a linker

## fun emittable

```mach
pub fun emittable(e: *Emit, ix: u32) bool;
```

## fun callee_count

```mach
pub fun callee_count(e: *Emit, ix: u32) u32;
```

## fun callee_at

```mach
pub fun callee_at(e: *Emit, ix: u32, k: u32) u32;
```

## fun fn_lookup

```mach
pub fun fn_lookup(e: *Emit, name: intern.StrId) u32;
```

## fun fn_ir

```mach
pub fun fn_ir(e: *Emit, ix: u32) *me_ir.Function;
```

## fun fn_mod

```mach
pub fun fn_mod(e: *Emit, ix: u32) *me_ir.Module;
```

## fun fn_mir

```mach
pub fun fn_mir(e: *Emit, ix: u32) *lang_mir.MirFunction;
```

## fun ir_fn_spelling

```mach
pub fun ir_fn_spelling(e: *Emit, ix: u32) res[str, fail.Fail];
```

a function's name as its author wrote it, qualified by its module outside the root

## fun words_emit

```mach
pub fun words_emit(e: *Emit, opcode: u32, w0: u32, w1: u32, w2: u32, w3: u32, n: u32);
```

## fun label_emit

```mach
pub fun label_emit(e: *Emit, label: u32);
```

## fun pair_emit

```mach
pub fun pair_emit(e: *Emit, opcode: u32, ty: u32, a: u32, b: u32) u32;
```

## val FN_NONE

```mach
pub val FN_NONE: u32 = mir_unit.UNIT_NONE
```

## fun refuse

```mach
pub fun refuse(e: *Emit, k: diagnostic_kind.Kind, msg: str) err[fail.Fail];
```

the program asks for what this backend cannot express: an error located at
the instruction under emission (else its function) on the module's store,
naming the function

## fun unsupported

```mach
pub fun unsupported(e: *Emit, msg: str) err[fail.Fail];
```

## fun reject_at

```mach
pub fun reject_at(e: *Emit, k: diagnostic_kind.Kind, loc: lang_source.Location, msg: str) fail.Fail;
```

## fun reject_made_at

```mach
pub fun reject_made_at(e: *Emit, k: diagnostic_kind.Kind, loc: lang_source.Location, made: res[str, fail.Fail]) fail.Fail;
```

the rejection carrying a made text, or the failure that refused to make it

## fun unsupported_made

```mach
pub fun unsupported_made(e: *Emit, made: res[str, fail.Fail]) err[fail.Fail];
```

## fun reject_module

```mach
pub fun reject_module(e: *Emit, k: diagnostic_kind.Kind, msg: str) fail.Fail;
```

a refusal of the module or its interface, which no one instruction locates

## fun read_failure

```mach
pub fun read_failure(e: *Emit, f: fail.Fail) err[fail.Fail];
```

an operand read's failure at the instruction that read it: a rejection is
already on the store and passes through, a defect is named with its place

## fun defect

```mach
pub fun defect(e: *Emit, msg: str) err[fail.Fail];
```

a compiler defect met while emitting, named with its instruction and place

## fun refusal_fn_name

```mach
pub fun refusal_fn_name(e: *Emit) res[str, fail.Fail];
```

the function under emission as a refusal names it; module scope when none is

