[mach](../README.md) / [lower](README.md) / **context**

# mach.lower.context

the state of one lowering, and the questions every lowering walk asks through it

a `LowerContext` holds the request, the mirl module being built, the scope of the module or
template being read, and the walk's locals, loops and `fin` blocks. it answers what the walks
share: the mirl type of a source type, the debug types an object describes, the secret byte
ranges of a value, function signatures and the secrecy they declare, linkage, and the functions,
globals, local instances and class copies the object declares and defines.

Source: [`src/lower/context.mach`](../../../src/lower/context.mach)

Uses types from [`mach.base.diagnostic`](../base/diagnostic.md), [`mach.base.diagnostic.kind`](../base/diagnostic/kind.md), [`mach.base.fail`](../base/fail.md), [`mach.base.float`](../base/float.md), [`mach.base.intern`](../base/intern.md), [`mach.base.source`](../base/source.md), [`mach.front.ast`](../front/ast.md), [`mach.front.ast.decl`](../front/ast/decl.md), [`mach.front.ast.expr`](../front/ast/expr.md), [`mach.front.ast.id`](../front/ast/id.md), [`mach.front.ast.type`](../front/ast/type.md), [`mach.front.comptime.capability`](../front/comptime/capability.md), [`mach.front.comptime.deep`](../front/comptime/deep.md), [`mach.front.comptime.failure`](../front/comptime/failure.md), [`mach.front.comptime.scope`](../front/comptime/scope.md), [`mach.front.comptime.value`](../front/comptime/value.md), [`mach.front.resolve`](../front/resolve.md), [`mach.front.sema.instance`](../front/sema/instance.md), [`mach.front.sema.interface`](../front/sema/interface.md), [`mach.front.sema.product`](../front/sema/product.md), [`mach.front.sema.template`](../front/sema/template.md), [`mach.lower.builder`](builder.md), [`mach.lower.linkage`](linkage.md), [`mach.lower.value`](value.md), [`mach.report.phase`](../report/phase.md), [`mach.session`](../session.md), [`mach.session.module`](../session/module.md), [`mach.target.assembled`](../target/assembled.md), [`mach.target.facts`](../target/facts.md), [`mach.type`](../type.md), [`mach.type.kind`](../type/kind.md), [`mach.type.query.extent`](../type/query/extent.md), [`mach.type.shape`](../type/shape.md).

