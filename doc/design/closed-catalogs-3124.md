# Closed catalogs (#3124)

A closed catalog is a finite enumeration the compiler dispatches on: a `pub def
X: u8|u16|u32` with `pub val` members, a descriptor table indexed by an opcode,
or a callback record of function pointers. There are 155 integer typedefs under
`src/`, 131 of them catalogs (the rest are ids, revisions and bit sets). This
page is the census the issue asks for: for each catalog, where an unknown
member can enter, what the compiler does with it, and which of the three
classes that site belongs to. Line numbers are as of the commit that landed
this page; the census in `test/census.sh` holds the count so the page does not
have to.

## The three classes

- **malformed input**: the member arrived from user input (a manifest, a CLI
  flag, an object file read from disk, a cache artifact) or from a cross-module
  product that another compilation unit wrote (an object image, a query
  surface). A bad value is the input's fault and is reported as a user
  diagnostic naming the catalog and the tag.
- **unsupported capability**: the member is valid, and a target, object format
  or phase declares it cannot honor it (a relocation kind COFF has no type for,
  a vector form the ISA lacks, a Windows subsystem on an ELF target). This is
  reported as unsupported naming the catalog, the member and the declarer, and
  is never an internal error.
- **impossible internal state**: the compiler produced the member itself (an
  AST kind, an IR opcode, a MIR operand kind) and no arm of its own dispatch
  names it. This is an internal failure naming the catalog and the tag.

Most catalog members enter the compiler through a string: a manifest key, a
CLI flag, a decorator, a target name. The mapping from string to member is the
input boundary, it rejects an unknown spelling as a user diagnostic that quotes
the spelling, and the member it produces is then compiler-owned. Those
catalogs are internal from the member's point of view: the member cannot be
unknown unless the compiler itself broke. Only a catalog whose *numeric*
member crosses a boundary (a request record handed between layers, a cache
entry, a query surface, an object image) can carry an unknown member into a
dispatch, and those are the catalogs the census lists.

## The one policy

`mach.lang.fail` owns the policy: `fail.Catalog { class, catalog, member,
tag, where }` with `CATALOG_MALFORMED`, `CATALOG_UNSUPPORTED`,
`CATALOG_INTERNAL`, built by `fail.malformed_member`, `fail.unsupported_member`
and `fail.unknown_member` (or `fail.catalog_member` with a `where`).
`fail.catalog_text` renders the one message shape:

| class | message |
| --- | --- |
| malformed | `malformed <catalog> tag <n>` (`... in <where>`) |
| unsupported | `<catalog> <member> is unsupported by <declarer>` |
| internal | `unknown <catalog> tag <n>` (`... in <where>`) |

`fail.catalog_interned` and `fail.catalog_message` own the text in an
interner, `fail.catalog_message_or` degrades to a caller-written static text
naming the catalog when the site has no interner or allocator (a borrowed
view, a test fixture), and `fail.catalog_status` lands a class on a
`PhaseStatus` (input and capability faults reject, an internal member is
internal). Every layer adapter routes through it:

| adapter | layer kind |
| --- | --- |
| `resolve.unknown_catalog` / `unknown_kind` | `fail.PhaseStatus` INTERNAL |
| `comptime.unknown_catalog` / `checked_value` | `EvalFail` INTERNAL |
| `verify` VC_OPCODE_KNOWN / VC_VALUE_KNOWN via `describe_located` | verifier violation with tag |
| `mir.catalog_failure` | codegen `str` error |
| `encode.opcode_failure` | codegen `str` error, catalog `<isa>.Opcode` |
| `outcome.catalog` / `outcome.unknown_catalog` | `outcome.Fail` USER (malformed, unsupported) or INTERNAL |
| `of.malformed_member_message` | object-image validator, malformed |
| `of.reloc_kind_rejection` / `of.section_kind_rejection` | format writers and the linker: unsupported by `<format>` for a declared kind, internal for a tag outside the catalog |
| `linker.malformed_input_member` | an input object's member, malformed `in <object>` |
| `mangle.unencodable_value` | `fail.Fail` message |
| `driver.query_compute` | `fail.Fail` message |

The classes land on distinct kinds at every reporting surface: `PhaseStatus`
REJECTED versus INTERNAL, `outcome.Fail` FAIL_USER (exit 1) versus
FAIL_INTERNAL (exit 2), the cache codec MALFORMED versus INTERNAL. The
malformed and unsupported classes share the user-facing kind and differ in the
message lead, which is what the tests assert.

