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

No site answers an unknown member of a listed catalog with a default. The
census `catalog-defaults` in `test/census.sh` lists every production function
that takes a type listed in `test/census/input-catalogs.txt`, branches on that
parameter, and ends with an unconditional literal return, and asserts zero.

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
| `BinOp` | `fe/ast/expr.mach:34` | 19 | internal | infer `:1215`, comptime `:1605`; residual predicates in `sema/coerce.mach:220`, `sema/check.mach:133`, `sema/infer.mach:1308,1371`, `me/pass/constfold.mach:195,275,289,312`, `me/pass/algebraic.mach:130`, `me/pass/scalarize.mach:1573`, `me/lower/expr.mach` answer "not this op" after the guard | internal |
| `UnOp` | `fe/ast/expr.mach:56` | 5 | internal | infer `:1677`, comptime `:1668` | internal |
| `Choice` | `fe/ast/interpretation.mach:11` | 5 | internal | consumed by exhaustive `if` chains in sema with no fallthrough answer | internal |
| `token.Kind` | `fe/token.mach:5` | 48 | internal (lexer) | parser rejects any unexpected kind as a parse diagnostic naming the token; `kind_str` `:109` names an unknown kind `unknown`, `infix_precedence` `:161` answers 0 (not an operator); `grammar.mach:2761` `bin_op_from_kind` is only reached for kinds the caller matched | internal |
| `LexErrorCode` | `fe/lexer.mach:16` | 4 | internal | `error_message` `:458` fallthrough is the last code's text | internal |
| `ParseFailKind` | `fe/parser/state.mach:32` | 4 | internal | `fuzz/parser.mach:110` name fallthrough (fuzz harness) | internal |

### Frontend: sema, types and comptime

| catalog | defined | members | enters | today | class |
| --- | --- | --- | --- | --- | --- |
| `type.TypeKind` | `type.mach:38` | 25 | internal (sema) | 677 dispatch sites; sema returns `TYPE_ERROR` for a shape it cannot type; `sema/recipe.mach:184` names an unknown kind through the policy; `sema/typename.mach:137,197` panic naming the catalog (two-pass string sizing, no Result channel) | internal |
| `PrimClass` | `type.mach:77` | 4 | internal | derived from `TypeKind`; predicates only | internal |
| `VecFormStatus`, `GParamMemo`, `VisitMark` | `type.mach:267,1038,853` | 4, 3, 2 | internal | exhaustive `if`/`or` | internal |
| `FloatWidth` | `float.mach:5` | 3 | internal | `type.mach:261` `float_width_of` answers `FLOAT_W_NONE` for a non-float kind (a partition, not a default) | internal |
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
| `CtOp` | `ct.mach:5` | 5 | internal (sema and MIR classification) | `ct.ct_cap` is an Option: `fe/sema/infer.mach:1383`, `be/codegen/mir/lower.mach:596`, `be/codegen/ctvalidate.mach:300`, `target/isa/asm.mach` refuse `unknown CtOp tag n`; before this change an unknown op answered `CT_CAP_OK` (no capability needed) | internal |
| `CtCap` | `ct.mach:13` | 4 | internal | `target_provides` `:36` answers false for an unknown capability (fail-closed) | unsupported (target declares no trust) |

### Middle end: IR and passes

