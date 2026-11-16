[mach](../../README.md) / [front](../README.md) / [sema](README.md) / **context**

# mach.front.sema.context

the state of one sema walk: the module typing it shares, the module it reads, the
instance it types and its own walk state, with the reads and reports every check makes

<sub>source: [`src/front/sema/context.mach`](../../../../src/front/sema/context.mach) · imported by 22 modules</sub>

<sub>`sema_instance` is [`mach.front.sema.instance`](instance.md) · `A` is `std.allocator` · `map` is `std.collections.map` · `fail` is [`mach.base.fail`](../../base/fail.md) · `type_kind` is [`mach.type.kind`](../../type/kind.md) · `type_scan` is [`mach.type.scan`](../../type/scan.md) · `ast_id` is [`mach.front.ast.id`](../ast/id.md) · `intern` is [`mach.base.intern`](../../base/intern.md) · `base_source` is [`mach.base.source`](../../base/source.md) · `sema_product` is [`mach.front.sema.product`](product.md) · `comptime_capability` is [`mach.front.comptime.capability`](../comptime/capability.md) · `sema_growth` is [`mach.front.sema.growth`](growth.md) · `session` is [`mach.session`](../../session.md) · `type` is [`mach.type`](../../type.md) · `module` is [`mach.session.module`](../../session/module.md) · `comptime_deep` is [`mach.front.comptime.deep`](../comptime/deep.md) · `diagnostic` is [`mach.base.diagnostic`](../../base/diagnostic.md) · `ast` is [`mach.front.ast`](../ast.md) · `resolve` is [`mach.front.resolve`](../resolve.md) · `comptime_scope` is [`mach.front.comptime.scope`](../comptime/scope.md) · `report_phase` is [`mach.report.phase`](../../report/phase.md) · `diagnostic_kind` is [`mach.base.diagnostic.kind`](../../base/diagnostic/kind.md) · `ast_expr` is [`mach.front.ast.expr`](../ast/expr.md) · `comptime_failure` is [`mach.front.comptime.failure`](../comptime/failure.md) · `ast_type` is [`mach.front.ast.type`](../ast/type.md) · `extent` is [`mach.type.query.extent`](../../type/query/extent.md) · `type_field` is [`mach.type.field`](../../type/field.md) · `comptime_value` is [`mach.front.comptime.value`](../comptime/value.md) · `interface` is [`mach.front.sema.interface`](interface.md) · `type_query` is [`mach.type.query`](../../type/query.md) · `ast_decl` is [`mach.front.ast.decl`](../ast/decl.md)</sub>

