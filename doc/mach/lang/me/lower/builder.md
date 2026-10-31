# mach.lang.me.lower.builder

the emission layer over mirl's builder: each operation lowering asks for, made at the
cursor in the function being built. a constant, function or global value becomes a mirl
value where an instruction uses it, an aggregate travels as the address of its storage
and is loaded whole only where a call or a return passes it, and a bool is a byte, so
a comparison is widened to one and a condition is compared with zero

## rec Builder

```mach
pub rec Builder;
```

## rec Computed

```mach
pub rec Computed;
```

a computation on a secret: where it is and its opcode

## fun init

```mach
pub fun init(m: *module.Module, a: *A.Allocator, target: *mirl_target.Target, ctx: opt[*core_state.Context], slots: u32, sources: *lang_source.SourceMap, debug_info: bool) Builder;
```

a builder over `m`, whose types the target's layout lays out under the build's context,
with its stack slots in the address space `slots`

## fun dnit

```mach
pub fun dnit(b: *Builder);
```

## fun fault

```mach
pub fun fault(b: *Builder, f: fail.Fail);
```

## fun failed

```mach
pub fun failed(b: *Builder) bool;
```

## fun done

```mach
pub fun done(b: *Builder) err[fail.Fail];
```

the first failure of the work: lowering's own, else the one mirl's builder kept

## fun after_err_pub

```mach
pub fun after_err_pub(b: *Builder) err[fail.Fail];
```

## fun bind_storage

```mach
pub fun bind_storage(b: *Builder, slot: value.Value, binding: debug.Binding) err[fail.Fail];
```

bind a variable to the stack slot `slot` holds it in: the slot's alloca names it

## fun bind_value

```mach
pub fun bind_value(b: *Builder, binding: debug.Binding, v: value.Value);
```

bind a variable to `v` from the cursor's point on

## fun take

```mach
pub fun take(b: *Builder, r: res[value.Value, fail.Fail]) value.Value;
```

## fun take_type

```mach
pub fun take_type(b: *Builder, r: res[Type, fail.Fail]) Type;
```

## fun take_err

```mach
pub fun take_err(b: *Builder, r: err[fail.Fail]);
```

## fun types_of

```mach
pub fun types_of(b: *Builder) *types.Table;
```

## fun ptr_type

```mach
pub fun ptr_type(b: *Builder) Type;
```

a pointer into the storage a function's own slots live in, which a pointer the source
names no other storage of points into

## fun ptr_in

```mach
pub fun ptr_in(b: *Builder, space: u32) Type;
```

a pointer into the address space `space`

## fun slot_space

```mach
pub fun slot_space(b: *Builder) u32;
```

the address space a stack slot lives in, as mirl's target states it

## fun global_space

```mach
pub fun global_space(b: *Builder, ix: u32) u32;
```

the address space global `ix` lives in: its storage class where the target's pointers
name one, the default space elsewhere

## fun classes

```mach
pub fun classes(b: *Builder) bool;
```

whether the target's pointers name a storage class each, so a function's pointer
parameters take the classes of the storage their arguments point into

## fun int_type

```mach
pub fun int_type(b: *Builder, bits: u32) Type;
```

## fun index_type

```mach
pub fun index_type(b: *Builder) Type;
```

the integer as wide as a pointer, which addresses are compared and stepped in. a
logical pointer is no address and has no width, so an index into what it points to
is a 32-bit word, as spir-v writes a constant index

## fun size_of

```mach
pub fun size_of(b: *Builder, ty: Type) u64;
```

## fun align_of

```mach
pub fun align_of(b: *Builder, ty: Type) u64;
```

## fun offset_of

```mach
pub fun offset_of(b: *Builder, ty: Type, member: u32) u64;
```

## fun enter

```mach
pub fun enter(b: *Builder, f: id.Function, params: *Type, secrets: *bool, n: u32) id.Block;
```

build in function `f`, whose entry block takes `params`, each secret as `secrets` says

## fun seal

```mach
pub fun seal(b: *Builder) err[fail.Fail];
```

end every block left open with unreachable: a block nothing branches to, such as the one
after a return