No site answers an unknown member of any catalog with a default. The census
`catalog-defaults` in `test/census.sh` derives every catalog from the source
(an integer typedef with two or more literal members; an id with sentinel
members only and a bit set whose members are all powers of two are not
catalogs), lists every production function that takes a catalog type,
branches on that parameter, and ends with an unconditional literal return
(bare or wrapped in `opt.some` / `res.ok`), and asserts zero. A function may
keep such a literal only as a recorded partition: a test in the same file
named `<fn>:partition_<Catalog>` that names every member of the catalog, which
the census checks member by member, so a member added later fails the census
until the test classifies it. `test/census/catalog-exceptions.txt` lists a
catalog the census skips and is empty.

## Inventory

Columns: where an unknown member can **enter**; what happens **today** (with
the rejecting site); the **class** of that rejection. "guarded by X" means the
dispatch itself has no fallthrough of its own because X refuses the member
before it is reached.

### Frontend: AST and tokens

| catalog | defined | members | enters | today | class |
| --- | --- | --- | --- | --- | --- |
| `ExprKind` | `fe/ast/expr.mach:7` | 24 | internal (parser) | resolve `fe/resolve.mach:1998` `unknown_kind`, sema `fe/sema/infer.mach:1013`, comptime `fe/comptime.mach:1156`: `unknown ExprKind tag n` | internal |
| `StmtKind` | `fe/ast/stmt.mach:4` | 14 | internal | resolve `:1815`, sema `fe/sema.mach:2174` | internal |
| `DeclKind` | `fe/ast/decl.mach:6` | 13 | internal | resolve `:961`, `:1411`, `:1853`, sema `fe/sema.mach:2738`, infer `:80` | internal |
| `TypeKind` (ast) | `fe/ast/type.mach:6` | 12 | internal | resolve `:2593`, infer `:3055` as `AstTypeKind` | internal |
| `BinOp` | `fe/ast/expr.mach:34` | 19 | internal | infer `:1215`, comptime `:1605`; `is_value_preserving_binop` and `scalar_binop_ct_op` are recorded partitions; `infer_vector_binary` names every operator and routes the impossible arm through `unknown_binop` | internal |
| `UnOp` | `fe/ast/expr.mach:56` | 5 | internal | infer `:1677`, comptime `:1668` | internal |
| `Choice` | `fe/ast/interpretation.mach:11` | 5 | internal | consumed by exhaustive `if` chains in sema with no fallthrough answer | internal |
| `token.Kind` | `fe/token.mach:5` | 48 | internal (lexer) | parser rejects any unexpected kind as a parse diagnostic naming the token; `kind_str` answers `opt[str]`, `infix_precedence` answers 0 for every non-operator kind (a recorded partition); `grammar.mach` `bin_op_from_kind` is an `opt` lookup whose absence is a parser ICE naming the catalog | internal |
| `LexErrorCode` | `fe/lexer.mach:16` | 4 | internal | `error_message` answers `opt[str]`; `emit_diagnostics` refuses an absent code naming the catalog | internal |
| `ParseFailKind` | `fe/parser/state.mach:32` | 4 | internal | `fuzz/parser.mach` `fail_kind_name` answers `opt[str]`; the harness reports an absent kind as a violation | internal |

### Frontend: sema, types and comptime