| Type | | |
| :--- | :--- | :--- |
| [`InstWorklist`](#instworklist) | `rec` | the instances a typing asks for, in the order they were first asked for: a module's, published with its result, or an instance's, its own set headed by the instance |
| [`UniReports`](#unireports) | `rec` |  |
| [`SecrecyWalk`](#secrecywalk) | `rec` | the walk the union-secrecy checks of every annotation in one scope share, under the one substitution the scope is typed with, so a type several annotations reach is walked once |
| [`Guard`](#guard) | `rec` | one lexical payload guard: the tag place and the case it is known to hold; frames live on the walker's stack and chain through `prev` |
| [`Offer`](#offer) | `rec` | a fix an error offers: `close` at `tail`, and with `open` set, `open` at `head` too, as one fix labelled `label` |
| [`Typing`](#typing) | `rec` | the typing of one module that every walk of it shares: the instances it asks for, its growing cycles, and the reports each instance would otherwise repeat |
| [`ModuleView`](#moduleview) | `rec` | the module a walk reads and the typing tables it writes: the module being typed, or the declaring module of an instance |
| [`Instance`](#instance) | `rec` | the instance a walk types, or none |
| [`Walk`](#walk) | `rec` | the state of one walk over syntax, fresh at each body and scoped to the construct that sets it |
| [`SemaContext`](#semacontext) | `rec` |  |
| [`StmtListVisitor`](#stmtlistvisitor) | `def` |  |

| On `InstWorklist` | |
| :--- | :--- |
| [`inst_worklist_init`](#inst_worklist_init) | |
| [`inst_worklist_take`](#inst_worklist_take) | the set a worklist collected, handed to `into`, and the worklist released |

| On `UniReports` | |
| :--- | :--- |
| [`uni_report_mark`](#uni_report_mark) | whether a report about `key` goes out: the first time it is marked, or always with no record |

| On `Typing` | |
| :--- | :--- |
| [`init`](#init) | the one way a walk is made: over `view`, typing `instance`, sharing `typing`, with a fresh walk |

| On `Instance` | |
| :--- | :--- |
| [`instance_none`](#instance_none) | outside any instance: typing the template, recording into its frame |

| On `Walk` | |
| :--- | :--- |
| [`walk_make`](#walk_make) | |

| On `SemaContext` | |
| :--- | :--- |
| [`foreign`](#foreign) | whether `sym` names a binding a module other than this view's declares |
| [`imported_definition`](#imported_definition) · [`symbol_definition`](#symbol_definition) | |
| [`walk_begin`](#walk_begin) | a construct that sets walk state opens the walk and closes it on the way out, which undoes everything the construct set; the count of deferred names met carries out |
| [`walk_end`](#walk_end) | |
| [`report_deferred_name`](#report_deferred_name) | a name resolve left for the arm sema selects, reported once at the name itself whichever instantiation walks it, and never for an arm sema discards |
| [`report_deferred_type`](#report_deferred_type) · [`resolved_type_of`](#resolved_type_of) · [`expression`](#expression) · [`comptime_expression`](#comptime_expression) | |
| [`type_node`](#type_node) | the type node `tid` as name resolution read it, a literal head it split naming the type by its path's prefix and the case by the last segment; none, with the walk failed, when it cannot be read |
| [`comptime_head`](#comptime_head) | |
| [`is_case_literal`](#is_case_literal) | whether the literal `eid` constructs a tag case, its head read as name resolution read it |
| [`apply_subst`](#apply_subst) | |
| [`awaits_instance`](#awaits_instance) | true while a type still names a type parameter: no operand predicate has an answer for it, and every question about it waits for an instantiation |
| [`record_instance`](#record_instance) | an instance the walk asks for at `eid`: added to the module's set when new, and noted as what `eid` names in the walk's frame |
| [`inst_worklist_dnit`](#inst_worklist_dnit) | |
| [`expr_type_set`](#expr_type_set) | the type the walk gives expression `eid`, recorded in its frame when the frame is not the module's own typing |
| [`decl_type_set`](#decl_type_set) | |
| [`walk_binds_values`](#walk_binds_values) | the walk is typing a value instance, so the comptime parameters of the function it instantiates are bound |
| [`walk_binds_pack`](#walk_binds_pack) | the walk is typing a pack instance, so the pack's element types are bound |
| [`gate_set`](#gate_set) | the verdict the walk gives a comptime gate in its frame |
| [`decl_type_for`](#decl_type_for) | |
| [`expr_in_view`](#expr_in_view) | whether expression `eid` lies in this view's syntax; one past it records an internal failure |
| [`decl_in_view`](#decl_in_view) | whether declaration `did` lies in this view's syntax; one past it records an internal failure |
| [`symbol_for_expr`](#symbol_for_expr) | the symbol an expression names, none when it names none |
| [`comptime_ident_is_runtime`](#comptime_ident_is_runtime) | |
| [`symbol_for_type`](#symbol_for_type) | the symbol a type names, none when it names none |
| [`laying_of`](#laying_of) | what the build lays its types out by, nil where it opened no target |
| [`report`](#report) | a diagnostic of kind `k`; only an error rejects the module, a warning leaves it accepted |
| [`refuse_nesting`](#refuse_nesting) | the walkers recurse as deep as the input nests, on frames larger than the parser's, so each asks for stack before it descends and refuses the node it stands on |
| [`refuse_nesting_in`](#refuse_nesting_in) | the same refusal of a node in another module's syntax, which a walk across modules meets |
| [`refuse`](#refuse) | a failure a walker without a diagnostic store returned, reported against `span`: a refusal of the input under its own kind, any other failure as internal |
| [`check_handle_in_array`](#check_handle_in_array) | |
| [`report_internal`](#report_internal) | fail the phase internally with `f`, a defect met at `span` of the syntax in view; a failure that names a place of its own keeps it |
| [`report_note`](#report_note) | |
| [`text`](#text) | `fmt` formatted with `args`, owned by the caller and released with `text_free`; a refused format fails the phase as an internal failure and gives none |
| [`text_free`](#text_free) | |
| [`reportf`](#reportf) | a diagnostic whose message is `fmt` formatted with `args`; a refused format fails the phase as an internal failure, here and nowhere else |
| [`field_table_stage`](#field_table_stage) · [`field_table_stage_push_entry`](#field_table_stage_push_entry) · [`field_table_publish`](#field_table_publish) · [`field_table_for`](#field_table_for) · [`field_lookup`](#field_lookup) | |
| [`field_index`](#field_index) | the position of field `name` in table `ft`, looked for from position `from` on and then from the first: a literal mostly names its fields in declaration order |
| [`type_store`](#type_store) | the type store a comptime evaluation inside this walk reads |
| [`resolve_type_comparison_operand`](#resolve_type_comparison_operand) | the type of an operand of a type comparison, none when the walk has no typing of it |
| [`resolve_module_member_const`](#resolve_module_member_const) | the value of a module member constant an expression names, none when it names none or the walk has no resolution |
| [`own_nominal_declaration`](#own_nominal_declaration) | |
| [`interface_of`](#interface_of) | the interface module `origin` published, read through the walk's acquisition of it |
| [`each_set`](#each_set) | the constant array the `$each` at `sid` reads in the walk's frame |
| [`definition_ast`](#definition_ast) | the syntax of module `origin`, nil when it cannot be had, the reason recorded in the status |
| [`generic_param_type_for`](#generic_param_type_for) · [`inferred_len_of`](#inferred_len_of) · [`set_inferred_len`](#set_inferred_len) · [`record_embed_path`](#record_embed_path) | |
| [`query_of`](#query_of) | a question to the type store whose internal failures land in this context |
| [`holds_secret`](#holds_secret) | the answer of a query that cannot refuse the input, or `recovery` once it failed, which fails the phase as internal whether a value of type `ty` holds a secret, which gives it no compile-time value |
| [`recover`](#recover) | |
| [`recover_at`](#recover_at) | the answer of a query asked at `span`, or `recovery` once it failed: a refusal of the input is reported there, and any other failure fails the phase as internal |
| [`record_at`](#record_at) | the outcome of a query asked at `span` that answers nothing, reported as recover_at reports it |
| [`query_settle`](#query_settle) | a query built by query_of whose walk refused the input, reported at `span` |
| [`record_eval_result`](#record_eval_result) | |
| [`decorator_string_arg`](#decorator_string_arg) | the string argument `ord` of a decorator evaluates to |
| [`require_constant_arg`](#require_constant_arg) | an integer argument that is not a constant expression is refused as one; a question only a later phase answers, a layout or an instance, is left to it |
| [`decorator_arg_span`](#decorator_arg_span) | |

| Functions | |
| :--- | :--- |
| [`generic_owner_of_decl`](#generic_owner_of_decl) | the identity a declaration's type parameters carry (type.TypeGenericParam owner fields): the one key both interning a parameter and substituting for it use |
| [`source_text_of`](#source_text_of) · [`is_global_binding`](#is_global_binding) · [`is_fun_or_global`](#is_fun_or_global) | |

| Constants | |
| :--- | :--- |
| [`TYPE_MEMO_NONE`](#constants) · [`TYPE_MEMO_RESOLVED`](#constants) · [`TYPE_MEMO_ACTIVE`](#constants) · [`LAYOUT_PENDING`](#constants) · [`LAYOUT_ACTIVE`](#constants) · [`LAYOUT_READY`](#constants) · [`LAYOUT_FAILED`](#constants) | |
| [`TIMED_NONE`](#constants) | the constant-time modes a walked body may hold, as `Walk.timed` |
| [`TIMED_SOFT`](#constants) · [`TIMED_HARD`](#constants) | |
| [`ARM_DECIDED`](#constants) | which walk reports a `$error` a comptime arm reaches, as `Walk.arm`: this one; none, in an arm this walk cannot select; or this value instance, in an arm it selected that its template could not, whether or not the instance is quiet |
| [`ARM_DEFERRED`](#constants) · [`ARM_SELECTED`](#constants) | |

---

## InstWorklist

```mach
pub rec InstWorklist {
    set:   sema_instance.Set;
    alloc: *A.Allocator;
    # how many instances of each growing cycle, keyed by module and cycle, this module expanded
    expansions: map.Map[u64, u32];
}
```

the instances a typing asks for, in the order they were first asked for: a module's, published
with its result, or an instance's, its own set headed by the instance

### inst_worklist_init

```mach
pub fun inst_worklist_init(alloc: *A.Allocator) res[*InstWorklist, fail.Fail]
```

### inst_worklist_take

```mach
pub fun inst_worklist_take(wl: *InstWorklist, into: *sema_instance.Set)
```

the set a worklist collected, handed to `into`, and the worklist released

## UniReports

```mach
pub rec UniReports {
    seen: [UNI_REPORT_CAP]type_kind.TypeId;
    len:  u32;
}
```

### uni_report_mark

```mach
pub fun uni_report_mark(ur: *UniReports, key: type_kind.TypeId) bool
```

whether a report about `key` goes out: the first time it is marked, or always with no record

## SecrecyWalk

```mach
pub rec SecrecyWalk {
    nominals: type_scan.Scan;
    walked:   type_scan.Pair;
    open:     bool;
    visits:   u32;
}
```

the walk the union-secrecy checks of every annotation in one scope share, under the one
substitution the scope is typed with, so a type several annotations reach is walked once

| Field | Type | |
| :--- | :--- | :--- |
| `nominals` | `type_scan.Scan` | the nominals already walked, each by the instantiation that keys it |
| `walked` | `type_scan.Pair` | the types already walked, each paired with the nominal instantiation whose arguments it was walked under, or nil for the scope's own substitution |
| `visits` | `u32` | how many types the walk has visited |

## Guard

```mach
pub rec Guard {
    place:     ast_id.ExprId;
    case_name: intern.StrId;
    prev:      *Guard;
}
```

one lexical payload guard: the tag place and the case it is known to hold; frames live on the
walker's stack and chain through `prev`

## Offer

```mach
pub rec Offer {
    label: str;
    head:  base_source.Span;
    open:  str;
    tail:  base_source.Span;
    close: str;
}
```

a fix an error offers: `close` at `tail`, and with `open` set, `open` at
`head` too, as one fix labelled `label`

## Typing

```mach
pub rec Typing {
    deps:           *sema_product.SemaDeps;
    caps:           comptime_capability.PhaseCapabilities[SemaContext];
    type_caps:      comptime_capability.PhaseCapabilities[SemaContext];
    insts:          *InstWorklist;
    growth:         *sema_growth.Analysis;
    collect_insts:  bool;
    uni_reports:    *UniReports;
    handle_reports: *UniReports;
    # the module's tests typed so far, by name
    tests: *map.Map[u32, u32];
    # the symbols the module's definitions state so far, each by name, to the offset of
    # the declaration stating it
    symbols: *map.Map[u32, u64];
    # the types a logical-addressing target was asked to declare, each judged once
    logical_seen:        *map.Map[type_kind.TypeId, bool];
    has_secret:          bool;
    subst_fn:            fun(*session.Session, type_kind.TypeId, *type.GenericOwner, *type_kind.TypeId, u32) res[type_kind.TypeId, fail.Fail];
    annotation_check_fn: fun(*SemaContext, ast_id.TypeId);
    infer_fn:            fun(*SemaContext, ast_id.ExprId) type_kind.TypeId;
    type_fn:             fun(*SemaContext, ast_id.TypeId) type_kind.TypeId;
    module:              module.StableModuleId;
    values:              *comptime_deep.Store;
}
```

the typing of one module that every walk of it shares: the instances it asks for, its
growing cycles, and the reports each instance would otherwise repeat

| Field | Type | |
| :--- | :--- | :--- |
| `caps` | `comptime_capability.PhaseCapabilities[SemaContext]` | what a comptime evaluation in the walk may ask of sema now |
| `type_caps` | `comptime_capability.PhaseCapabilities[SemaContext]` | the whole semantic set, which a module constant is always evaluated with |
| `collect_insts` | `bool` | whether the walk records the instances it asks for |
| `subst_fn` | `fun(*session.Session, type_kind.TypeId, *type.GenericOwner, *type_kind.TypeId, u32) res[type_kind.TypeId, fail.Fail]` | substitutes an instance's type arguments into a type |
| `annotation_check_fn` | `fun(*SemaContext, ast_id.TypeId)` | the per-instance checks over one type annotation, run as each is resolved |
| `infer_fn` | `fun(*SemaContext, ast_id.ExprId) type_kind.TypeId` | types one expression, as an array length is before it is evaluated |
| `type_fn` | `fun(*SemaContext, ast_id.TypeId) type_kind.TypeId` | resolves one type node, as one the type prepass has yet to reach is where an expression reads it |
| `module` | `module.StableModuleId` | the stable id of the module being typed |
| `values` | `*comptime_deep.Store` | the store the module's constant values are built in, which its interface publishes a walk outside a module's typing, as a check of one construct is, leaves the parts it does without nil, and each reads as absent: with no `deps` no imported surface is acquired, with no `insts` the walk records no instance or frame and types the template, with no `logical_seen` no type is refused for a logical-addressing target, with no `uni_reports` every report goes out, and with no `deps.watch.expanded` nothing hears of a growing cycle |

### init

```mach
pub fun init(s: *session.Session, diags: *diagnostic.DiagnosticStore, typing: Typing, view: ModuleView,
    instance: Instance) SemaContext
```

the one way a walk is made: over `view`, typing `instance`, sharing `typing`, with a fresh walk

## ModuleView

```mach
pub rec ModuleView {
    a:          *ast.Ast;
    rr:         *resolve.ResolveResult;
    ctx:        *comptime_scope.ComptimeCtx;
    own_module: module.ModuleId;
    source:     str;

    expr_type:          *type_kind.TypeId;
    type_resolved_ty:   *type_kind.TypeId;
    decl_type:          *type_kind.TypeId;
    type_resolved_memo: *u8;
    layout_state:       *u8;
    # how far each module `val` is bound, and the node of its value in the module's store once
    # built (see sema.constant); nil in a view of another module
    constant_state: *u8;
    constant_node:  *u32;
    inferred_len:   *u64;
    embed_paths:    *intern.StrId;
    embed_path_len: u32;
    embed_path_cap: u32;
}
```

the module a walk reads and the typing tables it writes: the module being typed, or the
declaring module of an instance. a walk over one construct, as a check is, may leave a table
it does without nil: with no `rr` no name is resolved and no deferred name is reported, and a
typing table nil is not written and a read of it answers as unrecorded, with the error or nil
type, no state or no operand type, except a type read through `resolved_type_of`, which also
records the failure. `inferred_len`, `layout_state`, `type_resolved_memo` and the
constant tables are the module's own and nil in a view of another module

## Instance

```mach
pub rec Instance {
    depth:       u32;
    parent:      u32;
    frame:       u32;
    requester:   u32;
    pack_types:  *type_kind.TypeId;
    pack_len:    u32;
    quiet:       bool;
    site:        base_source.Span;
    site_file:   base_source.FileId;
    site_set:    bool;
    subst:       *type_kind.TypeId;
    subst_len:   u32;
    subst_owner: type.GenericOwner;
    secrecy:     *SecrecyWalk;
    handles:     *type_scan.Scan;
}
```

the instance a walk types, or none

| Field | Type | |
| :--- | :--- | :--- |
| `depth` | `u32` | how deep the chain of instances that asked for this one runs |
| `parent` | `u32` | the instance being typed, INST_NONE outside one |
| `frame` | `u32` | the typing frame the walk records into (see sema_instance.Frame) |
| `requester` | `u32` | who asks for an instance from the frame (sema_instance.FROM_* or an instance id) |
| `pack_types` | `*type_kind.TypeId` | the element types of the pack the instance binds, `pack_len` of them; nil outside a pack instance |
| `quiet` | `bool` | a value or non-generic pack instance retypes what its template already checked, so it reports only what the template could not decide: its comptime arguments, and the bodies of an `$each` over its pack |
| `site` | `base_source.Span` | where the instance was asked for, while `site_set` |
| `subst` | `*type_kind.TypeId` | the type arguments that replace the parameters of `subst_owner`, position for position; a parameter of any other declaration is left alone |
| `secrecy` | `*SecrecyWalk` | the union-secrecy walk the instance's annotations share; nil outside one |
| `handles` | `*type_scan.Scan` | the types the handle checks of the instance's annotations have walked; nil outside one |

### instance_none

```mach
pub fun instance_none() Instance
```

outside any instance: typing the template, recording into its frame

## Walk

```mach
pub rec Walk {
    silent:           bool;
    type_prepass:     bool;
    deferred_hits:    u32;
    offer:            *Offer;
    arm:              u8;
    callee_eid:       ast_id.ExprId;
    write_place:      ast_id.ExprId;
    in_comptime_gate: bool;
    in_fin:           bool;
    fin_loop_depth:   u32;
    in_test:          bool;
    timed:            u8;
    guards:           *Guard;
}
```

the state of one walk over syntax, fresh at each body and scoped to the construct that
sets it

| Field | Type | |
| :--- | :--- | :--- |
| `type_prepass` | `bool` | resolving every type node up front, before any arm is selected |
| `deferred_hits` | `u32` | how many deferred names type resolution has met, so a consumer can tell a type that failed on one from a type that failed on its own |
| `offer` | `*Offer` | a fix the next error raised offers, set by a check that knows one and cleared by it once the check is done; nil offers none |
| `arm` | `u8` | which walk reports a `$error` the comptime arm walked reaches, as `ARM_*` |
| `write_place` | `ast_id.ExprId` | the place under a store or an address-of, whose root is written or addressed rather than read |
| `in_test` | `bool` | inside a `test` body, where a `ret` value is a status in 0..255 |
| `timed` | `u8` | the constant-time mode of the function whose body is walked, `TIMED_NONE` outside one |

### walk_make

```mach
pub fun walk_make() Walk
```

## SemaContext

```mach
pub rec SemaContext {
    s:        *session.Session;
    diags:    *diagnostic.DiagnosticStore;
    status:   report_phase.PhaseStatus;
    typing:   Typing;
    view:     ModuleView;
    instance: Instance;
    walk:     Walk;
}
```

### foreign

```mach
pub fun foreign(sc: *SemaContext, sym: *resolve.Symbol) bool
```

whether `sym` names a binding a module other than this view's declares

### imported_definition

```mach
pub fun imported_definition(sc: *SemaContext, sym: *resolve.Symbol,
    phase: sema_product.DefinitionPhase) res[sema_product.DefinedSymbol, fail.Fail]
```

### symbol_definition

```mach
pub fun symbol_definition(sc: *SemaContext, sym: *resolve.Symbol,
    phase: sema_product.DefinitionPhase) res[sema_product.DefinedSymbol, fail.Fail]
```

### walk_begin

```mach
pub fun walk_begin(sc: *SemaContext) Walk
```

a construct that sets walk state opens the walk and closes it on the way out, which
undoes everything the construct set; the count of deferred names met carries out

### walk_end

```mach
pub fun walk_end(sc: *SemaContext, outer: Walk)
```

### report_deferred_name

```mach
pub fun report_deferred_name(sc: *SemaContext, k: diagnostic_kind.Kind, span: base_source.Span, prefix: str)
```

a name resolve left for the arm sema selects, reported once at the name itself
whichever instantiation walks it, and never for an arm sema discards

### report_deferred_type

```mach
pub fun report_deferred_type(sc: *SemaContext, ast_tid: ast_id.TypeId)
```

### resolved_type_of

```mach
pub fun resolved_type_of(sc: *SemaContext, ast_tid: ast_id.TypeId) type_kind.TypeId
```

### expression

```mach
pub fun expression(sc: *SemaContext, eid: ast_id.ExprId) res[ast_expr.Expr, fail.Fail]
```

### comptime_expression

```mach
pub fun comptime_expression(sc: *SemaContext, a: *ast.Ast, eid: ast_id.ExprId) res[ast_expr.Expr, comptime_failure.EvalFail]
```

### type_node

```mach
pub fun type_node(sc: *SemaContext, tid: ast_id.TypeId) opt[ast_type.Type]
```

the type node `tid` as name resolution read it, a literal head it split naming the type by its
path's prefix and the case by the last segment; none, with the walk failed, when it cannot be read

### comptime_head

```mach
pub fun comptime_head(sc: *SemaContext, a: *ast.Ast, tid: ast_id.TypeId) res[ast_type.Type, comptime_failure.EvalFail]
```

### is_case_literal

```mach
pub fun is_case_literal(sc: *SemaContext, eid: ast_id.ExprId) bool
```

whether the literal `eid` constructs a tag case, its head read as name resolution read it

### apply_subst

```mach
pub fun apply_subst(sc: *SemaContext, tid: type_kind.TypeId) type_kind.TypeId
```

### awaits_instance

```mach
pub fun awaits_instance(sc: *SemaContext, tid: type_kind.TypeId) bool
```

true while a type still names a type parameter: no operand predicate has an answer
for it, and every question about it waits for an instantiation

### record_instance

```mach
pub fun record_instance(sc: *SemaContext, item: sema_instance.Instance, eid: ast_id.ExprId) err[fail.Fail]
```

an instance the walk asks for at `eid`: added to the module's set when new, and noted as
what `eid` names in the walk's frame. an instance whose type arguments or pack still
mention a type parameter is the template under other names: nothing in its body is
settled until the caller is itself instantiated, and that caller's walk asks for the
closed instance. `item` carries the kind, the declaration, its arguments, name,
signature and site; the rest is the set's

### inst_worklist_dnit

```mach
pub fun inst_worklist_dnit(sc: *SemaContext)
```

### expr_type_set

```mach
pub fun expr_type_set(sc: *SemaContext, eid: ast_id.ExprId, ty: type_kind.TypeId)
```

the type the walk gives expression `eid`, recorded in its frame when the frame is not
the module's own typing

### decl_type_set

```mach
pub fun decl_type_set(sc: *SemaContext, did: ast_id.DeclId, ty: type_kind.TypeId)
```

### walk_binds_values

```mach
pub fun walk_binds_values(sc: *SemaContext) bool
```

the walk is typing a value instance, so the comptime parameters of the function it
instantiates are bound

### walk_binds_pack

```mach
pub fun walk_binds_pack(sc: *SemaContext) bool
```

the walk is typing a pack instance, so the pack's element types are bound

### gate_set

```mach
pub fun gate_set(sc: *SemaContext, cond: ast_id.ExprId, active: bool)
```

the verdict the walk gives a comptime gate in its frame

### decl_type_for

```mach
pub fun decl_type_for(sc: *SemaContext, sym: *resolve.Symbol) type_kind.TypeId
```

### expr_in_view

```mach
pub fun expr_in_view(sc: *SemaContext, eid: ast_id.ExprId) bool
```

whether expression `eid` lies in this view's syntax; one past it records an internal failure

### decl_in_view

```mach
pub fun decl_in_view(sc: *SemaContext, did: ast_id.DeclId) bool
```

whether declaration `did` lies in this view's syntax; one past it records an internal failure

### symbol_for_expr

```mach
pub fun symbol_for_expr(sc: *SemaContext, eid: ast_id.ExprId) opt[*resolve.Symbol]
```

the symbol an expression names, none when it names none

### comptime_ident_is_runtime

```mach
pub fun comptime_ident_is_runtime(sc: *SemaContext, eid: ast_id.ExprId) bool
```

### symbol_for_type

```mach
pub fun symbol_for_type(sc: *SemaContext, tid: ast_id.TypeId) opt[*resolve.Symbol]
```

the symbol a type names, none when it names none

### laying_of

```mach
pub fun laying_of(sc: *SemaContext) *extent.Laying
```

what the build lays its types out by, nil where it opened no target

### report

```mach
pub fun report(sc: *SemaContext, k: diagnostic_kind.Kind, span: base_source.Span, message: str)
```

a diagnostic of kind `k`; only an error rejects the module, a warning leaves it accepted

### refuse_nesting

```mach
pub fun refuse_nesting(sc: *SemaContext, span: base_source.Span)
```

the walkers recurse as deep as the input nests, on frames larger than the parser's, so
each asks for stack before it descends and refuses the node it stands on. a silent walk
refuses aloud too: the nesting stops every walk alike, so no probe may read it as an answer

### refuse_nesting_in

```mach
pub fun refuse_nesting_in(sc: *SemaContext, at: base_source.Location)
```

the same refusal of a node in another module's syntax, which a walk across modules meets

### refuse

```mach
pub fun refuse(sc: *SemaContext, span: base_source.Span, f: fail.Fail)
```

a failure a walker without a diagnostic store returned, reported against `span`: a
refusal of the input under its own kind, any other failure as internal

### check_handle_in_array

```mach
pub fun check_handle_in_array(sc: *SemaContext, ty: type_kind.TypeId, span: base_source.Span)
```

### report_internal

```mach
pub fun report_internal(sc: *SemaContext, span: base_source.Span, f: fail.Fail)
```

fail the phase internally with `f`, a defect met at `span` of the syntax in view; a failure
that names a place of its own keeps it

### report_note

```mach
pub fun report_note(sc: *SemaContext, k: diagnostic_kind.Kind, span: base_source.Span, message: str, note: str)
```

### text

```mach
pub fun text(sc: *SemaContext, fmt: str, args: ...) opt[str]
```

`fmt` formatted with `args`, owned by the caller and released with `text_free`; a refused
format fails the phase as an internal failure and gives none

### text_free

```mach
pub fun text_free(sc: *SemaContext, t: str)
```

### reportf

```mach
pub fun reportf(sc: *SemaContext, k: diagnostic_kind.Kind, span: base_source.Span, fmt: str, args: ...)
```

a diagnostic whose message is `fmt` formatted with `args`; a refused format fails the
phase as an internal failure, here and nowhere else

### field_table_stage

```mach
pub fun field_table_stage(sc: *SemaContext, additional: u32) res[u32, fail.Fail]
```

### field_table_stage_push_entry

```mach
pub fun field_table_stage_push_entry(sc: *SemaContext, entry: type_field.Entry) err[fail.Fail]
```

### field_table_publish

```mach
pub fun field_table_publish(sc: *SemaContext, ty: type_kind.TypeId, fields_start: u32, fields_len: u32) err[fail.Fail]
```

### field_table_for

```mach
pub fun field_table_for(sc: *SemaContext, ty: type_kind.TypeId) opt[*FieldTable]
```

### field_lookup

```mach
pub fun field_lookup(sc: *SemaContext, ty: type_kind.TypeId, name: intern.StrId) opt[type_kind.TypeId]
```

### field_index

```mach
pub fun field_index(sc: *SemaContext, ft: *FieldTable, name: intern.StrId, from: u32) opt[u32]
```

the position of field `name` in table `ft`, looked for from position `from` on and then from
the first: a literal mostly names its fields in declaration order

### type_store

```mach
pub fun type_store(sc: *SemaContext) *type.TypeInterner
```

the type store a comptime evaluation inside this walk reads

### resolve_type_comparison_operand

```mach
pub fun resolve_type_comparison_operand(sc: *SemaContext, eid: ast_id.ExprId) res[opt[u32], comptime_failure.EvalFail]
```

the type of an operand of a type comparison, none when the walk has no typing of it

### resolve_module_member_const

```mach
pub fun resolve_module_member_const(sc: *SemaContext, eid: ast_id.ExprId) res[opt[comptime_value.CTValue], comptime_failure.EvalFail]
```

the value of a module member constant an expression names, none when it names none or the walk
has no resolution

### own_nominal_declaration

```mach
pub fun own_nominal_declaration(sc: *SemaContext, nominal: *type.Type) res[ast_id.DeclId, fail.Fail]
```

### interface_of

```mach
pub fun interface_of(sc: *SemaContext, origin: module.ModuleId) res[*interface.Interface, fail.Fail]
```

the interface module `origin` published, read through the walk's acquisition of it

### each_set

```mach
pub fun each_set(sc: *SemaContext, sid: ast_id.StmtId, array: comptime_value.CTValue)
```

the constant array the `$each` at `sid` reads in the walk's frame

### definition_ast

```mach
pub fun definition_ast(sc: *SemaContext, origin: module.ModuleId) *ast.Ast
```

the syntax of module `origin`, nil when it cannot be had, the reason recorded in the status

### generic_param_type_for

```mach
pub fun generic_param_type_for(sc: *SemaContext, sym: *resolve.Symbol) type_kind.TypeId
```

### inferred_len_of

```mach
pub fun inferred_len_of(sc: *SemaContext, tid: ast_id.TypeId) u64
```

### set_inferred_len

```mach
pub fun set_inferred_len(sc: *SemaContext, tid: ast_id.TypeId, len: u64)
```

### record_embed_path

```mach
pub fun record_embed_path(sc: *SemaContext, path_id: intern.StrId) err[fail.Fail]
```

### query_of

```mach
pub fun query_of(sc: *SemaContext) type_query.Query
```

a question to the type store whose internal failures land in this context

### holds_secret

```mach
pub fun holds_secret(sc: *SemaContext, ty: type_kind.TypeId) bool
```

the answer of a query that cannot refuse the input, or `recovery` once it failed, which
fails the phase as internal
whether a value of type `ty` holds a secret, which gives it no compile-time value

### recover

```mach
pub fun recover[T](sc: *SemaContext, r: res[T, fail.Fail], recovery: T) T
```

### recover_at

```mach
pub fun recover_at[T](sc: *SemaContext, span: base_source.Span, r: res[T, fail.Fail], recovery: T) T
```

the answer of a query asked at `span`, or `recovery` once it failed: a refusal of the input
is reported there, and any other failure fails the phase as internal

### record_at

```mach
pub fun record_at(sc: *SemaContext, span: base_source.Span, r: err[fail.Fail])
```

the outcome of a query asked at `span` that answers nothing, reported as recover_at reports it

### query_settle

```mach
pub fun query_settle(sc: *SemaContext, q: *type_query.Query, span: base_source.Span)
```

a query built by query_of whose walk refused the input, reported at `span`

### record_eval_result

```mach
pub fun record_eval_result(sc: *SemaContext, r: res[comptime_value.CTValue, comptime_failure.EvalFail])
```

### decorator_string_arg

```mach
pub fun decorator_string_arg(sc: *SemaContext, dec: *ast_decl.Decorator, ord: u32, kind: diagnostic_kind.Kind, kind_msg: str) opt[str]
```

the string argument `ord` of a decorator evaluates to. one that is
not a constant expression is refused with `decorator.not_constant`, naming
the decorator, and a constant that is no string with `kind` and `kind_msg`

### require_constant_arg

```mach
pub fun require_constant_arg(sc: *SemaContext, dec: *ast_decl.Decorator, ord: u32)
```

an integer argument that is not a constant expression is refused as one;
a question only a later phase answers, a layout or an instance, is left to it

### decorator_arg_span

```mach
pub fun decorator_arg_span(sc: *SemaContext, eid: ast_id.ExprId, fallback: base_source.Span) base_source.Span
```

## StmtListVisitor

```mach
pub def StmtListVisitor: fun(ptr, *SemaContext, u32, u32) err[fail.Fail]
```

## Functions

### generic_owner_of_decl

```mach
pub fun generic_owner_of_decl(s: *session.Session, a: *ast.Ast, origin: module.ModuleId,
    decl_id: ast_id.DeclId) res[type.GenericOwner, fail.Fail]
```

the identity a declaration's type parameters carry (type.TypeGenericParam owner
fields): the one key both interning a parameter and substituting for it use

### source_text_of

```mach
pub fun source_text_of(s: *session.Session, a: *ast.Ast) res[str, fail.Fail]
```

### is_global_binding

```mach
pub fun is_global_binding(kind: ast_decl.DeclKind) bool
```

### is_fun_or_global

```mach
pub fun is_fun_or_global(kind: ast_decl.DeclKind) bool
```

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `TYPE_MEMO_NONE` | `u8` | `0` |  |
| `TYPE_MEMO_RESOLVED` | `u8` | `1` |  |
| `TYPE_MEMO_ACTIVE` | `u8` | `2` |  |
| `LAYOUT_PENDING` | `u8` | `0` |  |
| `LAYOUT_ACTIVE` | `u8` | `1` |  |
| `LAYOUT_READY` | `u8` | `2` |  |
| `LAYOUT_FAILED` | `u8` | `3` |  |
| `TIMED_NONE` | `u8` | `0` | the constant-time modes a walked body may hold, as `Walk.timed` |
| `TIMED_SOFT` | `u8` | `1` |  |
| `TIMED_HARD` | `u8` | `2` |  |
| `ARM_DECIDED` | `u8` | `0` | which walk reports a `$error` a comptime arm reaches, as `Walk.arm`: this one; none, in an arm this walk cannot select; or this value instance, in an arm it selected that its template could not, whether or not the instance is quiet |
| `ARM_DEFERRED` | `u8` | `1` |  |
| `ARM_SELECTED` | `u8` | `2` |  |