## fun leave

```mach
pub fun leave(b: *Builder);
```

leave the function: nothing is built until the next is entered

## fun in_function

```mach
pub fun in_function(b: *Builder) bool;
```

## fun function

```mach
pub fun function(b: *Builder) id.Function;
```

## fun entry_block

```mach
pub fun entry_block(b: *Builder) id.Block;
```

## fun new_block

```mach
pub fun new_block(b: *Builder) res[id.Block, fail.Fail];
```

## fun new_block_with

```mach
pub fun new_block_with(b: *Builder, params: *Type, secrets: *bool, n: u32) id.Block;
```

a block whose parameters are `params`, which every edge into it passes

## fun block_param

```mach
pub fun block_param(b: *Builder, k: id.Block, i: u32, ty: Type) value.Value;
```

parameter `i` of block `k`, typed `ty`

## fun set_block

```mach
pub fun set_block(b: *Builder, k: id.Block);
```

## fun current_block

```mach
pub fun current_block(b: *Builder) id.Block;
```

## fun block_terminated

```mach
pub fun block_terminated(b: *Builder, k: id.Block) bool;
```

## fun terminated

```mach
pub fun terminated(b: *Builder) bool;
```

whether the block being built already ends

## fun set_loc

```mach
pub fun set_loc(b: *Builder, loc: lang_source.Location);
```

## fun current_loc

```mach
pub fun current_loc(b: *Builder) lang_source.Location;
```

## fun location_of

```mach
pub fun location_of(b: *Builder, loc: lang_source.Location) opt[debug.Location];
```

the debug table's location of a source location, none when it has no file

## fun debug_location

```mach
pub fun debug_location(b: *Builder) opt[debug.Location];
```

the debug location the next instructions are built for

## fun set_secret

```mach
pub fun set_secret(b: *Builder, s: bool);
```

whether the next instructions are marked secret: a load, call or atomic read of a secret

## fun set_volatile

```mach
pub fun set_volatile(b: *Builder, v: bool);
```

whether the next memory accesses are volatile

## fun set_kept

```mach
pub fun set_kept(b: *Builder, k: bool);
```

whether the next writes are kept: a wipe of memory that held a secret

## fun spec_constant

```mach
pub fun spec_constant(b: *Builder, spec_id: u32, default: value.Value) value.Value;
```

the specialization constant `spec_id`, whose default is the integer or float constant `default`

## fun constant_of

```mach
pub fun constant_of(b: *Builder, v: value.Value) id.Constant;
```

## fun use

```mach
pub fun use(b: *Builder, v: value.Value) id.Value;
```

the mirl value `v` stands for in the function being built

## fun emit_store_marked

```mach
pub fun emit_store_marked(b: *Builder, v: value.Value, ptr: value.Value, vol: bool) err[fail.Fail];
```

a store marked volatile where `vol` holds: an access of a volatile place

## fun emit_load_marked

```mach
pub fun emit_load_marked(b: *Builder, ptr: value.Value, ty: Type, vol: bool) res[value.Value, fail.Fail];
```

a load marked volatile where `vol` holds

## fun emit_add

```mach
pub fun emit_add(b: *Builder, x: value.Value, y: value.Value) res[value.Value, fail.Fail];
```

## fun emit_sub

```mach
pub fun emit_sub(b: *Builder, x: value.Value, y: value.Value) res[value.Value, fail.Fail];
```

## fun emit_mul

```mach
pub fun emit_mul(b: *Builder, x: value.Value, y: value.Value) res[value.Value, fail.Fail];
```

## fun emit_div_s

```mach
pub fun emit_div_s(b: *Builder, x: value.Value, y: value.Value) res[value.Value, fail.Fail];
```

## fun emit_div_u

```mach
pub fun emit_div_u(b: *Builder, x: value.Value, y: value.Value) res[value.Value, fail.Fail];
```

## fun emit_rem_s

```mach
pub fun emit_rem_s(b: *Builder, x: value.Value, y: value.Value) res[value.Value, fail.Fail];
```

## fun emit_rem_u

