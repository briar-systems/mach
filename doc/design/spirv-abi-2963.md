# SPIR-V value-oriented module ABI (#2963, #2940, roadmap N4)

This document records what the SPIR-V calling convention on `dev` does today for
every parameter and return class, where the synthetic register bank the N1 census
counted still leaks into the emitter, what the candidate-era work adds, the
contract this lane lands, and the interface L6 (tag transport through SPIR-V) will
call. Sources: `gh issue view 2963`, `gh issue view 2940`,
`doc/design/backend-census-2212.md` section 5 and section 11 (N4),
`doc/design/v5-release-contract.md` (SPIR-V boundary), the archived integration
head `archive/3218-candidate-era` (commit `2c463ae7` and the value-ABI hunks folded
into `c2fbd635`), and the tree at `dev` `2633d5ba4`.

## 1. Inventory: the convention on `dev` today

`abi/spirv.mach` satisfies the register-machine `AbiVTable` with an identity
convention: argument `i` is classified into nominal general register `i`
(`classify_arg` returns `CLASS_GP` with `reg = index`, or `CLASS_AGG` with
`reg = index` for an aggregate), the return is classified into nominal register 0,
and `arg_regs` publishes a bank of `ARG_REG_COUNT = 16` eight-byte registers.
`register.mach` declares two 16-wide register classes (`gpr`, `fpr`) for a machine
that has none. Shared MIR lowering then pre-colours every argument into a `PREG`
and the emitter (`spirv/emit.mach`) keeps a 16-slot shadow file `rec Regs`
(`val_id`, `imm`, `is_imm`, `is_bool`) per structured node that maps those nominal
registers back to SSA ids.

