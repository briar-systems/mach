[mach](../README.md) / [lower](README.md) / **builder**

# mach.lower.builder

the emission layer over mirl's builder: each operation lowering asks for, made at the
cursor in the function being built. a constant, function or global value becomes a mirl
value where an instruction uses it, an aggregate travels as the address of its storage
and is loaded whole only where a call or a return passes it, and a bool is a byte, so
a comparison is widened to one and a condition is compared with zero

<sub>source: [`src/lower/builder.mach`](../../../src/lower/builder.mach) · imported by 6 modules</sub>

<sub>`module` is `mirl.ir.module` · `A` is `std.allocator` · `mbuild` is `mirl.build.builder` · `mirl_record` is `mirl.target.record` · `mirl_layout` is `mirl.target.layout` · `id` is `mirl.ir.id` · `value` is [`mach.lower.value`](value.md) · `vector` is `std.collections.vector` · `base_source` is [`mach.base.source`](../base/source.md) · `fail` is [`mach.base.fail`](../base/fail.md) · `core_state` is `masc.core.state` · `debug` is `mirl.ir.debug.record` · `mirl_global` is `mirl.ir.global` · `types` is `mirl.ir.type.table` · `native` is `mirl.ir.native`</sub>

| Type | | |
| :--- | :--- | :--- |
| [`Builder`](#builder) | `rec` |  |

| On `Builder` | |
| :--- | :--- |
| [`init`](#init) | a builder over `m`, whose types the target's layout lays out under the build's context, with its stack slots in the address space `slots` |
| [`dnit`](#dnit) · [`fault`](#fault) · [`failed`](#failed) | |
| [`done`](#done) | the first failure of the work: lowering's own, else the one mirl's builder kept |
| [`after_err_pub`](#after_err_pub) | |
| [`bind_storage`](#bind_storage) | bind a variable to the stack slot `slot` holds it in: the slot's alloca names it |
| [`state_publics`](#state_publics) | state the bytes of the stack slot `slot` that the source type of what it holds lays out as public, `ranges` as the module takes them: the public ranges of its alloca, which the walk of a constant-time function's code takes as typed frame extents, and every other byte of it as of a secret type |
| [`bind_value`](#bind_value) | bind a variable to `v` from the cursor's point on |
| [`take`](#take) · [`take_type`](#take_type) · [`take_err`](#take_err) · [`types_of`](#types_of) | |
| [`ptr_type`](#ptr_type) | a pointer into the storage a function's own slots live in, which a pointer the source names no other storage of points into |
| [`ptr_in`](#ptr_in) | a pointer into the address space `space` |
| [`slot_space`](#slot_space) | the address space a stack slot lives in, as mirl's target states it |
| [`global_space`](#global_space) | the address space global `ix` lives in: its storage class where the target's pointers name one, the default space elsewhere |
| [`classes`](#classes) | whether the target's pointers name a storage class each, so a function's pointer parameters take the classes of the storage their arguments point into |
| [`int_type`](#int_type) | |
| [`index_type`](#index_type) | the integer as wide as a pointer, which addresses are compared and stepped in |
| [`size_of`](#size_of) · [`align_of`](#align_of) · [`offset_of`](#offset_of) | |
| [`enter`](#enter) | build in function `f`, whose entry block takes `params`, each secret as `secrets` says |
| [`seal`](#seal) | end every block left open with unreachable: a block nothing branches to, such as the one after a return |
| [`leave`](#leave) | leave the function: nothing is built until the next is entered |
| [`in_function`](#in_function) · [`function`](#function) · [`entry_block`](#entry_block) · [`block_make`](#block_make) | |
| [`block_make_with`](#block_make_with) | a block whose parameters are `params`, which every edge into it passes |
| [`block_param`](#block_param) | parameter `i` of block `k`, typed `ty` |
| [`set_block`](#set_block) · [`current_block`](#current_block) · [`block_terminated`](#block_terminated) | |
| [`terminated`](#terminated) | whether the block being built already ends |
| [`set_loc`](#set_loc) · [`current_loc`](#current_loc) | |
| [`location_of`](#location_of) | the debug table's location of a source location, none when it has no file |
| [`debug_location`](#debug_location) | the debug location the next instructions are built for |
| [`set_secret`](#set_secret) | whether the next instructions are marked secret: a load, call or atomic read of a secret |
| [`set_access`](#set_access) | the next accesses through an address state the memory they read or write as the source type says: secret, which marks them so, or public |
| [`end_access`](#end_access) | the next accesses state nothing of the memory they reach |
| [`set_volatile`](#set_volatile) | whether the next memory accesses are volatile |
| [`set_kept`](#set_kept) | whether the next writes are kept: a wipe of memory that held a secret |
| [`spec_constant`](#spec_constant) | the specialization constant `spec_id`, whose default is the integer or float constant `default` |
| [`constant_of`](#constant_of) | |
| [`use`](#use) | the mirl value `v` stands for in the function being built |
| [`emit_store_marked`](#emit_store_marked) | a store marked volatile where `vol` holds: an access of a volatile place |
| [`emit_load_marked`](#emit_load_marked) | a load marked volatile where `vol` holds |
| [`emit_add`](#emit_add) · [`emit_sub`](#emit_sub) · [`emit_mul`](#emit_mul) · [`emit_div_s`](#emit_div_s) · [`emit_div_u`](#emit_div_u) · [`emit_rem_s`](#emit_rem_s) · [`emit_rem_u`](#emit_rem_u) · [`emit_and`](#emit_and) · [`emit_or`](#emit_or) · [`emit_xor`](#emit_xor) · [`emit_shl`](#emit_shl) · [`emit_shr_s`](#emit_shr_s) · [`emit_shr_u`](#emit_shr_u) · [`emit_neg`](#emit_neg) · [`emit_not`](#emit_not) · [`emit_cmp_eq`](#emit_cmp_eq) · [`emit_cmp_ne`](#emit_cmp_ne) · [`emit_cmp_lt_s`](#emit_cmp_lt_s) · [`emit_cmp_lt_u`](#emit_cmp_lt_u) · [`emit_cmp_le_s`](#emit_cmp_le_s) · [`emit_cmp_le_u`](#emit_cmp_le_u) · [`emit_cmp_gt_s`](#emit_cmp_gt_s) · [`emit_cmp_gt_u`](#emit_cmp_gt_u) · [`emit_cmp_ge_s`](#emit_cmp_ge_s) · [`emit_cmp_ge_u`](#emit_cmp_ge_u) | |
| [`emit_vcompare`](#emit_vcompare) | a lanewise comparison of two vectors as a mask of `result_ty`, every bit of a lane set where it holds |
| [`truth`](#truth) | an i1 that holds where a bool byte, or any integer, is not zero |
| [`emit_select`](#emit_select) | `x` where `c` holds, else `y`; a vector condition picks lane by lane |
| [`resize_int`](#resize_int) | an integer of `to`'s width from `x`: kept, truncated or extended by `signed` |
| [`emit_trunc`](#emit_trunc) · [`emit_sext`](#emit_sext) · [`emit_zext`](#emit_zext) · [`emit_fp_trunc`](#emit_fp_trunc) · [`emit_fp_ext`](#emit_fp_ext) | |
| [`emit_fp_resize`](#emit_fp_resize) | a float of `to`'s format from `x`, promoted or demoted |
| [`emit_fp_to_si`](#emit_fp_to_si) · [`emit_fp_to_ui`](#emit_fp_to_ui) · [`emit_si_to_fp`](#emit_si_to_fp) · [`emit_ui_to_fp`](#emit_ui_to_fp) | |
| [`emit_bitcast`](#emit_bitcast) | the same bits as `to`: an address read as an integer and back, a resize between integers of different widths, or a reinterpretation of equal widths |
| [`emit_classify`](#emit_classify) | `x` made secret: a block parameter declared secret, which the branch into it passes `x` |
| [`emit_declassify`](#emit_declassify) | |
| [`emit_alloca`](#emit_alloca) | a stack slot of `ty`, made in the entry block so every path reaches it |
| [`alloca_aligned`](#alloca_aligned) | |
| [`emit_whole`](#emit_whole) | the aggregate `v` held whole: itself where it is, a constant as its own value, and one at an address read from it whole, as secret as `v` and stated as its type |
| [`emit_extract`](#emit_extract) | member `k` of the aggregate `v` held whole: a structure's member, an array's element or a union's member read from its first byte |
| [`emit_insert`](#emit_insert) | the aggregate `v` held whole with member `k` replaced by `part`, held whole where it is an aggregate |
| [`emit_spill`](#emit_spill) | the aggregate `v` held whole written to a stack slot of its own, whose address is the value: what a read of its bytes as another type needs |
| [`emit_load`](#emit_load) | a value of `ty` read from `addr`: an aggregate is copied into a slot of its own, whose address is the value, as secret as the read or `addr` |
| [`emit_load_as`](#emit_load_as) | a value of `ty` read from `addr` that is as secret as `secret`, whatever the reads around it are marked |
| [`emit_load_discriminator`](#emit_load_discriminator) | a tag's discriminator of `ty` read from `addr`, stated public: the layout keeps it apart from the bytes of every case, so it is public whatever the cases hold |
| [`emit_store`](#emit_store) | write `v` to `addr`: an aggregate, held at an address, is copied |
| [`emit_copy`](#emit_copy) | copy the bytes of a `ty` from `src` to `dst`, marked secret where the read is or `src` is |
| [`copy_publics`](#copy_publics) | the bytes of each of `ranges` of an aggregate of `ty` at `src`, which its type lays out as public, copied again to the same bytes at `dst` after the whole copy, each copy stated public, so the walk of a constant-time function sees those bytes of `dst` written with what public storage held where the whole copy, stated secret, moved them in words that also held a secret |
| [`emit_memzero`](#emit_memzero) | zero the bytes of a `ty` at `addr`, a place whose type holds a secret where `secret` holds, which the fill states of the memory it writes |
| [`emit_wipe`](#emit_wipe) | zero the bytes of `count` of `ty` at `addr`, a place whose type holds a secret where `secret` holds, through a kept fill: no pass removes or narrows it, and its length is the size of its target alone |
| [`emit_offset`](#emit_offset) | the address `bytes` past `addr` |
| [`emit_member`](#emit_member) | the address of member `k` of the aggregate `agg` at `addr`: a structure's member, an array's or vector's element, and a union's every member, which all start where it does |
| [`emit_element`](#emit_element) | the address of element `ix` of the array or vector `agg` at `addr` |
| [`emit_step`](#emit_step) | the address `ix` elements of `elem` past `addr`, as a pointer steps |
| [`emit_splat`](#emit_splat) | a vector of `ty` with `scalar` in every lane |
| [`emit_vec_extract`](#emit_vec_extract) | lane `lane` of `vec`, read as `ty`: a constant lane is read in place, any other through the vector's bytes in a slot |
| [`emit_vec_insert`](#emit_vec_insert) | |
| [`emit_vec_build`](#emit_vec_build) | a vector of `ty` from one value per lane |
| [`emit_shuffle`](#emit_shuffle) | lanes of `x` and `y` joined, picked by `picks`, as a vector of `ty` |
| [`emit_reduce`](#emit_reduce) | a reduction of a vector's lanes to one scalar |
| [`emit_unary`](#emit_unary) | a lanewise or scalar operation of one operand |
| [`emit_binary`](#emit_binary) | a lanewise or scalar operation of two operands |
| [`emit_ternary`](#emit_ternary) | a lanewise or scalar operation of three operands |
| [`emit_atomic_load`](#emit_atomic_load) | an atomic read of the integer `ty` at `addr` |
| [`emit_atomic_store`](#emit_atomic_store) | |
| [`emit_atomic_rmw`](#emit_atomic_rmw) | an atomic read-modify-write, giving the value held before |
| [`emit_atomic_cmpxchg`](#emit_atomic_cmpxchg) | an atomic compare and exchange, giving the value held before |
| [`emit_fence`](#emit_fence) | |
| [`emit_call`](#emit_call) | a call of `callee` through the signature `sig`, every argument named |
| [`emit_call_named`](#emit_call_named) | a call of `callee` through the signature `sig`, of which `named` states how many arguments are named, the rest being a C-variadic call's unnamed ones |
| [`result_whole`](#result_whole) | hold the next call's aggregate result whole, with no slot, unless it lands in a destination |
| [`result_into`](#result_into) | land the next call's aggregate result in `slot`, storage of the result's type nothing reads yet |
| [`param`](#param) | parameter `i` of the function being built, of `ty`: an aggregate held `whole` is the parameter itself, and any other lands in a slot of its own, whose address is the value |
| [`emit_br`](#emit_br) | |
| [`emit_br_args`](#emit_br_args) | a branch to `target`, passing one argument per parameter of it |
| [`emit_cbr`](#emit_cbr) | |
| [`emit_switch`](#emit_switch) | a branch on `sel` to the target of the case it equals, else to `otherwise` |
| [`emit_ret`](#emit_ret) | return `v` from the function being built: an aggregate is read whole from its address, as secret as `v` is |
| [`emit_ret_void`](#emit_ret_void) | |
| [`emit_asm`](#emit_asm) | an inline assembly block at the cursor, binding the `n` values at `values` in its operands' order |
| [`emit_target_op`](#emit_target_op) | the target op `op` at the cursor over the `n` values at `values`, in its operands' order, its result the one `op` states or none |
| [`emit_unreachable`](#emit_unreachable) · [`add`](#add) · [`sub`](#sub) · [`mul`](#mul) · [`band`](#band) · [`bor`](#bor) · [`bxor`](#bxor) · [`shl`](#shl) · [`shr_u`](#shr_u) · [`shr_s`](#shr_s) · [`eq`](#eq) · [`ne`](#ne) · [`lt_s`](#lt_s) · [`lt_u`](#lt_u) · [`le_u`](#le_u) · [`trunc`](#trunc) · [`zext`](#zext) · [`sext`](#sext) · [`bitcast`](#bitcast) · [`br`](#br) · [`cbr`](#cbr) | |

---

## Builder

```mach
pub rec Builder {
    m:       *module.Module;
    a:       *A.Allocator;
    mb:      mbuild.Builder;
    target:  *mirl_record.Target;
    layout:  mirl_layout.Layout;
    slots:   u32;
    fn:      opt[id.Function];
    entry:   id.Block;
    block:   id.Block;
    allocas: u32;
    # the slot the next call's aggregate result lands in, in place of one of its own
    into:     value.Value;
    into_set: bool;
    # the next call's aggregate result is held whole, with no slot
    whole:    bool;
    ended:    vector.Vector[bool];
    secret:   bool;
    stated:   bool;
    volatile: bool;
    loc:      base_source.Location;
    debug:    bool;
    sources:  *base_source.SourceMap;
    files:    vector.Vector[u32];
    failure:  opt[fail.Fail];
}
```

### init

```mach
pub fun init(m: *module.Module, a: *A.Allocator, target: *mirl_record.Target, ctx: opt[*core_state.Context], slots: u32, sources: *base_source.SourceMap, debug_info: bool) Builder
```

a builder over `m`, whose types the target's layout lays out under the build's context,
with its stack slots in the address space `slots`

### dnit

```mach
pub fun dnit(b: *Builder)
```

### fault

```mach
pub fun fault(b: *Builder, f: fail.Fail)
```

### failed

```mach
pub fun failed(b: *Builder) bool
```

### done

```mach
pub fun done(b: *Builder) err[fail.Fail]
```

the first failure of the work: lowering's own, else the one mirl's builder kept

### after_err_pub

```mach
pub fun after_err_pub(b: *Builder) err[fail.Fail]
```

### bind_storage

```mach
pub fun bind_storage(b: *Builder, slot: value.Value, binding: debug.Binding) err[fail.Fail]
```

bind a variable to the stack slot `slot` holds it in: the slot's alloca names it

### state_publics

```mach
pub fun state_publics(b: *Builder, slot: value.Value, ranges: Slice[mirl_global.Range]) err[fail.Fail]
```

state the bytes of the stack slot `slot` that the source type of what it holds lays out as public, `ranges` as the
module takes them: the public ranges of its alloca, which the walk of a constant-time function's code takes as typed
frame extents, and every other byte of it as of a secret type. none states every byte so

### bind_value

```mach
pub fun bind_value(b: *Builder, binding: debug.Binding, v: value.Value)
```

bind a variable to `v` from the cursor's point on

### take

```mach
pub fun take(b: *Builder, r: res[value.Value, fail.Fail]) value.Value
```

### take_type

```mach
pub fun take_type(b: *Builder, r: res[Type, fail.Fail]) Type
```

### take_err

```mach
pub fun take_err(b: *Builder, r: err[fail.Fail])
```

### types_of

```mach
pub fun types_of(b: *Builder) *types.Table
```

### ptr_type

```mach
pub fun ptr_type(b: *Builder) Type
```

a pointer into the storage a function's own slots live in, which a pointer the source
names no other storage of points into

### ptr_in

```mach
pub fun ptr_in(b: *Builder, space: u32) Type
```

a pointer into the address space `space`

### slot_space

```mach
pub fun slot_space(b: *Builder) u32
```

the address space a stack slot lives in, as mirl's target states it

### global_space

```mach
pub fun global_space(b: *Builder, ix: u32) u32
```

the address space global `ix` lives in: its storage class where the target's pointers
name one, the default space elsewhere

### classes

```mach
pub fun classes(b: *Builder) bool
```

whether the target's pointers name a storage class each, so a function's pointer
parameters take the classes of the storage their arguments point into

### int_type

```mach
pub fun int_type(b: *Builder, bits: u32) Type
```

### index_type

```mach
pub fun index_type(b: *Builder) Type
```

the integer as wide as a pointer, which addresses are compared and stepped in. a
logical pointer is no address and has no width, so an index into what it points to
is a 32-bit word, as spir-v writes a constant index

### size_of

```mach
pub fun size_of(b: *Builder, ty: Type) u64
```

### align_of

```mach
pub fun align_of(b: *Builder, ty: Type) u64
```

### offset_of

```mach
pub fun offset_of(b: *Builder, ty: Type, member: u32) u64
```

### enter

```mach
pub fun enter(b: *Builder, f: id.Function, params: *Type, secrets: *bool, n: u32) id.Block
```

build in function `f`, whose entry block takes `params`, each secret as `secrets` says

### seal

```mach
pub fun seal(b: *Builder) err[fail.Fail]
```

end every block left open with unreachable: a block nothing branches to, such as the one
after a return

### leave

```mach
pub fun leave(b: *Builder)
```

leave the function: nothing is built until the next is entered

### in_function

```mach
pub fun in_function(b: *Builder) bool
```

### function

```mach
pub fun function(b: *Builder) id.Function
```

### entry_block

```mach
pub fun entry_block(b: *Builder) id.Block
```

### block_make

```mach
pub fun block_make(b: *Builder) res[id.Block, fail.Fail]
```

### block_make_with

```mach
pub fun block_make_with(b: *Builder, params: *Type, secrets: *bool, n: u32) id.Block
```

a block whose parameters are `params`, which every edge into it passes

### block_param

```mach
pub fun block_param(b: *Builder, k: id.Block, i: u32, ty: Type) value.Value
```

parameter `i` of block `k`, typed `ty`

### set_block

```mach
pub fun set_block(b: *Builder, k: id.Block)
```

### current_block

```mach
pub fun current_block(b: *Builder) id.Block
```

### block_terminated

```mach
pub fun block_terminated(b: *Builder, k: id.Block) bool
```

### terminated

```mach
pub fun terminated(b: *Builder) bool
```

whether the block being built already ends

### set_loc

```mach
pub fun set_loc(b: *Builder, loc: base_source.Location)
```

### current_loc

```mach
pub fun current_loc(b: *Builder) base_source.Location
```

### location_of

```mach
pub fun location_of(b: *Builder, loc: base_source.Location) opt[debug.Location]
```

the debug table's location of a source location, none when it has no file

### debug_location

```mach
pub fun debug_location(b: *Builder) opt[debug.Location]
```

the debug location the next instructions are built for

### set_secret

```mach
pub fun set_secret(b: *Builder, s: bool)
```

whether the next instructions are marked secret: a load, call or atomic read of a secret

### set_access

```mach
pub fun set_access(b: *Builder, secret: bool)
```

the next accesses through an address state the memory they read or write as the source
type says: secret, which marks them so, or public. other accesses state nothing, and the
constant-time walk holds what they reach as it holds any access it cannot place

### end_access

```mach
pub fun end_access(b: *Builder)
```

the next accesses state nothing of the memory they reach

### set_volatile

```mach
pub fun set_volatile(b: *Builder, v: bool)
```

whether the next memory accesses are volatile

### set_kept

```mach
pub fun set_kept(b: *Builder, k: bool)
```

whether the next writes are kept: a wipe of memory that held a secret

### spec_constant

```mach
pub fun spec_constant(b: *Builder, spec_id: u32, default: value.Value) value.Value
```

the specialization constant `spec_id`, whose default is the integer or float constant `default`

### constant_of

```mach
pub fun constant_of(b: *Builder, v: value.Value) id.Constant
```

### use

```mach
pub fun use(b: *Builder, v: value.Value) id.Value
```

the mirl value `v` stands for in the function being built

### emit_store_marked

```mach
pub fun emit_store_marked(b: *Builder, v: value.Value, ptr: value.Value, vol: bool) err[fail.Fail]
```

a store marked volatile where `vol` holds: an access of a volatile place

### emit_load_marked

```mach
pub fun emit_load_marked(b: *Builder, ptr: value.Value, ty: Type, vol: bool) res[value.Value, fail.Fail]
```

a load marked volatile where `vol` holds

### emit_add

```mach
pub fun emit_add(b: *Builder, x: value.Value, y: value.Value) res[value.Value, fail.Fail]
```

### emit_sub

```mach
pub fun emit_sub(b: *Builder, x: value.Value, y: value.Value) res[value.Value, fail.Fail]
```

### emit_mul

```mach
pub fun emit_mul(b: *Builder, x: value.Value, y: value.Value) res[value.Value, fail.Fail]
```

### emit_div_s

```mach
pub fun emit_div_s(b: *Builder, x: value.Value, y: value.Value) res[value.Value, fail.Fail]
```

### emit_div_u

```mach
pub fun emit_div_u(b: *Builder, x: value.Value, y: value.Value) res[value.Value, fail.Fail]
```

### emit_rem_s

```mach
pub fun emit_rem_s(b: *Builder, x: value.Value, y: value.Value) res[value.Value, fail.Fail]
```

### emit_rem_u

```mach
pub fun emit_rem_u(b: *Builder, x: value.Value, y: value.Value) res[value.Value, fail.Fail]
```

### emit_and

```mach
pub fun emit_and(b: *Builder, x: value.Value, y: value.Value) res[value.Value, fail.Fail]
```

### emit_or

```mach
pub fun emit_or(b: *Builder, x: value.Value, y: value.Value) res[value.Value, fail.Fail]
```

### emit_xor

```mach
pub fun emit_xor(b: *Builder, x: value.Value, y: value.Value) res[value.Value, fail.Fail]
```

### emit_shl

```mach
pub fun emit_shl(b: *Builder, x: value.Value, n: value.Value) res[value.Value, fail.Fail]
```

### emit_shr_s

```mach
pub fun emit_shr_s(b: *Builder, x: value.Value, n: value.Value) res[value.Value, fail.Fail]
```

### emit_shr_u

```mach
pub fun emit_shr_u(b: *Builder, x: value.Value, n: value.Value) res[value.Value, fail.Fail]
```

### emit_neg

```mach
pub fun emit_neg(b: *Builder, x: value.Value) res[value.Value, fail.Fail]
```

### emit_not

```mach
pub fun emit_not(b: *Builder, x: value.Value) res[value.Value, fail.Fail]
```

### emit_cmp_eq

```mach
pub fun emit_cmp_eq(b: *Builder, x: value.Value, y: value.Value) res[value.Value, fail.Fail]
```

### emit_cmp_ne

```mach
pub fun emit_cmp_ne(b: *Builder, x: value.Value, y: value.Value) res[value.Value, fail.Fail]
```

### emit_cmp_lt_s

```mach
pub fun emit_cmp_lt_s(b: *Builder, x: value.Value, y: value.Value) res[value.Value, fail.Fail]
```

### emit_cmp_lt_u

```mach
pub fun emit_cmp_lt_u(b: *Builder, x: value.Value, y: value.Value) res[value.Value, fail.Fail]
```

### emit_cmp_le_s

```mach
pub fun emit_cmp_le_s(b: *Builder, x: value.Value, y: value.Value) res[value.Value, fail.Fail]
```

### emit_cmp_le_u

```mach
pub fun emit_cmp_le_u(b: *Builder, x: value.Value, y: value.Value) res[value.Value, fail.Fail]
```

### emit_cmp_gt_s

```mach
pub fun emit_cmp_gt_s(b: *Builder, x: value.Value, y: value.Value) res[value.Value, fail.Fail]
```

### emit_cmp_gt_u

```mach
pub fun emit_cmp_gt_u(b: *Builder, x: value.Value, y: value.Value) res[value.Value, fail.Fail]
```

### emit_cmp_ge_s

```mach
pub fun emit_cmp_ge_s(b: *Builder, x: value.Value, y: value.Value) res[value.Value, fail.Fail]
```

### emit_cmp_ge_u

```mach
pub fun emit_cmp_ge_u(b: *Builder, x: value.Value, y: value.Value) res[value.Value, fail.Fail]
```

### emit_vcompare

```mach
pub fun emit_vcompare(b: *Builder, int_code: Opcode, float_code: Opcode, x: value.Value, y: value.Value, result_ty: Type) res[value.Value, fail.Fail]
```

a lanewise comparison of two vectors as a mask of `result_ty`, every bit of a lane set where it holds

### truth

```mach
pub fun truth(b: *Builder, c: value.Value) value.Value
```

an i1 that holds where a bool byte, or any integer, is not zero

### emit_select

```mach
pub fun emit_select(b: *Builder, c: value.Value, x: value.Value, y: value.Value) res[value.Value, fail.Fail]
```

`x` where `c` holds, else `y`; a vector condition picks lane by lane

### resize_int

```mach
pub fun resize_int(b: *Builder, x: value.Value, to: Type, signed: bool) res[value.Value, fail.Fail]
```

an integer of `to`'s width from `x`: kept, truncated or extended by `signed`

### emit_trunc

```mach
pub fun emit_trunc(b: *Builder, x: value.Value, to: Type) res[value.Value, fail.Fail]
```

### emit_sext

```mach
pub fun emit_sext(b: *Builder, x: value.Value, to: Type) res[value.Value, fail.Fail]
```

### emit_zext

```mach
pub fun emit_zext(b: *Builder, x: value.Value, to: Type) res[value.Value, fail.Fail]
```

### emit_fp_trunc

```mach
pub fun emit_fp_trunc(b: *Builder, x: value.Value, to: Type) res[value.Value, fail.Fail]
```

### emit_fp_ext

```mach
pub fun emit_fp_ext(b: *Builder, x: value.Value, to: Type) res[value.Value, fail.Fail]
```

### emit_fp_resize

```mach
pub fun emit_fp_resize(b: *Builder, x: value.Value, to: Type) res[value.Value, fail.Fail]
```

a float of `to`'s format from `x`, promoted or demoted

### emit_fp_to_si

```mach
pub fun emit_fp_to_si(b: *Builder, x: value.Value, to: Type) res[value.Value, fail.Fail]
```

### emit_fp_to_ui

```mach
pub fun emit_fp_to_ui(b: *Builder, x: value.Value, to: Type) res[value.Value, fail.Fail]
```

### emit_si_to_fp

```mach
pub fun emit_si_to_fp(b: *Builder, x: value.Value, to: Type) res[value.Value, fail.Fail]
```

### emit_ui_to_fp

```mach
pub fun emit_ui_to_fp(b: *Builder, x: value.Value, to: Type) res[value.Value, fail.Fail]
```

### emit_bitcast

```mach
pub fun emit_bitcast(b: *Builder, x: value.Value, to: Type) res[value.Value, fail.Fail]
```

the same bits as `to`: an address read as an integer and back, a resize between integers
of different widths, or a reinterpretation of equal widths

### emit_classify

```mach
pub fun emit_classify(b: *Builder, x: value.Value) res[value.Value, fail.Fail]
```

`x` made secret: a block parameter declared secret, which the branch into it passes `x`

### emit_declassify

```mach
pub fun emit_declassify(b: *Builder, x: value.Value) res[value.Value, fail.Fail]
```

### emit_alloca

```mach
pub fun emit_alloca(b: *Builder, ty: Type) res[value.Value, fail.Fail]
```

a stack slot of `ty`, made in the entry block so every path reaches it

### alloca_aligned

```mach
pub fun alloca_aligned(b: *Builder, ty: Type, align: u64) res[value.Value, fail.Fail]
```

### emit_whole

```mach
pub fun emit_whole(b: *Builder, v: value.Value) res[value.Value, fail.Fail]
```

the aggregate `v` held whole: itself where it is, a constant as its own value, and one at
an address read from it whole, as secret as `v` and stated as its type

### emit_extract

```mach
pub fun emit_extract(b: *Builder, v: value.Value, k: u32) res[value.Value, fail.Fail]
```

member `k` of the aggregate `v` held whole: a structure's member, an array's element or a
union's member read from its first byte. an aggregate member is held whole in turn

### emit_insert

```mach
pub fun emit_insert(b: *Builder, v: value.Value, part: value.Value, k: u32) res[value.Value, fail.Fail]
```

the aggregate `v` held whole with member `k` replaced by `part`, held whole where it is an
aggregate

### emit_spill

```mach
pub fun emit_spill(b: *Builder, v: value.Value) res[value.Value, fail.Fail]
```

the aggregate `v` held whole written to a stack slot of its own, whose address is the value:
what a read of its bytes as another type needs

### emit_load

```mach
pub fun emit_load(b: *Builder, addr: value.Value, ty: Type) res[value.Value, fail.Fail]
```

a value of `ty` read from `addr`: an aggregate is copied into a slot of its own, whose
address is the value, as secret as the read or `addr`

### emit_load_as

```mach
pub fun emit_load_as(b: *Builder, addr: value.Value, ty: Type, secret: bool) res[value.Value, fail.Fail]
```

a value of `ty` read from `addr` that is as secret as `secret`, whatever the reads around
it are marked

### emit_load_discriminator

```mach
pub fun emit_load_discriminator(b: *Builder, addr: value.Value, ty: Type) res[value.Value, fail.Fail]
```

a tag's discriminator of `ty` read from `addr`, stated public: the layout keeps it apart from the bytes of every
case, so it is public whatever the cases hold

### emit_store

```mach
pub fun emit_store(b: *Builder, v: value.Value, addr: value.Value) err[fail.Fail]
```

write `v` to `addr`: an aggregate, held at an address, is copied. a store where the access is stated states the
memory it writes as the access does, secret with its mark and public with the public flag

### emit_copy

```mach
pub fun emit_copy(b: *Builder, dst: value.Value, src: value.Value, ty: Type) err[fail.Fail]
```

copy the bytes of a `ty` from `src` to `dst`, marked secret where the read is or `src` is. a copy
whose access is stated, as a store of an aggregate states the place it writes, states the bytes
it moves: secret with its mark, or public where the place's type holds no secret, which sema
holds the source's type to as well

### copy_publics

```mach
pub fun copy_publics(b: *Builder, dst: value.Value, src: value.Value, ty: Type, ranges: Slice[mirl_global.Range]) err[fail.Fail]
```

the bytes of each of `ranges` of an aggregate of `ty` at `src`, which its type lays out as public, copied again to
the same bytes at `dst` after the whole copy, each copy stated public, so the walk of a constant-time function sees
those bytes of `dst` written with what public storage held where the whole copy, stated secret, moved them in words
that also held a secret

### emit_memzero

```mach
pub fun emit_memzero(b: *Builder, addr: value.Value, ty: Type, secret: bool) err[fail.Fail]
```

zero the bytes of a `ty` at `addr`, a place whose type holds a secret where `secret` holds, which
the fill states of the memory it writes

### emit_wipe

```mach
pub fun emit_wipe(b: *Builder, addr: value.Value, count: value.Value, signed: bool, ty: Type, secret: bool) err[fail.Fail]
```

zero the bytes of `count` of `ty` at `addr`, a place whose type holds a secret where `secret`
holds, through a kept fill: no pass removes or narrows it, and its length is the size of its
target alone

### emit_offset

```mach
pub fun emit_offset(b: *Builder, addr: value.Value, bytes: value.Value) res[value.Value, fail.Fail]
```

the address `bytes` past `addr`

### emit_member

```mach
pub fun emit_member(b: *Builder, addr: value.Value, agg: Type, k: u32) res[value.Value, fail.Fail]
```

the address of member `k` of the aggregate `agg` at `addr`: a structure's member, an
array's or vector's element, and a union's every member, which all start where it does

### emit_element

```mach
pub fun emit_element(b: *Builder, addr: value.Value, agg: Type, ix: value.Value) res[value.Value, fail.Fail]
```

the address of element `ix` of the array or vector `agg` at `addr`

### emit_step

```mach
pub fun emit_step(b: *Builder, addr: value.Value, elem: Type, ix: value.Value) res[value.Value, fail.Fail]
```

the address `ix` elements of `elem` past `addr`, as a pointer steps

### emit_splat

```mach
pub fun emit_splat(b: *Builder, ty: Type, scalar: value.Value) res[value.Value, fail.Fail]
```

a vector of `ty` with `scalar` in every lane

### emit_vec_extract

```mach
pub fun emit_vec_extract(b: *Builder, vec: value.Value, lane: value.Value, ty: Type) res[value.Value, fail.Fail]
```

lane `lane` of `vec`, read as `ty`: a constant lane is read in place, any other through
the vector's bytes in a slot

### emit_vec_insert

```mach
pub fun emit_vec_insert(b: *Builder, vec: value.Value, lane: value.Value, x: value.Value) res[value.Value, fail.Fail]
```

### emit_vec_build

```mach
pub fun emit_vec_build(b: *Builder, ty: Type, lanes: *value.Value, n: u32) res[value.Value, fail.Fail]
```

a vector of `ty` from one value per lane

### emit_shuffle

```mach
pub fun emit_shuffle(b: *Builder, ty: Type, x: value.Value, y: value.Value, picks: *u32, n: u32) res[value.Value, fail.Fail]
```

lanes of `x` and `y` joined, picked by `picks`, as a vector of `ty`

### emit_reduce

```mach
pub fun emit_reduce(b: *Builder, code: Opcode, x: value.Value) res[value.Value, fail.Fail]
```

a reduction of a vector's lanes to one scalar

### emit_unary

```mach
pub fun emit_unary(b: *Builder, code: Opcode, x: value.Value) res[value.Value, fail.Fail]
```

a lanewise or scalar operation of one operand

### emit_binary

```mach
pub fun emit_binary(b: *Builder, code: Opcode, x: value.Value, y: value.Value) res[value.Value, fail.Fail]
```

a lanewise or scalar operation of two operands

### emit_ternary

```mach
pub fun emit_ternary(b: *Builder, code: Opcode, x: value.Value, y: value.Value, z: value.Value) res[value.Value, fail.Fail]
```

a lanewise or scalar operation of three operands

### emit_atomic_load

```mach
pub fun emit_atomic_load(b: *Builder, addr: value.Value, ty: Type, order: Ordering) res[value.Value, fail.Fail]
```

an atomic read of the integer `ty` at `addr`

### emit_atomic_store

```mach
pub fun emit_atomic_store(b: *Builder, addr: value.Value, x: value.Value, order: Ordering) err[fail.Fail]
```

### emit_atomic_rmw

```mach
pub fun emit_atomic_rmw(b: *Builder, code: Opcode, addr: value.Value, x: value.Value, order: Ordering) res[value.Value, fail.Fail]
```

an atomic read-modify-write, giving the value held before

### emit_atomic_cmpxchg

```mach
pub fun emit_atomic_cmpxchg(b: *Builder, addr: value.Value, expected: value.Value, desired: value.Value, success: Ordering, failure_order: Ordering) res[value.Value, fail.Fail]
```

an atomic compare and exchange, giving the value held before

### emit_fence

```mach
pub fun emit_fence(b: *Builder, order: Ordering) err[fail.Fail]
```

### emit_call

```mach
pub fun emit_call(b: *Builder, callee: value.Value, sig: Type, args: *value.Value, n: u32) res[value.Value, fail.Fail]
```

a call of `callee` through the signature `sig`, every argument named. an aggregate
argument is passed whole, read from the address it travels at as secret as the argument
is, and an aggregate result lands in a slot of its own

### emit_call_named

```mach
pub fun emit_call_named(b: *Builder, callee: value.Value, sig: Type, args: *value.Value, n: u32, named: Named) res[value.Value, fail.Fail]
```

a call of `callee` through the signature `sig`, of which `named` states how many
arguments are named, the rest being a C-variadic call's unnamed ones

### result_whole

```mach
pub fun result_whole(b: *Builder)
```

hold the next call's aggregate result whole, with no slot, unless it lands in a destination

### result_into

```mach
pub fun result_into(b: *Builder, slot: value.Value)
```

land the next call's aggregate result in `slot`, storage of the result's type nothing reads yet

### param

```mach
pub fun param(b: *Builder, i: u32, ty: Type, whole: bool) res[value.Value, fail.Fail]
```

parameter `i` of the function being built, of `ty`: an aggregate held `whole` is the
parameter itself, and any other lands in a slot of its own, whose address is the value

### emit_br

```mach
pub fun emit_br(b: *Builder, target: id.Block) err[fail.Fail]
```

### emit_br_args

```mach
pub fun emit_br_args(b: *Builder, target: id.Block, args: *value.Value, n: u32) err[fail.Fail]
```

a branch to `target`, passing one argument per parameter of it

### emit_cbr

```mach
pub fun emit_cbr(b: *Builder, c: value.Value, then_b: id.Block, else_b: id.Block) err[fail.Fail]
```

### emit_switch

```mach
pub fun emit_switch(b: *Builder, selector: value.Value, otherwise: id.Block, cases: *u64, targets: *id.Block, n: u32) err[fail.Fail]
```

a branch on `sel` to the target of the case it equals, else to `otherwise`. `cases` holds
strictly increasing values, one per target

### emit_ret

```mach
pub fun emit_ret(b: *Builder, v: value.Value) err[fail.Fail]
```

return `v` from the function being built: an aggregate is read whole from its address, as
secret as `v` is

### emit_ret_void

```mach
pub fun emit_ret_void(b: *Builder) err[fail.Fail]
```

### emit_asm

```mach
pub fun emit_asm(b: *Builder, block: *Assembly, values: *value.Value, n: u32) err[fail.Fail]
```

an inline assembly block at the cursor, binding the `n` values at `values` in its operands'
order. the instruction copies the block, so it may be borrowed

### emit_target_op

```mach
pub fun emit_target_op(b: *Builder, op: *native.Op, values: *value.Value, n: u32) res[value.Value, fail.Fail]
```

the target op `op` at the cursor over the `n` values at `values`, in its operands' order,
its result the one `op` states or none. the instruction copies the op, so it may be borrowed

### emit_unreachable

```mach
pub fun emit_unreachable(b: *Builder) err[fail.Fail]
```

### add

```mach
pub fun add(b: *Builder, x: value.Value, y: value.Value) value.Value
```

### sub

```mach
pub fun sub(b: *Builder, x: value.Value, y: value.Value) value.Value
```

### mul

```mach
pub fun mul(b: *Builder, x: value.Value, y: value.Value) value.Value
```

### band

```mach
pub fun band(b: *Builder, x: value.Value, y: value.Value) value.Value
```

### bor

```mach
pub fun bor(b: *Builder, x: value.Value, y: value.Value) value.Value
```

### bxor

```mach
pub fun bxor(b: *Builder, x: value.Value, y: value.Value) value.Value
```

### shl

```mach
pub fun shl(b: *Builder, x: value.Value, y: value.Value) value.Value
```

### shr_u

```mach
pub fun shr_u(b: *Builder, x: value.Value, y: value.Value) value.Value
```

### shr_s

```mach
pub fun shr_s(b: *Builder, x: value.Value, y: value.Value) value.Value
```

### eq

```mach
pub fun eq(b: *Builder, x: value.Value, y: value.Value) value.Value
```

### ne

```mach
pub fun ne(b: *Builder, x: value.Value, y: value.Value) value.Value
```

### lt_s

```mach
pub fun lt_s(b: *Builder, x: value.Value, y: value.Value) value.Value
```

### lt_u

```mach
pub fun lt_u(b: *Builder, x: value.Value, y: value.Value) value.Value
```

### le_u

```mach
pub fun le_u(b: *Builder, x: value.Value, y: value.Value) value.Value
```

### trunc

```mach
pub fun trunc(b: *Builder, x: value.Value, to: Type) value.Value
```

### zext

```mach
pub fun zext(b: *Builder, x: value.Value, to: Type) value.Value
```

### sext

```mach
pub fun sext(b: *Builder, x: value.Value, to: Type) value.Value
```

### bitcast

```mach
pub fun bitcast(b: *Builder, x: value.Value, to: Type) value.Value
```

### br

```mach
pub fun br(b: *Builder, target: id.Block)
```

### cbr

```mach
pub fun cbr(b: *Builder, c: value.Value, then_b: id.Block, else_b: id.Block)
```