```mach
pub fun emit_rem_u(b: *Builder, x: value.Value, y: value.Value) res[value.Value, fail.Fail];
```

## fun emit_and

```mach
pub fun emit_and(b: *Builder, x: value.Value, y: value.Value) res[value.Value, fail.Fail];
```

## fun emit_or

```mach
pub fun emit_or(b: *Builder, x: value.Value, y: value.Value) res[value.Value, fail.Fail];
```

## fun emit_xor

```mach
pub fun emit_xor(b: *Builder, x: value.Value, y: value.Value) res[value.Value, fail.Fail];
```

## fun emit_shl

```mach
pub fun emit_shl(b: *Builder, x: value.Value, n: value.Value) res[value.Value, fail.Fail];
```

## fun emit_shr_s

```mach
pub fun emit_shr_s(b: *Builder, x: value.Value, n: value.Value) res[value.Value, fail.Fail];
```

## fun emit_shr_u

```mach
pub fun emit_shr_u(b: *Builder, x: value.Value, n: value.Value) res[value.Value, fail.Fail];
```

## fun emit_neg

```mach
pub fun emit_neg(b: *Builder, x: value.Value) res[value.Value, fail.Fail];
```

## fun emit_not

```mach
pub fun emit_not(b: *Builder, x: value.Value) res[value.Value, fail.Fail];
```

## fun emit_cmp_eq

```mach
pub fun emit_cmp_eq(b: *Builder, x: value.Value, y: value.Value) res[value.Value, fail.Fail];
```

## fun emit_cmp_ne

```mach
pub fun emit_cmp_ne(b: *Builder, x: value.Value, y: value.Value) res[value.Value, fail.Fail];
```

## fun emit_cmp_lt_s

```mach
pub fun emit_cmp_lt_s(b: *Builder, x: value.Value, y: value.Value) res[value.Value, fail.Fail];
```

## fun emit_cmp_lt_u

```mach
pub fun emit_cmp_lt_u(b: *Builder, x: value.Value, y: value.Value) res[value.Value, fail.Fail];
```

## fun emit_cmp_le_s

```mach
pub fun emit_cmp_le_s(b: *Builder, x: value.Value, y: value.Value) res[value.Value, fail.Fail];
```

## fun emit_cmp_le_u

```mach
pub fun emit_cmp_le_u(b: *Builder, x: value.Value, y: value.Value) res[value.Value, fail.Fail];
```

## fun emit_cmp_gt_s

```mach
pub fun emit_cmp_gt_s(b: *Builder, x: value.Value, y: value.Value) res[value.Value, fail.Fail];
```

## fun emit_cmp_gt_u

```mach
pub fun emit_cmp_gt_u(b: *Builder, x: value.Value, y: value.Value) res[value.Value, fail.Fail];
```

## fun emit_cmp_ge_s

```mach
pub fun emit_cmp_ge_s(b: *Builder, x: value.Value, y: value.Value) res[value.Value, fail.Fail];
```

## fun emit_cmp_ge_u

```mach
pub fun emit_cmp_ge_u(b: *Builder, x: value.Value, y: value.Value) res[value.Value, fail.Fail];
```

## fun emit_vcompare

```mach
pub fun emit_vcompare(b: *Builder, int_code: Opcode, float_code: Opcode, x: value.Value, y: value.Value, result_ty: Type) res[value.Value, fail.Fail];
```

a lanewise comparison of two vectors as a mask of `result_ty`, every bit of a lane set where it holds

## fun truth

```mach
pub fun truth(b: *Builder, c: value.Value) value.Value;
```

an i1 that holds where a bool byte, or any integer, is not zero

## fun emit_select

```mach
pub fun emit_select(b: *Builder, c: value.Value, x: value.Value, y: value.Value) res[value.Value, fail.Fail];
```

`x` where `c` holds, else `y`; a vector condition picks lane by lane

## fun resize_int

```mach
pub fun resize_int(b: *Builder, x: value.Value, to: Type, signed: bool) res[value.Value, fail.Fail];
```

an integer of `to`'s width from `x`: kept, truncated or extended by `signed`

## fun emit_trunc