| class | how it crosses on `dev` | site | verdict |
|---|---|---|---|
| scalar (int, float, bool) | `MIR_MOV vreg -> PREG i` at the call, `PREG i -> vreg` at entry; the emitter reads `Regs.val_id[i]` (or an immediate) and widens a bool to the parameter type | `abi/spirv.mach:14-21`, `emit.mach:2686 emit_call`, `:3193 emit_mov`, `:3224 reg_value` | accepted (works, but through the bank) |
| vector (`u32x4`, `f32x4`, ...) | as scalar: one nominal register per vector, typed by `ir_value_spv_type` | same | accepted (through the bank) |
| composite record by value | `CLASS_AGG`: `MIR_AGG_LOAD PREG i <- mem` at the call, `MIR_AGG_STORE mem <- PREG i` at entry; the emitter loads the whole `OpTypeStruct` and parks it in `Regs.val_id[i]` | `mir/context.mach:586-594`, `mir/abi.mach:967,1206`, `emit.mach:2529-2610` | accepted (through the bank) |
| array by value | as composite (`OpTypeArray` is a whole value) | same | accepted (through the bank) |
| pointer to record | `spv_type_of` has no `IRT_PTR` arm and returns 0; `emit_function` refuses "unsupported parameter type in the SPIR-V target" with no source location, parameter name or type | `emit.mach:2164`, `:1500-1503` | gap (#2940) |
| pointer to scalar | same refusal; `mem/ptr_arith` indexes off it, which logical addressing cannot express | same, `test/golden/spirv/SKIPS` | refused-by-policy (pointer arithmetic); the whole-object pointer itself is a gap |
| function reference (`call/call_indirect`) | same refusal; logical addressing has no function pointer | same | refused-by-policy |
| return (scalar, vector, composite) | callee: `MIR_AGG_LOAD PREG 0` / `MIR_MOV PREG 0` then bare `MIR_RET`; `emit_terminator` reads `Regs.val_id[0]` for `OpReturnValue`; caller: `MIR_CALL_RES` then `MIR_MOV vreg <- PREG 0` / `MIR_AGG_STORE mem <- PREG 0` | `mir/abi.mach:1294`, `emit.mach:1748-1755`, `:2780-2786` | accepted (through the bank, result position 0) |
| more than 16 arguments (`call/call_mixed`, #2923) | refused "more than 16 parameters is not supported by the SPIR-V target" at `emit_function` and `emit_call`; `types.type_fn` silently returns 0 past `MAX_FN_PARAMS` | `emit.mach:1494,2718`, `types.mach:53,451` | gap (#2963) |
| `-g` on a SPIR-V target | `of/spv.mach:117` registers no debug model (`default_debug = ""`); `passes.mach:969 debug_info_of` refuses "debug info was requested, but this target registers no debug model"; the corpus driver maps that exact text to a `DEBUG_UNSUPPORTED` skip cell (`test/lib/driver.py:24-26,518-526`) | | refused-by-policy (v5 contract: "Debug requests are explicitly refused") |

### 1.1 Where the synthetic bank leaks into the emitter

The N1 census counted one synthetic 16-register bank. It is load-bearing at these
sites on `dev`, every one of which the acceptance below removes:

- `abi/spirv.mach:11 ARG_REG_COUNT = 16` and `:40 arg_regs` (the identity bank).
- `spirv/register.mach:25-32` two 16-wide `RegClass` rows and `reg_class_len = 2`.
- `spirv/types.mach:53 MAX_FN_PARAMS = 16`, `:451 type_fn` returning 0 past it, and
  the `[18]u32` operand buffer at `:463`.
- `spirv/emit.mach:76 rec Regs` and every `preg_val: *Regs` parameter threaded
  through `emit_node`, `emit_terminator`, `read_condition`, `read_operand`,
  `emit_mov`, `emit_call`, `emit_spirv_op`, `emit_agg_load`, `emit_agg_store`,
  `reg_value`, `operand_is_bool`; the `[16]u32` `ptypes`/`param_ids`/`argty` and
  `[19]u32` call operand buffers; the `MIR_OP_PREG` arms in `read_operand`,
  `emit_mov`, `is_reg_copy`; the twelve-parameter cap in `emit_spirv_op`.
- `spirv/emit.mach:1863-1935 build_vmaps` reconstructing a vreg's type by pairing
  `MIR_CALL`/`MIR_CALL_RES` with the following `MIR_MOV vreg <- PREG`.
- `mir.mach:168-170 MIR_AGG_LOAD`/`MIR_AGG_STORE` (whole objects in and out of
  nominal positions) and `mir/context.mach:586-594` their constructors.
- `mir/abi.mach:33 MAX_GP_ARG_REGS = 16` and `:45 abi_gp_arg_reg` filling a fixed
  `[16]isa.Register` from the convention's bank (the shared overrun hazard #2962
  guards), reachable for SPIR-V only because the bank exists. It stays for the
  carriers conventions and is unreachable from a values convention, which
  publishes no bank.

### 1.2 What the candidate era adds that `dev` lacks

- `2c463ae7 test(abi): census the value-based convention constructor`: adds
  `value_abi_vtable` in `src/lang/target/abi.mach` to the `isa/tests.mach`
  construction census allow-list. Test-only; it presumes the constructor.
- The value-ABI hunks of `c2fbd635` (an omnibus "local validation candidate"
  commit, never landed): `abi.CLASS_VALUE` replacing `CLASS_AGG`;
  `abi.PassingModel` (`PASSING_CARRIERS`, `PASSING_VALUES`) on `AbiVTable`;
  `abi.value_abi_vtable` and `abi.make_slot_value`; `descriptor_valid` and
  `register` splitting their checks by passing model; `mir.MIR_VALUE_CALL` and
  `mir.MIR_VALUE_PARAM` replacing `MIR_AGG_LOAD`/`MIR_AGG_STORE`;
  `mir.REG_CLASS_VALUE`; `mir/abi.mach lower_value_call`/`lower_value_params` and
  the `PASSING_VALUES` forks in `lower_call`/`lower_params`/`lower_ret`;
  `mir/context.mach` selecting `REG_CLASS_VALUE` for every vreg of a value target;
  `target.mach TUPLE_ABI_TRANSPORT` and the passing model in the fingerprint;
  `abi/spirv.mach` reduced to two `make_slot_value` classifiers; `spirv/emit.mach`
  with `Regs` deleted, `read_value`/`write_value`/`emit_param` over logical
  operands, an `IRT_PTR` arm in `spv_type_of`, per-parameter located refusals, and
  heap-sized operand buffers bounded by the 65535-word instruction encoding;
  `spirv/types.mach pointer_shape` and `instruction_operands_fit`;
  `spirv/register.mach` publishing no register classes; the driver test
  `logical_value_abi_preserves_mixed_arguments_and_references_on_spirv`
  (21 arguments: 17 `u32`, one `f32`, one `Pair` by value, one `*Pair`, one
  `u32x4`, returning `Pair`, checked by evaluating the module and by finding a
  25-word `OpFunctionCall`) and
  `logical_shader_reference_capabilities_refuse_returns_subobjects_and_cycles`.
- The candidate also rewrote unrelated things in the same files (security
  provenance on `MirInstr`, RISC-V feature selection, ELF attribute validation,
  the removal of the MOS 6502 convention, and a reversal of `dev`'s later
  `46b3214bc` defs-count refactor). None of that is N4 and none of it is ported.

## 2. The contract this lane lands

The four open questions in #2963 are settled as follows.

1. **`CLASS_VALUE` subsumes `CLASS_AGG`.** Both meant "the whole object crosses as
   itself"; the only difference was that `CLASS_AGG` named a nominal register. A
   value slot names no register (`reg = -1`), no pieces, no offset and no indirect
   extent, and `abi.make_slot_value(size)` is its only constructor. Register
   machines never produce it: their aggregates keep classifying into carriers
   (`CLASS_GP`/`CLASS_FP` pieces, `CLASS_BYREF`, `CLASS_SRET`, ...) exactly as
   before, so no machine ABI changes behaviour.
2. **The axis is declared on the convention, not derived from the machine.**
   `AbiVTable.passing` is `PASSING_CARRIERS` (every register ABI, built by
   `abi_vtable`) or `PASSING_VALUES` (built by `value_abi_vtable`, which takes
   only the two classifiers and refuses a bank, a callee-saved set, a variadic
   model, a slot granularity, a stack alignment, a red zone, shadow space or an
   indirect-result register). `target.tuple_capability` pairs the two models with
   the instruction-set family: a values convention needs a whole-module emitter
   and a carriers convention needs a machine, else `TUPLE_ABI_TRANSPORT`. The
   model is part of the target fingerprint.
3. **MIR carries value calls directly.** `MIR_VALUE_CALL callee, result, args...`
   and `MIR_VALUE_PARAM dest, index` are the only call-boundary instructions a
   values target lowers to (`SEL_CLASS_SPIRV_ONLY`, no shape). An aggregate
   operand is its owned home (`op_mem_value`), a scalar is its vreg. Every vreg of
   a values function is `REG_CLASS_VALUE`, so no register allocator, spill or
   pre-colouring pass can apply. `MIR_RET value` returns the value operand itself.
4. **Returns take the same treatment.** The callee's `MIR_RET` names the value;
   `emit_terminator` reads it with `read_value`, and the caller's `MIR_VALUE_CALL`
   result operand is written with `write_value` (an SSA id for a scalar or vector,
   an `OpStore` into the owned home for a composite). Nominal position 0 is gone.

The emitter side: `OpFunctionParameter` per logical parameter typed by
`ir_value_spv_type`; `OpTypeFunction` and `OpFunctionCall` sized by the encoding
(`types.instruction_operands_fit`), so the only parameter-count bound left is the
SPIR-V word-count field; a pointer parameter is `OpTypePointer Function T` and the
argument is the `OpVariable` (or forwarded `OpFunctionParameter`) itself.

## 3. Environment policy for limits and refusals

SPIR-V's 255 parameters is a minimum every consumer supports, not a maximum
(Khronos universal limits). The compiler therefore imposes no count of its own:
`emit_function` and `emit_call` refuse only when the instruction would not fit its
16-bit word count, and that refusal is a located `unsupported` diagnostic.

Pointer classes, checked against `spirv-val` 2026.3 on this host with a
hand-written module under `spv1.0`, `spv1.3`, `spv1.4`, `spv1.6`, `vulkan1.0`,
`vulkan1.1`, `vulkan1.2` and `vulkan1.3`:

| class | outcome | why |
|---|---|---|
| `*Rec` parameter, argument is a local `var` (memory object declaration) | accepted | logical addressing permits a `Function`-storage pointer parameter whose argument is an `OpVariable` |
| `*Rec` parameter forwarded from a `*Rec` parameter | accepted | an `OpFunctionParameter` is a memory object declaration |
| `*Rec` argument that is a subobject access chain (`?outer.pair`, `?n.w.m`) | refused, located | `spirv-val` rejects `OpFunctionCall` with an access-chain pointer operand in every declared environment: "Pointer operand must be a memory object declaration". Not lifted by any capability inside the declared ceilings. This is why `mem/rec_layout` stays in `SKIPS`, reclassified from MACH DEFECT to TARGET LIMITATION |
| pointer result (`fun f(p: *Rec) *Rec`) | refused, located | a `Function`-storage pointer result needs VariablePointers-class capabilities outside the declared environments |
| pointer held in a record or array, pointer-valued local | refused, located | a pointer in memory is a variable pointer |
| recursive reference type (`rec Node { next: *Node; }`) by pointer | refused at lowering, located | the pointee type itself contains a pointer |
| function value parameter | refused, located, "unsupported parameter N of type `fun(...) ...`: logical addressing has no function pointers" | no function pointers in logical addressing |
| raw union parameter | refused, located, "unsupported parameter N of type `uni{...}`: SPIR-V has no untyped union" | no untyped union type |
| pointer arithmetic (`mem/ptr_arith`) | refused | unchanged target limitation |

Debug: the v5 release contract says SPIR-V "explicitly refuses debug requests".
The refusal site is `passes.mach debug_info_of` because the `spv` object format
registers no debug model. The driver test pins it on a SPIR-V target selection.

## 4. Acceptance map

Every class in section 1 is demonstrated by a test that runs `spirv-val` on the
module (the driver tests evaluate the module as well) or by a corpus cell whose
layer A cell is `spirv-val` and whose layer B golden is `spirv-dis` text.

| class | demonstration | evidence |
|---|---|---|
| scalar, vector, composite by value, pointer to record, more than 16 arguments, composite return | `mach.lang.driver:logical_value_abi_preserves_mixed_arguments_and_references_on_spirv` (21 parameters: 17 `u32`, `f32`, `Pair`, `*Pair`, `u32x4`; returns `Pair`; evaluates the module and finds the 25-word `OpFunctionCall`) | `src/lang/driver/tests.mach` |
| more than 16 arguments in the corpus (#2923) | `call/call_mixed` layer A (o0, o2) and layer B; `mixed` has 20 `OpFunctionParameter` | `test/golden/spirv/call/call_mixed.dis` |
| pointer to record, whole object (#2940 repro) | builds and validates at `-O0` and `-O2`: `%7 = OpTypePointer Function %Pair`, `fill` takes `%9 = OpFunctionParameter %7`, each field write is `OpAccessChain %12 %9 ...` + `OpStore`, the caller passes `%26 = OpVariable %7 Function` directly: `%33 = OpFunctionCall %4 %1 %26 %32` | this document, section 4 |
| pointer to record, forwarded parameter | the driver test above (`forwarded(p, 8)` re-passes an `OpFunctionParameter`) | |
| pointer to record in the corpus | `mem/rec_layout` layer B golden blessed; layer A o2 passes; o0 declared a target limitation (subobject argument) | `test/golden/spirv/mem/rec_layout.dis`, `test/golden/spirv/SKIPS` |
| pointer to scalar | `mem/ptr_arith`'s `bump(p: *u64)` parameter is accepted; the refusal moved to the `MIR_GEP` that indexes off it | `SKIPS` entry quotes the new message |
| illegal pointer classes still refused, located | `mach.lang.driver:logical_shader_reference_capabilities_refuse_returns_subobjects_and_cycles` (pointer result, subobject argument, recursive reference type), each with `main.mach:` in the diagnostic | |
| function reference refused, located, naming the parameter and type | `mach.lang.driver:logical_shader_refuses_a_function_value_parameter_with_its_type`; corpus `call/call_indirect` quotes "unsupported parameter 1 of type `fun(i64, i64) i64` ..." | |
| union refused naming the type | corpus `mem/uni_layout` quotes "unsupported parameter 2 of type `uni{f32, i32, i32, [4]i8}` ..." | |
| readonly storage binding reached through a reference argument | `mach.lang.driver:spirv_refuses_an_escaping_readonly_storage_address` (a reference argument of a value call is a potential write to its origin) | |
| `-g` refused by policy | `mach.lang.driver:spirv_debug_request_is_refused_by_the_declared_target_policy`; the corpus reports 83 `debug unsupported` cells for spirv | |
| limit refusals are typed | `types.instruction_operands_fit` bounds `OpTypeFunction`, `OpFunctionCall` and mapped instructions by the 16-bit word count; `mach.lang.target.isa.spirv.types:logical_signature_has_no_synthetic_parameter_bank` interns a 256-parameter signature | `src/lang/target/isa/spirv/types.mach` |
| no bank anywhere | `abi/spirv.mach` publishes no `gp_arg_regs`; `register.mach` publishes no register class; `emit.mach` has no `Regs`, no `MIR_OP_PREG` arm; `mach.lang.target.abi_vtable:granularity_matches_its_classifier` checks the values convention publishes nothing a bank would need | |
| machine targets unchanged | corpus layer B on `x86_64-linux`: 91 pass, 0 fail, before and after; `tuple_capability` refuses a values convention on a register machine (`TUPLE_ABI_TRANSPORT`) | |

Two rules landed beside the port because the first green run exposed them:

- `MIR_RET` is `MIRF_NO_SHAPE`. On a values target the return carries its value
  operand, and `instr_defines_shape` would otherwise type that operand as a fresh
  definition of the machine word (an `f32` return became `f64`, a `u8` counter
  `i64`). Machine targets emit `MIR_RET` with no operands, so the flag is inert
  there.
- A reference argument of `MIR_VALUE_CALL` is a potential write to its origin.
  The candidate marked every argument read-only, which let a `*f32x4` argument
  rooted in a `readonly` storage binding pass the readonly check and fail later
  with an unrelated message.
- An `OpFunctionCall` result id is allocated after its operands are read, so the
  constants a call materializes precede the result. Twenty layer B goldens are
  renumbered by exactly that; for each the id-stripped text, the opcode multiset
  and the line count are unchanged (checked mechanically before blessing).

## 5. Verification record

| bar | result |
|---|---|
| full suite, from-source compiler | 2738 passed, 0 failed (baseline on `dev` 2633d5ba4: 2732); the six new tests are the two logical-ABI driver tests, the function-value refusal, the debug refusal, the 256-parameter signature and the typed-reference identity test |
| `sh test/census.sh` | ok |
| corpus spirv layers A and B | 248 pass, 0 fail, 100 skip (83 debug-unsupported cells, 17 declared); 22 goldens blessed: 2 new, 20 renumbered |
| corpus x86_64-linux layer B | 91 pass, 0 fail |
| `spirv-val` probe of the subobject-argument rule | rejected under spv1.0, spv1.3, spv1.4, spv1.6, vulkan1.0 through vulkan1.3 |
| mutation controls | each refusal deleted in turn, its test fails: debug refusal in `passes.mach debug_info_of` (exit 4); subobject argument check in `read_value` (exit 14); pointer-result check in `emit_function` (exit 4); recursive-reference guard in `lower/context.mach` (segfault: unbounded `lower_type` recursion); parameter-type refusal in `emit_function` (exit 5). The `MIR_RET` shape flag and the reference-argument write rule were each seen failing four and one existing tests before they landed |

## 6. The interface L6 will call

L6 carries a tag (declared discriminator plus payload composite) through SPIR-V
storage, composites, calls and returns. The surface it uses, and what may not
change:

- `abi.value_abi_vtable(id, name, arch_id, arg_passing, ret_passing)` and
  `abi.make_slot_value(size)`: a values convention classifies every argument and
  return, tag included, as one `CLASS_VALUE` slot sized by the object's extent.
  May not change: a value slot never names a register, piece or indirect extent.
- `abi.AbiVTable.passing` with `abi.PASSING_VALUES`: the one predicate lowering
  and the target layer consult. May not change: nothing derives it from the machine.
- `mir.MIR_VALUE_CALL` (`callee, result, args...`) and `mir.MIR_VALUE_PARAM`
  (`dest, index`): a tag argument or result is its owned home (`op_mem_value`).
  May not change: operand order, and that an aggregate crosses as its home.
- `mir.REG_CLASS_VALUE`: every vreg of a values function. May not change: it is
  never a bank index.
- `spirv/emit.mach spv_type_of`: a tag's IR struct (discriminator field then the
  payload) maps to `OpTypeStruct` with the same member order; `IRT_PTR` maps to
  `OpTypePointer Function`. May not change: member order is the IR field order.
- `spirv/emit.mach read_value`/`write_value`: the only two paths a logical value
  takes into an `OpFunctionCall` operand or `OpReturnValue`, and out of a result
  into a vreg or an `OpStore` into its home. May not change: a composite argument
  is an `OpLoad` of the whole object and a composite result is an `OpStore` of the
  whole object, never a per-field copy.
- `spirv/emit.mach emit_access_chain` over `MIR_GEP`: a discriminator or payload
  field of a tag in `Function` storage is reached by `OpAccessChain` with constant
  member ordinals. May not change: member ordinals are constant.
- `spirv/types.mach pointer_shape(tt, id, &storage)` and
  `instruction_operands_fit(fixed, variable)`: the pointer predicate and the only
  size bound. May not change: their signatures.
- `isa.TargetDefs`/`isa.Environment`/`environment_profile` (frozen by N1 section
  11): the environment ceilings a tag payload's scalar types are checked against.