| catalog | defined | members | enters | today | class |
| --- | --- | --- | --- | --- | --- |
| `InstrKind` | `me/ir/instruction.mach:10` | 50 | internal, **cross-module** (inline bodies copied between modules in process) | verifier `me/ir/verify.mach:492` VC_OPCODE_KNOWN, rendered `:199` `unknown InstrKind tag n`; `opdesc.desc` `me/ir/opdesc.mach:680` answers the conservative `UNKNOWN_DESCRIPTOR` row (every effect, ordered) for a tag the verifier already refused | internal |
| `ValueKind` | `me/ir/value.mach:9` | 9 | internal, **cross-module** (inline bodies) | verifier `:513` VC_VALUE_KNOWN `unknown ValueKind tag n`; `me/ir/body.mach:327` static `inline body: unknown ValueKind`; `be/codegen/mir/context.mach:333` panics naming the site (no Result channel; guarded by the verifier) | internal |
| `IrTypeKind` incl. `IRT_TAG` | `me/ir/type.mach:22` | 11 | internal, **cross-module** (inline bodies) | `me/ir/body.mach:186` static `inline body: unknown IrTypeKind`; type-table accessors panic on a wrong-kind precondition (`me/ir/type.mach:162..484`) | internal |
| `opdesc` columns: `ArityKind`, `OperandRole`, `ResultTyping`, `IrVecOp`, `VecClass`, `CtClass`, `LowerRoute` | `me/ir/opdesc.mach:64,46,72,79,95,103,110` | 5, 15, 4, 13, 5, 4, 20 | internal (table rows) | rows are static; `me/vecform.mach:20` maps an unknown `IrVecOp` to `VEC_OP_NONE` (the scalar expansion) after the verifier; `lower_route` dispatch in `be/codegen/mir/lower.mach` falls to LOWER_GENERIC | internal |
| `RefRole`, `MetaKind` | `me/ir/refs.mach:16,28` | 9, 6 | internal | exhaustive | internal |
| `VerifyCheck` | `me/ir/verify.mach:35` | 22 | internal | `describe` `:131` names an unknown check `unknown verification check` | internal |
| `DepVerdict`, `FoldResult`, `VersionStatus` | `me/analysis/dependence.mach:30`, `me/lower/constagg.mach:25`, `me/transform/versioning.mach:24` | 2, 2, 7 | internal | exhaustive | internal |
| `OptLevel` | `me/pipeline.mach:43` | 2 | input (`-O`, manifest `opt`) as a string; numeric on the request | `request.validate` `build/request.mach:261` `unknown OptLevel tag n` | internal (spelling rejected at the CLI/manifest) |
| `layout.CheckCause`, `ExtentCause`, `NodeKind` | `layout.mach:9,1279,1244` | 13, 10, 9 | internal | name helpers `:362`, `:1523` answer a generic text for an unknown cause | internal |

### Backend: MIR and codegen

| catalog | defined | members | enters | today | class |
| --- | --- | --- | --- | --- | --- |
| `MirOpcode` | `be/codegen/mir.mach:31` | 90 generic + per-ISA native opcodes | internal (selector) | `mir.validate_catalog` `:1070` `unknown MirOpcode tag n`; `mir.desc` `:664` answers `UNKNOWN_MIR_DESCRIPTOR` after that; each ISA encoder's fallthrough is `encode.opcode_failure` (`x64/encode.mach:2316,2409`, `arm64/encode.mach:2787`, `riscv/encode.mach:2140`) `unknown <isa>.Opcode tag n in '<fn>'`; x64 `packed_opbyte` is an Option whose absence fails the encoder (was a panic) | internal |
| `MirOperandKind` | `be/codegen/mir.mach:22` | 6 | internal | `mir.validate_catalog` `:1075`; regalloc rejects `unknown MirOperandKind tag n` | internal |
| `MirCtClass`, `SelectionClass`, `OperandBank`, `MirSlotKind` | `be/codegen/mir.mach:205,213,602,917` | 5, 9, 5, 2 | internal (table rows) | exhaustive | internal |
| `RegClassKind` | `target/isa.mach:109` | 4 | internal | `be/codegen/regalloc.mach:156` `unknown RegClassKind tag n`; `:169` `MirRegisterClass` against the target's declared class count | internal |
| `RuleKind`, `RuleGate` | `be/codegen/rules.mach:18,24` | 3, 3 | internal (rule tables) | exhaustive | internal |
| `LinkMode` | `be/linker.mach:47` | 3 | internal (from the request goal) | `link_mode_valid` `:55` refuses before linking; `loaderless_request_message` `:6693` answers no message for a mode that needs no loader | internal |
| encoder output `RelocKind`, `ConstKind`, `FrameStepKind` | `be/codegen.mach:1220,1229,1246` | | internal (encoder) | `validate_encoder_output` names the tag through the policy | internal |

### Target: instruction sets, ABI and vtables