```mach
pub fun emit_trunc(b: *Builder, x: value.Value, to: Type) res[value.Value, fail.Fail];
```

## fun emit_sext

```mach
pub fun emit_sext(b: *Builder, x: value.Value, to: Type) res[value.Value, fail.Fail];
```

## fun emit_zext

```mach
pub fun emit_zext(b: *Builder, x: value.Value, to: Type) res[value.Value, fail.Fail];
```

## fun emit_fp_trunc

```mach
pub fun emit_fp_trunc(b: *Builder, x: value.Value, to: Type) res[value.Value, fail.Fail];
```

## fun emit_fp_ext

```mach
pub fun emit_fp_ext(b: *Builder, x: value.Value, to: Type) res[value.Value, fail.Fail];
```

## fun emit_fp_resize

```mach
pub fun emit_fp_resize(b: *Builder, x: value.Value, to: Type) res[value.Value, fail.Fail];
```

a float of `to`'s format from `x`, promoted or demoted

## fun emit_fp_to_si

```mach
pub fun emit_fp_to_si(b: *Builder, x: value.Value, to: Type) res[value.Value, fail.Fail];
```

## fun emit_fp_to_ui

```mach
pub fun emit_fp_to_ui(b: *Builder, x: value.Value, to: Type) res[value.Value, fail.Fail];
```

## fun emit_si_to_fp

```mach
pub fun emit_si_to_fp(b: *Builder, x: value.Value, to: Type) res[value.Value, fail.Fail];
```

## fun emit_ui_to_fp

```mach
pub fun emit_ui_to_fp(b: *Builder, x: value.Value, to: Type) res[value.Value, fail.Fail];
```

## fun emit_bitcast

```mach
pub fun emit_bitcast(b: *Builder, x: value.Value, to: Type) res[value.Value, fail.Fail];
```

the same bits as `to`: an address read as an integer and back, a resize between integers
of different widths, or a reinterpretation of equal widths

## fun emit_classify

```mach
pub fun emit_classify(b: *Builder, x: value.Value) res[value.Value, fail.Fail];
```

`x` made secret: a block parameter declared secret, which the branch into it passes `x`

## fun emit_declassify

```mach
pub fun emit_declassify(b: *Builder, x: value.Value) res[value.Value, fail.Fail];
```

## fun emit_alloca

```mach
pub fun emit_alloca(b: *Builder, ty: Type) res[value.Value, fail.Fail];
```

a stack slot of `ty`, made in the entry block so every path reaches it

## fun alloca_aligned

```mach
pub fun alloca_aligned(b: *Builder, ty: Type, align: u64) res[value.Value, fail.Fail];
```

## fun emit_load

```mach
pub fun emit_load(b: *Builder, addr: value.Value, ty: Type) res[value.Value, fail.Fail];
```

a value of `ty` read from `addr`: an aggregate is copied into a slot of its own, whose
address is the value

## fun emit_store

```mach
pub fun emit_store(b: *Builder, v: value.Value, addr: value.Value) err[fail.Fail];
```

write `v` to `addr`: an aggregate, held at an address, is copied

## fun emit_copy

```mach
pub fun emit_copy(b: *Builder, dst: value.Value, src: value.Value, ty: Type) err[fail.Fail];
```

copy the bytes of a `ty` from `src` to `dst`

## fun emit_memzero

```mach
pub fun emit_memzero(b: *Builder, addr: value.Value, ty: Type) err[fail.Fail];
```

zero the bytes of a `ty` at `addr`

## fun emit_offset

```mach
pub fun emit_offset(b: *Builder, addr: value.Value, bytes: value.Value) res[value.Value, fail.Fail];
```

the address `bytes` past `addr`

## fun emit_member

```mach
pub fun emit_member(b: *Builder, addr: value.Value, agg: Type, k: u32) res[value.Value, fail.Fail];
```

the address of member `k` of the aggregate `agg` at `addr`: a structure's member, an
array's or vector's element, and a union's every member, which all start where it does

## fun emit_element

```mach
pub fun emit_element(b: *Builder, addr: value.Value, agg: Type, ix: value.Value) res[value.Value, fail.Fail];
```