| Type | | |
| :--- | :--- | :--- |
| [`LoopFrame`](#loopframe) | `rec` | a loop the walk is inside, which `brk` and `cnt` branch out of |
| [`Machine`](#machine) | `rec` | the target lowering builds for: the tuple its modules are named for, the target mirl assembled from it, and the context the build's selection opened, none for a target with no baseline slots:    the address space the target's frame slots live in directed: the format exports only what export directives name sealed:   the visibility an exported definition no load may preempt takes |
| [`FnInfo`](#fninfo) | `rec` | what the driver reads of a lowered module beside its mirl module: its name and, for each function of the module by index, what mach declared it as |
| [`Unit`](#unit) | `rec` | what mach keeps of a lowered module once its mirl module is compiled and gone: its name and the facts of its functions |
| [`NormalObject`](#normalobject) | `rec` | a module's test object holds its tests and `#[testing]` declarations beside its normal object, which it never changes |
| [`Signed`](#signed) | `rec` | the function that stated a symbol of the unit first: an `ext fun` or a definition, its syntax and its declaration |
| [`Defines`](#defines) | `tag` | what the object defines under a symbol, from a declaration written where its name is |
| [`Meaning`](#meaning) | `rec` | what the object means by a symbol: its own definition, or what another module declares |
| [`ModuleScope`](#modulescope) | `rec` | the module or template a lowering reads code from |
| [`SpecClaim`](#specclaim) | `rec` | the `#[gpu.spec]` var that claims a specialization id: the module that declares it, its declaration there and its qualified name |
| [`EachElement`](#eachelement) | `rec` | the element of a constant array a `$each` loop variable names: the array's expression and the element's index in it, where the element is stored |
| [`ScopeMark`](#scopemark) | `rec` | what scope_enter hands back for the matching scope_leave |
| [`ClassCopy`](#classcopy) | `rec` | a copy of a function whose pointer parameters take the classes its call sites pass, lowered into this module once the module's own declarations are |
| [`Defining`](#defining) | `tag` | how an instance's definition is bound in an object that reaches it |
| [`Reached`](#reached) | `rec` | an instance the object's code reaches, as a set being read names it: the set and its id there, and whether constant-time code calls it |
| [`InstanceReader`](#instancereader) | `rec` | how lowering reads an instance: the instance product of the instance a set names, which the reader's owner keeps alive for the lowering's life, and how its definition is bound where no constant-time code calls it |
| [`LowerRequest`](#lowerrequest) | `rec` | what one lowering of a module is asked to do and what it reads, made by request the fields not listed here are described in place |
| [`LowerContext`](#lowercontext) | `rec` | the state of one lowering: what its request gave, the mirl module and its builder, the scope being read, and the walk's locals, loops, `fin` blocks and per-function state |
| [`Declared`](#declared-1) | `rec` | a module-level declaration as the interface of its module describes it |
| [`Known`](#known) | `rec` | a tag local whose case is known in the arm being lowered, one frame on the lowering's own stack per arm it opens on |
| [`FnSig`](#fnsig) | `rec` | a function's signature: its parameters and result, each narrow integer stating the extension its source type's signedness gives it a function's ir signature and the source types it was lowered from |

| On `Machine` | |
| :--- | :--- |
| [`machine_of`](#machine_of) | the machine of a target the owners assembled, borrowing it |
| [`module_init`](#module_init) | an empty mirl module in `a`, built for the machine's target and stated as mach's |
| [`request`](#request) | a lowering request for one module, checked for every reader and table it needs |

| On `Unit` | |
| :--- | :--- |
| [`unit_init`](#unit_init) | an empty unit named `name`, its facts held in `a` |
| [`unit_dnit`](#unit_dnit) | release a unit's facts |
| [`fn_info`](#fn_info) | the facts of function `f` of a unit, none past its functions |

| On `ModuleScope` | |
| :--- | :--- |
| [`module_scope`](#module_scope) | the scope of a module lowered from its own syntax tree and sema's results |

| On `LowerContext` | |
| :--- | :--- |
| [`normal_defines`](#normal_defines) | lowering the test object, which declares `name` because the normal object defines it |
| [`origin_scope`](#origin_scope) | the scope another module's template is read in: the copy its interface publishes, with a comptime scope over its constants that this lowering owns, so what an instance binds stays here |
| [`scope_enter`](#scope_enter) | read code from `next` until the matching scope_leave, keeping the current scope |
| [`scope_leave`](#scope_leave) | return to the scope the matching scope_enter left |
| [`init_context`](#init_context) | make a lowering context for the `count` requests at `reqs`, which builds into `m` from the first and records its functions' facts in `unit` |
| [`module_enter`](#module_enter) | the context moved into the module `req` names, whose declarations it lowers next into the same mirl module: its scope, the instance set its code is read through and how it reads instances |
| [`dnit_context`](#dnit_context) | release a lowering context; a nil context holds nothing |
| [`each_element_of`](#each_element_of) | the element the innermost `$each` loop variable `name` names |
| [`interface_of`](#interface_of) | the interface of the module `origin`: the one the lowering module published, or an import's |
| [`foreign`](#foreign) | whether `sym` names a binding a module other than the one in scope declares |
| [`declared`](#declared) | the declaration `sym` names: one of the module in scope by its id, or another module's by its kind and canonical name |
| [`constant_store`](#constant_store) | the store of constant values the module with stable id `module_index` publishes in its interface, or the store of the instance being lowered |
| [`declared_named`](#declared_named) | the one declaration of `kind` the module `origin` defines under `canon` |
| [`declared_scalar`](#declared_scalar) | the scalar a module `val` holds, none when it holds no constant or an aggregate |
| [`internal`](#internal) | fail the lowering internally with `f`, a defect met at the node the lowering stands on; a failure that names a place of its own keeps it |
| [`report_gate`](#report_gate) | reject the input with a diagnostic at `span` of the module in scope |
| [`refuse_nesting`](#refuse_nesting) | the lowering recurses as deep as the input nests, on frames far larger than the checks before it, so each walk asks for stack before it descends and refuses the node it stands on |
| [`refuse_type_nesting`](#refuse_type_nesting) | the same refusal of a walk over a type, which has no place of its own, at the node the lowering stands on |
| [`refuse`](#refuse) | a failure a walk with no place of its own returned: a refusal of the input is reported under its own kind at the node the lowering stands on; any other failure is the lowering's own |
| [`refuse_at`](#refuse_at) | the same at `span`, the declaration whose walk returned it |
| [`record_eval_result`](#record_eval_result) | fail the lowering internally when a comptime evaluation failed internally, nothing otherwise |
| [`type_node`](#type_node) | the type node `tid` as name resolution read it, a literal head it split naming the type by its path's prefix and the case by the last segment |
| [`expression`](#expression) | the expression node `eid` of the module in scope as name resolution read it |
| [`resolve_module_member_const`](#resolve_module_member_const) | a lowering capability; a nil context, which the capability check refuses before any call, answers as an evaluation with no lowering would |
| [`comptime_ident_kind`](#comptime_ident_kind) | a lowering capability; a nil context, which the capability check refuses before any call, answers as an evaluation with no lowering would |
| [`resolve_type_comparison_operand`](#resolve_type_comparison_operand) | a lowering capability; a nil context, which the capability check refuses before any call, answers as an evaluation with no lowering would |
| [`resolve_type_query`](#resolve_type_query) | a lowering capability; a nil context, which the capability check refuses before any call, answers as an evaluation with no lowering would |
| [`laid_type`](#laid_type) | the type the layout intrinsics read for source type `tid`: a handle sizes and aligns as the target's `ptr`, as a name for a resource the target mints or an inert one it does not, and any other type as it lowers |
| [`resolve_layout_intrinsic`](#resolve_layout_intrinsic) | the value of the `$size_of`, `$align_of`, `$length_of`, `$offset_of` or `$type_id` call at `eid`, none for any other expression a lowering capability |
| [`member_offset`](#member_offset) | the byte offset of field or case `index` of the source type `tid`, lowered as `ir`: a record's field, a union's member at zero, or a tag case's payload, which a case with none does not have |
| [`field_offset`](#field_offset) | a field descriptor's offset, from the layout lowering gives the owning type |
| [`type_store`](#type_store) | the type store a comptime evaluation during lowering reads |
| [`begin_function`](#begin_function) | reset the per-function state to lower function `fn_index`, whose mirl result type is `ret_type`: no locals, loops or `fin` blocks, and no armed destination, result slot, exit or pack |
| [`bind_local`](#bind_local) | bind a local to `v`, the stack slot it lives in, in place of any direct binding |
| [`binds_direct`](#binds_direct) | whether a local of source type `tid`, held in `ir`, is bound to its value and has no slot: a scalar that holds no secret, or an aggregate held whole, which nothing stores to, takes the address of or names in an asm block, and, for an aggregate, whose parts nothing writes or takes the address of |
| [`holds_whole`](#holds_whole) | whether a value of source type `tid`, held in the aggregate `ir`, is held whole, a mirl value with no storage: a record, tag or union of at most two pointer widths that holds no secret, is not volatile and has no array or vector in it, so each of its members is reached by a constant path, on a target whose memory a value's bytes can be reinterpreted in |
| [`returns_whole`](#returns_whole) | whether the function being lowered returns an aggregate held whole, which its returns pass to its one exit as the exit's parameter |
| [`bind_direct`](#bind_direct) | bind a local to the value it is |
| [`lookup_direct`](#lookup_direct) | the value a local bound by bind_direct is |
| [`lookup_local`](#lookup_local) | the stack slot a local bound by bind_local lives in |
| [`push_loop`](#push_loop) | enter a loop whose `cnt` branches to `header` and `brk` to `exit` |
| [`pop_loop`](#pop_loop) | leave the innermost loop, nothing when none is open |
| [`current_loop`](#current_loop) | the innermost loop |
| [`push_fin`](#push_fin) | open a `fin` block, run when the walk leaves its enclosing block |
| [`fin_count`](#fin_count) | how many `fin` blocks are open |
| [`fin_at`](#fin_at) | the `fin` block opened `i`th, counting from the outermost |
| [`pop_fins_to`](#pop_fins_to) | close every `fin` block opened after `mark` |
| [`local_for`](#local_for) | a slot for a local whose storage begins here and ends where control leaves its block (`end_scopes_from`) |
| [`scope_count`](#scope_count) | how many locals' storage has begun and not ended |
| [`end_scopes_from`](#end_scopes_from) | the storage of each local begun since `mark` ended, the latest first, where control still reaches |
| [`pop_scopes_to`](#pop_scopes_to) | forget every local begun after `mark`, whose storage its block's exits have ended |
| [`ptr_or_none`](#ptr_or_none) | the pointer type, which every address is held in |
| [`builder_state`](#builder_state) | the first refusal the builder met, which every builder call after it skips |
| [`int_type`](#int_type) | the mirl integer type of `bits` |
| [`ptr_type`](#ptr_type) | the pointer type every address is held in, which never fails |
| [`lower_type`](#lower_type) | the mirl type a value of the source type `tid` is held in |
| [`texel_number`](#texel_number) | the number the texels of the handle type `ty` are, as the target's catalog relates them to a word operand of its kind, none for a handle whose kind relates none, one whose operand names any, or another type |
| [`tag_case_has_payload`](#tag_case_has_payload) | whether case `i` of the tag `tid` holds a payload |
| [`tag_carried`](#tag_carried) | where the tag `tid` holds the payload of case `i`, which has one: the area of the tag's structure and the member of that area's union, as its placement gives it |
| [`storage_align`](#storage_align) | the alignment a value held in `ir` is stored at: the target's layout of `ir`, which states a decorated record's alignment and packing |
| [`leave`](#leave) | return `v` from the function, none for no value |
| [`emit_exit`](#emit_exit) | write the one exit the function's returns branch to, where it has one |
| [`known_case`](#known_case) | the case known of the local named by `eid` where lowering stands, none where none is |
| [`tag_payload`](#tag_payload) | the area and member that hold case `i` of the tag `tid`, none for a case that holds nothing |
| [`result_slot`](#result_slot) | the slot an aggregate result of the function being lowered is built in and returned from: one for every return, so the result is built where the caller receives it, made on first ask |
| [`take_into`](#take_into) | the armed destination, taken once, when the expression being lowered is the armed one and builds an aggregate of `ir`, which the destination holds; none for any other |
| [`slot_for`](#slot_for) | a stack slot for a value of the source type `tid`, held in `ir`, with the bytes `tid` lays out as public stated |
| [`state_slot`](#state_slot) | state on the alloca of `slot`, which holds a value of source type `tid` held in `ir`, the bytes `tid` lays out as public (`public_ranges`), nothing for a type that is no type |
| [`public_ranges`](#public_ranges) | the public byte ranges of a value of source type `tid`, held in `ir`, into `out`, ascending and none touching another: every byte its secret ranges leave, a tag's discriminant and padding included |
| [`debug_type_of`](#debug_type_of) | the debug type a debug record names for source type `tid`; none when this lowering keeps no debug information |
| [`debug_own_types`](#debug_own_types) | the types this object describes for every other: each record, union and tag the module declares that the object emits the declaration of, the test object a `#[testing]` one and the normal object any other |
| [`debug_types_finish`](#debug_types_finish) | describe the types this object owns, then every pointee left waiting, then link the table |
| [`block_terminated`](#block_terminated) | whether the block being built already ends |
| [`is_void_ir`](#is_void_ir) | whether the mirl type `tid` is no type, as a function with no result returns |
| [`expr_type_of`](#expr_type_of) | an expression's type inside an instance's scope is the instance's, never the template's: sema records the template's and every type decision here is made against this instance an expression's type as the frame the code is read through decided it, else the template's under the instance's substitution |
| [`expr_float_width`](#expr_float_width) | the float width of expression `eid`'s type, a secret one's base |
| [`expr_is_secret`](#expr_is_secret) | whether expression `eid`'s type is outermost secret |
| [`type_is_secret`](#type_is_secret) | whether the source type `tid` is outermost secret |
| [`type_contains_secret`](#type_contains_secret) | whether a value of the source type `tid` holds a secret in any of its own bytes |
| [`secret_ranges`](#secret_ranges) | the secret byte ranges of a value of source type `tid`, held in `ir`, laid out from `base`, appended to `out` in no order: a secret's every byte, an aggregate's secret members and elements, every case of a union or a tag's payload, and nothing of a tag's discriminant or of padding |
| [`global_secrecy`](#global_secrecy) | how much of a global of source type `tid`, held in `ir`, is secret, its ranges held in `ranges`, which must outlive the answer until the module copies it |
| [`substitute_type`](#substitute_type) | a template's type as the instance being lowered holds it, applied once where a type is read from sema's tables, so every type the lowering passes on is already the instance's |
| [`type_resolved_of`](#type_resolved_of) | the source type the type node `tid` names where the code is read: a frame's own where its comptime bindings size the node, else the template's under the instance's substitution |
| [`decl_type_of`](#decl_type_of) | the source type of declaration `did`, as the frame the code is read through decided it, else the template's under the instance's substitution |
| [`instance_id_of_call`](#instance_id_of_call) | the index in the set being read of the instance sema decided the call or generic reference `eid` names, in the frame the code is read through |
| [`instance_link_name`](#instance_link_name) | the symbol an instance is emitted and referenced under |
| [`gate_of`](#gate_of) | the verdict sema gave a statement gate in the frame the code is read through |
| [`each_array`](#each_array) | the published value of the array sema typed the `$each` at `stmt` over, in the frame the code is read through |
| [`iteration_frame`](#iteration_frame) | the frame of iteration `index` of the `$each` at `stmt` under the current frame |
| [`decl_ret_sem`](#decl_ret_sem) | the source result type of the function `did` declares |
| [`lower_fn_sig`](#lower_fn_sig) | the signature of the function `did` declares |
| [`fn_sig_of_type`](#fn_sig_of_type) | the signature a value of the source function type `tid` is called through |
| [`c_variadic_fixed`](#c_variadic_fixed) | how many parameters the C-variadic function type `tid` names, none for any other type |
| [`variadic_sig_of_type`](#variadic_sig_of_type) | the signature one call of the C-variadic function type `tid` is made through: its named parameters and then the `tail_len` types at `tail` of the arguments it passes unnamed, as mirl states a variadic call |
| [`ext_of`](#ext_of) | the extension a convention gives an integer narrower than a register: its declared signedness |
| [`declared_secrets`](#declared_secrets) | the secret parameters a function of signature `sig` declares, into `out`, its secret result, into `result`, and the positions of its parameters that point to storage holding no secret, into `pointers`: each as secret as its source type holds a secret in its own bytes, the bytes a pointer reaches being its pointee's and never the pointer's |
| [`pack_instance_sig`](#pack_instance_sig) | the signature of a pack instance of the function typed `fn_ty`: its fixed parameters, then one per element of the pack |
| [`instance_ref_sig`](#instance_ref_sig) | the signature an instance is referenced under: the one sema settled when it asked for it |
| [`expr_symbol_of`](#expr_symbol_of) | the symbol name resolution bound expression `eid` to, rejecting the phase for a rejected one |
| [`module_source`](#module_source) | the source of the module in scope, read when the scope was made |
| [`emit_dbg_birth`](#emit_dbg_birth) | a source variable named at `name_span` begins: one held in a stack slot is bound to the slot, which describes it wherever it lives, and any other, a parameter, to its value from here on |
| [`describe_global`](#describe_global) | the source variable the module's global `g` is, named at `name_span` with source type `ty_sem`, which a debugger reads its type and declaration line from; nothing when this lowering keeps no debug information |
| [`symbol_of`](#symbol_of) | the symbol `sid` of the module in scope |
| [`symbol_name`](#symbol_name) | the handle of the symbol name `name` in the module's names, interned once, by which its function or global is declared |
| [`image_linkage`](#image_linkage) | the linkage of a definition every module of the image may reach and no other image sees, `pub` or not, unless `#[export]` says so, and of a reference to a private declaration, which an instance another module lowers reaches |
| [`internal_visibility`](#internal_visibility) | the visibility that keeps a definition inside its image: hidden, or the default where the format exports only what its export directives name and states no visibility, as PE does |
| [`add_function`](#add_function) | add a function to the module with what mach declares of it |
| [`function_named`](#function_named) | the function of the module named `name`, none when no function is |
| [`global_named`](#global_named) | the global of the module named `name`, none when no global is |
| [`define_function`](#define_function) | the module's definition of a function: a reference earlier in the module declared it, and the definition takes the declaration's place |
| [`hold_signature`](#hold_signature) | the unit holds one function for a symbol, so the `ext fun`s and the definition its modules state the symbol `name` with, as the function `d` declared by `did` does, state one signature |
| [`hold_definition`](#hold_definition) | the object defines the symbol `name` as `what`, from the declaration whose name the module or template in scope writes at `span` |
| [`hold_reference`](#hold_reference) | the object refers, at the node the lowering stands on, to the declaration `decl` of the module `origin` by the symbol `name` |
| [`bytes_name`](#bytes_name) | the symbol of the bytes the string `val` linked as `name` points at, which the val's module defines beside it |
| [`define_global`](#define_global) | the module's definition of the global `name`, in place of a declaration a reference made |
| [`ensure_extern_function`](#ensure_extern_function) | the module's function `name`, declared here with `linkage` when another object defines it, with the mode `timed` its declaration states and the secret parameters and result its signature does, which are what a caller may trust of it |
| [`stated_callee`](#stated_callee) | the declaration a call through a value of the function type `tid` states its callee keeps, none for a type marked with no constant-time mode: the type's mode, and the secret parameters, secret result and public pointers a function of that signature declares, so the call is trusted as one naming such a function is |
| [`mode_of`](#mode_of) | the mode the function `decl` of module `origin` is declared with, as its module's interface publishes it |
| [`class_copy`](#class_copy) | the copy of the function `decl` of `origin`, linked as `base`, whose pointer parameters take the classes `sig` states, declared here and lowered once the module's own declarations are, of the instance `instance` of the set being read where it is one |
| [`building_constant_time`](#building_constant_time) | the function being built requires constant time |
| [`reach_instance`](#reach_instance) | the instance `id` of the set being read, named `name`, is reached from the code being lowered: it is lowered into this object once the object's own code is, and when `local`, constant-time code calls it, its copy is local, since the walk that verifies the caller reads only the code its object holds |
| [`localize`](#localize) | the instance this object defines under `name`, reached after it was lowered from constant-time code, bound local to the object |
| [`ensure_extern_global`](#ensure_extern_global) | the module's global `name` of `ty`, of source type `sem` (TYPE_NIL for bytes no source type describes), declared here with `linkage` when another object defines it, which the program never writes where `constant` holds |
| [`thread_model`](#thread_model) | how a thread-local of this lowering's image is reached: an executable's own block sits at an offset the link fixes, and a shared library's at one its loader fixes at start, so a library that loads after start has no static block to hold it (no general-dynamic access is lowered) |
| [`declared_thread`](#declared_thread) | the thread model a reference to the module binding `d` declares takes, none for one that is not thread-local |
| [`add_global`](#add_global) | add a global to the module |
| [`constant_global`](#constant_global) | a read-only global of the module's own holding the constant `v`, whose address is the value |

| On `FnSig` | |
| :--- | :--- |
| [`bare_sig`](#bare_sig) | the ir signature `sig` with no source types |

| Functions | |
| :--- | :--- |
| [`external_linkage`](#external_linkage) | the linkage of a symbol another module defines, or a definition every module sees |
| [`declaration_mode`](#declaration_mode) | the mode the declaration `d` states its function holds constant time in: none for one a definition elsewhere may replace, as a `#[weak]` default, since what runs need not be the body that was checked |

---

## LoopFrame

```mach
pub rec LoopFrame {
    header:      mirl.ir.id.Block;
    exit:        mirl.ir.id.Block;
    fin_depth:   u32;
    scope_depth: u32;
}
```

a loop the walk is inside, which `brk` and `cnt` branch out of

| Field | Type | |
| :--- | :--- | :--- |
| `header` | `mirl.ir.id.Block` | the block `cnt` branches to |
| `exit` | `mirl.ir.id.Block` | the block `brk` branches to |
| `fin_depth` | `u32` | the `fin` blocks open when the loop began, which a jump out of it runs down to |
| `scope_depth` | `u32` | the locals whose storage had begun when the loop began, whose storage a jump out of it does not end |

## Machine

```mach
pub rec Machine {
    tuple:    mirl.ir.module.Tuple;
    target:   *mirl.target.record.Target;
    ctx:      opt[*masc.core.state.Context];
    slots:    u32;
    directed: bool;
    sealed:   *mink.model.symbol.Visibility;
}
```

the target lowering builds for: the tuple its modules are named for, the target mirl
assembled from it, and the context the build's selection opened, none for a target with
no baseline
slots:    the address space the target's frame slots live in
directed: the format exports only what export directives name
sealed:   the visibility an exported definition no load may preempt takes

### machine_of

```mach
pub fun machine_of(x: *mach.target.assembled.Assembled) res[Machine, mach.base.fail.Fail]
```

the machine of a target the owners assembled, borrowing it

### module_init

```mach
pub fun module_init(a: *std.allocator.Allocator, machine: *Machine) res[mirl.ir.module.Module, mach.base.fail.Fail]
```

an empty mirl module in `a`, built for the machine's target and stated as mach's

### request

```mach
pub fun request(
    s: *mach.session.Session,
    l: *mach.lower.linkage.Linkage,
    named: *mach.lower.linkage.Named,
    facts: *mach.target.facts.Facts,
    machine: *Machine,
    laying: *mach.type.query.extent.Laying,
    malloc: *std.allocator.Allocator,
    a: *mach.front.ast.Ast,
    fqn: mach.base.intern.StrId,
    own_module: mach.session.module.ModuleId,
    sema_result: *mach.front.sema.product.SemaResult,
    rr: *mach.front.resolve.ResolveResult,
    ctx: *mach.front.comptime.scope.ComptimeCtx,
    normal_object: *NormalObject,
    shared_artifact: bool,
    debug: bool,
    standalone: bool,
    checks: bool,
    diags: *mach.base.diagnostic.DiagnosticStore,
    interfaces: mach.front.sema.interface.Reader,
    typed: InstanceReader) res[LowerRequest, mach.base.fail.Fail]
```

a lowering request for one module, checked for every reader and table it needs

| Parameter | |
| :--- | :--- |
| `s` | the session |
| `l` | the project's linkage |
| `facts` | the target's facts |
| `machine` | the target lowering builds for |
| `laying` | what the build lays its types out by |
| `malloc` | allocator for the unit's facts |
| `a` | the module's syntax tree |
| `fqn` | its qualified name |
| `own_module` | its module id |
| `sema_result` | sema's result for it |
| `rr` | its name resolution |
| `ctx` | its comptime context |
| `normal_object` | nil for the normal object, what it defines for the test object |
| `shared_artifact` | whether the artifact fed is a shared library |
| `debug` | whether debug information is kept |
| `standalone` | whether the objects are an archive's members |
| `checks` | whether runtime checks are emitted |
| `diags` | the diagnostic store |
| `interfaces` | the reader of imported interfaces |
| `typed` | the reader of typed instances |
| **returns** | the request, borrowing what it is given, or the refusal of a missing piece |

## FnInfo

```mach
pub rec FnInfo {
    name:     mach.base.intern.StrId;
    line:     u32;
    test:     bool;
    noreturn: bool;
    defined:  bool;
}
```

what the driver reads of a lowered module beside its mirl module: its name and, for
each function of the module by index, what mach declared it as

| Field | Type | |
| :--- | :--- | :--- |
| `test` | `bool` | a test, run by the dispatcher under `name` |
| `noreturn` | `bool` | it never returns, so falling off its end is unreachable |
| `defined` | `bool` | the module gives it a body |

## Unit

```mach
pub rec Unit {
    name: mach.base.intern.StrId;
    fns:  std.collections.vector.Vector[FnInfo];
}
```

what mach keeps of a lowered module once its mirl module is compiled and gone: its name
and the facts of its functions

### unit_init

```mach
pub fun unit_init(a: *std.allocator.Allocator, name: mach.base.intern.StrId) Unit
```

an empty unit named `name`, its facts held in `a`

### unit_dnit

```mach
pub fun unit_dnit(u: *Unit)
```

release a unit's facts

### fn_info

```mach
pub fun fn_info(u: *Unit, f: u32) opt[*FnInfo]
```

the facts of function `f` of a unit, none past its functions

## NormalObject

```mach
pub rec NormalObject {
    defined: std.collections.map.Map[mach.base.intern.StrId, bool];
}
```

a module's test object holds its tests and `#[testing]` declarations beside its
normal object, which it never changes. every definition the normal object already
holds is declared in the test object instead, so each symbol has one definition

| Field | Type | |
| :--- | :--- | :--- |
| `defined` | `std.collections.map.Map[mach.base.intern.StrId, bool]` | the global symbols the normal object defines |

## Signed

```mach
pub rec Signed {
    a:    *mach.front.ast.Ast;
    decl: mach.front.ast.id.DeclId;
    sem:  mach.type.kind.TypeId;
}
```

the function that stated a symbol of the unit first: an `ext fun` or a definition, its syntax
and its declaration

| Field | Type | |
| :--- | :--- | :--- |
| `a` | `*mach.front.ast.Ast` | the syntax that declares it |
| `decl` | `mach.front.ast.id.DeclId` | its declaration |
| `sem` | `mach.type.kind.TypeId` | its function type |

## Defines

```mach
pub tag Defines: u8 {
    declaration;
    default;
    instance;
    copy;
    testing;
}
```

what the object defines under a symbol, from a declaration written where its name is. a `#[weak]` default is
replaced at link by the definition another module gives, so it is not the module's meaning of the symbol

## Meaning

```mach
pub rec Meaning {
    defined:  opt[Definition];
    referred: opt[Referral];
}
```

what the object means by a symbol: its own definition, or what another module declares

## ModuleScope

```mach
pub rec ModuleScope {
    a:           *mach.front.ast.Ast;
    source:      str;
    fqn:         mach.base.intern.StrId;
    own_module:  mach.session.module.ModuleId;
    own_stable:  mach.session.module.StableModuleId;
    typing:      mach.front.sema.template.Typing;
    iface:       *mach.front.sema.interface.Interface;
    rr:          *mach.front.resolve.ResolveResult;
    env:         mach.front.comptime.scope.ComptimeEnv;
    ctx:         *mach.front.comptime.scope.ComptimeCtx;
    subst:       *mach.type.kind.TypeId;
    subst_len:   u32;
    subst_owner: mach.type.GenericOwner;
    # the typing frame of the lowering module's instance set its code is read through:
    # the module's own typing, an instance's, or one of their iterations; NONE for another
    # module's code read outside an instance
    frame: u32;
}
```

the module or template a lowering reads code from

| Field | Type | |
| :--- | :--- | :--- |
| `a` | `*mach.front.ast.Ast` | its syntax tree |
| `source` | `str` | its source text |
| `fqn` | `mach.base.intern.StrId` | its qualified name |
| `own_module` | `mach.session.module.ModuleId` | its module id |
| `own_stable` | `mach.session.module.StableModuleId` | its stable module id |
| `typing` | `mach.front.sema.template.Typing` | sema's types of its expressions, type nodes and declarations |
| `iface` | `*mach.front.sema.interface.Interface` | the interface it publishes |
| `rr` | `*mach.front.resolve.ResolveResult` | its name resolution |
| `env` | `mach.front.comptime.scope.ComptimeEnv` | the comptime environment its constants are read in |
| `ctx` | `*mach.front.comptime.scope.ComptimeCtx` | its comptime context |
| `subst` | `*mach.type.kind.TypeId` | the type arguments an instance substitutes, nil for none |
| `subst_len` | `u32` | how many there are |
| `subst_owner` | `mach.type.GenericOwner` | the generic whose parameters they stand for |
| `frame` | `u32` | the typing frame, described in place |

### module_scope

```mach
pub fun module_scope(
    s: *mach.session.Session,
    own_module: mach.session.module.ModuleId,
    a: *mach.front.ast.Ast,
    fqn: mach.base.intern.StrId,
    sema_result: *mach.front.sema.product.SemaResult,
    rr: *mach.front.resolve.ResolveResult,
    ctx: *mach.front.comptime.scope.ComptimeCtx) res[ModuleScope, mach.base.fail.Fail]
```

the scope of a module lowered from its own syntax tree and sema's results

| Parameter | |
| :--- | :--- |
| `s` | the session |
| `own_module` | the module's id |
| `a` | its syntax tree |
| `fqn` | its qualified name |
| `sema_result` | sema's result for it |
| `rr` | its name resolution |
| `ctx` | its comptime context |
| **returns** | the scope, borrowing each piece, or the refusal of a missing piece or of pieces made from another tree or module |

## SpecClaim

```mach
pub rec SpecClaim {
    origin: mach.session.module.ModuleId;
    decl:   mach.front.ast.id.DeclId;
    name:   str;
}
```

the `#[gpu.spec]` var that claims a specialization id: the module that declares it, its
declaration there and its qualified name

## EachElement

```mach
pub rec EachElement {
    name:  mach.base.intern.StrId;
    array: mach.front.ast.id.ExprId;
    index: u32;
}
```

the element of a constant array a `$each` loop variable names: the array's expression and the
element's index in it, where the element is stored

## ScopeMark

```mach
pub rec ScopeMark {
    owner:      ptr;
    depth:      u32;
    generation: u64;
    parent:     u64;
}
```

what scope_enter hands back for the matching scope_leave

| Field | Type | |
| :--- | :--- | :--- |
| `owner` | `ptr` | the context that made it |
| `depth` | `u32` | the scope stack's depth after the enter |
| `generation` | `u64` | the enter's generation, unique within the context |
| `parent` | `u64` | the generation of the enclosing enter, 0 for none |

## ClassCopy

```mach
pub rec ClassCopy {
    origin:   mach.session.module.ModuleId;
    decl:     mach.front.ast.id.DeclId;
    name:     mach.base.intern.StrId;
    sig:      Type;
    instance: u32;
    set:      *mach.front.sema.instance.Set;
}
```

a copy of a function whose pointer parameters take the classes its call sites pass,
lowered into this module once the module's own declarations are

| Field | Type | |
| :--- | :--- | :--- |
| `origin` | `mach.session.module.ModuleId` | the module that declares the function |
| `decl` | `mach.front.ast.id.DeclId` | its declaration there |
| `name` | `mach.base.intern.StrId` | the copy's linkage name, the function's own with the classes it takes |
| `sig` | `Type` | the copy's signature |
| `instance` | `u32` | the instance of `set` the copy is of, sema_instance.NONE for a plain function |
| `set` | `*mach.front.sema.instance.Set` | the instance set whose frames the code asking for the copy was read through |

## Defining

```mach
pub tag Defining: u8 {
    local;
    instance;
    merged: u64;
}
```

how an instance's definition is bound in an object that reaches it

| Case | Payload | |
| :--- | :--- | :--- |
| `local` |  | the copy constant-time code of this object calls, local to it |
| `instance` |  | weak, every definition of its name equivalent, for a target whose objects hold no merged definitions |
| `merged` | `u64` | a merged definition of this source identity (mink.model.merged), every copy of its name and identity made from the same source |

## Reached

```mach
pub rec Reached {
    set:   *mach.front.sema.instance.Set;
    id:    u32;
    local: bool;
}
```

an instance the object's code reaches, as a set being read names it: the set and its id there,
and whether constant-time code calls it. the code lowered for it is read through the instance's
own typing, which the request's reader gives

## InstanceReader

```mach
pub rec InstanceReader {
    ctx:  ptr;
    read: fun(ptr, *mach.front.sema.instance.Instance) res[*mach.front.sema.product.InstanceResult, mach.base.fail.Fail];
    bind: fun(ptr, *mach.front.sema.instance.Instance) res[Defining, mach.base.fail.Fail];
}
```

how lowering reads an instance: the instance product of the instance a set names, which the
reader's owner keeps alive for the lowering's life, and how its definition is bound where no
constant-time code calls it

## LowerRequest

```mach
pub rec LowerRequest {
    # the declarations' export names, the project's
    linkage: *mach.lower.linkage.Linkage;
    # the names the lowering's declarations link under, kept as it names them
    named:      *mach.lower.linkage.Named;
    interfaces: mach.front.sema.interface.Reader;
    typed:      InstanceReader;
    instances:  *mach.front.sema.instance.Set;
    diags:      *mach.base.diagnostic.DiagnosticStore;
    s:          *mach.session.Session;
    facts:      *mach.target.facts.Facts;
    machine:    *Machine;
    # what the build lays its types out by, which places a tag's cases for sema and lowering
    laying: *mach.type.query.extent.Laying;
    malloc: *std.allocator.Allocator;
    module: ModuleScope;
    # nil when lowering the module's normal object; when lowering its test object, what
    # the normal object defines
    normal_object: *NormalObject;
    # the artifact this lowering feeds is a shared library, so a `fwd` that
    # exports nothing is worth a warning
    shared_artifact: bool;
    debug:           bool;
    # the objects are an archive's members, which a link takes only some of, so each
    # describes every type it names (see debug_describe)
    standalone: bool;
    checks:     bool;
}
```

what one lowering of a module is asked to do and what it reads, made by request
the fields not listed here are described in place

| Field | Type | |
| :--- | :--- | :--- |
| `interfaces` | `mach.front.sema.interface.Reader` | the reader of imported modules' interfaces |
| `typed` | `InstanceReader` | the reader of typed instances |
| `instances` | `*mach.front.sema.instance.Set` | the module's own instance set |
| `diags` | `*mach.base.diagnostic.DiagnosticStore` | the store diagnostics are reported to |
| `s` | `*mach.session.Session` | the session |
| `facts` | `*mach.target.facts.Facts` | the target's facts |
| `machine` | `*Machine` | the target lowering builds for |
| `malloc` | `*std.allocator.Allocator` | the allocator the unit's facts are held in |
| `module` | `ModuleScope` | the scope of the module lowered |
| `debug` | `bool` | whether debug information is kept |
| `checks` | `bool` | whether debug-profile runtime checks are emitted |

## LowerContext

```mach
pub rec LowerContext {
    interfaces: mach.front.sema.interface.Reader;
    # the scopes another module's templates are lowered in, one per module, which this
    # lowering owns and binds in
    foreign:          std.collections.vector.Vector[ForeignScope];
    diags:            *mach.base.diagnostic.DiagnosticStore;
    status:           mach.report.phase.PhaseStatus;
    s:                *mach.session.Session;
    linkage:          *mach.lower.linkage.Linkage;
    named:            *mach.lower.linkage.Named;
    facts:            *mach.target.facts.Facts;
    machine:          *Machine;
    laying:           *mach.type.query.extent.Laying;
    scope:            ModuleScope;
    scopes:           *ModuleScope;
    scope_len:        u32;
    scope_cap:        u32;
    scope_generation: u64;
    scope_top:        u64;
    caps:             mach.front.comptime.capability.PhaseCapabilities[LowerContext];
    shared_artifact:  bool;
    debug:            bool;
    standalone:       bool;
    # debug-profile runtime checks: the release pipeline emits none
    checks:  bool;
    unit:    *Unit;
    m:       *mirl.ir.module.Module;
    builder: mach.lower.builder.Builder;
    locals:  std.collections.map.Map[mach.front.resolve.SymbolId, mach.lower.value.Value];
    # the locals bound to their value, which have no slot (see binds_direct)
    values: std.collections.map.Map[mach.front.resolve.SymbolId, mach.lower.value.Value];

    fn_index:   u32;
    ret_type:   Type;
    fn_ret_sem: mach.type.kind.TypeId;
    # the signature of the function being built, whose pointer parameters a copy of a
    # function takes in the classes its call sites pass
    fn_sig: Type;
    # the function being built is `#[naked]`, its body asm that binds no local
    fn_naked: bool;

    # the module's debug types by the substituted source type each describes
    debug_types: std.collections.map.Map[mach.type.kind.TypeId, mirl.ir.debug.record.Type];
    # whether each type holds a secret in its own storage, asked once per type: 1 when
    # it does, 2 when it does not
    held: std.collections.map.Map[mach.type.kind.TypeId, u8];
    # the mirl type of each aggregate source type, by the substituted type: lowered once
    lowered: std.collections.map.Map[mach.type.kind.TypeId, Type];
    # the declaration a call through a value of each function type marked constant time states its callee keeps,
    # stated once per type
    stated: std.collections.map.Map[mach.type.kind.TypeId, mirl.ir.id.Stated];
    # the first function each symbol is stated by, where the unit takes several modules
    signed: std.collections.map.Map[mach.base.intern.StrId, Signed];
    # the requests of the modules the unit lowers into its one object, several for an artifact
    # compiled whole, and how many
    requests:    *LowerRequest;
    request_len: usize;
    # what the object means by each symbol it defines or refers to another module's declaration by
    meanings: std.collections.map.Map[mach.base.intern.StrId, Meaning];
    # the read-only global holding each string's bytes, by its text: one per module
    strings: std.collections.map.Map[mach.base.intern.StrId, mach.lower.value.Value];
    # the pointees the debug types name by key, described once every other type is
    debug_pointees: std.collections.vector.Vector[mach.type.kind.TypeId];
    # the records, unions and tags this object describes for every other, in the order the
    # module declares them, and each by its type (see debug_own_types)
    debug_owns:  std.collections.vector.Vector[mach.type.kind.TypeId];
    debug_owned: std.collections.map.Map[mach.type.kind.TypeId, bool];
    # where the first of them is declared, which states the module's file to an object
    # whose code states none
    debug_owned_at: mach.base.source.Span;

    loops:    *LoopFrame;
    loop_len: u32;
    loop_cap: u32;

    fins:    *mach.front.ast.id.StmtId;
    fin_len: u32;
    fin_cap: u32;

    # the slots of the locals whose storage has begun and not yet ended, innermost last
    scoped: std.collections.vector.Vector[mach.lower.value.Value];

    # the instance set whose frames the code being lowered is read through: the module's own,
    # or the set of the instance being lowered, and the store that instance's constants are in
    instances:       *mach.front.sema.instance.Set;
    instance_values: *mach.front.comptime.deep.Store;
    typed:           InstanceReader;
    # the `$each` elements the statements being lowered bind, innermost last
    elements: std.collections.vector.Vector[EachElement];
    # the class copies the module's calls asked for, in the order they asked
    copies: std.collections.vector.Vector[ClassCopy];
    # the instances the object's code reaches, each lowered into this object, in the order they
    # were reached, and each one's place there by its symbol
    reached:    std.collections.vector.Vector[Reached];
    reached_at: std.collections.map.Map[mach.base.intern.StrId, u32];
    # the scratch store of the zeros an element literal's omitted fields hold, one tree per type
    # for the context's life
    zeros: mach.front.comptime.deep.Store;
    # the `#[gpu.spec]` var each specialization id the unit reads names, by the id
    specs: std.collections.map.Map[u32, SpecClaim];

    pack_slots:    *mach.lower.value.Value;
    pack_types:    *mach.type.kind.TypeId;
    pack_len:      u32;
    pack_ret_type: mach.type.kind.TypeId;

    normal_object: *NormalObject;

    # while armed, the call argument whose aggregate read may use its source in place;
    # borrow_now says the read being lowered is that one, borrow_hit that it did
    borrow_armed: bool;
    borrow_eid:   mach.front.ast.id.ExprId;
    borrow_now:   bool;
    borrow_hit:   bool;
    # while armed, the expression whose aggregate value may be built in `into_slot`, storage
    # nothing else reads yet; into_now says the expression being lowered is that one, into_hit
    # that its value was built there
    into_armed: bool;
    into_eid:   mach.front.ast.id.ExprId;
    into_slot:  mach.lower.value.Value;
    into_now:   bool;
    into_hit:   bool;
    # the slot every return of the function being lowered hands an aggregate back from, made
    # at the first return
    ret_slot:     mach.lower.value.Value;
    ret_slot_set: bool;
    # the statements of the list being lowered after the one being lowered, from `rest_start`
    # to `rest_end` of the statement id table, which a local's declaration reads its scope from
    rest_start: u32;
    rest_end:   u32;
    # the innermost tag local whose case is known where lowering stands (see Known)
    known: *Known;
    # the function's one exit, made at its first return (see leave)
    exit:     mirl.ir.id.Block;
    exit_set: bool;
    # the expression statement being lowered, whose value nothing reads
    discard_eid: mach.front.ast.id.ExprId;
}
```

the state of one lowering: what its request gave, the mirl module and its builder, the
scope being read, and the walk's locals, loops, `fin` blocks and per-function state.
init_context makes it and dnit_context releases it, and its fields are described in place

### normal_defines

```mach
pub fun normal_defines(lc: *LowerContext, name: mach.base.intern.StrId) bool
```

lowering the test object, which declares `name` because the normal object defines it

### origin_scope

```mach
pub fun origin_scope(lc: *LowerContext, origin: mach.session.module.ModuleId) res[ModuleScope, mach.base.fail.Fail]
```

the scope another module's template is read in: the copy its interface publishes, with a
comptime scope over its constants that this lowering owns, so what an instance binds stays here

### scope_enter

```mach
pub fun scope_enter(lc: *LowerContext, next: ModuleScope) res[ScopeMark, mach.base.fail.Fail]
```

read code from `next` until the matching scope_leave, keeping the current scope

| Parameter | |
| :--- | :--- |
| `lc` | the lowering context |
| `next` | the scope to read from |
| **returns** | the mark scope_leave takes, or the refusal of a nil context, an exhausted generation, or a stack that cannot grow |

### scope_leave

```mach
pub fun scope_leave(lc: *LowerContext, mark: ScopeMark) err[mach.base.fail.Fail]
```

return to the scope the matching scope_enter left

| Parameter | |
| :--- | :--- |
| `lc` | the lowering context |
| `mark` | the mark scope_enter handed back |
| **returns** | nothing, or the refusal of a mark of another context or out of order |

### init_context

```mach
pub fun init_context(lc: *LowerContext, reqs: *LowerRequest, count: usize, m: *mirl.ir.module.Module, unit: *Unit) err[mach.base.fail.Fail]
```

make a lowering context for the `count` requests at `reqs`, which builds into `m` from the first
and records its functions' facts in `unit`

| Parameter | |
| :--- | :--- |
| `lc` | the lowering context |
| `reqs` | the lowering requests, one per module the unit lowers, borrowed |
| `count` | how many there are |
| `m` | the mirl module built into, borrowed |
| `unit` | the unit the facts go to, borrowed |
| **returns** | nothing, or the refusal, which leaves nothing for dnit_context to release |

### module_enter

```mach
pub fun module_enter(lc: *LowerContext, req: *LowerRequest)
```

the context moved into the module `req` names, whose declarations it lowers next into the same
mirl module: its scope, the instance set its code is read through and how it reads instances.
what the context already lowered and reached stays, so a unit of several modules defines each
symbol once

### dnit_context

```mach
pub fun dnit_context(lc: *LowerContext)
```

release a lowering context; a nil context holds nothing

### each_element_of

```mach
pub fun each_element_of(lc: *LowerContext, name: mach.base.intern.StrId) opt[EachElement]
```

the element the innermost `$each` loop variable `name` names

### interface_of

```mach
pub fun interface_of(lc: *LowerContext, origin: mach.session.module.ModuleId) res[*mach.front.sema.interface.Interface, mach.base.fail.Fail]
```

the interface of the module `origin`: the one the lowering module published, or an import's

### foreign

```mach
pub fun foreign(lc: *LowerContext, sym: *mach.front.resolve.Symbol) bool
```

whether `sym` names a binding a module other than the one in scope declares

### declared

```mach
pub fun declared(lc: *LowerContext, sym: *mach.front.resolve.Symbol) res[Declared, mach.base.fail.Fail]
```

the declaration `sym` names: one of the module in scope by its id, or another module's by
its kind and canonical name

### constant_store

```mach
pub fun constant_store(lc: *LowerContext, module_index: u32) res[*mach.front.comptime.deep.Store, mach.base.fail.Fail]
```

the store of constant values the module with stable id `module_index` publishes in its interface,
or the store of the instance being lowered

### declared_named

```mach
pub fun declared_named(lc: *LowerContext, origin: mach.session.module.ModuleId, kind: mach.front.resolve.SymKind, canon: mach.base.intern.StrId) res[Declared, mach.base.fail.Fail]
```

the one declaration of `kind` the module `origin` defines under `canon`

### declared_scalar

```mach
pub fun declared_scalar(lc: *LowerContext, d: Declared) opt[mach.front.comptime.value.CTValue]
```

the scalar a module `val` holds, none when it holds no constant or an aggregate

### internal

```mach
pub fun internal(lc: *LowerContext, f: mach.base.fail.Fail)
```

fail the lowering internally with `f`, a defect met at the node the lowering stands on; a
failure that names a place of its own keeps it

### report_gate

```mach
pub fun report_gate(lc: *LowerContext, k: mach.base.diagnostic.kind.Kind, span: mach.base.source.Span, message: str) mach.base.fail.Fail
```

reject the input with a diagnostic at `span` of the module in scope

| Parameter | |
| :--- | :--- |
| `lc` | the lowering context |
| `k` | the diagnostic's kind |
| `span` | where it is reported |
| `message` | what it says |
| **returns** | the phase failure the walk returns |

### refuse_nesting

```mach
pub fun refuse_nesting(lc: *LowerContext, span: mach.base.source.Span) mach.base.fail.Fail
```

the lowering recurses as deep as the input nests, on frames far larger than the checks
before it, so each walk asks for stack before it descends and refuses the node it stands on

### refuse_type_nesting

```mach
pub fun refuse_type_nesting(lc: *LowerContext) mach.base.fail.Fail
```

the same refusal of a walk over a type, which has no place of its own, at the node the
lowering stands on

### refuse

```mach
pub fun refuse(lc: *LowerContext, f: mach.base.fail.Fail) mach.base.fail.Fail
```

a failure a walk with no place of its own returned: a refusal of the input is reported under
its own kind at the node the lowering stands on; any other failure is the lowering's own

### refuse_at

```mach
pub fun refuse_at(lc: *LowerContext, span: mach.base.source.Span, f: mach.base.fail.Fail) mach.base.fail.Fail
```

the same at `span`, the declaration whose walk returned it

### record_eval_result

```mach
pub fun record_eval_result(lc: *LowerContext, r: res[mach.front.comptime.value.CTValue, mach.front.comptime.failure.EvalFail])
```

fail the lowering internally when a comptime evaluation failed internally, nothing otherwise

| Parameter | |
| :--- | :--- |
| `lc` | the lowering context |
| `r` | the evaluation's result |

### type_node

```mach
pub fun type_node(lc: *LowerContext, tid: mach.front.ast.id.TypeId) res[mach.front.ast.type.Type, mach.base.fail.Fail]
```

the type node `tid` as name resolution read it, a literal head it split naming the type by its
path's prefix and the case by the last segment

### expression

```mach
pub fun expression(lc: *LowerContext, eid: mach.front.ast.id.ExprId) res[mach.front.ast.expr.Expr, mach.base.fail.Fail]
```

the expression node `eid` of the module in scope as name resolution read it

| Parameter | |
| :--- | :--- |
| `lc` | the lowering context |
| `eid` | the expression |
| **returns** | the node, or the failure, reported internal, of one that cannot be read or was rejected |

### resolve_module_member_const

```mach
pub fun resolve_module_member_const(lc: *LowerContext, eid: mach.front.ast.id.ExprId) res[opt[mach.front.comptime.value.CTValue], mach.front.comptime.failure.EvalFail]
```

a lowering capability; a nil context, which the capability check refuses before any call,
answers as an evaluation with no lowering would

### comptime_ident_kind

```mach
pub fun comptime_ident_kind(lc: *LowerContext, eid: mach.front.ast.id.ExprId) mach.front.comptime.capability.NameKind
```

a lowering capability; a nil context, which the capability check refuses before any call,
answers as an evaluation with no lowering would

### resolve_type_comparison_operand

```mach
pub fun resolve_type_comparison_operand(lc: *LowerContext, eid: mach.front.ast.id.ExprId) res[opt[u32], mach.front.comptime.failure.EvalFail]
```

a lowering capability; a nil context, which the capability check refuses before any call,
answers as an evaluation with no lowering would

### resolve_type_query

```mach
pub fun resolve_type_query(lc: *LowerContext, tid: u32, which: u8) res[opt[mach.front.comptime.value.CTValue], mach.front.comptime.failure.EvalFail]
```

a lowering capability; a nil context, which the capability check refuses before any call,
answers as an evaluation with no lowering would

### laid_type

```mach
pub fun laid_type(lc: *LowerContext, tid: mach.type.kind.TypeId) res[Type, mach.base.fail.Fail]
```

the type the layout intrinsics read for source type `tid`: a handle sizes and aligns as the
target's `ptr`, as a name for a resource the target mints or an inert one it does not, and any
other type as it lowers

### resolve_layout_intrinsic

```mach
pub fun resolve_layout_intrinsic(lc: *LowerContext, eid: u32) res[opt[mach.front.comptime.value.CTValue], mach.front.comptime.failure.EvalFail]
```

the value of the `$size_of`, `$align_of`, `$length_of`, `$offset_of` or `$type_id` call at
`eid`, none for any other expression
a lowering capability. a nil context, which the capability check refuses before any call,
answers as an evaluation with no lowering would

### member_offset

```mach
pub fun member_offset(lc: *LowerContext, tid: mach.type.kind.TypeId, ir: Type, index: u32) res[u64, mach.base.fail.Fail]
```

the byte offset of field or case `index` of the source type `tid`, lowered as `ir`: a
record's field, a union's member at zero, or a tag case's payload, which a case with none
does not have

### field_offset

```mach
pub fun field_offset(lc: *LowerContext, owner: u32, index: u32) res[opt[mach.front.comptime.value.CTValue], mach.front.comptime.failure.EvalFail]
```

a field descriptor's offset, from the layout lowering gives the owning type

### type_store

```mach
pub fun type_store(lc: *LowerContext) *mach.type.TypeInterner
```

the type store a comptime evaluation during lowering reads

### begin_function

```mach
pub fun begin_function(lc: *LowerContext, fn_index: u32, ret_type: Type)
```

reset the per-function state to lower function `fn_index`, whose mirl result type is `ret_type`:
no locals, loops or `fin` blocks, and no armed destination, result slot, exit or pack

### bind_local

```mach
pub fun bind_local(lc: *LowerContext, sym: mach.front.resolve.SymbolId, v: mach.lower.value.Value) err[mach.base.fail.Fail]
```

bind a local to `v`, the stack slot it lives in, in place of any direct binding

| Parameter | |
| :--- | :--- |
| `lc` | the lowering context |
| `sym` | the local's symbol |
| `v` | its slot |
| **returns** | nothing, or the refusal when the map cannot grow |

### binds_direct

```mach
pub fun binds_direct(lc: *LowerContext, sym: mach.front.resolve.SymbolId, tid: mach.type.kind.TypeId, ir: Type) bool
```

whether a local of source type `tid`, held in `ir`, is bound to its value and has no slot:
a scalar that holds no secret, or an aggregate held whole, which nothing stores to, takes the
address of or names in an asm block, and, for an aggregate, whose parts nothing writes or
takes the address of. a `val` or a parameter is then the value it began as wherever it is read

### holds_whole

```mach
pub fun holds_whole(lc: *LowerContext, tid: mach.type.kind.TypeId, ir: Type) bool
```

whether a value of source type `tid`, held in the aggregate `ir`, is held whole, a mirl value
with no storage: a record, tag or union of at most two pointer widths that holds no secret,
is not volatile and has no array or vector in it, so each of its members is reached by a
constant path, on a target whose memory a value's bytes can be reinterpreted in

### returns_whole

```mach
pub fun returns_whole(lc: *LowerContext) bool
```

whether the function being lowered returns an aggregate held whole, which its returns pass to
its one exit as the exit's parameter

### bind_direct

```mach
pub fun bind_direct(lc: *LowerContext, sym: mach.front.resolve.SymbolId, v: mach.lower.value.Value) err[mach.base.fail.Fail]
```

bind a local to the value it is. a bool is read as its byte, since the test it came from
may stand in another block

### lookup_direct

```mach
pub fun lookup_direct(lc: *LowerContext, sym: mach.front.resolve.SymbolId) opt[mach.lower.value.Value]
```

the value a local bound by bind_direct is

| Parameter | |
| :--- | :--- |
| `lc` | the lowering context |
| `sym` | the local's symbol |
| **returns** | the value, none when it is not bound directly |

### lookup_local

```mach
pub fun lookup_local(lc: *LowerContext, sym: mach.front.resolve.SymbolId) opt[mach.lower.value.Value]
```

the stack slot a local bound by bind_local lives in

| Parameter | |
| :--- | :--- |
| `lc` | the lowering context |
| `sym` | the local's symbol |
| **returns** | the slot, none when it is not bound to one |

### push_loop

```mach
pub fun push_loop(lc: *LowerContext, header: mirl.ir.id.Block, exit: mirl.ir.id.Block) err[mach.base.fail.Fail]
```

enter a loop whose `cnt` branches to `header` and `brk` to `exit`

| Parameter | |
| :--- | :--- |
| `lc` | the lowering context |
| `header` | the loop's header block |
| `exit` | the block after it |
| **returns** | nothing, or the refusal when the stack cannot grow |

### pop_loop

```mach
pub fun pop_loop(lc: *LowerContext)
```

leave the innermost loop, nothing when none is open

| Parameter | |
| :--- | :--- |
| `lc` | the lowering context |

### current_loop

```mach
pub fun current_loop(lc: *LowerContext) *LoopFrame
```

the innermost loop

| Parameter | |
| :--- | :--- |
| `lc` | the lowering context |
| **returns** | its frame, borrowed until the next push, or nil outside a loop |

### push_fin

```mach
pub fun push_fin(lc: *LowerContext, body: mach.front.ast.id.StmtId) err[mach.base.fail.Fail]
```

open a `fin` block, run when the walk leaves its enclosing block

| Parameter | |
| :--- | :--- |
| `lc` | the lowering context |
| `body` | the block's statement |
| **returns** | nothing, or the refusal when the stack cannot grow |

### fin_count

```mach
pub fun fin_count(lc: *LowerContext) u32
```

how many `fin` blocks are open

| Parameter | |
| :--- | :--- |
| `lc` | the lowering context |
| **returns** | the count, a mark pop_fins_to takes |

### fin_at

```mach
pub fun fin_at(lc: *LowerContext, i: u32) mach.front.ast.id.StmtId
```

the `fin` block opened `i`th, counting from the outermost

| Parameter | |
| :--- | :--- |
| `lc` | the lowering context |
| `i` | its position |
| **returns** | its statement, or STMT_NIL past the open ones |

### pop_fins_to

```mach
pub fun pop_fins_to(lc: *LowerContext, mark: u32)
```

close every `fin` block opened after `mark`

| Parameter | |
| :--- | :--- |
| `lc` | the lowering context |
| `mark` | a count fin_count gave |

### local_for

```mach
pub fun local_for(lc: *LowerContext, tid: mach.type.kind.TypeId, ir: Type) res[mach.lower.value.Value, mach.base.fail.Fail]
```

a slot for a local whose storage begins here and ends where control leaves its block (`end_scopes_from`)

| Parameter | |
| :--- | :--- |
| `lc` | the lowering context |
| `tid` | the local's source type |
| `ir` | the ir type the slot holds |
| **returns** | the slot's address, or the failure of making it or of stating where its storage begins |

### scope_count

```mach
pub fun scope_count(lc: *LowerContext) u32
```

how many locals' storage has begun and not ended

| Parameter | |
| :--- | :--- |
| `lc` | the lowering context |
| **returns** | the count, a mark `end_scopes_from` and `pop_scopes_to` take |

### end_scopes_from

```mach
pub fun end_scopes_from(lc: *LowerContext, mark: u32) err[mach.base.fail.Fail]
```

the storage of each local begun since `mark` ended, the latest first, where control still reaches

| Parameter | |
| :--- | :--- |
| `lc` | the lowering context |
| `mark` | a count scope_count gave |
| **returns** | nothing, or the failure mirl's builder kept |

### pop_scopes_to

```mach
pub fun pop_scopes_to(lc: *LowerContext, mark: u32)
```

forget every local begun after `mark`, whose storage its block's exits have ended

| Parameter | |
| :--- | :--- |
| `lc` | the lowering context |
| `mark` | a count scope_count gave |

### ptr_or_none

```mach
pub fun ptr_or_none(lc: *LowerContext) Type
```

the pointer type, which every address is held in

### builder_state

```mach
pub fun builder_state(lc: *LowerContext) err[mach.base.fail.Fail]
```

the first refusal the builder met, which every builder call after it skips

### int_type

```mach
pub fun int_type(lc: *LowerContext, bits: u32) res[Type, mach.base.fail.Fail]
```

the mirl integer type of `bits`

| Parameter | |
| :--- | :--- |
| `lc` | the lowering context |
| `bits` | width in bits |
| **returns** | the type, or the refusal of mirl's type table |

### ptr_type

```mach
pub fun ptr_type(lc: *LowerContext) res[Type, mach.base.fail.Fail]
```

the pointer type every address is held in, which never fails

| Parameter | |
| :--- | :--- |
| `lc` | the lowering context |
| **returns** | the type |

### lower_type

```mach
pub fun lower_type(lc: *LowerContext, tid: mach.type.kind.TypeId) res[Type, mach.base.fail.Fail]
```

the mirl type a value of the source type `tid` is held in

### texel_number

```mach
pub fun texel_number(lc: *LowerContext, ty: Type) opt[mirl.target.capability.Number]
```

the number the texels of the handle type `ty` are, as the target's catalog relates them to a word
operand of its kind, none for a handle whose kind relates none, one whose operand names any, or
another type

### tag_case_has_payload

```mach
pub fun tag_case_has_payload(lc: *LowerContext, tid: mach.type.kind.TypeId, i: u32) bool
```

whether case `i` of the tag `tid` holds a payload

### tag_carried

```mach
pub fun tag_carried(lc: *LowerContext, tid: mach.type.kind.TypeId, i: u32) res[mach.type.shape.Carried, mach.base.fail.Fail]
```

where the tag `tid` holds the payload of case `i`, which has one: the area of the tag's
structure and the member of that area's union, as its placement gives it

### storage_align

```mach
pub fun storage_align(lc: *LowerContext, ir: Type) u64
```

the alignment a value held in `ir` is stored at: the target's
layout of `ir`, which states a decorated record's alignment and packing

### leave

```mach
pub fun leave(lc: *LowerContext, v: mach.lower.value.Value) err[mach.base.fail.Fail]
```

return `v` from the function, none for no value. every return branches to one exit block,
which holds the function's only `ret`, so the frame is undone in one place however many
returns the source has. a scalar result travels as the exit's parameter

### emit_exit

```mach
pub fun emit_exit(lc: *LowerContext) err[mach.base.fail.Fail]
```

write the one exit the function's returns branch to, where it has one

### known_case

```mach
pub fun known_case(lc: *LowerContext, eid: mach.front.ast.id.ExprId) opt[*Known]
```

the case known of the local named by `eid` where lowering stands, none where none is

### tag_payload

```mach
pub fun tag_payload(lc: *LowerContext, tid: mach.type.kind.TypeId, i: u32) res[opt[mach.type.shape.Carried], mach.base.fail.Fail]
```

the area and member that hold case `i` of the tag `tid`, none for a case that holds nothing

### result_slot

```mach
pub fun result_slot(lc: *LowerContext) res[mach.lower.value.Value, mach.base.fail.Fail]
```

the slot an aggregate result of the function being lowered is built in and returned from:
one for every return, so the result is built where the caller receives it, made on first ask

| Parameter | |
| :--- | :--- |
| `lc` | the lowering context |
| **returns** | the slot, or the refusal to make it |

### take_into

```mach
pub fun take_into(lc: *LowerContext, ir: Type) opt[mach.lower.value.Value]
```

the armed destination, taken once, when the expression being lowered is the armed one and
builds an aggregate of `ir`, which the destination holds; none for any other

### slot_for

```mach
pub fun slot_for(lc: *LowerContext, tid: mach.type.kind.TypeId, ir: Type) res[mach.lower.value.Value, mach.base.fail.Fail]
```

a stack slot for a value of the source type `tid`, held in `ir`, with the bytes `tid` lays out as public stated

### state_slot

```mach
pub fun state_slot(lc: *LowerContext, slot: mach.lower.value.Value, tid: mach.type.kind.TypeId, ir: Type) err[mach.base.fail.Fail]
```

state on the alloca of `slot`, which holds a value of source type `tid` held in `ir`, the bytes `tid` lays out as
public (`public_ranges`), nothing for a type that is no type

### public_ranges

```mach
pub fun public_ranges(lc: *LowerContext, tid: mach.type.kind.TypeId, ir: Type, out: *std.collections.vector.Vector[mirl.ir.global.Range]) err[mach.base.fail.Fail]
```

the public byte ranges of a value of source type `tid`, held in `ir`, into `out`, ascending and none touching
another: every byte its secret ranges leave, a tag's discriminant and padding included. padding of a type that holds
a secret is written zero wherever a value of it is made or stored whole, and nothing types a part of it, so it holds
no secret

### debug_type_of

```mach
pub fun debug_type_of(lc: *LowerContext, tid: mach.type.kind.TypeId) res[opt[mirl.ir.debug.record.Type], mach.base.fail.Fail]
```

the debug type a debug record names for source type `tid`; none when this lowering keeps
no debug information

### debug_own_types

```mach
pub fun debug_own_types(lc: *LowerContext) err[mach.base.fail.Fail]
```

the types this object describes for every other: each record, union and tag the module
declares that the object emits the declaration of, the test object a `#[testing]` one
and the normal object any other. a generic is described by its instances, in every
module that names one

### debug_types_finish

```mach
pub fun debug_types_finish(lc: *LowerContext) err[mach.base.fail.Fail]
```

describe the types this object owns, then every pointee left waiting, then link the
table. a pointee holding by value an instance of an expansive generic the module
describes no value of is not described past its pointer, since such a generic has
unboundedly many instances behind its pointers: a record, union, tag or instance is
named by its key alone, as another object describes it, and any other is opaque

### block_terminated

```mach
pub fun block_terminated(lc: *LowerContext) bool
```

whether the block being built already ends

### is_void_ir

```mach
pub fun is_void_ir(lc: *LowerContext, tid: Type) bool
```

whether the mirl type `tid` is no type, as a function with no result returns

| Parameter | |
| :--- | :--- |
| `lc` | the lowering context |
| `tid` | the mirl type |
| **returns** | true for irt.NONE |

### expr_type_of

```mach
pub fun expr_type_of(lc: *LowerContext, eid: mach.front.ast.id.ExprId) mach.type.kind.TypeId
```

an expression's type inside an instance's scope is the instance's, never the template's:
sema records the template's and every type decision here is made against this instance
an expression's type as the frame the code is read through decided it, else the template's
under the instance's substitution

### expr_float_width

```mach
pub fun expr_float_width(lc: *LowerContext, eid: mach.front.ast.id.ExprId) mach.base.float.FloatWidth
```

the float width of expression `eid`'s type, a secret one's base

| Parameter | |
| :--- | :--- |
| `lc` | the lowering context |
| `eid` | the expression |
| **returns** | the width, FLOAT_W_NONE for a type that is no float |

### expr_is_secret

```mach
pub fun expr_is_secret(lc: *LowerContext, eid: mach.front.ast.id.ExprId) bool
```

whether expression `eid`'s type is outermost secret

| Parameter | |
| :--- | :--- |
| `lc` | the lowering context |
| `eid` | the expression |
| **returns** | true for a `^` type |

### type_is_secret

```mach
pub fun type_is_secret(lc: *LowerContext, tid: mach.type.kind.TypeId) bool
```

whether the source type `tid` is outermost secret

| Parameter | |
| :--- | :--- |
| `lc` | the lowering context |
| `tid` | the type, nil reads as public |
| **returns** | true for a `^` type |

### type_contains_secret

```mach
pub fun type_contains_secret(lc: *LowerContext, tid: mach.type.kind.TypeId) bool
```

whether a value of the source type `tid` holds a secret in any of its own bytes

| Parameter | |
| :--- | :--- |
| `lc` | the lowering context |
| `tid` | the type, nil or error reads as public |
| **returns** | true when it does, or when the walk failed the lowering |

### secret_ranges

```mach
pub fun secret_ranges(lc: *LowerContext, tid: mach.type.kind.TypeId, ir: Type, base: u64, out: *std.collections.vector.Vector[mirl.ir.global.Range]) err[mach.base.fail.Fail]
```

the secret byte ranges of a value of source type `tid`, held in `ir`, laid out from
`base`, appended to `out` in no order: a secret's every byte, an aggregate's secret
members and elements, every case of a union or a tag's payload, and nothing of a tag's
discriminant or of padding

### global_secrecy

```mach
pub fun global_secrecy(lc: *LowerContext, tid: mach.type.kind.TypeId, ir: Type, ranges: *std.collections.vector.Vector[mirl.ir.global.Range]) res[mirl.ir.global.Secrecy, mach.base.fail.Fail]
```

how much of a global of source type `tid`, held in `ir`, is secret, its ranges held in
`ranges`, which must outlive the answer until the module copies it

### substitute_type

```mach
pub fun substitute_type(lc: *LowerContext, tid: mach.type.kind.TypeId) mach.type.kind.TypeId
```

a template's type as the instance being lowered holds it, applied once where a type is read
from sema's tables, so every type the lowering passes on is already the instance's

### type_resolved_of

```mach
pub fun type_resolved_of(lc: *LowerContext, tid: mach.front.ast.id.TypeId) mach.type.kind.TypeId
```

the source type the type node `tid` names where the code is read: a frame's own where its
comptime bindings size the node, else the template's under the instance's substitution

| Parameter | |
| :--- | :--- |
| `lc` | the lowering context |
| `tid` | the type node |
| **returns** | the type, TYPE_NIL for a nil or unknown node |

### decl_type_of

```mach
pub fun decl_type_of(lc: *LowerContext, did: mach.front.ast.id.DeclId) mach.type.kind.TypeId
```

the source type of declaration `did`, as the frame the code is read through decided it,
else the template's under the instance's substitution

| Parameter | |
| :--- | :--- |
| `lc` | the lowering context |
| `did` | the declaration |
| **returns** | the type, TYPE_NIL for a nil or unknown declaration |

### instance_id_of_call

```mach
pub fun instance_id_of_call(lc: *LowerContext, eid: mach.front.ast.id.ExprId) res[u32, mach.base.fail.Fail]
```

the index in the set being read of the instance sema decided the call or generic reference
`eid` names, in the frame the code is read through

### instance_link_name

```mach
pub fun instance_link_name(lc: *LowerContext, item: *mach.front.sema.instance.Instance) res[mach.base.intern.StrId, mach.base.fail.Fail]
```

the symbol an instance is emitted and referenced under

### gate_of

```mach
pub fun gate_of(lc: *LowerContext, cond: mach.front.ast.id.ExprId) opt[bool]
```

the verdict sema gave a statement gate in the frame the code is read through

### each_array

```mach
pub fun each_array(lc: *LowerContext, stmt: mach.front.ast.id.StmtId) res[mach.front.comptime.value.CTValue, mach.base.fail.Fail]
```

the published value of the array sema typed the `$each` at `stmt` over, in the frame the code
is read through

### iteration_frame

```mach
pub fun iteration_frame(lc: *LowerContext, stmt: mach.front.ast.id.StmtId, index: u32) res[u32, mach.base.fail.Fail]
```

the frame of iteration `index` of the `$each` at `stmt` under the current frame

### decl_ret_sem

```mach
pub fun decl_ret_sem(lc: *LowerContext, did: mach.front.ast.id.DeclId) mach.type.kind.TypeId
```

the source result type of the function `did` declares

| Parameter | |
| :--- | :--- |
| `lc` | the lowering context |
| `did` | the declaration |
| **returns** | the result type, TYPE_NIL for no result or a declaration that is no function |

### lower_fn_sig

```mach
pub fun lower_fn_sig(lc: *LowerContext, did: mach.front.ast.id.DeclId) res[FnSig, mach.base.fail.Fail]
```

the signature of the function `did` declares

### fn_sig_of_type

```mach
pub fun fn_sig_of_type(lc: *LowerContext, tid: mach.type.kind.TypeId) res[FnSig, mach.base.fail.Fail]
```

the signature a value of the source function type `tid` is called through

### c_variadic_fixed

```mach
pub fun c_variadic_fixed(lc: *LowerContext, tid: mach.type.kind.TypeId) opt[u32]
```

how many parameters the C-variadic function type `tid` names, none for any other type

### variadic_sig_of_type

```mach
pub fun variadic_sig_of_type(lc: *LowerContext, tid: mach.type.kind.TypeId, tail: *mach.type.kind.TypeId, tail_len: u32) res[FnSig, mach.base.fail.Fail]
```

the signature one call of the C-variadic function type `tid` is made through: its
named parameters and then the `tail_len` types at `tail` of the arguments it passes
unnamed, as mirl states a variadic call

### ext_of

```mach
pub fun ext_of(lc: *LowerContext, sem: mach.type.kind.TypeId) mirl.ir.type.table.Extension
```

the extension a convention gives an integer narrower than a register: its declared
signedness. a bool is a byte read unsigned

### declared_secrets

```mach
pub fun declared_secrets(lc: *LowerContext, sig: FnSig, out: *std.collections.vector.Vector[mirl.ir.global.Secrecy], result: *mirl.ir.global.Secrecy, ranges: *std.collections.vector.Vector[mirl.ir.global.Range], pointers: *std.collections.vector.Vector[u32]) err[mach.base.fail.Fail]
```

the secret parameters a function of signature `sig` declares, into `out`, its secret result, into `result`, and the
positions of its parameters that point to storage holding no secret, into `pointers`: each as secret as its source
type holds a secret in its own bytes, the bytes a pointer reaches being its pointee's and never the pointer's. `out`
holds none where no parameter is secret, and their ranges are `ranges`, which must outlive them until the module
copies them. this is the one place a declaration's secrecy is made, of a definition and of a declaration alike, so a
caller and the definition it calls state one contract

### pack_instance_sig

```mach
pub fun pack_instance_sig(lc: *LowerContext, fn_ty: mach.type.kind.TypeId, elems: *mach.type.kind.TypeId, elem_len: u32) res[FnSig, mach.base.fail.Fail]
```

the signature of a pack instance of the function typed `fn_ty`: its fixed parameters, then
one per element of the pack

### instance_ref_sig

```mach
pub fun instance_ref_sig(lc: *LowerContext, item: *mach.front.sema.instance.Instance) res[FnSig, mach.base.fail.Fail]
```

the signature an instance is referenced under: the one sema settled when it asked for it

### expr_symbol_of

```mach
pub fun expr_symbol_of(lc: *LowerContext, eid: mach.front.ast.id.ExprId) mach.front.resolve.SymbolId
```

the symbol name resolution bound expression `eid` to, rejecting the phase for a rejected one

| Parameter | |
| :--- | :--- |
| `lc` | the lowering context |
| `eid` | the expression |
| **returns** | the symbol, or SYMBOL_NIL for none or a rejected one |

### module_source

```mach
pub fun module_source(lc: *LowerContext) str
```

the source of the module in scope, read when the scope was made

### emit_dbg_birth

```mach
pub fun emit_dbg_birth(lc: *LowerContext, name_span: mach.base.source.Span, ty_sem: mach.type.kind.TypeId, param: opt[u32], val_v: mach.lower.value.Value,
    slot: opt[mach.lower.value.Value]) err[mach.base.fail.Fail]
```

a source variable named at `name_span` begins: one held in a stack slot is bound to the
slot, which describes it wherever it lives, and any other, a parameter, to its value
from here on

### describe_global

```mach
pub fun describe_global(lc: *LowerContext, g: u32, name_span: mach.base.source.Span, ty_sem: mach.type.kind.TypeId) err[mach.base.fail.Fail]
```

the source variable the module's global `g` is, named at `name_span` with source type
`ty_sem`, which a debugger reads its type and declaration line from; nothing when this
lowering keeps no debug information

### symbol_of

```mach
pub fun symbol_of(lc: *LowerContext, sid: mach.front.resolve.SymbolId) opt[*mach.front.resolve.Symbol]
```

the symbol `sid` of the module in scope

| Parameter | |
| :--- | :--- |
| `lc` | the lowering context |
| `sid` | the symbol id |
| **returns** | the symbol, borrowed from name resolution, none for nil or out of range |

### symbol_name

```mach
pub fun symbol_name(lc: *LowerContext, name: mach.base.intern.StrId) Id[Name]
```

the handle of the symbol name `name` in the module's names, interned once, by which its
function or global is declared. a refusal faults the builder, which the declaration reports

### image_linkage

```mach
pub fun image_linkage(lc: *LowerContext) Linkage
```

the linkage of a definition every module of the image may reach and no other image sees,
`pub` or not, unless `#[export]` says so, and of a reference to a private declaration, which an
instance another module lowers reaches

### internal_visibility

```mach
pub fun internal_visibility(lc: *LowerContext) *mink.model.symbol.Visibility
```

the visibility that keeps a definition inside its image: hidden, or the default where the
format exports only what its export directives name and states no visibility, as PE does

### add_function

```mach
pub fun add_function(lc: *LowerContext, data: FunctionData, info: FnInfo) res[u32, mach.base.fail.Fail]
```

add a function to the module with what mach declares of it

### function_named

```mach
pub fun function_named(lc: *LowerContext, name: mach.base.intern.StrId) opt[u32]
```

the function of the module named `name`, none when no function is

### global_named

```mach
pub fun global_named(lc: *LowerContext, name: mach.base.intern.StrId) opt[u32]
```

the global of the module named `name`, none when no global is

### define_function

```mach
pub fun define_function(lc: *LowerContext, data: FunctionData, info: FnInfo, span: mach.base.source.Span, what: Defines) res[u32, mach.base.fail.Fail]
```

the module's definition of a function: a reference earlier in the module declared it,
and the definition takes the declaration's place

### hold_signature

```mach
pub fun hold_signature(lc: *LowerContext, name: mach.base.intern.StrId, did: mach.front.ast.id.DeclId, d: *mach.front.ast.decl.Decl) err[mach.base.fail.Fail]
```

the unit holds one function for a symbol, so the `ext fun`s and the definition its modules state
the symbol `name` with, as the function `d` declared by `did` does, state one signature. sema holds
each module to this alone, and this holds the modules of a unit that takes several

### hold_definition

```mach
pub fun hold_definition(lc: *LowerContext, name: mach.base.intern.StrId, span: mach.base.source.Span, what: Defines) err[mach.base.fail.Fail]
```

the object defines the symbol `name` as `what`, from the declaration whose name the module or
template in scope writes at `span`. one symbol means one thing in a module, so a reference the
object made under it to another module's declaration is refused. a second definition is
define_function's and define_global's to refuse

### hold_reference

```mach
pub fun hold_reference(lc: *LowerContext, name: mach.base.intern.StrId, origin: mach.session.module.ModuleId, decl: mach.front.ast.id.DeclId) err[mach.base.fail.Fail]
```

the object refers, at the node the lowering stands on, to the declaration `decl` of the module
`origin` by the symbol `name`. a declaration of a module the object compiles is its own, and
the symbol of any other means what the link binds it to, which the object's own definition of
the symbol would stand in for, so that pair is refused whichever of the two comes first. a
`#[weak]` default of another module is replaced at link by the definition that meets it, so a
reference to one takes no part

### bytes_name

```mach
pub fun bytes_name(lc: *LowerContext, name: mach.base.intern.StrId) res[mach.base.intern.StrId, mach.base.fail.Fail]
```

the symbol of the bytes the string `val` linked as `name` points at, which the val's module defines beside it

### define_global

```mach
pub fun define_global(lc: *LowerContext, name: mach.base.intern.StrId, g: GlobalData, span: mach.base.source.Span) res[u32, mach.base.fail.Fail]
```

the module's definition of the global `name`, in place of a declaration a reference made

### ensure_extern_function

```mach
pub fun ensure_extern_function(lc: *LowerContext, name: mach.base.intern.StrId, sig: FnSig, linkage: Linkage, timed: ConstantTime) res[u32, mach.base.fail.Fail]
```

the module's function `name`, declared here with `linkage` when another object defines it, with the mode `timed`
its declaration states and the secret parameters and result its signature does, which are what a caller may trust of it

### stated_callee

```mach
pub fun stated_callee(lc: *LowerContext, tid: mach.type.kind.TypeId) res[opt[mirl.ir.id.Stated], mach.base.fail.Fail]
```

the declaration a call through a value of the function type `tid` states its callee keeps, none for a type marked
with no constant-time mode: the type's mode, and the secret parameters, secret result and public pointers a function
of that signature declares, so the call is trusted as one naming such a function is. a value of the type holds only
a function declared so, which sema decides, and the declaration names no symbol

### mode_of

```mach
pub fun mode_of(lc: *LowerContext, origin: mach.session.module.ModuleId, decl: mach.front.ast.id.DeclId) res[ConstantTime, mach.base.fail.Fail]
```

the mode the function `decl` of module `origin` is declared with, as its module's interface publishes it

### class_copy

```mach
pub fun class_copy(lc: *LowerContext, origin: mach.session.module.ModuleId, decl: mach.front.ast.id.DeclId, base: mach.base.intern.StrId, sig: Type, instance: u32) res[u32, mach.base.fail.Fail]
```

the copy of the function `decl` of `origin`, linked as `base`, whose pointer parameters
take the classes `sig` states, declared here and lowered once the module's own
declarations are, of the instance `instance` of the set being read where it is one. one
name is one copy, so a copy asked for twice is asked for once

### building_constant_time

```mach
pub fun building_constant_time(lc: *LowerContext) bool
```

the function being built requires constant time

### reach_instance

```mach
pub fun reach_instance(lc: *LowerContext, id: u32, name: mach.base.intern.StrId, local: bool) err[mach.base.fail.Fail]
```

the instance `id` of the set being read, named `name`, is reached from the code being lowered: it
is lowered into this object once the object's own code is, and when `local`, constant-time code
calls it, its copy is local, since the walk that verifies the caller reads only the code its
object holds. reached twice, it is lowered once, local when either reach is

### localize

```mach
pub fun localize(lc: *LowerContext, name: mach.base.intern.StrId)
```

the instance this object defines under `name`, reached after it was lowered from constant-time
code, bound local to the object

### ensure_extern_global

```mach
pub fun ensure_extern_global(lc: *LowerContext, name: mach.base.intern.StrId, ty: Type, tls: opt[mirl.ir.global.Tls], sem: mach.type.kind.TypeId, linkage: Linkage, constant: bool) res[u32, mach.base.fail.Fail]
```

the module's global `name` of `ty`, of source type `sem` (TYPE_NIL for bytes no source
type describes), declared here with `linkage` when another object defines it, which the
program never writes where `constant` holds

### thread_model

```mach
pub fun thread_model(lc: *LowerContext, foreign_image: bool) opt[mirl.ir.global.Tls]
```

how a thread-local of this lowering's image is reached: an executable's own block sits
at an offset the link fixes, and a shared library's at one its loader fixes at start,
so a library that loads after start has no static block to hold it (no general-dynamic
access is lowered). a thread-local another image may define is reached as a library's

### declared_thread

```mach
pub fun declared_thread(lc: *LowerContext, d: Declared) opt[mirl.ir.global.Tls]
```

the thread model a reference to the module binding `d` declares takes, none for one
that is not thread-local

### add_global

```mach
pub fun add_global(lc: *LowerContext, g: GlobalData) res[u32, mach.base.fail.Fail]
```

add a global to the module

| Parameter | |
| :--- | :--- |
| `lc` | the lowering context |
| `g` | what it is |
| **returns** | its index, or the builder's first failure, lowering's own before mirl's |

### constant_global

```mach
pub fun constant_global(lc: *LowerContext, v: mach.lower.value.Value) res[mach.lower.value.Value, mach.base.fail.Fail]
```

a read-only global of the module's own holding the constant `v`, whose address is the value

## Declared

```mach
pub rec Declared {
    iface:       *mach.front.sema.interface.Interface;
    declaration: *mach.front.sema.interface.Declaration;
}
```

a module-level declaration as the interface of its module describes it

## Known

```mach
pub rec Known {
    sym:     mach.front.resolve.SymbolId;
    tag_ty:  mach.type.kind.TypeId;
    case_ix: u32;
    prev:    *Known;
}
```

a tag local whose case is known in the arm being lowered, one frame on the lowering's own
stack per arm it opens on

| Field | Type | |
| :--- | :--- | :--- |
| `sym` | `mach.front.resolve.SymbolId` | the local or parameter |
| `tag_ty` | `mach.type.kind.TypeId` | its tag type |
| `case_ix` | `u32` | the case it holds there |
| `prev` | `*Known` | the frame of the arm around this one, nil at the outermost |

## FnSig

```mach
pub rec FnSig {
    sig:       Type;
    sem:       mach.type.kind.TypeId;
    fixed:     u32;
    extra:     *mach.type.kind.TypeId;
    extra_len: u32;
    ret:       mach.type.kind.TypeId;
}
```

a function's signature: its parameters and result, each narrow integer stating the
extension its source type's signedness gives it
a function's ir signature and the source types it was lowered from

| Field | Type | |
| :--- | :--- | :--- |
| `sig` | `Type` | its ir signature |
| `sem` | `mach.type.kind.TypeId` | the source function type, type_kind.TYPE_NIL for a signature no source type states, as a class copy's |
| `fixed` | `u32` | how many of its parameters are the source type's own |
| `extra` | `*mach.type.kind.TypeId` | the source type of each parameter past those, a pack instance's element types |
| `extra_len` | `u32` | how many there are |
| `ret` | `mach.type.kind.TypeId` | the source type of its result, type_kind.TYPE_NIL where no source type states it |

### bare_sig

```mach
pub fun bare_sig(sig: Type) FnSig
```

the ir signature `sig` with no source types

## Functions

### external_linkage

```mach
pub fun external_linkage() Linkage
```

the linkage of a symbol another module defines, or a definition every module sees

### declaration_mode

```mach
pub fun declaration_mode(d: *mach.front.sema.interface.Declaration) ConstantTime
```

the mode the declaration `d` states its function holds constant time in: none for one a definition elsewhere may
replace, as a `#[weak]` default, since what runs need not be the body that was checked