| catalog | defined | members | enters | today | class |
| --- | --- | --- | --- | --- | --- |
| `x64.Opcode`, `arm64.Opcode`, `riscv.Opcode` | `target/isa/x64.mach:54`, `arm64.mach:82`, `riscv.mach:97` | 135, 31, 18 | internal (selector) | encoder fallthrough `encode.opcode_failure` (above) | internal |
| `mos6502.Opcode` (deleted) | was `target/isa/mos6502.mach:40` | was 6 | none | the withdrawn MOS 6502 target was deleted under #3226 with its instruction set, ABI, registry rows, OS row and `arch` id 4 (`arch.MOS6502_WITHDRAWN`, carried by no catalog row); `mos6502` as an isa or abi spelling is refused by name at target resolution (`target.WITHDRAWN_MOS6502_MSG`), and the `b-be-7` census no longer lists `ARCH_MOS6502` | removed |
| `riscv.MachOp` | `target/isa/riscv/inst.mach:4` | 142 | internal (riscv encoder) | residual: `encode.mach:248` `shape_of` SH_NONE, `:2160` `invert_branch` MOP_NONE, `:2806` `rv_fields` false, `:2900` `rv_amo_funct5` AMOMAXU (only reached for AMO ops), `inst.mach:183` `xlens` XL_ANY, `:210` `required_extensions` 0, `printer.mach:292` `mnemonic` nil | internal (see residuals) |
| `isa.OperandKind` | `target/isa.mach:48` | 6 | internal (encoder-built `isa.Inst`) | encoders return a zero-length encoding for an operand shape they do not encode and the driver reports the opcode; the constant-time validator refuses an operand kind outside the machine-operand catalog (`ctvalidate.mach` RANGE_OPERAND_KIND_DETAIL) | internal |
| `isa.SymModifier` | `target/isa.mach:65` | 5 | internal | `riscv/printer.mach:192` `modifier_text` answers empty for no modifier | internal |
| `isa.VecOp`, `isa.VectorForm` | `target/isa.mach:121,151` | 13, 3 | internal; the ISA declares which cells it packs | `isa.mach:1568` registration refuses an ISA that declares neither a packed form nor the scalar expansion for a retained cell; `me/vecform.mach:118` names an unknown op `operation` | unsupported (declared per ISA) |
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
| `Subsystem` | `target/of.mach:1103` | 2 | input (`--subsystem`, manifest) as a string; numeric on the request | `subsystem_from_name` refuses a spelling; `request.validate` `build/request.mach:270` `unknown Subsystem tag n`; only the PE writer consumes the member (`coff.mach:3399`), an ELF or Mach-O unit carries it unread (`plan.mach:548`), which is a silent ignore rather than an unsupported report and is listed as a follow-up | malformed (spelling); internal (tag); unsupported not yet reported |
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
| `FrontendPhase`, `LoadStatus`, `RootSet`, `BuildMode`, `TargetOpt` | `driver.mach:304`, `driver/project.mach:193,40,48,54` | 3, 3, 3, 2, 3 | internal | exhaustive; `project.mach:708` `map_opt` maps the two manifest levels and answers the default level otherwise (guarded by the manifest parser) | internal |
| `ProgressEventKind`, `fail.Kind`, `fail.PhaseKind`, `fail.CatalogClass` | `readout.mach:66`, `fail.mach:16,46,95` | 3, 2, 3, 3 | internal | exhaustive; `catalog_text` treats any class but unsupported and malformed as internal | internal |
| `Precondition`, `Parents` | `publication.mach:24,29` | 2, 2 | internal | exhaustive | internal |
| `SubprocessState` | `subprocess.mach:39` | 6 | internal (process runner) | exhaustive | internal |
| `EmbedStatus` | `embed.mach:35` | 5 | internal | exhaustive | internal |

### Manifest and CLI

| catalog | defined | members | enters | today | class |
| --- | --- | --- | --- | --- | --- |
| `MOpt`, `SimdMode`, `LibKind`, `LinkSource`, `DepSource` | `manifest.mach:152,161,169,293,4383` | 2, 2, 2, 3, 2 | input (`mach.toml` strings) | the parser refuses a spelling naming the accepted set (`manifest.mach:930` simd, `:1453` kind); `dep_key_name` `:4531` and `key_known`/`key_deprecated` `:979,1084` answer over the produced member | malformed input (spelling) |
| `TableKind`, `ProjectPathViolation` | `manifest.mach:176,1128` | 7, 7 | input (table names) | `:972` `unknown table or key`, `:958` `unknown key` | malformed input |
| `CommandId`, `DepAction`, `OptionArity`, `OperandClass`, `HelpRouteKind`, `EntryKind` | `cli/args.mach:252,279,239`, `cli/util.mach:202`, `cli/cmd/help.mach:26`, `cli/cmd/clean.mach:37` | 11, 8, 2, 2, 4, 2 | input (argv) | the parser leaves an unknown command or flag as `has_unknown` (`cli/args.mach:876`) and the CLI reports it with usage | malformed input |
| `diagnostic.Severity`, `ChildKind` | `diagnostic.mach:26,37` | 4, 2 | internal | `cli/diagnostic.mach:403` `severity_label` answers `help:` for the last severity | internal |
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