| catalog | defined | members | enters | today | class |
| --- | --- | --- | --- | --- | --- |
| `type.TypeKind` | `type.mach:38` | 25 | internal (sema) | 677 dispatch sites; sema returns `TYPE_ERROR` for a shape it cannot type; `sema/recipe.mach:184` names an unknown kind through the policy; `sema/typename.mach` sizing and write passes answer `res[usize, fail.Fail]` naming the tag and the pass | internal |
| `PrimClass` | `type.mach:77` | 4 | internal | derived from `TypeKind`; predicates only | internal |
| `VecFormStatus`, `GParamMemo`, `VisitMark` | `type.mach:267,1038,853` | 4, 3, 2 | internal | exhaustive `if`/`or` | internal |
| `FloatWidth` | `float.mach:5` | 3 | internal | `type.mach` `float_width_of` answers `FLOAT_W_NONE` for every non-float kind (a recorded partition) | internal |
| `SymKind` | `fe/resolve.mach:48` | 14 | internal, **cross-module** (public symbol surface bytes) | decoder `driver/query.mach:540` `malformed SymKind tag n` via `resolve.sym_kind_valid`; resolver dispatch exhaustive | malformed at the surface, internal after |
| `TypeSpellStatus` | `fe/resolve.mach:807` | 3 | internal | exhaustive | internal |
| `CTKind` | `fe/comptime.mach:40` | 8 | internal, **cross-module** (constant surface bytes) | decoder `driver/query.mach:711` `malformed CTKind tag n`; encoder `:425` static internal text; comptime `:1065` `unknown CTKind tag n`; `me/lower/mangle.mach:316` | malformed at the surface, internal after |
| `ConstKind` | `fe/comptime.mach:442` | 5 | internal | comptime fold arms; the encoder's constant pool (`be/codegen/encode.mach` CONST_*) is validated at `be/codegen.mach:1229` `unknown ConstKind tag n` | internal |
| `EvalFailKind` | `fe/comptime.mach:389` | 7 | internal | exhaustive mapping to diagnostics | internal |
| `GateState`, `GateOutcome`, `GateProbe`, `GateNodeVerdict`, `GateScope` | `fe/comptime.mach:67,74,1932,1944,2023` | 4, 8, 7, 3, 2 | internal | exhaustive | internal |
| `PhaseCapabilityKind` | `fe/comptime.mach:178` | 6 | internal | exhaustive | internal |
| `CoerceKind` | `fe/sema/coerce.mach:18` | 3 | internal | exhaustive | internal |
| `CaptureMode`, `SecrecyVerdict`, `DefinitionPhase`, `ReadStatus` (abitype, handle) | `fe/sema/fields.mach:38`, `generics.mach:410`, `context.mach:82`, `abitype.mach:24`, `handle.mach:32` | 2, 2, 3, 2, 3 | internal | exhaustive | internal |
| `ItemKind` | `fe/doclint.mach:26` | 4 | internal | exhaustive | internal |
| `EmbedStatus` | `embed.mach:35` | 5 | internal | infer maps each status to a diagnostic | internal |
| `CtOp` | `ct.mach:5` | 5 | internal (sema and MIR classification) | `ct.ct_cap` is an Option; `scalar_binop_ct_op` is a recorded partition over `BinOp`: `fe/sema/infer.mach:1383`, `be/codegen/mir/lower.mach:596`, `be/codegen/ctvalidate.mach:300`, `target/isa/asm.mach` refuse `unknown CtOp tag n`; before this change an unknown op answered `CT_CAP_OK` (no capability needed) | internal |
| `CtCap` | `ct.mach:13` | 4 | internal | `target_provides` answers false for every capability outside its four arms (a recorded partition) | unsupported (target declares no trust) |

### Middle end: IR and passes

| catalog | defined | members | enters | today | class |
| --- | --- | --- | --- | --- | --- |
| `InstrKind` | `me/ir/instruction.mach:10` | 50 | internal, **cross-module** (inline bodies copied between modules in process) | verifier `me/ir/verify.mach:492` VC_OPCODE_KNOWN, rendered `:199` `unknown InstrKind tag n`; `opdesc.desc` `me/ir/opdesc.mach:680` answers the conservative `UNKNOWN_DESCRIPTOR` row (every effect, ordered) for a tag the verifier already refused | internal |
| `ValueKind` | `me/ir/value.mach:9` | 9 | internal, **cross-module** (inline bodies) | verifier `:513` VC_VALUE_KNOWN `unknown ValueKind tag n`; `me/ir/body.mach:327` static `inline body: unknown ValueKind`; `be/codegen/mir/context.mach` `lower_value` answers `res[MirOperand, fail.Fail]`: `const_bytes` and `const_agg` are unsupported as a MIR operand by name, a tag outside the catalog is internal | internal |
| `IrTypeKind` incl. `IRT_TAG` | `me/ir/type.mach:22` | 11 | internal, **cross-module** (inline bodies) | `me/ir/body.mach:186` static `inline body: unknown IrTypeKind`; type-table accessors panic on a wrong-kind precondition (`me/ir/type.mach:162..484`) | internal |
| `opdesc` columns: `ArityKind`, `OperandRole`, `ResultTyping`, `IrVecOp`, `VecClass`, `CtClass`, `LowerRoute` | `me/ir/opdesc.mach:64,46,72,79,95,103,110` | 5, 15, 4, 13, 5, 4, 20 | internal (table rows) | rows are static; `me/vecform.mach` `vec_op_of` answers `opt[isa.VecOp]` and the pipeline names an absent `IrVecOp` tag in its diagnostic; `lower_route` dispatch in `be/codegen/mir/lower.mach` falls to LOWER_GENERIC | internal |
| `RefRole`, `MetaKind` | `me/ir/refs.mach:16,28` | 9, 6 | internal | exhaustive | internal |
| `VerifyCheck` | `me/ir/verify.mach:35` | 22 | internal | `describe` answers `opt[str]`; `describe_located` names an absent check `unknown VerifyCheck tag n` | internal |
| `DepVerdict`, `FoldResult`, `VersionStatus` | `me/analysis/dependence.mach:30`, `me/lower/constagg.mach:25`, `me/transform/versioning.mach:24` | 2, 2, 7 | internal | exhaustive | internal |
| `OptLevel` | `me/pipeline.mach:43` | 2 | input (`-O`, manifest `opt`) as a string; numeric on the request | `request.validate` `build/request.mach:261` `unknown OptLevel tag n` | internal (spelling rejected at the CLI/manifest) |
| `layout.CheckCause`, `ExtentCause`, `NodeKind` | `layout.mach:9,1279,1244` | 13, 10, 9 | internal | `check_cause_name` and `cause_name` answer `opt[str]`; sema reports an absent cause as `unknown ExtentCause tag n` | internal |