the address of element `ix` of the array or vector `agg` at `addr`

## fun emit_step

```mach
pub fun emit_step(b: *Builder, addr: value.Value, elem: Type, ix: value.Value) res[value.Value, fail.Fail];
```

the address `ix` elements of `elem` past `addr`, as a pointer steps

## fun emit_splat

```mach
pub fun emit_splat(b: *Builder, ty: Type, scalar: value.Value) res[value.Value, fail.Fail];
```

a vector of `ty` with `scalar` in every lane

## fun emit_vec_extract

```mach
pub fun emit_vec_extract(b: *Builder, vec: value.Value, lane: value.Value, ty: Type) res[value.Value, fail.Fail];
```

lane `lane` of `vec`, read as `ty`: a constant lane is read in place, any other through
the vector's bytes in a slot

## fun emit_vec_insert

```mach
pub fun emit_vec_insert(b: *Builder, vec: value.Value, lane: value.Value, x: value.Value) res[value.Value, fail.Fail];
```

## fun emit_vec_build

```mach
pub fun emit_vec_build(b: *Builder, ty: Type, lanes: *value.Value, n: u32) res[value.Value, fail.Fail];
```

a vector of `ty` from one value per lane

## fun emit_shuffle

```mach
pub fun emit_shuffle(b: *Builder, ty: Type, x: value.Value, y: value.Value, picks: *u32, n: u32) res[value.Value, fail.Fail];
```

lanes of `x` and `y` joined, picked by `picks`, as a vector of `ty`

## fun emit_reduce

```mach
pub fun emit_reduce(b: *Builder, code: Opcode, x: value.Value) res[value.Value, fail.Fail];
```

a reduction of a vector's lanes to one scalar

## fun emit_unary

```mach
pub fun emit_unary(b: *Builder, code: Opcode, x: value.Value) res[value.Value, fail.Fail];
```

a lanewise or scalar operation of one operand

## fun emit_binary

```mach
pub fun emit_binary(b: *Builder, code: Opcode, x: value.Value, y: value.Value) res[value.Value, fail.Fail];
```

a lanewise or scalar operation of two operands

## fun emit_ternary

```mach
pub fun emit_ternary(b: *Builder, code: Opcode, x: value.Value, y: value.Value, z: value.Value) res[value.Value, fail.Fail];
```

a lanewise or scalar operation of three operands

## fun emit_atomic_load

```mach
pub fun emit_atomic_load(b: *Builder, addr: value.Value, ty: Type, order: Ordering) res[value.Value, fail.Fail];
```

an atomic read of the integer `ty` at `addr`

## fun emit_atomic_store

```mach
pub fun emit_atomic_store(b: *Builder, addr: value.Value, x: value.Value, order: Ordering) err[fail.Fail];
```

## fun emit_atomic_rmw

```mach
pub fun emit_atomic_rmw(b: *Builder, code: Opcode, addr: value.Value, x: value.Value, order: Ordering) res[value.Value, fail.Fail];
```

an atomic read-modify-write, giving the value held before

## fun emit_atomic_cmpxchg

```mach
pub fun emit_atomic_cmpxchg(b: *Builder, addr: value.Value, expected: value.Value, desired: value.Value, success: Ordering, failure_order: Ordering) res[value.Value, fail.Fail];
```

an atomic compare and exchange, giving the value held before

## fun emit_fence

```mach
pub fun emit_fence(b: *Builder, order: Ordering) err[fail.Fail];
```

## fun emit_call

```mach
pub fun emit_call(b: *Builder, callee: value.Value, sig: Type, args: *value.Value, n: u32) res[value.Value, fail.Fail];
```

a call of `callee` through the signature `sig`. an aggregate argument is passed whole,
read from the address it travels at, and an aggregate result lands in a slot of its own

## fun param

```mach
pub fun param(b: *Builder, i: u32, ty: Type) res[value.Value, fail.Fail];
```

parameter `i` of the function being built, of `ty`: an aggregate lands in a slot of its
own, whose address is the value

## fun emit_br

```mach
pub fun emit_br(b: *Builder, target: id.Block) err[fail.Fail];
```