Running the census detector over every catalog (not only the listed ones)
finds 43 functions that take a catalog parameter, branch on it, and end in a
literal. None takes a member that crosses a boundary; each sits behind the
guard named in the inventory. They fall in three groups:

- **partitions**: the literal is the answer for "every other member", true by
  construction today (`is_value_preserving_binop`, `kind_is_public_leaf`,
  `is_selected_vector_alu`, `expr_binds_tighter_than_cast`, `key_known`,
  `key_deprecated`, `simplify_binary`, `fold_*`, `eval_*_compare`,
  `vec_binop_module`, `infix_precedence`, `target_provides`, `ct_op_detail`,
  `secret_ct_op_unsupported`, `loaderless_request_message`, `xlens`,
  `required_extensions`, `rv_fields`, `shape_of`, `invert_branch`,
  `modifier_text`, `mnemonic`, `float_width_of`, `float_cmp_cc`);
- **name helpers** that spell an unknown member generically (`kind_str`,
  `describe`, `check_cause_name`, `cause_name`, `vec_op_name`,
  `error_message`, `fail_kind_name`, `severity_label`, `dep_key_name`,
  `unencodable_message`);
- **true defaults** reached only for members the caller already matched
  (`bin_op_from_kind`, `map_opt`, `rv_amo_funct5`, `isa_vec_op`,
  `scalar_binop_ct_op`, `infer_vector_binary`).

Two declared-but-unhonored capabilities are ignored rather than reported as
unsupported and are owner decisions because reporting them changes what a
build accepts: a `--subsystem` on an ELF or Mach-O unit (`plan.mach:548`),
and a manifest `[target]` subsystem on the same. Converting the 43 functions
above to Option-returning lookups is mechanical and touches only
internal catalogs; it is the natural next slice if the owner wants the census
to cover every catalog rather than the boundary-crossing ones. The three
remaining panics on a catalog member (`sema/typename.mach:137,197` and
`be/codegen/mir/context.mach:333`) sit behind the verifier and sema and have
no Result channel to their callers; they name the catalog in the panic text.

## Census

`test/census.sh` `catalog-defaults` reads `test/census/input-catalogs.txt`
(`PhaseKind`, `BuildGoal`, `SubsystemFlag`, `Subsystem`, `SectionKind`,
`RelocKind`, `CTKind`, `SymKind`: the catalogs whose numeric member crosses a
boundary) and fails on any production function that takes one of those types,
branches on it, and ends in an unconditional literal return. It caught the 14
section and relocation sites this page converted and passes with zero; its
own mutation control (appending a two-arm function on `SectionKind`) fails it.

## Tests

One per class, each with the mutation that its assertion catches:

| class | test | mutation caught |
| --- | --- | --- |
| malformed | `of.validate_object:...rejects_malformed_relations` asserts `malformed SectionKind tag 255` and `malformed RelocKind tag 255`; `build.cache.image:section_kind_outside_the_catalog_is_a_malformed_entry` asserts a MALFORMED refusal without interning | dropping the kind check in `validate_object_view`; dropping validation from the cache decoder |
| unsupported | `of.coff.reloc_type_for:declared_unsupported_kind_and_unknown_tag_are_distinct_classes` asserts `RelocKind page21 is unsupported by coff` beside `unknown RelocKind tag 255` | reporting a declared kind as an unknown tag |
| internal | `build.outcome.catalog:three_classes_land_on_distinct_kinds...` asserts FAIL_USER for the first two and FAIL_INTERNAL for `unknown PhaseKind tag 255`; `build.engine.run_phase:...`, `build.request.validate:...`, `fail.catalog:...` | dropping the internal branch of `outcome.catalog` |
| CLI surface | `cli.diagnostic.render_fail:...` asserts `error: unknown FailKind tag 255` exits 2; `outcome_code` likewise for `BuildSeverity` | returning exit 1 silently |