### Backend: MIR and codegen

| catalog | defined | members | enters | today | class |
| --- | --- | --- | --- | --- | --- |
| `MirOpcode` | `be/codegen/mir.mach:31` | 90 generic + per-ISA native opcodes | internal (selector) | `mir.validate_catalog` `:1070` `unknown MirOpcode tag n`; `mir.desc` `:664` answers `UNKNOWN_MIR_DESCRIPTOR` after that; each ISA encoder's fallthrough is `encode.opcode_failure` (`x64/encode.mach:2316,2409`, `arm64/encode.mach:2787`, `riscv/encode.mach:2140`) `unknown <isa>.Opcode tag n in '<fn>'`; x64 `packed_opbyte` is an Option whose absence fails the encoder (was a panic); x64 `is_selected_vector_alu` is a recorded partition | internal |
| `MirOperandKind` | `be/codegen/mir.mach:22` | 6 | internal | `mir.validate_catalog` `:1075`; regalloc rejects `unknown MirOperandKind tag n` | internal |
| `MirCtClass`, `SelectionClass`, `OperandBank`, `MirSlotKind` | `be/codegen/mir.mach:205,213,602,917` | 5, 9, 5, 2 | internal (table rows) | exhaustive | internal |
| `RegClassKind` | `target/isa.mach:109` | 4 | internal | `be/codegen/regalloc.mach:156` `unknown RegClassKind tag n`; `:169` `MirRegisterClass` against the target's declared class count | internal |
| `RuleKind`, `RuleGate` | `be/codegen/rules.mach:18,24` | 3, 3 | internal (rule tables) | exhaustive | internal |
| `LinkMode` | `be/linker.mach:47` | 3 | internal (from the request goal) | `link_mode_valid` `:55` refuses before linking; `mode_needs_loader` and `mode_allocates_commons` are `opt` lookups whose absence is `unknown LinkMode tag n` | internal |
| encoder output `RelocKind`, `ConstKind`, `FrameStepKind` | `be/codegen.mach:1220,1229,1246` | | internal (encoder) | `validate_encoder_output` names the tag through the policy | internal |

### Target: instruction sets, ABI and vtables

