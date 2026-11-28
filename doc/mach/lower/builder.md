[mach](../README.md) / [lower](README.md) / **builder**

# mach.lower.builder

the emission layer over mirl's builder: each operation lowering asks for, made at the
cursor in the function being built. a constant, function or global value becomes a mirl
value where an instruction uses it, an aggregate travels as the address of its storage
and is loaded whole only where a call or a return passes it, and a bool is a byte, so
a comparison is widened to one and a condition is compared with zero

Source: [`src/lower/builder.mach`](../../../src/lower/builder.mach)

Uses types from [`mach.base.fail`](../base/fail.md), [`mach.base.source`](../base/source.md), [`mach.lower.value`](value.md).

| Type | | |
| :--- | :--- | :--- |
| [`Builder`](#builder) | `rec` | the state lowering emits a module's code through |
| [`Opening`](#opening) | `rec` | where a scope of temporaries opened: past instruction `after` of `block`, or with `after` NO_INSTRUCTION at the block's start, past the entry's stack slots when it is the entry |
| [`Temps`](#temps) | `rec` | how the scoping of temporaries stood before a scope took it over (`temps_begin`) |

| On `Builder` | |
| :--- | :--- |
| [`init`](#init) | a builder over `m`, whose types the target's layout lays out under the build's context, with its stack slots in the address space `slots` |
| [`dnit`](#dnit) | release the builder's own storage, the module stays |
| [`fault`](#fault) | keep `f` as the builder's failure unless one is already kept, placed at the source the builder is lowering when `f` names no place of its own, since it is reported long after |
| [`failed`](#failed) | whether lowering or mirl's builder has kept a failure |
| [`done`](#done) | the first failure of the work: lowering's own, else the one mirl's builder kept |
| [`after_err_pub`](#after_err_pub) | mirl's refusal, once its builder has kept one, as lowering's failure |
| [`bind_storage`](#bind_storage) | bind a variable to the stack slot `slot` holds it in: the slot's alloca names it |
| [`state_publics`](#state_publics) | state the bytes of the stack slot `slot` that the source type of what it holds lays out as public, `ranges` as the module takes them: the public ranges of its alloca, which the walk of a constant-time function's code takes as typed frame extents, and every other byte of it as of a secret type |
| [`bind_value`](#bind_value) | bind a variable to `v` from the cursor's point on |
| [`take`](#take) | the value `r` holds, a failure kept as the builder's |
| [`take_type`](#take_type) | the type `r` holds, a failure kept as the builder's |
| [`take_err`](#take_err) | keep the failure `r` holds as the builder's |
| [`types_of`](#types_of) | the type table of the module being built |
| [`ptr_type`](#ptr_type) | a pointer into the storage a function's own slots live in, which a pointer the source names no other storage of points into |
| [`ptr_in`](#ptr_in) | a pointer into the address space `space` |
| [`slot_space`](#slot_space) | the address space a stack slot lives in, as mirl's target states it |
| [`global_space`](#global_space) | the address space global `ix` lives in: its storage class where the target's pointers name one, the default space elsewhere |
| [`classes`](#classes) | whether the target's pointers name a storage class each, so a function's pointer parameters take the classes of the storage their arguments point into |
| [`physical_space`](#physical_space) | the address space of a physical pointer where the target's pointers name a storage class each: the one space whose pointers the target models as addresses, as spir-v's PhysicalStorageBuffer |
| [`is_physical`](#is_physical) | whether `ty` is a physical pointer |
| [`int_type`](#int_type) | the integer type of `bits`, a failure kept as the builder's |
| [`index_type`](#index_type) | the integer as wide as a pointer, which addresses are compared and stepped in |
| [`size_of`](#size_of) | the byte size of `ty` under the target's layout, a failure kept as the builder's |
| [`align_of`](#align_of) | the alignment in bytes an access of `ty` states, a failure kept as the builder's |
| [`offset_of`](#offset_of) | the byte offset of member `member` of `ty`, a failure kept as the builder's |
| [`enter`](#enter) | build in function `f`, whose entry block takes `params`, each secret as `secrets` says |
| [`seal`](#seal) | end every block left open with unreachable: a block nothing branches to, such as the one after a return |
| [`leave`](#leave) | leave the function: nothing is built until the next is entered |
| [`in_function`](#in_function) | whether a function is being built |
| [`function`](#function) | the function being built |
| [`block_make`](#block_make) | a new block of no parameters in the function being built |
| [`block_make_with`](#block_make_with) | a block whose parameters are `params`, which every edge into it passes |
| [`block_param`](#block_param) | parameter `i` of block `k`, typed `ty` |
| [`set_block`](#set_block) | move the cursor to the end of block `k` |
| [`block_terminated`](#block_terminated) | whether block `k` already ends |
| [`terminated`](#terminated) | whether the block being built already ends |
| [`set_loc`](#set_loc) | the source location of what is built next, recorded for debug info when it is on |
| [`current_loc`](#current_loc) | the source location of what is being built |
| [`location_of`](#location_of) | the debug table's location of a source location, none when it has no file |
| [`set_secret`](#set_secret) | whether the next instructions are marked secret: a load, call or atomic read of a secret |
| [`set_access`](#set_access) | the next accesses through an address state the memory they read or write as the source type says: secret, which marks them so, or public |
| [`end_access`](#end_access) | the next accesses state nothing of the memory they reach |
| [`set_volatile`](#set_volatile) | whether the next memory accesses are volatile |
| [`set_kept`](#set_kept) | whether the next writes are kept: a wipe of memory that held a secret |
| [`spec_constant`](#spec_constant) | the specialization constant `spec_id`, whose default is the integer or float constant `default` |
| [`constant_of`](#constant_of) | the module constant a constant value stands for |
| [`use`](#use) | the mirl value `v` stands for in the function being built |
| [`emit_function_address`](#emit_function_address) | the address of function value `v` as a value of the function being built |
| [`emit_store_marked`](#emit_store_marked) | a store marked volatile where `vol` holds: an access of a volatile place |
| [`emit_load_marked`](#emit_load_marked) | a load marked volatile where `vol` holds |
| [`emit_add`](#emit_add) | the sum of `x` and `y`: a pointer and an integer step the pointer by that many bytes, floats add as floats |
| [`emit_sub`](#emit_sub) | `x` minus `y`: a pointer less an integer steps the pointer back by that many bytes, two pointers give the difference of their addresses, floats subtract as floats |
| [`emit_mul`](#emit_mul) | the product of `x` and `y`, floats multiplied as floats |
| [`emit_div_s`](#emit_div_s) | `x` divided by `y` as signed integers, floats divided as floats |
| [`emit_div_u`](#emit_div_u) | `x` divided by `y` as unsigned integers, floats divided as floats |
| [`emit_rem_s`](#emit_rem_s) | the remainder of `x` divided by `y` as signed integers, floats as floats |
| [`emit_rem_u`](#emit_rem_u) | the remainder of `x` divided by `y` as unsigned integers, floats as floats |
| [`emit_and`](#emit_and) | the bitwise and of `x` and `y` |
| [`emit_or`](#emit_or) | the bitwise or of `x` and `y` |
| [`emit_xor`](#emit_xor) | the bitwise exclusive or of `x` and `y` |
| [`emit_shl`](#emit_shl) | `x` shifted left by `n`, the count resized or splat to `x`'s type |
| [`emit_shr_s`](#emit_shr_s) | `x` shifted right, filling with its sign bit by `n`, the count resized or splat to `x`'s type |
| [`emit_shr_u`](#emit_shr_u) | `x` shifted right, filling with zeros by `n`, the count resized or splat to `x`'s type |
| [`emit_neg`](#emit_neg) | `x` negated, a float as a float |
| [`emit_not`](#emit_not) | the bitwise complement of `x` |
| [`emit_cmp_eq`](#emit_cmp_eq) | whether `x` equals `y`, as a bool byte: pointers compare their addresses, floats compared ordered |
| [`emit_cmp_ne`](#emit_cmp_ne) | whether `x` differs from `y`, as a bool byte: pointers compare their addresses, floats compared unordered, so a nan differs |
| [`emit_cmp_lt_s`](#emit_cmp_lt_s) | whether `x` is less than `y` as signed integers, as a bool byte: pointers compare their addresses, floats compared ordered |
| [`emit_cmp_lt_u`](#emit_cmp_lt_u) | whether `x` is less than `y` as unsigned integers, as a bool byte: pointers compare their addresses, floats compared ordered |
| [`emit_cmp_le_s`](#emit_cmp_le_s) | whether `x` is at most `y` as signed integers, as a bool byte: pointers compare their addresses, floats compared ordered |
| [`emit_cmp_le_u`](#emit_cmp_le_u) | whether `x` is at most `y` as unsigned integers, as a bool byte: pointers compare their addresses, floats compared ordered |
| [`emit_cmp_gt_s`](#emit_cmp_gt_s) | whether `x` is greater than `y` as signed integers, as a bool byte: pointers compare their addresses, floats compared ordered |
| [`emit_cmp_gt_u`](#emit_cmp_gt_u) | whether `x` is greater than `y` as unsigned integers, as a bool byte: pointers compare their addresses, floats compared ordered |
| [`emit_cmp_ge_s`](#emit_cmp_ge_s) | whether `x` is at least `y` as signed integers, as a bool byte: pointers compare their addresses, floats compared ordered |
| [`emit_cmp_ge_u`](#emit_cmp_ge_u) | whether `x` is at least `y` as unsigned integers, as a bool byte: pointers compare their addresses, floats compared ordered |
| [`emit_vcompare`](#emit_vcompare) | a lanewise comparison of two vectors as a mask of `result_ty`, every bit of a lane set where it holds |
| [`truth`](#truth) | an i1 that holds where a bool byte, or any integer, is not zero |
| [`resize_int`](#resize_int) | an integer of `to`'s width from `x`: kept, truncated or extended by `signed` |
| [`emit_trunc`](#emit_trunc) | `x` resized to the integer width of `to`, truncated or zero extended |
| [`emit_sext`](#emit_sext) | `x` resized to the integer width of `to`, truncated or sign extended |
| [`emit_zext`](#emit_zext) | `x` resized to the integer width of `to`, truncated or zero extended |
| [`emit_fp_trunc`](#emit_fp_trunc) | `x` demoted to the float type `to`, `x` itself when it already is one |
| [`emit_fp_ext`](#emit_fp_ext) | `x` promoted to the float type `to`, `x` itself when it already is one |
| [`emit_fp_resize`](#emit_fp_resize) | a float of `to`'s format from `x`, promoted or demoted |
| [`emit_fp_to_si`](#emit_fp_to_si) | the float `x` converted to the signed integer type `to` |
| [`emit_fp_to_ui`](#emit_fp_to_ui) | the float `x` converted to the unsigned integer type `to` |
| [`emit_si_to_fp`](#emit_si_to_fp) | the signed integer `x` converted to the float type `to` |
| [`emit_ui_to_fp`](#emit_ui_to_fp) | the unsigned integer `x` converted to the float type `to` |
| [`emit_bitcast`](#emit_bitcast) | the same bits as `to`: an address read as an integer and back, a resize between integers of different widths, or a reinterpretation of equal widths |
| [`emit_classify`](#emit_classify) | `x` made secret: a block parameter declared secret, which the branch into it passes `x` |
| [`emit_declassify`](#emit_declassify) | `x` made public: a constant as it is, an aggregate copied a word at a time into a public slot of its own, any other value through a declassify |
| [`emit_alloca`](#emit_alloca) | a stack slot of `ty` for a temporary, made in the entry block so every path reaches it (`temp_alloca`) |
| [`temps_begin`](#temps_begin) | open the scope of temporaries of a statement or a condition about to be lowered, here |
| [`temps_mark`](#temps_mark) | how many temporaries have begun, where a loop's body starts: a `brk` or `cnt` ends those begun past it |
| [`temps_close`](#temps_close) | end the storage of every temporary begun past `from`, the latest first, here, where control still reaches |
| [`temps_drop`](#temps_drop) | give the scoping back to the enclosing scope, the closed scope's temporaries ended on every way out of it already |
| [`temps_end`](#temps_end) | close the scope `kept` was taken for here and give the scoping back to the enclosing scope |
| [`temp_alloca`](#temp_alloca) | a stack slot of `ty` aligned to `align` for a temporary of the scope being lowered, made in the entry block, its storage begun where the scope opened (`temps_begin`) |
| [`alloca_aligned`](#alloca_aligned) | a stack slot of `ty` aligned to `align`, made in the entry block so every path reaches it |
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
| [`emit_vec_insert`](#emit_vec_insert) | `vec` with lane `lane` replaced by `x`: a constant lane in place, any other through the vector's bytes in a slot |
| [`emit_vec_build`](#emit_vec_build) | a vector of `ty` from one value per lane |
| [`emit_shuffle`](#emit_shuffle) | lanes of `x` and `y` joined, picked by `picks`, as a vector of `ty` |
| [`emit_operation`](#emit_operation) | the operation `code` at the cursor over the `n` values at `values`, its operands in order |
| [`emit_atomic_load`](#emit_atomic_load) | an atomic read of the integer `ty` at `addr` |
| [`emit_atomic_store`](#emit_atomic_store) | an atomic write of `x` to `addr` |
| [`emit_atomic_rmw`](#emit_atomic_rmw) | an atomic read-modify-write, giving the value held before |
| [`emit_atomic_cmpxchg`](#emit_atomic_cmpxchg) | an atomic compare and exchange, giving the value held before |
| [`emit_scope`](#emit_scope) | where the storage of `slot`, an alloca's result, begins or ends, unmarked whatever the builder marks |
| [`emit_fence`](#emit_fence) | a memory fence of `order` |
| [`emit_call`](#emit_call) | a call of `callee` through the signature `sig`, every argument named |
| [`emit_call_named`](#emit_call_named) | a call of `callee` through the signature `sig`, of which `named` states how many arguments are named, the rest being a C-variadic call's unnamed ones |
| [`call_states`](#call_states) | the declaration the next call, one through a value of a function type marked constant time, states its callee keeps (`lower_context.stated_callee`) |
| [`result_whole`](#result_whole) | hold the next call's aggregate result whole, with no slot, unless it lands in a destination |
| [`result_into`](#result_into) | land the next call's aggregate result in `slot`, storage of the result's type nothing reads yet |
| [`param`](#param) | parameter `i` of the function being built, of `ty`: an aggregate held `whole` is the parameter itself, and any other lands in a slot of its own, whose address is the value |
| [`emit_br`](#emit_br) | end the block with a branch to `target`, which takes no arguments |
| [`emit_br_args`](#emit_br_args) | a branch to `target`, passing one argument per parameter of it |
| [`emit_cbr`](#emit_cbr) | end the block with a branch on `c`, compared with zero, to `then_b` or `else_b` |
| [`emit_switch`](#emit_switch) | a branch on `selector` to the target of the case it equals, else to `otherwise` |
| [`emit_ret`](#emit_ret) | return `v` from the function being built: an aggregate is read whole from its address, as secret as `v` is |
| [`emit_ret_void`](#emit_ret_void) | end the block with a return of no value |
| [`emit_asm`](#emit_asm) | an inline assembly block at the cursor, binding the `n` values at `values` in its operands' order, the results of the operands that give one written to `results` in that order |
| [`emit_target_op`](#emit_target_op) | the target op `op` at the cursor over the `n` values at `values`, in its operands' order, its result the one `op` states or none, a pointer's in the space its caller places it |
| [`emit_unreachable`](#emit_unreachable) | end the block as unreachable |
| [`add`](#add) | emit_add with a failure kept as the builder's |
| [`sub`](#sub) | emit_sub with a failure kept as the builder's |
| [`mul`](#mul) | emit_mul with a failure kept as the builder's |
| [`band`](#band) | emit_and with a failure kept as the builder's |
| [`bor`](#bor) | emit_or with a failure kept as the builder's |
| [`bxor`](#bxor) | emit_xor with a failure kept as the builder's |
| [`shl`](#shl) | emit_shl with a failure kept as the builder's |
| [`shr_u`](#shr_u) | emit_shr_u with a failure kept as the builder's |
| [`shr_s`](#shr_s) | emit_shr_s with a failure kept as the builder's |
| [`eq`](#eq) | emit_cmp_eq with a failure kept as the builder's |
| [`ne`](#ne) | emit_cmp_ne with a failure kept as the builder's |
| [`lt_s`](#lt_s) | emit_cmp_lt_s with a failure kept as the builder's |
| [`lt_u`](#lt_u) | emit_cmp_lt_u with a failure kept as the builder's |
| [`le_u`](#le_u) | emit_cmp_le_u with a failure kept as the builder's |
| [`trunc`](#trunc) | emit_trunc with a failure kept as the builder's |
| [`zext`](#zext) | emit_zext with a failure kept as the builder's |
| [`sext`](#sext) | emit_sext with a failure kept as the builder's |
| [`bitcast`](#bitcast) | emit_bitcast with a failure kept as the builder's |
| [`br`](#br) | emit_br with a failure kept as the builder's |
| [`cbr`](#cbr) | emit_cbr with a failure kept as the builder's |

| Functions | |
| :--- | :--- |
| [`module_refusal`](#module_refusal) | mirl's refusal of what `what` names, as lowering's failure: storage refused where it stands for that, else a defect of the lowering |

---

## Builder

```mach
pub rec Builder {
    m:      *mirl.ir.module.Module;
    a:      *std.allocator.Allocator;
    mb:     mirl.build.builder.Builder;
    target: *mirl.target.record.Target;
    layout: mirl.target.layout.Layout;
    slots:  u32;
    fn:     opt[mirl.ir.id.Function];
    entry:  mirl.ir.id.Block;
    block:  mirl.ir.id.Block;
    # the last stack slot made at the head of the entry, none yet
    slot_last: mirl.ir.id.Instruction;
    # the slot the next call's aggregate result lands in, in place of one of its own
    into:     mach.lower.value.Value;
    into_set: bool;
    # the next call's aggregate result is held whole, with no slot
    whole: bool;
    # the declaration the next call, one through a value, states its callee keeps
    callee:   opt[mirl.ir.id.Stated];
    ended:    std.collections.vector.Vector[bool];
    secret:   bool;
    stated:   bool;
    volatile: bool;
    loc:      mach.base.source.Location;
    debug:    bool;
    sources:  *mach.base.source.SourceMap;
    files:    std.collections.vector.Vector[u32];
    failure:  opt[mach.base.fail.Fail];
    # where the scope of the statement or condition being lowered opened, none outside one (`temps_begin`)
    temp_open: opt[Opening];
    # the temporaries whose storage has begun and not ended, the innermost scope's last
    temps: std.collections.vector.Vector[mach.lower.value.Value];
}
```

the state lowering emits a module's code through

| Field | Type | |
| :--- | :--- | :--- |
| `m` | `*mirl.ir.module.Module` | the module being built |
| `a` | `*std.allocator.Allocator` | allocator for the builder's own scratch |
| `mb` | `mirl.build.builder.Builder` | mirl's builder, which holds the cursor |
| `target` | `*mirl.target.record.Target` | the target the module is built for |
| `layout` | `mirl.target.layout.Layout` | the target's layout of the module's types |
| `slots` | `u32` | the address space stack slots live in |
| `fn` | `opt[mirl.ir.id.Function]` | the function being built, none between functions |
| `entry` | `mirl.ir.id.Block` | the entry block of the function being built |
| `block` | `mirl.ir.id.Block` | the block the cursor is in |
| `slot_last` | `mirl.ir.id.Instruction` | the last stack slot made at the head of the entry block, NO_INSTRUCTION before the first |
| `into` | `mach.lower.value.Value` | the slot the next call's aggregate result lands in |
| `into_set` | `bool` | whether into is set |
| `whole` | `bool` | the next call's aggregate result is held whole |
| `callee` | `opt[mirl.ir.id.Stated]` | the declaration the next call, one through a value, states its callee keeps |
| `ended` | `std.collections.vector.Vector[bool]` | per block, whether it already ends |
| `secret` | `bool` | whether the next instructions are marked secret |
| `stated` | `bool` | whether the next accesses state the memory they reach |
| `volatile` | `bool` | whether the next memory accesses are volatile |
| `loc` | `mach.base.source.Location` | the source location of what is being built |
| `debug` | `bool` | whether locations are recorded for debug info |
| `sources` | `*mach.base.source.SourceMap` | the source files locations are read against |
| `files` | `std.collections.vector.Vector[u32]` | per source file, its debug table file, NO_FILE before one is made |
| `failure` | `opt[mach.base.fail.Fail]` | lowering's first failure, kept until done |

### init

```mach
pub fun init(m: *mirl.ir.module.Module, a: *std.allocator.Allocator, target: *mirl.target.record.Target, ctx: opt[*masc.core.state.Context], slots: u32, sources: *mach.base.source.SourceMap, debug_info: bool) Builder
```

a builder over `m`, whose types the target's layout lays out under the build's context,
with its stack slots in the address space `slots`

### dnit

```mach
pub fun dnit(b: *Builder)
```

release the builder's own storage, the module stays

| Parameter | |
| :--- | :--- |
| `b` | the builder |

### fault

```mach
pub fun fault(b: *Builder, f: mach.base.fail.Fail)
```

keep `f` as the builder's failure unless one is already kept, placed at the source the builder
is lowering when `f` names no place of its own, since it is reported long after

| Parameter | |
| :--- | :--- |
| `b` | the builder |
| `f` | the failure |

### failed

```mach
pub fun failed(b: *Builder) bool
```

whether lowering or mirl's builder has kept a failure

| Parameter | |
| :--- | :--- |
| `b` | the builder |
| **returns** | true once either has failed |

### done

```mach
pub fun done(b: *Builder) err[mach.base.fail.Fail]
```

the first failure of the work: lowering's own, else the one mirl's builder kept

### after_err_pub

```mach
pub fun after_err_pub(b: *Builder) err[mach.base.fail.Fail]
```

mirl's refusal, once its builder has kept one, as lowering's failure

| Parameter | |
| :--- | :--- |
| `b` | the builder |
| **returns** | nothing, or the refusal |

### bind_storage

```mach
pub fun bind_storage(b: *Builder, slot: mach.lower.value.Value, binding: mirl.ir.debug.record.Binding) err[mach.base.fail.Fail]
```

bind a variable to the stack slot `slot` holds it in: the slot's alloca names it

### state_publics

```mach
pub fun state_publics(b: *Builder, slot: mach.lower.value.Value, ranges: Slice[mirl.ir.global.Range]) err[mach.base.fail.Fail]
```

state the bytes of the stack slot `slot` that the source type of what it holds lays out as public, `ranges` as the
module takes them: the public ranges of its alloca, which the walk of a constant-time function's code takes as typed
frame extents, and every other byte of it as of a secret type. none states every byte so

### bind_value

```mach
pub fun bind_value(b: *Builder, binding: mirl.ir.debug.record.Binding, v: mach.lower.value.Value)
```

bind a variable to `v` from the cursor's point on

### take

```mach
pub fun take(b: *Builder, r: res[mach.lower.value.Value, mach.base.fail.Fail]) mach.lower.value.Value
```

the value `r` holds, a failure kept as the builder's

| Parameter | |
| :--- | :--- |
| `b` | the builder |
| `r` | a result of an emission |
| **returns** | the value, or none on failure |

### take_type

```mach
pub fun take_type(b: *Builder, r: res[Type, mach.base.fail.Fail]) Type
```

the type `r` holds, a failure kept as the builder's

| Parameter | |
| :--- | :--- |
| `b` | the builder |
| `r` | a result naming a type |
| **returns** | the type, or irt.NONE on failure |

### take_err

```mach
pub fun take_err(b: *Builder, r: err[mach.base.fail.Fail])
```

keep the failure `r` holds as the builder's

| Parameter | |
| :--- | :--- |
| `b` | the builder |
| `r` | an outcome of an emission |

### types_of

```mach
pub fun types_of(b: *Builder) *mirl.ir.type.table.Table
```

the type table of the module being built

| Parameter | |
| :--- | :--- |
| `b` | the builder |
| **returns** | the table, borrowed from the module |

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

### physical_space

```mach
pub fun physical_space(b: *Builder) opt[u32]
```

the address space of a physical pointer where the target's pointers name a storage class
each: the one space whose pointers the target models as addresses, as spir-v's
PhysicalStorageBuffer. a pointer the source holds in memory, returns, makes from an
integer or spells `nil` is of this space (types.md, "pointers on spir-v")

### is_physical

```mach
pub fun is_physical(b: *Builder, ty: Type) bool
```

whether `ty` is a physical pointer

### int_type

```mach
pub fun int_type(b: *Builder, bits: u32) Type
```

the integer type of `bits`, a failure kept as the builder's

| Parameter | |
| :--- | :--- |
| `b` | the builder |
| `bits` | width in bits |
| **returns** | the type, or irt.NONE on failure |

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

the byte size of `ty` under the target's layout, a failure kept as the builder's

| Parameter | |
| :--- | :--- |
| `b` | the builder |
| `ty` | the type |
| **returns** | the size, or 0 on failure |

### align_of

```mach
pub fun align_of(b: *Builder, ty: Type) u64
```

the alignment in bytes an access of `ty` states, a failure kept as the builder's

| Parameter | |
| :--- | :--- |
| `b` | the builder |
| `ty` | the type |
| **returns** | the alignment, 1 for any pointer where pointers name a storage class, for a handle, or on failure |

### offset_of

```mach
pub fun offset_of(b: *Builder, ty: Type, member: u32) u64
```

the byte offset of member `member` of `ty`, a failure kept as the builder's

| Parameter | |
| :--- | :--- |
| `b` | the builder |
| `ty` | the aggregate type |
| `member` | the member's index |
| **returns** | the offset, or 0 on failure |

### enter

```mach
pub fun enter(b: *Builder, f: mirl.ir.id.Function, params: *Type, secrets: *bool, n: u32) mirl.ir.id.Block
```

build in function `f`, whose entry block takes `params`, each secret as `secrets` says

### seal

```mach
pub fun seal(b: *Builder) err[mach.base.fail.Fail]
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

whether a function is being built

| Parameter | |
| :--- | :--- |
| `b` | the builder |
| **returns** | true between enter and leave |

### function

```mach
pub fun function(b: *Builder) mirl.ir.id.Function
```

the function being built

| Parameter | |
| :--- | :--- |
| `b` | the builder |
| **returns** | its id, or 0xFFFFFFFF outside a function |

### block_make

```mach
pub fun block_make(b: *Builder) res[mirl.ir.id.Block, mach.base.fail.Fail]
```

a new block of no parameters in the function being built

| Parameter | |
| :--- | :--- |
| `b` | the builder |
| **returns** | the block, or the builder's failure |

### block_make_with

```mach
pub fun block_make_with(b: *Builder, params: *Type, secrets: *bool, n: u32) mirl.ir.id.Block
```

a block whose parameters are `params`, which every edge into it passes

### block_param

```mach
pub fun block_param(b: *Builder, k: mirl.ir.id.Block, i: u32, ty: Type) mach.lower.value.Value
```

parameter `i` of block `k`, typed `ty`

### set_block

```mach
pub fun set_block(b: *Builder, k: mirl.ir.id.Block)
```

move the cursor to the end of block `k`

| Parameter | |
| :--- | :--- |
| `b` | the builder |
| `k` | the block to build in |

### block_terminated

```mach
pub fun block_terminated(b: *Builder, k: mirl.ir.id.Block) bool
```

whether block `k` already ends

| Parameter | |
| :--- | :--- |
| `b` | the builder |
| `k` | the block |
| **returns** | true once a terminator was built in it |

### terminated

```mach
pub fun terminated(b: *Builder) bool
```

whether the block being built already ends

### set_loc

```mach
pub fun set_loc(b: *Builder, loc: mach.base.source.Location)
```

the source location of what is built next, recorded for debug info when it is on

| Parameter | |
| :--- | :--- |
| `b` | the builder |
| `loc` | the location |

### current_loc

```mach
pub fun current_loc(b: *Builder) mach.base.source.Location
```

the source location of what is being built

| Parameter | |
| :--- | :--- |
| `b` | the builder |
| **returns** | the location set_loc last set |

### location_of

```mach
pub fun location_of(b: *Builder, loc: mach.base.source.Location) opt[mirl.ir.debug.record.Location]
```

the debug table's location of a source location, none when it has no file

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
pub fun spec_constant(b: *Builder, spec_id: u32, default: mach.lower.value.Value) mach.lower.value.Value
```

the specialization constant `spec_id`, whose default is the integer or float constant `default`

### constant_of

```mach
pub fun constant_of(b: *Builder, v: mach.lower.value.Value) mirl.ir.id.Constant
```

the module constant a constant value stands for

| Parameter | |
| :--- | :--- |
| `b` | the builder |
| `v` | an integer, float, nil, constant, global or function value |
| **returns** | the constant, or 0xFFFFFFFF with the builder faulted for any other value |

### use

```mach
pub fun use(b: *Builder, v: mach.lower.value.Value) mirl.ir.id.Value
```

the mirl value `v` stands for in the function being built

### emit_function_address

```mach
pub fun emit_function_address(b: *Builder, v: mach.lower.value.Value) res[mach.lower.value.Value, mach.base.fail.Fail]
```

the address of function value `v` as a value of the function being built

| Parameter | |
| :--- | :--- |
| `b` | the builder |
| `v` | a function value |
| **returns** | the address, or the failure mirl's builder kept |

### emit_store_marked

```mach
pub fun emit_store_marked(b: *Builder, v: mach.lower.value.Value, ptr: mach.lower.value.Value, vol: bool) err[mach.base.fail.Fail]
```

a store marked volatile where `vol` holds: an access of a volatile place

### emit_load_marked

```mach
pub fun emit_load_marked(b: *Builder, ptr: mach.lower.value.Value, ty: Type, vol: bool) res[mach.lower.value.Value, mach.base.fail.Fail]
```

a load marked volatile where `vol` holds

### emit_add

```mach
pub fun emit_add(b: *Builder, x: mach.lower.value.Value, y: mach.lower.value.Value) res[mach.lower.value.Value, mach.base.fail.Fail]
```

the sum of `x` and `y`: a pointer and an integer step the pointer by that many
bytes, floats add as floats

| Parameter | |
| :--- | :--- |
| `b` | the builder |
| `x` | the left operand |
| `y` | the right operand |
| **returns** | the result, or the failure mirl's builder kept |

### emit_sub

```mach
pub fun emit_sub(b: *Builder, x: mach.lower.value.Value, y: mach.lower.value.Value) res[mach.lower.value.Value, mach.base.fail.Fail]
```

`x` minus `y`: a pointer less an integer steps the pointer back by that many bytes,
two pointers give the difference of their addresses, floats subtract as floats

| Parameter | |
| :--- | :--- |
| `b` | the builder |
| `x` | the left operand |
| `y` | the right operand |
| **returns** | the result, or the refusal of a logical pointer's address or mirl's failure |

### emit_mul

```mach
pub fun emit_mul(b: *Builder, x: mach.lower.value.Value, y: mach.lower.value.Value) res[mach.lower.value.Value, mach.base.fail.Fail]
```

the product of `x` and `y`, floats multiplied as floats

| Parameter | |
| :--- | :--- |
| `b` | the builder |
| `x` | the left operand |
| `y` | the right operand |
| **returns** | the result, or the failure mirl's builder kept |

### emit_div_s

```mach
pub fun emit_div_s(b: *Builder, x: mach.lower.value.Value, y: mach.lower.value.Value) res[mach.lower.value.Value, mach.base.fail.Fail]
```

`x` divided by `y` as signed integers, floats divided as floats

| Parameter | |
| :--- | :--- |
| `b` | the builder |
| `x` | the left operand |
| `y` | the right operand |
| **returns** | the result, or the failure mirl's builder kept |

### emit_div_u

```mach
pub fun emit_div_u(b: *Builder, x: mach.lower.value.Value, y: mach.lower.value.Value) res[mach.lower.value.Value, mach.base.fail.Fail]
```

`x` divided by `y` as unsigned integers, floats divided as floats

| Parameter | |
| :--- | :--- |
| `b` | the builder |
| `x` | the left operand |
| `y` | the right operand |
| **returns** | the result, or the failure mirl's builder kept |

### emit_rem_s

```mach
pub fun emit_rem_s(b: *Builder, x: mach.lower.value.Value, y: mach.lower.value.Value) res[mach.lower.value.Value, mach.base.fail.Fail]
```

the remainder of `x` divided by `y` as signed integers, floats as floats

| Parameter | |
| :--- | :--- |
| `b` | the builder |
| `x` | the left operand |
| `y` | the right operand |
| **returns** | the result, or the failure mirl's builder kept |

### emit_rem_u

```mach
pub fun emit_rem_u(b: *Builder, x: mach.lower.value.Value, y: mach.lower.value.Value) res[mach.lower.value.Value, mach.base.fail.Fail]
```

the remainder of `x` divided by `y` as unsigned integers, floats as floats

| Parameter | |
| :--- | :--- |
| `b` | the builder |
| `x` | the left operand |
| `y` | the right operand |
| **returns** | the result, or the failure mirl's builder kept |

### emit_and

```mach
pub fun emit_and(b: *Builder, x: mach.lower.value.Value, y: mach.lower.value.Value) res[mach.lower.value.Value, mach.base.fail.Fail]
```

the bitwise and of `x` and `y`

| Parameter | |
| :--- | :--- |
| `b` | the builder |
| `x` | the left operand |
| `y` | the right operand |
| **returns** | the result, or the failure mirl's builder kept |

### emit_or

```mach
pub fun emit_or(b: *Builder, x: mach.lower.value.Value, y: mach.lower.value.Value) res[mach.lower.value.Value, mach.base.fail.Fail]
```

the bitwise or of `x` and `y`

| Parameter | |
| :--- | :--- |
| `b` | the builder |
| `x` | the left operand |
| `y` | the right operand |
| **returns** | the result, or the failure mirl's builder kept |

### emit_xor

```mach
pub fun emit_xor(b: *Builder, x: mach.lower.value.Value, y: mach.lower.value.Value) res[mach.lower.value.Value, mach.base.fail.Fail]
```

the bitwise exclusive or of `x` and `y`

| Parameter | |
| :--- | :--- |
| `b` | the builder |
| `x` | the left operand |
| `y` | the right operand |
| **returns** | the result, or the failure mirl's builder kept |

### emit_shl

```mach
pub fun emit_shl(b: *Builder, x: mach.lower.value.Value, n: mach.lower.value.Value) res[mach.lower.value.Value, mach.base.fail.Fail]
```

`x` shifted left by `n`, the count resized or splat to `x`'s type

| Parameter | |
| :--- | :--- |
| `b` | the builder |
| `x` | the value shifted |
| `n` | the shift count |
| **returns** | the result, or the failure mirl's builder kept |

### emit_shr_s

```mach
pub fun emit_shr_s(b: *Builder, x: mach.lower.value.Value, n: mach.lower.value.Value) res[mach.lower.value.Value, mach.base.fail.Fail]
```

`x` shifted right, filling with its sign bit by `n`, the count resized or splat to `x`'s type

| Parameter | |
| :--- | :--- |
| `b` | the builder |
| `x` | the value shifted |
| `n` | the shift count |
| **returns** | the result, or the failure mirl's builder kept |

### emit_shr_u

```mach
pub fun emit_shr_u(b: *Builder, x: mach.lower.value.Value, n: mach.lower.value.Value) res[mach.lower.value.Value, mach.base.fail.Fail]
```

`x` shifted right, filling with zeros by `n`, the count resized or splat to `x`'s type

| Parameter | |
| :--- | :--- |
| `b` | the builder |
| `x` | the value shifted |
| `n` | the shift count |
| **returns** | the result, or the failure mirl's builder kept |

### emit_neg

```mach
pub fun emit_neg(b: *Builder, x: mach.lower.value.Value) res[mach.lower.value.Value, mach.base.fail.Fail]
```

`x` negated, a float as a float

| Parameter | |
| :--- | :--- |
| `b` | the builder |
| `x` | the operand |
| **returns** | the result, or the failure mirl's builder kept |

### emit_not

```mach
pub fun emit_not(b: *Builder, x: mach.lower.value.Value) res[mach.lower.value.Value, mach.base.fail.Fail]
```

the bitwise complement of `x`

| Parameter | |
| :--- | :--- |
| `b` | the builder |
| `x` | the operand |
| **returns** | the result, or the failure mirl's builder kept |

### emit_cmp_eq

```mach
pub fun emit_cmp_eq(b: *Builder, x: mach.lower.value.Value, y: mach.lower.value.Value) res[mach.lower.value.Value, mach.base.fail.Fail]
```

whether `x` equals `y`, as a bool byte: pointers compare their addresses,
floats compared ordered

| Parameter | |
| :--- | :--- |
| `b` | the builder |
| `x` | the left operand |
| `y` | the right operand |
| **returns** | the bool byte, or the refusal of a logical pointer's address or mirl's failure |

### emit_cmp_ne

```mach
pub fun emit_cmp_ne(b: *Builder, x: mach.lower.value.Value, y: mach.lower.value.Value) res[mach.lower.value.Value, mach.base.fail.Fail]
```

whether `x` differs from `y`, as a bool byte: pointers compare their addresses,
floats compared unordered, so a nan differs

| Parameter | |
| :--- | :--- |
| `b` | the builder |
| `x` | the left operand |
| `y` | the right operand |
| **returns** | the bool byte, or the refusal of a logical pointer's address or mirl's failure |

### emit_cmp_lt_s

```mach
pub fun emit_cmp_lt_s(b: *Builder, x: mach.lower.value.Value, y: mach.lower.value.Value) res[mach.lower.value.Value, mach.base.fail.Fail]
```

whether `x` is less than `y` as signed integers, as a bool byte: pointers compare their addresses,
floats compared ordered

| Parameter | |
| :--- | :--- |
| `b` | the builder |
| `x` | the left operand |
| `y` | the right operand |
| **returns** | the bool byte, or the refusal of a logical pointer's address or mirl's failure |

### emit_cmp_lt_u

```mach
pub fun emit_cmp_lt_u(b: *Builder, x: mach.lower.value.Value, y: mach.lower.value.Value) res[mach.lower.value.Value, mach.base.fail.Fail]
```

whether `x` is less than `y` as unsigned integers, as a bool byte: pointers compare their addresses,
floats compared ordered

| Parameter | |
| :--- | :--- |
| `b` | the builder |
| `x` | the left operand |
| `y` | the right operand |
| **returns** | the bool byte, or the refusal of a logical pointer's address or mirl's failure |

### emit_cmp_le_s

```mach
pub fun emit_cmp_le_s(b: *Builder, x: mach.lower.value.Value, y: mach.lower.value.Value) res[mach.lower.value.Value, mach.base.fail.Fail]
```

whether `x` is at most `y` as signed integers, as a bool byte: pointers compare their addresses,
floats compared ordered

| Parameter | |
| :--- | :--- |
| `b` | the builder |
| `x` | the left operand |
| `y` | the right operand |
| **returns** | the bool byte, or the refusal of a logical pointer's address or mirl's failure |

### emit_cmp_le_u

```mach
pub fun emit_cmp_le_u(b: *Builder, x: mach.lower.value.Value, y: mach.lower.value.Value) res[mach.lower.value.Value, mach.base.fail.Fail]
```

whether `x` is at most `y` as unsigned integers, as a bool byte: pointers compare their addresses,
floats compared ordered

| Parameter | |
| :--- | :--- |
| `b` | the builder |
| `x` | the left operand |
| `y` | the right operand |
| **returns** | the bool byte, or the refusal of a logical pointer's address or mirl's failure |

### emit_cmp_gt_s

```mach
pub fun emit_cmp_gt_s(b: *Builder, x: mach.lower.value.Value, y: mach.lower.value.Value) res[mach.lower.value.Value, mach.base.fail.Fail]
```

whether `x` is greater than `y` as signed integers, as a bool byte: pointers compare their addresses,
floats compared ordered

| Parameter | |
| :--- | :--- |
| `b` | the builder |
| `x` | the left operand |
| `y` | the right operand |
| **returns** | the bool byte, or the refusal of a logical pointer's address or mirl's failure |

### emit_cmp_gt_u

```mach
pub fun emit_cmp_gt_u(b: *Builder, x: mach.lower.value.Value, y: mach.lower.value.Value) res[mach.lower.value.Value, mach.base.fail.Fail]
```

whether `x` is greater than `y` as unsigned integers, as a bool byte: pointers compare their addresses,
floats compared ordered

| Parameter | |
| :--- | :--- |
| `b` | the builder |
| `x` | the left operand |
| `y` | the right operand |
| **returns** | the bool byte, or the refusal of a logical pointer's address or mirl's failure |

### emit_cmp_ge_s

```mach
pub fun emit_cmp_ge_s(b: *Builder, x: mach.lower.value.Value, y: mach.lower.value.Value) res[mach.lower.value.Value, mach.base.fail.Fail]
```

whether `x` is at least `y` as signed integers, as a bool byte: pointers compare their addresses,
floats compared ordered

| Parameter | |
| :--- | :--- |
| `b` | the builder |
| `x` | the left operand |
| `y` | the right operand |
| **returns** | the bool byte, or the refusal of a logical pointer's address or mirl's failure |

### emit_cmp_ge_u

```mach
pub fun emit_cmp_ge_u(b: *Builder, x: mach.lower.value.Value, y: mach.lower.value.Value) res[mach.lower.value.Value, mach.base.fail.Fail]
```

whether `x` is at least `y` as unsigned integers, as a bool byte: pointers compare their addresses,
floats compared ordered

| Parameter | |
| :--- | :--- |
| `b` | the builder |
| `x` | the left operand |
| `y` | the right operand |
| **returns** | the bool byte, or the refusal of a logical pointer's address or mirl's failure |

### emit_vcompare

```mach
pub fun emit_vcompare(b: *Builder, int_code: Opcode, float_code: Opcode, x: mach.lower.value.Value, y: mach.lower.value.Value, result_ty: Type) res[mach.lower.value.Value, mach.base.fail.Fail]
```

a lanewise comparison of two vectors as a mask of `result_ty`, every bit of a lane set where it holds

### truth

```mach
pub fun truth(b: *Builder, c: mach.lower.value.Value) mach.lower.value.Value
```

an i1 that holds where a bool byte, or any integer, is not zero

### resize_int

```mach
pub fun resize_int(b: *Builder, x: mach.lower.value.Value, to: Type, signed: bool) res[mach.lower.value.Value, mach.base.fail.Fail]
```

an integer of `to`'s width from `x`: kept, truncated or extended by `signed`

### emit_trunc

```mach
pub fun emit_trunc(b: *Builder, x: mach.lower.value.Value, to: Type) res[mach.lower.value.Value, mach.base.fail.Fail]
```

`x` resized to the integer width of `to`, truncated or zero extended

| Parameter | |
| :--- | :--- |
| `b` | the builder |
| `x` | the integer |
| `to` | the integer type |
| **returns** | the result, or the failure mirl's builder kept |

### emit_sext

```mach
pub fun emit_sext(b: *Builder, x: mach.lower.value.Value, to: Type) res[mach.lower.value.Value, mach.base.fail.Fail]
```

`x` resized to the integer width of `to`, truncated or sign extended

| Parameter | |
| :--- | :--- |
| `b` | the builder |
| `x` | the integer |
| `to` | the integer type |
| **returns** | the result, or the failure mirl's builder kept |

### emit_zext

```mach
pub fun emit_zext(b: *Builder, x: mach.lower.value.Value, to: Type) res[mach.lower.value.Value, mach.base.fail.Fail]
```

`x` resized to the integer width of `to`, truncated or zero extended

| Parameter | |
| :--- | :--- |
| `b` | the builder |
| `x` | the integer |
| `to` | the integer type |
| **returns** | the result, or the failure mirl's builder kept |

### emit_fp_trunc

```mach
pub fun emit_fp_trunc(b: *Builder, x: mach.lower.value.Value, to: Type) res[mach.lower.value.Value, mach.base.fail.Fail]
```

`x` demoted to the float type `to`, `x` itself when it already is one

| Parameter | |
| :--- | :--- |
| `b` | the builder |
| `x` | the float |
| `to` | the narrower float type |
| **returns** | the result, or the failure mirl's builder kept |

### emit_fp_ext

```mach
pub fun emit_fp_ext(b: *Builder, x: mach.lower.value.Value, to: Type) res[mach.lower.value.Value, mach.base.fail.Fail]
```

`x` promoted to the float type `to`, `x` itself when it already is one

| Parameter | |
| :--- | :--- |
| `b` | the builder |
| `x` | the float |
| `to` | the wider float type |
| **returns** | the result, or the failure mirl's builder kept |

### emit_fp_resize

```mach
pub fun emit_fp_resize(b: *Builder, x: mach.lower.value.Value, to: Type) res[mach.lower.value.Value, mach.base.fail.Fail]
```

a float of `to`'s format from `x`, promoted or demoted

### emit_fp_to_si

```mach
pub fun emit_fp_to_si(b: *Builder, x: mach.lower.value.Value, to: Type) res[mach.lower.value.Value, mach.base.fail.Fail]
```

the float `x` converted to the signed integer type `to`

| Parameter | |
| :--- | :--- |
| `b` | the builder |
| `x` | the float |
| `to` | the integer type |
| **returns** | the result, or the failure mirl's builder kept |

### emit_fp_to_ui

```mach
pub fun emit_fp_to_ui(b: *Builder, x: mach.lower.value.Value, to: Type) res[mach.lower.value.Value, mach.base.fail.Fail]
```

the float `x` converted to the unsigned integer type `to`

| Parameter | |
| :--- | :--- |
| `b` | the builder |
| `x` | the float |
| `to` | the integer type |
| **returns** | the result, or the failure mirl's builder kept |

### emit_si_to_fp

```mach
pub fun emit_si_to_fp(b: *Builder, x: mach.lower.value.Value, to: Type) res[mach.lower.value.Value, mach.base.fail.Fail]
```

the signed integer `x` converted to the float type `to`

| Parameter | |
| :--- | :--- |
| `b` | the builder |
| `x` | the integer |
| `to` | the float type |
| **returns** | the result, or the failure mirl's builder kept |

### emit_ui_to_fp

```mach
pub fun emit_ui_to_fp(b: *Builder, x: mach.lower.value.Value, to: Type) res[mach.lower.value.Value, mach.base.fail.Fail]
```

the unsigned integer `x` converted to the float type `to`

| Parameter | |
| :--- | :--- |
| `b` | the builder |
| `x` | the integer |
| `to` | the float type |
| **returns** | the result, or the failure mirl's builder kept |

### emit_bitcast

```mach
pub fun emit_bitcast(b: *Builder, x: mach.lower.value.Value, to: Type) res[mach.lower.value.Value, mach.base.fail.Fail]
```

the same bits as `to`: an address read as an integer and back, a resize between integers
of different widths, or a reinterpretation of equal widths

### emit_classify

```mach
pub fun emit_classify(b: *Builder, x: mach.lower.value.Value) res[mach.lower.value.Value, mach.base.fail.Fail]
```

`x` made secret: a block parameter declared secret, which the branch into it passes `x`

### emit_declassify

```mach
pub fun emit_declassify(b: *Builder, x: mach.lower.value.Value) res[mach.lower.value.Value, mach.base.fail.Fail]
```

`x` made public: a constant as it is, an aggregate copied a word at a time
into a public slot of its own, any other value through a declassify

| Parameter | |
| :--- | :--- |
| `b` | the builder |
| `x` | the value |
| **returns** | the result, or the failure mirl's builder kept |

### emit_alloca

```mach
pub fun emit_alloca(b: *Builder, ty: Type) res[mach.lower.value.Value, mach.base.fail.Fail]
```

a stack slot of `ty` for a temporary, made in the entry block so every path reaches it (`temp_alloca`)

### temps_begin

```mach
pub fun temps_begin(b: *Builder) Temps
```

open the scope of temporaries of a statement or a condition about to be lowered, here. the storage of every
temporary it makes begins here (`temp_alloca`) and ends where it closes (`temps_close`, `temps_end`), the one
owner of every temporary's markers. a scope opens where control enters what it scopes and closes on every way out,
so each temporary is begun on every path to an end, once, and ended on every path that began it.

what makes that hold is that no expression contains a statement. the only branches an expression makes are its
own: `&&`, `||`, the debug profile's guard trap and a classification (`emit_classify`), each producing a bool or
another value and joining again, or trapping. so every block of a scope is dominated by its opening, and the
program cannot take a temporary's address, so nothing reads one past its scope:

- an expression statement, a declaration, an assignment and an `asm` statement open where they begin and close
  where they end. a value a declaration binds is copied out of the temporary or read from it whole before the
  end (`emit_whole`). mirl takes such a load apart where it stands, or copies a large one into a slot of its
  own there, and moves a load's bytes to a store from memory to memory only with no write of memory between
  them, which a marker is, so no read of the temporary lands past its end
- a condition of an `if`, of each arm of an `or` chain and of a loop opens where its evaluation begins, the
  loop's at the head of its header, so once per evaluation, and closes at the head of each block it branches to,
  each reached from the condition alone. the branch reads only the truth the condition computed
- a `ret` opens like a statement and never closes: its value may be held in a temporary until the return, past
  the `fin` bodies it replays, and the function's end ends every storage. each replayed `fin` statement scopes
  only its own temporaries, so nothing ends the return's on the way into them
- a `brk` or `cnt` ends every temporary of the scopes it leaves inside its loop, after the `fin` bodies it
  replays, before it branches (`temps_close` from the loop's mark). a condition's temporaries have ended by then
- a block, `if`, loop, `$if`, `$each` or `fin` statement makes no temporary of its own past its conditions, and
  each statement nested in it opens its own scope

the rule every statement kind keeps: a scope makes all its temporaries before it lowers any statement nested in
it. its temporaries begin where it opened, so one made after a nested statement would be begun on the paths of a
`brk`, `cnt` or `ret` inside that statement, which end only what has been made by then, and a `cnt` would begin
it again. a statement kind that evaluates an expression after a nested body, as a loop's step would, opens a
scope of its own where that expression is evaluated, as a condition does. one that holds a temporary across a
nested body, as a scrutinee held across arms would, makes it before the body, so a jump out ends it.

lowered in a block that has ended, nothing is scoped, as nothing there runs

### temps_mark

```mach
pub fun temps_mark(b: *Builder) usize
```

how many temporaries have begun, where a loop's body starts: a `brk` or `cnt` ends those begun past it

### temps_close

```mach
pub fun temps_close(b: *Builder, from: usize) err[mach.base.fail.Fail]
```

end the storage of every temporary begun past `from`, the latest first, here, where control still reaches. the
temporaries stay their scopes' own, so a scope closed on several ways out ends them on each

### temps_drop

```mach
pub fun temps_drop(b: *Builder, kept: Temps)
```

give the scoping back to the enclosing scope, the closed scope's temporaries ended on every way out of it already

### temps_end

```mach
pub fun temps_end(b: *Builder, kept: Temps) err[mach.base.fail.Fail]
```

close the scope `kept` was taken for here and give the scoping back to the enclosing scope

### temp_alloca

```mach
pub fun temp_alloca(b: *Builder, ty: Type, align: u64) res[mach.lower.value.Value, mach.base.fail.Fail]
```

a stack slot of `ty` aligned to `align` for a temporary of the scope being lowered, made in the entry block, its
storage begun where the scope opened (`temps_begin`)

### alloca_aligned

```mach
pub fun alloca_aligned(b: *Builder, ty: Type, align: u64) res[mach.lower.value.Value, mach.base.fail.Fail]
```

a stack slot of `ty` aligned to `align`, made in the entry block so every path
reaches it

| Parameter | |
| :--- | :--- |
| `b` | the builder |
| `ty` | the type the slot holds |
| `align` | the slot's alignment in bytes |
| **returns** | the slot's address, or the refusal outside a function or mirl's failure |

### emit_whole

```mach
pub fun emit_whole(b: *Builder, v: mach.lower.value.Value) res[mach.lower.value.Value, mach.base.fail.Fail]
```

the aggregate `v` held whole: itself where it is, a constant as its own value, and one at
an address read from it whole, as secret as `v` and stated as its type

### emit_extract

```mach
pub fun emit_extract(b: *Builder, v: mach.lower.value.Value, k: u32) res[mach.lower.value.Value, mach.base.fail.Fail]
```

member `k` of the aggregate `v` held whole: a structure's member, an array's element or a
union's member read from its first byte. an aggregate member is held whole in turn

### emit_insert

```mach
pub fun emit_insert(b: *Builder, v: mach.lower.value.Value, part: mach.lower.value.Value, k: u32) res[mach.lower.value.Value, mach.base.fail.Fail]
```

the aggregate `v` held whole with member `k` replaced by `part`, held whole where it is an
aggregate

### emit_spill

```mach
pub fun emit_spill(b: *Builder, v: mach.lower.value.Value) res[mach.lower.value.Value, mach.base.fail.Fail]
```

the aggregate `v` held whole written to a stack slot of its own, whose address is the value:
what a read of its bytes as another type needs

### emit_load

```mach
pub fun emit_load(b: *Builder, addr: mach.lower.value.Value, ty: Type) res[mach.lower.value.Value, mach.base.fail.Fail]
```

a value of `ty` read from `addr`: an aggregate is copied into a slot of its own, whose
address is the value, as secret as the read or `addr`

### emit_load_as

```mach
pub fun emit_load_as(b: *Builder, addr: mach.lower.value.Value, ty: Type, secret: bool) res[mach.lower.value.Value, mach.base.fail.Fail]
```

a value of `ty` read from `addr` that is as secret as `secret`, whatever the reads around
it are marked

### emit_load_discriminator

```mach
pub fun emit_load_discriminator(b: *Builder, addr: mach.lower.value.Value, ty: Type) res[mach.lower.value.Value, mach.base.fail.Fail]
```

a tag's discriminator of `ty` read from `addr`, stated public: the layout keeps it apart from the bytes of every
case, so it is public whatever the cases hold

### emit_store

```mach
pub fun emit_store(b: *Builder, v: mach.lower.value.Value, addr: mach.lower.value.Value) err[mach.base.fail.Fail]
```

write `v` to `addr`: an aggregate, held at an address, is copied. a store where the access is stated states the
memory it writes as the access does, secret with its mark and public with the public flag

### emit_copy

```mach
pub fun emit_copy(b: *Builder, dst: mach.lower.value.Value, src: mach.lower.value.Value, ty: Type) err[mach.base.fail.Fail]
```

copy the bytes of a `ty` from `src` to `dst`, marked secret where the read is or `src` is. a copy
whose access is stated, as a store of an aggregate states the place it writes, states the bytes
it moves: secret with its mark, or public where the place's type holds no secret, which sema
holds the source's type to as well

### copy_publics

```mach
pub fun copy_publics(b: *Builder, dst: mach.lower.value.Value, src: mach.lower.value.Value, ty: Type, ranges: Slice[mirl.ir.global.Range]) err[mach.base.fail.Fail]
```

the bytes of each of `ranges` of an aggregate of `ty` at `src`, which its type lays out as public, copied again to
the same bytes at `dst` after the whole copy, each copy stated public, so the walk of a constant-time function sees
those bytes of `dst` written with what public storage held where the whole copy, stated secret, moved them in words
that also held a secret

### emit_memzero

```mach
pub fun emit_memzero(b: *Builder, addr: mach.lower.value.Value, ty: Type, secret: bool) err[mach.base.fail.Fail]
```

zero the bytes of a `ty` at `addr`, a place whose type holds a secret where `secret` holds, which
the fill states of the memory it writes

### emit_wipe

```mach
pub fun emit_wipe(b: *Builder, addr: mach.lower.value.Value, count: mach.lower.value.Value, signed: bool, ty: Type, secret: bool) err[mach.base.fail.Fail]
```

zero the bytes of `count` of `ty` at `addr`, a place whose type holds a secret where `secret`
holds, through a kept fill: no pass removes or narrows it, and its length is the size of its
target alone

### emit_offset

```mach
pub fun emit_offset(b: *Builder, addr: mach.lower.value.Value, bytes: mach.lower.value.Value) res[mach.lower.value.Value, mach.base.fail.Fail]
```

the address `bytes` past `addr`

### emit_member

```mach
pub fun emit_member(b: *Builder, addr: mach.lower.value.Value, agg: Type, k: u32) res[mach.lower.value.Value, mach.base.fail.Fail]
```

the address of member `k` of the aggregate `agg` at `addr`: a structure's member, an
array's or vector's element, and a union's every member, which all start where it does

### emit_element

```mach
pub fun emit_element(b: *Builder, addr: mach.lower.value.Value, agg: Type, ix: mach.lower.value.Value) res[mach.lower.value.Value, mach.base.fail.Fail]
```

the address of element `ix` of the array or vector `agg` at `addr`

### emit_step

```mach
pub fun emit_step(b: *Builder, addr: mach.lower.value.Value, elem: Type, ix: mach.lower.value.Value) res[mach.lower.value.Value, mach.base.fail.Fail]
```

the address `ix` elements of `elem` past `addr`, as a pointer steps

### emit_splat

```mach
pub fun emit_splat(b: *Builder, ty: Type, scalar: mach.lower.value.Value) res[mach.lower.value.Value, mach.base.fail.Fail]
```

a vector of `ty` with `scalar` in every lane

### emit_vec_extract

```mach
pub fun emit_vec_extract(b: *Builder, vec: mach.lower.value.Value, lane: mach.lower.value.Value, ty: Type) res[mach.lower.value.Value, mach.base.fail.Fail]
```

lane `lane` of `vec`, read as `ty`: a constant lane is read in place, any other through
the vector's bytes in a slot

### emit_vec_insert

```mach
pub fun emit_vec_insert(b: *Builder, vec: mach.lower.value.Value, lane: mach.lower.value.Value, x: mach.lower.value.Value) res[mach.lower.value.Value, mach.base.fail.Fail]
```

`vec` with lane `lane` replaced by `x`: a constant lane in place, any other
through the vector's bytes in a slot

| Parameter | |
| :--- | :--- |
| `b` | the builder |
| `vec` | the vector |
| `lane` | the lane index |
| `x` | the new lane value |
| **returns** | the result, or the failure mirl's builder kept |

### emit_vec_build

```mach
pub fun emit_vec_build(b: *Builder, ty: Type, lanes: *mach.lower.value.Value, n: u32) res[mach.lower.value.Value, mach.base.fail.Fail]
```

a vector of `ty` from one value per lane

### emit_shuffle

```mach
pub fun emit_shuffle(b: *Builder, ty: Type, x: mach.lower.value.Value, y: mach.lower.value.Value, picks: *u32, n: u32) res[mach.lower.value.Value, mach.base.fail.Fail]
```

lanes of `x` and `y` joined, picked by `picks`, as a vector of `ty`

### emit_operation

```mach
pub fun emit_operation(b: *Builder, code: Opcode, values: *mach.lower.value.Value, n: u32) res[mach.lower.value.Value, mach.base.fail.Fail]
```

the operation `code` at the cursor over the `n` values at `values`, its operands in order

### emit_atomic_load

```mach
pub fun emit_atomic_load(b: *Builder, addr: mach.lower.value.Value, ty: Type, order: Ordering) res[mach.lower.value.Value, mach.base.fail.Fail]
```

an atomic read of the integer `ty` at `addr`

### emit_atomic_store

```mach
pub fun emit_atomic_store(b: *Builder, addr: mach.lower.value.Value, x: mach.lower.value.Value, order: Ordering) err[mach.base.fail.Fail]
```

an atomic write of `x` to `addr`

| Parameter | |
| :--- | :--- |
| `b` | the builder |
| `addr` | the address written |
| `x` | the value written |
| `order` | the memory ordering |
| **returns** | nothing, or the failure mirl's builder kept |

### emit_atomic_rmw

```mach
pub fun emit_atomic_rmw(b: *Builder, code: Opcode, addr: mach.lower.value.Value, x: mach.lower.value.Value, order: Ordering) res[mach.lower.value.Value, mach.base.fail.Fail]
```

an atomic read-modify-write, giving the value held before

### emit_atomic_cmpxchg

```mach
pub fun emit_atomic_cmpxchg(b: *Builder, addr: mach.lower.value.Value, expected: mach.lower.value.Value, desired: mach.lower.value.Value, success: Ordering, failure_order: Ordering) res[mach.lower.value.Value, mach.base.fail.Fail]
```

an atomic compare and exchange, giving the value held before

### emit_scope

```mach
pub fun emit_scope(b: *Builder, slot: mach.lower.value.Value, begins: bool) err[mach.base.fail.Fail]
```

where the storage of `slot`, an alloca's result, begins or ends, unmarked
whatever the builder marks

| Parameter | |
| :--- | :--- |
| `b` | the builder |
| `slot` | the alloca's result |
| `begins` | true for where its storage begins, false for where it ends |
| **returns** | nothing, or the failure mirl's builder kept |

### emit_fence

```mach
pub fun emit_fence(b: *Builder, order: Ordering) err[mach.base.fail.Fail]
```

a memory fence of `order`

| Parameter | |
| :--- | :--- |
| `b` | the builder |
| `order` | the memory ordering |
| **returns** | nothing, or the failure mirl's builder kept |

### emit_call

```mach
pub fun emit_call(b: *Builder, callee: mach.lower.value.Value, sig: Type, args: *mach.lower.value.Value, n: u32) res[mach.lower.value.Value, mach.base.fail.Fail]
```

a call of `callee` through the signature `sig`, every argument named. an aggregate
argument is passed whole, read from the address it travels at as secret as the argument
is, and an aggregate result lands in a slot of its own

### emit_call_named

```mach
pub fun emit_call_named(b: *Builder, callee: mach.lower.value.Value, sig: Type, args: *mach.lower.value.Value, n: u32, named: Named) res[mach.lower.value.Value, mach.base.fail.Fail]
```

a call of `callee` through the signature `sig`, of which `named` states how many
arguments are named, the rest being a C-variadic call's unnamed ones

### call_states

```mach
pub fun call_states(b: *Builder, callee: opt[mirl.ir.id.Stated])
```

the declaration the next call, one through a value of a function type marked constant time, states its callee
keeps (`lower_context.stated_callee`)

### result_whole

```mach
pub fun result_whole(b: *Builder)
```

hold the next call's aggregate result whole, with no slot, unless it lands in a destination

### result_into

```mach
pub fun result_into(b: *Builder, slot: mach.lower.value.Value)
```

land the next call's aggregate result in `slot`, storage of the result's type nothing reads yet

### param

```mach
pub fun param(b: *Builder, i: u32, ty: Type, whole: bool) res[mach.lower.value.Value, mach.base.fail.Fail]
```

parameter `i` of the function being built, of `ty`: an aggregate held `whole` is the
parameter itself, and any other lands in a slot of its own, whose address is the value

### emit_br

```mach
pub fun emit_br(b: *Builder, target: mirl.ir.id.Block) err[mach.base.fail.Fail]
```

end the block with a branch to `target`, which takes no arguments

| Parameter | |
| :--- | :--- |
| `b` | the builder |
| `target` | the block branched to |
| **returns** | nothing, or the failure mirl's builder kept |

### emit_br_args

```mach
pub fun emit_br_args(b: *Builder, target: mirl.ir.id.Block, args: *mach.lower.value.Value, n: u32) err[mach.base.fail.Fail]
```

a branch to `target`, passing one argument per parameter of it

### emit_cbr

```mach
pub fun emit_cbr(b: *Builder, c: mach.lower.value.Value, then_b: mirl.ir.id.Block, else_b: mirl.ir.id.Block) err[mach.base.fail.Fail]
```

end the block with a branch on `c`, compared with zero, to `then_b` or `else_b`

| Parameter | |
| :--- | :--- |
| `b` | the builder |
| `c` | the condition, an i1 or any integer |
| `then_b` | the block taken where `c` holds |
| `else_b` | the block taken otherwise |
| **returns** | nothing, or the failure mirl's builder kept |

### emit_switch

```mach
pub fun emit_switch(b: *Builder, selector: mach.lower.value.Value, otherwise: mirl.ir.id.Block, cases: *u64, targets: *mirl.ir.id.Block, n: u32) err[mach.base.fail.Fail]
```

a branch on `selector` to the target of the case it equals, else to `otherwise`. `cases` holds
strictly increasing values, one per target

### emit_ret

```mach
pub fun emit_ret(b: *Builder, v: mach.lower.value.Value) err[mach.base.fail.Fail]
```

return `v` from the function being built: an aggregate is read whole from its address, as
secret as `v` is

### emit_ret_void

```mach
pub fun emit_ret_void(b: *Builder) err[mach.base.fail.Fail]
```

end the block with a return of no value

| Parameter | |
| :--- | :--- |
| `b` | the builder |
| **returns** | nothing, or the failure mirl's builder kept |

### emit_asm

```mach
pub fun emit_asm(b: *Builder, block: *Assembly, values: *mach.lower.value.Value, n: u32, results: *mach.lower.value.Value) err[mach.base.fail.Fail]
```

an inline assembly block at the cursor, binding the `n` values at `values` in its operands'
order, the results of the operands that give one written to `results` in that order. the
instruction copies the block, so it may be borrowed

### emit_target_op

```mach
pub fun emit_target_op(b: *Builder, op: *mirl.ir.native.Op, values: *mach.lower.value.Value, n: u32) res[mach.lower.value.Value, mach.base.fail.Fail]
```

the target op `op` at the cursor over the `n` values at `values`, in its operands' order,
its result the one `op` states or none, a pointer's in the space its caller places it. the
instruction copies the op, so it may be borrowed

### emit_unreachable

```mach
pub fun emit_unreachable(b: *Builder) err[mach.base.fail.Fail]
```

end the block as unreachable

| Parameter | |
| :--- | :--- |
| `b` | the builder |
| **returns** | nothing, or the failure mirl's builder kept |

### add

```mach
pub fun add(b: *Builder, x: mach.lower.value.Value, y: mach.lower.value.Value) mach.lower.value.Value
```

emit_add with a failure kept as the builder's

| Parameter | |
| :--- | :--- |
| `b` | the builder |
| `x` | the left operand |
| `y` | the right operand |
| **returns** | the result, or none with the failure kept as the builder's |

### sub

```mach
pub fun sub(b: *Builder, x: mach.lower.value.Value, y: mach.lower.value.Value) mach.lower.value.Value
```

emit_sub with a failure kept as the builder's

| Parameter | |
| :--- | :--- |
| `b` | the builder |
| `x` | the left operand |
| `y` | the right operand |
| **returns** | the result, or none with the failure kept as the builder's |

### mul

```mach
pub fun mul(b: *Builder, x: mach.lower.value.Value, y: mach.lower.value.Value) mach.lower.value.Value
```

emit_mul with a failure kept as the builder's

| Parameter | |
| :--- | :--- |
| `b` | the builder |
| `x` | the left operand |
| `y` | the right operand |
| **returns** | the result, or none with the failure kept as the builder's |

### band

```mach
pub fun band(b: *Builder, x: mach.lower.value.Value, y: mach.lower.value.Value) mach.lower.value.Value
```

emit_and with a failure kept as the builder's

| Parameter | |
| :--- | :--- |
| `b` | the builder |
| `x` | the left operand |
| `y` | the right operand |
| **returns** | the result, or none with the failure kept as the builder's |

### bor

```mach
pub fun bor(b: *Builder, x: mach.lower.value.Value, y: mach.lower.value.Value) mach.lower.value.Value
```

emit_or with a failure kept as the builder's

| Parameter | |
| :--- | :--- |
| `b` | the builder |
| `x` | the left operand |
| `y` | the right operand |
| **returns** | the result, or none with the failure kept as the builder's |

### bxor

```mach
pub fun bxor(b: *Builder, x: mach.lower.value.Value, y: mach.lower.value.Value) mach.lower.value.Value
```

emit_xor with a failure kept as the builder's

| Parameter | |
| :--- | :--- |
| `b` | the builder |
| `x` | the left operand |
| `y` | the right operand |
| **returns** | the result, or none with the failure kept as the builder's |

### shl

```mach
pub fun shl(b: *Builder, x: mach.lower.value.Value, y: mach.lower.value.Value) mach.lower.value.Value
```

emit_shl with a failure kept as the builder's

| Parameter | |
| :--- | :--- |
| `b` | the builder |
| `x` | the value shifted |
| `y` | the shift count |
| **returns** | the result, or none with the failure kept as the builder's |

### shr_u

```mach
pub fun shr_u(b: *Builder, x: mach.lower.value.Value, y: mach.lower.value.Value) mach.lower.value.Value
```

emit_shr_u with a failure kept as the builder's

| Parameter | |
| :--- | :--- |
| `b` | the builder |
| `x` | the value shifted |
| `y` | the shift count |
| **returns** | the result, or none with the failure kept as the builder's |

### shr_s

```mach
pub fun shr_s(b: *Builder, x: mach.lower.value.Value, y: mach.lower.value.Value) mach.lower.value.Value
```

emit_shr_s with a failure kept as the builder's

| Parameter | |
| :--- | :--- |
| `b` | the builder |
| `x` | the value shifted |
| `y` | the shift count |
| **returns** | the result, or none with the failure kept as the builder's |

### eq

```mach
pub fun eq(b: *Builder, x: mach.lower.value.Value, y: mach.lower.value.Value) mach.lower.value.Value
```

emit_cmp_eq with a failure kept as the builder's

| Parameter | |
| :--- | :--- |
| `b` | the builder |
| `x` | the left operand |
| `y` | the right operand |
| **returns** | the result, or none with the failure kept as the builder's |

### ne

```mach
pub fun ne(b: *Builder, x: mach.lower.value.Value, y: mach.lower.value.Value) mach.lower.value.Value
```

emit_cmp_ne with a failure kept as the builder's

| Parameter | |
| :--- | :--- |
| `b` | the builder |
| `x` | the left operand |
| `y` | the right operand |
| **returns** | the result, or none with the failure kept as the builder's |

### lt_s

```mach
pub fun lt_s(b: *Builder, x: mach.lower.value.Value, y: mach.lower.value.Value) mach.lower.value.Value
```

emit_cmp_lt_s with a failure kept as the builder's

| Parameter | |
| :--- | :--- |
| `b` | the builder |
| `x` | the left operand |
| `y` | the right operand |
| **returns** | the result, or none with the failure kept as the builder's |

### lt_u

```mach
pub fun lt_u(b: *Builder, x: mach.lower.value.Value, y: mach.lower.value.Value) mach.lower.value.Value
```

emit_cmp_lt_u with a failure kept as the builder's

| Parameter | |
| :--- | :--- |
| `b` | the builder |
| `x` | the left operand |
| `y` | the right operand |
| **returns** | the result, or none with the failure kept as the builder's |

### le_u

```mach
pub fun le_u(b: *Builder, x: mach.lower.value.Value, y: mach.lower.value.Value) mach.lower.value.Value
```

emit_cmp_le_u with a failure kept as the builder's

| Parameter | |
| :--- | :--- |
| `b` | the builder |
| `x` | the left operand |
| `y` | the right operand |
| **returns** | the result, or none with the failure kept as the builder's |

### trunc

```mach
pub fun trunc(b: *Builder, x: mach.lower.value.Value, to: Type) mach.lower.value.Value
```

emit_trunc with a failure kept as the builder's

| Parameter | |
| :--- | :--- |
| `b` | the builder |
| `x` | the value converted |
| `to` | the type converted to |
| **returns** | the result, or none with the failure kept as the builder's |

### zext

```mach
pub fun zext(b: *Builder, x: mach.lower.value.Value, to: Type) mach.lower.value.Value
```

emit_zext with a failure kept as the builder's

| Parameter | |
| :--- | :--- |
| `b` | the builder |
| `x` | the value converted |
| `to` | the type converted to |
| **returns** | the result, or none with the failure kept as the builder's |

### sext

```mach
pub fun sext(b: *Builder, x: mach.lower.value.Value, to: Type) mach.lower.value.Value
```

emit_sext with a failure kept as the builder's

| Parameter | |
| :--- | :--- |
| `b` | the builder |
| `x` | the value converted |
| `to` | the type converted to |
| **returns** | the result, or none with the failure kept as the builder's |

### bitcast

```mach
pub fun bitcast(b: *Builder, x: mach.lower.value.Value, to: Type) mach.lower.value.Value
```

emit_bitcast with a failure kept as the builder's

| Parameter | |
| :--- | :--- |
| `b` | the builder |
| `x` | the value converted |
| `to` | the type converted to |
| **returns** | the result, or none with the failure kept as the builder's |

### br

```mach
pub fun br(b: *Builder, target: mirl.ir.id.Block)
```

emit_br with a failure kept as the builder's

| Parameter | |
| :--- | :--- |
| `b` | the builder |
| `target` | the block branched to |

### cbr

```mach
pub fun cbr(b: *Builder, c: mach.lower.value.Value, then_b: mirl.ir.id.Block, else_b: mirl.ir.id.Block)
```

emit_cbr with a failure kept as the builder's

| Parameter | |
| :--- | :--- |
| `b` | the builder |
| `c` | the condition |
| `then_b` | the block taken where `c` holds |
| `else_b` | the block taken otherwise |

## Opening

```mach
pub rec Opening {
    block: mirl.ir.id.Block;
    after: mirl.ir.id.Instruction;
}
```

where a scope of temporaries opened: past instruction `after` of `block`, or with `after` NO_INSTRUCTION at the
block's start, past the entry's stack slots when it is the entry

## Temps

```mach
pub rec Temps {
    open: opt[Opening];
    from: usize;
}
```

how the scoping of temporaries stood before a scope took it over (`temps_begin`)

| Field | Type | |
| :--- | :--- | :--- |
| `open` | `opt[Opening]` | where the enclosing scope opened, none outside one |
| `from` | `usize` | how many temporaries had begun before the scope |

## Functions

### module_refusal

```mach
pub fun module_refusal(e: mirl.ir.module.Error, what: str) mach.base.fail.Fail
```

mirl's refusal of what `what` names, as lowering's failure: storage refused where it stands for
that, else a defect of the lowering

| Parameter | |
| :--- | :--- |
| `e` | mirl's refusal |
| `what` | the defect's message |
| **returns** | the failure |