## fun emit_br_args

```mach
pub fun emit_br_args(b: *Builder, target: id.Block, args: *value.Value, n: u32) err[fail.Fail];
```

a branch to `target`, passing one argument per parameter of it

## fun emit_cbr

```mach
pub fun emit_cbr(b: *Builder, c: value.Value, then_b: id.Block, else_b: id.Block) err[fail.Fail];
```

## fun emit_switch

```mach
pub fun emit_switch(b: *Builder, selector: value.Value, otherwise: id.Block, cases: *u64, targets: *id.Block, n: u32) err[fail.Fail];
```

a branch on `sel` to the target of the case it equals, else to `otherwise`. `cases` holds
strictly increasing values, one per target

## fun emit_ret

```mach
pub fun emit_ret(b: *Builder, v: value.Value) err[fail.Fail];
```

return `v` from the function being built: an aggregate is read whole from its address

## fun emit_ret_void

```mach
pub fun emit_ret_void(b: *Builder) err[fail.Fail];
```

## fun emit_asm

```mach
pub fun emit_asm(b: *Builder, block: *Assembly, values: *value.Value, n: u32) err[fail.Fail];
```

an inline assembly block at the cursor, binding the `n` values at `values` in its operands'
order. the instruction copies the block, so it may be borrowed

## fun emit_target_op

```mach
pub fun emit_target_op(b: *Builder, op: *native.Op, values: *value.Value, n: u32) res[value.Value, fail.Fail];
```

the target op `op` at the cursor over the `n` values at `values`, in its operands' order,
its result the one `op` states or none. the instruction copies the op, so it may be borrowed

## fun emit_unreachable

```mach
pub fun emit_unreachable(b: *Builder) err[fail.Fail];
```

## fun add

```mach
pub fun add(b: *Builder, x: value.Value, y: value.Value) value.Value;
```

## fun sub

```mach
pub fun sub(b: *Builder, x: value.Value, y: value.Value) value.Value;
```

## fun mul

```mach
pub fun mul(b: *Builder, x: value.Value, y: value.Value) value.Value;
```

## fun band

```mach
pub fun band(b: *Builder, x: value.Value, y: value.Value) value.Value;
```

## fun bor

```mach
pub fun bor(b: *Builder, x: value.Value, y: value.Value) value.Value;
```

## fun bxor

```mach
pub fun bxor(b: *Builder, x: value.Value, y: value.Value) value.Value;
```

## fun shl

```mach
pub fun shl(b: *Builder, x: value.Value, y: value.Value) value.Value;
```

## fun shr_u

```mach
pub fun shr_u(b: *Builder, x: value.Value, y: value.Value) value.Value;
```

## fun shr_s

```mach
pub fun shr_s(b: *Builder, x: value.Value, y: value.Value) value.Value;
```

## fun eq

```mach
pub fun eq(b: *Builder, x: value.Value, y: value.Value) value.Value;
```

## fun ne

```mach
pub fun ne(b: *Builder, x: value.Value, y: value.Value) value.Value;
```

## fun lt_s

```mach
pub fun lt_s(b: *Builder, x: value.Value, y: value.Value) value.Value;
```

## fun lt_u

```mach
pub fun lt_u(b: *Builder, x: value.Value, y: value.Value) value.Value;
```

## fun le_u

```mach
pub fun le_u(b: *Builder, x: value.Value, y: value.Value) value.Value;
```

## fun trunc

```mach
pub fun trunc(b: *Builder, x: value.Value, to: Type) value.Value;
```

## fun zext

```mach
pub fun zext(b: *Builder, x: value.Value, to: Type) value.Value;
```

## fun sext

```mach
pub fun sext(b: *Builder, x: value.Value, to: Type) value.Value;
```

## fun bitcast

```mach
pub fun bitcast(b: *Builder, x: value.Value, to: Type) value.Value;
```

## fun br

```mach
pub fun br(b: *Builder, target: id.Block);
```

## fun cbr

```mach
pub fun cbr(b: *Builder, c: value.Value, then_b: id.Block, else_b: id.Block);
```