| catalog | defined | members | enters | today | class |
| --- | --- | --- | --- | --- | --- |
| `x64.Opcode`, `arm64.Opcode`, `riscv.Opcode` | `target/isa/x64.mach:54`, `arm64.mach:82`, `riscv.mach:97` | 135, 31, 18 | internal (selector) | encoder fallthrough `encode.opcode_failure` (above) | internal |
| `mos6502.Opcode` (deleted) | was `target/isa/mos6502.mach:40` | was 6 | none | the withdrawn MOS 6502 target was deleted under #3226 with its instruction set, ABI, registry rows, OS row and `arch` id 4 (`arch.MOS6502_WITHDRAWN`, carried by no catalog row); `mos6502` as an isa or abi spelling is refused by name at target resolution (`target.WITHDRAWN_MOS6502_MSG`), and the `b-be-7` census no longer lists `ARCH_MOS6502` | removed |
| `riscv.MachOp` | `target/isa/riscv/inst.mach:4` | 142 | internal (riscv encoder) | `shape_of`, `invert_branch`, `rv_fields`, `rv_amo_funct5`, `required_extensions` and `mnemonic` are `opt` lookups (totality tests walk `MOP_LAST`); `xlens` is a recorded partition; the encoder's notification gate refuses an opcode without a shape | internal |
| `isa.OperandKind` | `target/isa.mach:48` | 6 | internal (encoder-built `isa.Inst`) | encoders return a zero-length encoding for an operand shape they do not encode and the driver reports the opcode; the constant-time validator refuses an operand kind outside the machine-operand catalog (`ctvalidate.mach` RANGE_OPERAND_KIND_DETAIL) | internal |
| `isa.SymModifier` | `target/isa.mach:65` | 5 | internal | `riscv/printer.mach` `modifier_text` names every member and answers `opt[str]`; the emitter refuses an absent one | internal |
| `isa.VecOp`, `isa.VectorForm` | `target/isa.mach:121,151` | 13, 3 | internal; the ISA declares which cells it packs | `isa.mach:1568` registration refuses an ISA that declares neither a packed form nor the scalar expansion for a retained cell; `me/vecform.mach` `vec_op_name` answers `opt[str]` | unsupported (declared per ISA) |
| `isa.Endian`, `isa.BackendKind` | `target/isa.mach:27,1836` | 2, 2 | internal (target definition) | exhaustive | internal |
| `abi.ParamClass`, `abi.PieceKind`, `abi.PassingModel` | `target/abi.mach:93,105,156` | 9, 3, 2 | internal (ABI classifier) | `abi.piece_memory_width` `target/abi.mach:921` static `abi: piece has an unknown PieceKind`; `be/codegen/mir/abi.mach` dispatches the class exhaustively per passing model; `target.mach:707` refuses `PASSING_VALUES` for an emitter that cannot carry logical values | internal; unsupported (passing model per emitter) |
| `AbiVTable`, `IsaVTable`, `OsVTable`, `OfVTable`, `DebugVTable` callbacks | `target/abi.mach:160`, `isa.mach:693`, `os.mach:120`, `of.mach:1318,1741` | function pointers | internal (registration) | `register` in each module refuses a nil or incomplete descriptor (`abi.mach:401`, `of.mach:1545`, `isa.mach:1317`, `of.debug_hooks_complete` `:2037`); an absent optional callback is a declared absence (`os.mach:500..527`, `linker.mach:6668` `pie_exec`) and answers the capability as not provided | internal at registration; unsupported for a declared absence |
| `EncodeHooks`, `ExpansionBuilder`, `RelocSeam` | `be/codegen/encode.mach:499`, `rules.mach:32`, `isa.mach:757` | function pointers | internal | `isa.emits_relocations` and the encode driver refuse a seam with a nil hook before use; `ElfRelocTypeFn` is `O.Option[u32]` (was a zero sentinel) | internal |
| target ids: `arch`, `os`, `of` names and ids | `target/arch.mach`, `os.mach:50`, `of.mach:57` | registry | input (`--target`, manifest `[target]`) | `target.resolve` `target.mach:341` refuses an unregistered os, isa, abi or object format naming the registered set; `of_id_for` answers `OF_UNKNOWN` which `resolve` refuses | malformed input (spelling) |
| RISC-V extensions | `target/rvfeatures.mach` | selection string | input | `target.mach` `selection_message` names the unknown extension | malformed input |

### Object formats and the linker

| catalog | defined | members | enters | today | class |
| --- | --- | --- | --- | --- | --- |
| `SectionKind` | `target/of.mach:106` (`SECTION_KINDS` table) | 7 | **input** (parsed objects map format sections to a kind), **cross-module** (cache entries carry the byte) | `of.validate_object_view` `:714` `malformed SectionKind tag n` runs at every writer, linker read and cache decode; linker `be/linker.mach` `malformed_input_member` names the object; writers derive placement from the row (`elf.mach:1150`, `coff.mach:1430`, `macho.mach:1027,3226`, linker `seg_flags_for`) and refuse an absent row as internal | malformed at the boundary, internal after |
| `RelocKind` | `target/of.mach:118` (`RELOC_KINDS` table) | 28 | **input** (parsers map r_type), **cross-module** (cache entries) | `of.validate_object_view` `:825`; parsers refuse an r_type they do not map as unsupported (`elf.mach:441`, `coff.mach`, `macho.mach:243`); writers refuse a declared kind they have no type for as `RelocKind <name> is unsupported by <format>` (`elf.mach:474`, `coff.mach:216`, `macho.mach:311,315`) and a tag outside the catalog as `unknown RelocKind tag n`; the linker reports an unsupported kind by ISA (`linker.mach:6938`) | malformed at the boundary; unsupported per format or ISA; internal for a tag outside the catalog |
| `FrameStepKind` (`FRAME_STEP_*`) | `target/of.mach:321` | 5 | cross-module (cache) and internal (encoder) | `:861` malformed; `be/codegen.mach:1246` internal | malformed / internal |
| `ObjectFormat` ids (`OF_*`) on native sections and symbols | `target/of.mach:23` | 6 | cross-module (cache) | `:711`, `:766` `malformed ObjectFormat tag n` | malformed |
| native section and symbol kinds (`native.kind`) | per format | format values | input | `of.validate_native_sections` `:277` checks the field widths per format; the parser that produced them is the authority | malformed |
| `RelocError`, `RelocAddendMode` | `target/of.mach:1200,1205` | 3, 3 | internal (relocation seam) | linker maps each error to a message; exhaustive | internal |
| `Subsystem` | `target/of.mach:1103` | 2 | input (`--subsystem`, manifest) as a string; numeric on the request | `subsystem_from_name` refuses a spelling; `request.validate` `build/request.mach:270` `unknown Subsystem tag n`; `OfVTable.carries_subsystem` declares the formats that honor it (COFF) and `plan.mach` `subsystem_admitted` refuses a declared subsystem on any other as `Subsystem <name> is unsupported by <format>` | malformed (spelling); internal (tag); unsupported (declared per format) |
| `ArtifactOutputKind` | `target/of.mach:1390` | 4 | internal (from the goal) | exhaustive | internal |
| symbol flags (`SYM_OBJ_FLAG_*`) | `target/of.mach:166` | bit set | input, cross-module | `symbol_flags_valid` refuses an incompatible set | malformed |

### Build, driver and queries

| catalog | defined | members | enters | today | class |
| --- | --- | --- | --- | --- | --- |
| `PhaseKind` (build) | `build/plan.mach:54` | 8 | internal (plan), numeric on the unit | `plan.phase_name` is an Option; `engine.run_phase` `build/engine.mach:575` and `plan.render` refuse `unknown PhaseKind tag n` before execution | internal |
| `BuildGoal` | `build/request.mach:123` (`BUILD_GOALS` table) | 5 | input (`--emit`) as a string; numeric on the request | `goal_from_name` refuses a spelling; `request.validate` `:258` `unknown BuildGoal tag n` before planning, driver setup and hashing | malformed (spelling); internal (tag) |
| `SubsystemFlag`, `SimdMode`, `OptLevel`, `Subsystem` on the request | `build/request.mach:84`, `manifest.mach:161`, `me/pipeline.mach:43`, `target/of.mach:1103` | 3, 2, 2, 2 | numeric on the request | `request.validate` `:258..270` | internal |
| `LinkInputKind` | `build/plan.mach:72` | 2 | internal | exhaustive | internal |
| `ArtifactKind`, `BuildEventKind`, `BuildSeverity`, `FailKind` | `build/outcome.mach:85,106,134,26` | 5, 4, 4, 4 | internal | `outcome.fold_fail` escalates an unknown `FailKind` to internal; the CLI renderer `cli/diagnostic.mach` writes `error: unknown FailKind tag n` (exit 2), `unknown BuildSeverity tag n` and `unknown BuildEventKind tag n` (all three were panics); `editor.mach` `fail_owned` refuses an unknown kind as internal | internal |
| `fingerprint.Domain` | `build/fingerprint.mach:19` | 12 | internal (hash schema) | written, never dispatched on | internal |
| `QueryKind` | `query.mach:29` | 21 (non-contiguous) | internal (registration at session init) | `query.is_registered` guards every access; `driver.query_compute` `driver.mach:221` refuses an unregistered kind as `unknown QueryKind tag n` and a registered kind without a driver computation as `unknown QueryKind tag n in the driver compute dispatch`; the `q-kinds` census keeps the values distinct | internal |
| `ShardRole`, `Revision` | `query.mach:55,53` | 3 | internal | exhaustive | internal |
| `FrontendPhase`, `LoadStatus`, `RootSet`, `BuildMode`, `TargetOpt` | `driver.mach:304`, `driver/project.mach:193,40,48,54` | 3, 3, 3, 2, 3 | internal | exhaustive; `project.mach` `map_opt` is an `opt` lookup both callers report as `unknown MOpt tag n` | internal |
| `ProgressEventKind`, `fail.Kind`, `fail.PhaseKind`, `fail.CatalogClass` | `readout.mach:66`, `fail.mach:16,46,95` | 3, 2, 3, 3 | internal | exhaustive; `catalog_text` treats any class but unsupported and malformed as internal | internal |
| `Precondition`, `Parents` | `publication.mach:24,29` | 2, 2 | internal | exhaustive | internal |
| `SubprocessState` | `subprocess.mach:39` | 6 | internal (process runner) | exhaustive | internal |
| `EmbedStatus` | `embed.mach:35` | 5 | internal | exhaustive | internal |

### Manifest and CLI

| catalog | defined | members | enters | today | class |
| --- | --- | --- | --- | --- | --- |
| `MOpt`, `SimdMode`, `LibKind`, `LinkSource`, `DepSource` | `manifest.mach:152,161,169,293,4383` | 2, 2, 2, 3, 2 | input (`mach.toml` strings) | the parser refuses a spelling naming the accepted set (`manifest.mach:930` simd, `:1453` kind); `dep_key_name` and `key_known` answer `opt` with the absent arm reported as an unknown member; `key_removed` is a recorded partition | malformed input (spelling) |
| `TableKind`, `ProjectPathViolation` | `manifest.mach:176,1128` | 7, 7 | input (table names) | `:972` `unknown table or key`, `:958` `unknown key` | malformed input |
| `CommandId`, `DepAction`, `OptionArity`, `OperandClass`, `HelpRouteKind`, `EntryKind` | `cli/args.mach:252,279,239`, `cli/util.mach:202`, `cli/cmd/help.mach:26`, `cli/cmd/clean.mach:37` | 11, 8, 2, 2, 4, 2 | input (argv) | the parser leaves an unknown command or flag as `has_unknown` (`cli/args.mach:876`) and the CLI reports it with usage | malformed input |
| `diagnostic.Severity`, `ChildKind` | `diagnostic.mach:26,37` | 4, 2 | internal | `cli/diagnostic.mach` `severity_label` names every severity and answers `opt[str]`; the headline names an absent tag | internal |
| decorators, `$mach.os.*`/`arch`/`abi`/`mode` tags, pipeline stages | `fe/sema.mach:1928,1458`, `fe/comptime.mach:3539..3603` | strings | input (source text) | user diagnostics naming the accepted set | malformed input |

### Validation harness (test-only catalogs)

`ValidationCause`, `ValidationGateResultKind`, `InjectionDomain`,
`InjectionEffect`, `IoAttemptKind` (`validation.mach:31,21,88,96,150`),
`ValidationGateCategory`, `ValidationResourceClass`, `CatalogLookupKind`
(`validation/catalog.mach:18,46,156`), `CodegenTestPhase/Status/Expected/Case`
(`driver/tests.mach:1515..1541`) and `TestComputeMode` (`query.mach:1811`) are
produced and consumed by the validation and test harnesses; each dispatch is
exhaustive and an unknown member is a harness failure.

## Descriptor tables

The drop-in shape for a catalog every format and the linker consult is one row
per member, indexed by the member, with a lookup that answers absent for a tag
outside the table:

- `of.SECTION_KINDS` / `of.section_desc`: canonical name, loadable, executable,
  writable, has_bits, debug. ELF section type and flags, COFF symbol type and
  data presence, the linker's segment protection and the Mach-O
  `MACHO_SECTIONS` row (segment, section name, flags) derive from it.
- `of.RELOC_KINDS` / `of.reloc_desc`: display name and the kind a difference
  applies as. COFF's `COFF_RELOCS` row (AMD64 type, field width) and the ELF
  seam's `ElfRelocTypeFn` are the per-format columns; a declared kind without
  a row is unsupported by that format.
- `request.BUILD_GOALS`, `mir.SELECTION_CATALOG`, `opdesc.IR_OP_DESCRIPTORS`
  were already tables.

Adding a member is adding a row; the table tests
(`of.reloc_kinds`, `of.section_kinds`, `macho.section_table`) assert that
every member has its row and no other tag does.

## Residual default answers on internal catalogs

None. The 43 functions the first census run listed were converted in the
second slice (#3124, `fix/3124-residual`):

- **partitions** keep their literal as the answer for every other member and
  are recorded by a `<fn>:partition_<Catalog>` test naming every member:
  `token.infix_precedence` (`Kind`), `check.expr_binds_tighter_than_cast`
  (`ExprKind`), `coerce.is_value_preserving_binop` and
  `infer.scalar_binop_ct_op` (`BinOp`), `generics.kind_is_public_leaf` and
  `type.float_width_of` (`TypeKind`), `manifest.key_removed` (`TableKind`),
  `ct.target_provides` (`CtCap`), `x64.encode.is_selected_vector_alu`
  (`MirOpcode`) and `riscv.inst.xlens` (`MachOp`);
- **name helpers** answer `opt[str]` and every caller names an absent member
  through the policy or a static text naming the catalog: `token.kind_str`,
  `lexer.error_message`, `fuzz.parser.fail_kind_name`,
  `layout.check_cause_name` and `cause_name`, `cli.diagnostic.severity_label`,
  `manifest.dep_key_name`, `vecform.vec_op_name`, `verify.describe`, the
  arm64 and riscv `mnemonic` (the `MnemonicFn` hook is `fun(u16) opt[str]` on
  every ISA and the printers refuse by name), `ctvalidate.ct_op_detail`,
  `mir.lower.secret_ct_op_unsupported`, `riscv.printer.modifier_text`;
- **true defaults** are `opt` lookups whose impossible arm the caller reports
  as `unknown <Catalog> tag n`: `grammar.bin_op_from_kind` (a parser ICE),
  `project.map_opt` (`outcome.unknown_catalog` at both callers),
  `manifest.key_known`, `vecform.isa_vec_op` and `vec_op_of`,
  `constfold.eval_int_compare` / `eval_float_compare` / `eval_int_arith` /
  `eval_float_arith` and `algebraic.simplify_binary` (the folder and the
  simplifier carry `res[bool, fail.Fail]` through `opdesc.unknown_kind`),
  `mir.lower.float_cmp_cc` and the float-op dispatch, `linker.mode_needs_loader`
  and `mode_allocates_commons`, riscv `shape_of`, `invert_branch`, `rv_fields`,
  `rv_amo_funct5`, `required_extensions` (totality tests walk `MOP_LAST`),
  `macho.recover_addend` (a declared kind it recovers no addend for is
  `RelocKind <name> is unsupported by macho x86_64 addend recovery`);
- the sema `infer_vector_binary` names `BIN_AND`, `BIN_OR` and `BIN_ASSIGN`
  as unsupported on vectors and routes the impossible arm through
  `unknown_binop`; the scalarize fixture `vec_binop_module` is a `t_` test
  helper.

The three panics carry a `Result` channel: `sema/typename.mach`'s sizing and
write passes answer `res[usize, fail.Fail]` (`unknown TypeKind tag n in the
type spelling's sizing pass`, tested by mutating an interned type's kind) and
`be/codegen/mir/context.mach`'s `lower_value` and its wrappers answer
`res[mir.MirOperand, fail.Fail]` (`ValueKind const_bytes is unsupported by a
MIR operand`, `unknown ValueKind tag n in the operand lowering`).

The two declared-but-unhonored capabilities are reported as unsupported:
`of.OfVTable.carries_subsystem` declares that a format's image header records
a `Subsystem` (COFF only), and `build/plan.mach` `subsystem_admitted` refuses
a `--subsystem` flag or a written `[artifact.<name>].subsystem` key on a unit
whose format does not carry one as `Subsystem <name> is unsupported by
<format>`, located at the flag or the manifest key and naming the target. An
omitted key stays the console default; the manifest records
`subsystem_declared` so the default is never refused. This changes what a
build accepts and is recorded in the CHANGELOG and `doc/manifest.md`.

## Census

`test/census.sh` `catalog-defaults` derives the catalogs from `src/` (130 at
this commit), fails on any production function that takes one of those types,
branches on it, and ends in an unconditional literal return, bare or wrapped
in `opt.some` / `res.ok`, unless a `<fn>:partition_<Catalog>` test in the same
file names every member of that catalog (10 recorded partitions). It reports
`ok (130 catalogs, 10 recorded partitions)`; `test/census/catalog-exceptions.txt`
is empty. Its mutation controls: appending a two-arm function on any catalog
fails it; deleting a member name from a partition test fails it naming the
member; adding a member to `BinOp` fails it for the three `BinOp` partition
tests until they classify the member.

## Tests

One per class, each with the mutation that its assertion catches:

| class | test | mutation caught |
| --- | --- | --- |
| malformed | `of.validate_object:...rejects_malformed_relations` asserts `malformed SectionKind tag 255` and `malformed RelocKind tag 255`; `build.cache.image:section_kind_outside_the_catalog_is_a_malformed_entry` asserts a MALFORMED refusal without interning | dropping the kind check in `validate_object_view`; dropping validation from the cache decoder |
| unsupported | `of.coff.reloc_type_for:declared_unsupported_kind_and_unknown_tag_are_distinct_classes` asserts `RelocKind page21 is unsupported by coff` beside `unknown RelocKind tag 255` | reporting a declared kind as an unknown tag |
| internal | `build.outcome.catalog:three_classes_land_on_distinct_kinds...` asserts FAIL_USER for the first two and FAIL_INTERNAL for `unknown PhaseKind tag 255`; `build.engine.run_phase:...`, `build.request.validate:...`, `fail.catalog:...` | dropping the internal branch of `outcome.catalog` |
| CLI surface | `cli.diagnostic.render_fail:...` asserts `error: unknown FailKind tag 255` exits 2; `outcome_code` likewise for `BuildSeverity` | returning exit 1 silently |
