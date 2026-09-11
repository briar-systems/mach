# L6 tag transport through SPIR-V (#3218)

Inventory and contract for roadmap item L6 on feat/3218, recorded 2026-09-11
before any emitter change, then annotated with what the lane landed. Every
"today" cell was established by building a one-file shader project with the
feat/3218 compiler at b430e268e for the `spirv` target in both profiles, and
the representation decisions were checked against `spirv-val` 2026.3 on this
host under `spv1.0`, `spv1.3`, `spv1.6`, `vulkan1.0` through `vulkan1.3`.

Contract: `doc/design/tagged-values.md` ("Initialization and representation":
whole-module targets represent logical cases and payload types without invented
physical registers; "Payload places and guards": the debug-profile check).
Interface: `doc/design/spirv-abi-2963.md` section 6. Native layout and
carriers: `doc/design/l3-inventory-3218.md`, `doc/design/l5-abi-inventory-3218.md`.
Guard matrix: `doc/design/l8-lowering-acceptance-3218.md`.

## 1. Inventory: a tag on the spirv target today

The emitter has no `IRT_TAG` arm in `spv_type_of`, so every use of a tag that
needs a SPIR-V type fails at the first site that asks for one. The messages
below are the diagnostics the compiler printed, each located unless noted.

| cell | today | verdict |
|---|---|---|
| `Function`-storage local (`var r: Reply = Reply.value{41}`) | "unsupported local allocation type in the SPIR-V target" at the `var` | gap |
| module-scope interface variable, `#[input]`/`#[output]` of a tag type | "unsupported shader interface variable type in the SPIR-V target (in \<unknown function\>)", no source location | gap |
| module-scope `#[uniform]`/`#[storage]` block with a tag member | "unsupported uniform block member type in the SPIR-V target", no source location | gap (this lane settles it as refused-by-policy, section 2.4) |
| field inside a record, element of an array | same alloca refusal as the local: the record or array has no SPIR-V type because its member has none | gap |
| by-value parameter `fun bump(r: Reply)` | the body's construction fails first ("unsupported local allocation type"); with no construction the parameter refusal spells the type as `?`: "unsupported parameter 1 of type `?` ... this type has no SPIR-V form" | gap |
| return value `fun bump(...) Reply` | "unsupported return type in the SPIR-V target" at the function | gap |
| pointer to a tag `fun poke(p: *Reply)` (auto-deref `sel p.value`) | "unsupported parameter 1 of type `*?` in the SPIR-V Logical Shader environment: a logical reference must name a Function-storage object whose type SPIR-V can declare" | gap |
| `Type.case{payload}` construction | lowers to alloca + memzero + payload store + discriminator store; the alloca is refused as above | gap |
| `sel place.case` | lowers to a load of the declared discriminator scalar through the tag's address and a compare; refused at the alloca before the load is reached | gap |
| guarded payload read and write | lower to `gep [0, case]` and a load or store; refused before | gap |
| whole-value assignment | lowers to a whole-object copy (`memcpy`); refused before | gap |
| debug-profile discriminator check | lowers to `cbr(eq, cont, trap)` with `unreachable` in `trap` (`emit_case_check`); refused before | gap (realization settled in section 2.5) |
| comptime `sel` on a constant tag (`$if (sel K.value)`) | folded by lowering before any emitter question is asked; accepted. A runtime read of the constant's payload (`o = K.value` in the arm) is refused because `K` is plain module-scope storage: "a plain module-scope variable is not reachable from the SPIR-V target ... reaches a stage only by carrying an interface role" | accepted (the constant test), refused-by-policy (the storage read, general to every module-scope `val`, not a tag rule) |
| secret payload `tag SecretReply: u32 { failed: u32; value: ^u32; }` | refused at the alloca like any tag; the IR the emitter sees carries no secrecy qualifiers, and the target's secrecy policy is the existing one: secret scalars are carried, `#[oblivious]` bodies are refused (`mach.lang.driver:oblivious_spirv_refused`) | gap (carried once tags are, under the existing policy) |
| `-g` on a spirv target | "debug info was requested, but this target registers no debug model" (`passes.mach debug_info_of`) | refused-by-policy (v5 contract), unchanged; the debug *profile* (`opt = 0`, runtime checks) is a different axis and is accepted on spirv |
| environment ceiling (a `u8` discriminator under `env = "vulkan1.0"`) | refused at the alloca before the capability accounting runs | gap; after this lane the existing `check_environment` refusal applies (section 2.6) |

Corpus: `test/cases` carries no tag case on any target (the only tag fixture
under `test/` is the link leg's `3218-tag-abi-c-callee`), so there is no native
layer B golden for a tag to pair a spirv golden with. A new corpus case needs a
C reference translation and a golden on every column, which moves the native
control this lane is required to leave at 91/91, so the acceptance for L6 lives
in driver tests that build a module through the whole pipeline and run the
pinned `spirv-val` and `spirv-dis` on it, plus in-process assertions on the
module words that hold when the tools are absent.

## 2. The contract this lane lands

### 2.1 The SPIR-V type of a tag

A tag whose declared discriminator is `D` (one of `u8`, `u16`, `u32`, `u64`) and
whose cases are `c_0 .. c_{n-1}` is

```
OpTypeStruct %D %P_0 %P_1 ... %P_{n-1}
```

where `%D` is `OpTypeInt` of the declared width and `%P_c` is the SPIR-V type of
case `c`'s payload, or the unit composite (an `OpTypeStruct` with no members)
when the case has no payload. Member ordinals are constant: the discriminator is
member 0 and case `c`'s payload is member `1 + c`, so a `gep [0, c]` from
lowering is `OpAccessChain %ptr %tag %uint_0 ... %uint_{1+c}` with the same
index arithmetic as a record field, and `read_value`/`write_value` move the
whole object with one `OpLoad`/`OpStore` as section 6 of the N4 contract
requires. Member order is the IR order (discriminator first, then the cases).

Why per-case members and not the widest case as a word array: SPIR-V's logical
addressing has no reinterpretation of storage. A word array would need an
`OpBitcast` per scalar leaf, a compose or decompose for every 64-bit scalar and
every composite payload, and a per-leaf copy instead of a whole-object load or
store, which is exactly what section 6 forbids. Per-case members keep every
access an access chain with constant ordinals, keep `memzero` one
`OpConstantNull` store, keep copies whole, and let `spirv-val` type-check the
payload access against the case it names. The cost is that the logical
composite is wider than the native union; that is the module ABI being separate
from the physical carriers, which the roadmap row asks for. No `OpTypeUnion`
exists, so no representation could share the storage.

The empty `OpTypeStruct` for a payloadless case validates as a `Function`,
`Input` and `Output` storage member under every environment above
(`spirv-val` probe, this document's record), and it keeps the ordinal rule
uniform: a payloadless case is still a member, so ordinals never depend on
which cases carry payloads.

The type table interns a tag composite apart from a record with the same member
list: the emitter maps a tag's `gep` index to `1 + c` and resolves the
discriminator through member 0 by asking the SPIR-V type whether it is a tag,
so a record `rec { d: u32; p: u32; }` and a tag `tag T: u32 { a: u32; }` are two
`OpTypeStruct` ids even though their member lists agree (SPIR-V permits
duplicate structure types; the validator accepted the duplicate in the same
probe). `logically_match` treats the tag flag as part of the shape, so a copy
between a tag and its look-alike record is refused as a reinterpretation, which
is also the contract's rule for `::` casts that contain a tag.

The 16-member bound (`types.MAX_STRUCT_MEMBERS`) applies to the composite, so a
tag may have at most 15 cases on this target; more is a located refusal naming
the bound, not a silent `0`.

### 2.2 Zeroing, construction, replacement

Construction lowers to `memzero` of the slot, a payload store through
`gep [0, c]`, and a discriminator store. On this target that is
`OpStore %slot OpConstantNull %tag`, then an `OpStore` into member `1 + c`, then
an `OpStore` into member 0. Every inactive case member is the null of its type
after the sequence, which is the logical analogue of the contract's zeroed
inactive suffix and gap. A whole-value assignment is one `OpLoad`/`OpStore`
of the composite and carries every member, so replacement by a smaller case
leaves no stale payload behind: the source was constructed with its other
members null.

### 2.3 The discriminator: the leading-leaf rule

Lowering reads and writes the discriminator as a scalar of the declared width
through the tag's own address (`emit_load(base, disc)`, `emit_store(code, base)`),
which on a byte-addressed machine is offset zero. The emitter resolves a scalar
access through a pointer to a composite as an access to the composite's leading
leaf: member 0, recursively, until a scalar of the accessed width is reached.
For a tag that is member 0, the discriminator. The rule is stated for every
composite because it is the logical statement of what offset zero means, but no
other lowering path produces such an access today. A scalar access whose width
matches no leading leaf is refused as addressed memory the target cannot express.

### 2.4 Storage classes

- `Function` storage (locals, temporaries, owned homes of arguments and
  results): accepted.
- `Input`/`Output` interface variables of a tag type, and records or arrays
  holding one: accepted, as any composite interface variable is. The location
  span of a tag is the sum of its members' spans.
- `#[uniform]`/`#[storage]` blocks holding a tag anywhere in a member: refused,
  located at the block variable. A block is a host-visible layout, every case
  payload of a tag sits at one common host offset, and SPIR-V block members may
  not overlap, so no `Offset` decoration of the per-case members can describe
  the layout the host sees. This is a target limitation of the same class as
  the existing array-stride refusal in `laid_out_type`, and it is a refusal
  rather than a divergent layout because a block that silently disagrees with
  the host is worse than one that does not build.
- A pointer to a tag: `OpTypePointer Function %tag`, under the N4 reference
  rules (memory object declarations only, no pointer results).

### 2.5 The debug-profile check on a logical target

`emit_case_check` lowers each guarded payload access in the debug pipeline to a
discriminator load, an equality compare against the case code, and a
conditional branch whose failing edge is a block ending in `unreachable`. The
emitter renders that as `OpIEqual` feeding `OpSelectionMerge` +
`OpBranchConditional`, and the failing block is `OpLabel` + `OpUnreachable`.
That is a real check: the branch is taken on the discriminator value, and the
mismatch path is a block the module declares must never execute, which is what
a trap means to a consumer that has no trap instruction. `OpKill` is not used:
it exists only in fragment shaders and would discard silently rather than
signal. The release pipeline emits no compare, branch or dead block. The
structured-control-flow analysis already accepts a selection whose one arm
ends in `OpUnreachable` (the debug division checks in `float/*` goldens). The
policy refusal of `-g` is about debug *information* and is unchanged; the
debug *profile* is accepted on spirv, and the corpus's `o0` column runs it.

### 2.6 Environment refusals

Every scalar the tag composite declares goes through the same capability
accounting as any other scalar: an 8-bit discriminator needs `Int8`, a 16-bit
one `Int16`, a 64-bit one or payload `Int64`, an `f16` payload `Float16`, an
`f64` payload `Float64`. `check_environment` compares the module's needs
against the declared environment's ceiling and refuses with "this module needs
the `<cap>` capability, which the `<env>` environment does not guarantee; raise
`env` on the target or avoid the type that needs it". Under the declared
environments, `vulkan1.0` and `vulkan1.1` guarantee `Int16`, `Int64` and
`Float64` but not `Int8` or `Float16`, so the genuine refusal a tag reaches is
a `u8` discriminator (the common one) under `vulkan1.0`; a 64-bit payload is
inside every declared ceiling. The unqualified environment refuses nothing.

### 2.7 Comptime, secrecy

A comptime `sel` on a constant tag is decided by lowering and reaches the
emitter as a constant, so nothing changes. A `^` payload inside a tag is
carried: the emitter sees stripped IR types, the target's existing policy
carries secret scalars and refuses `#[oblivious]` bodies, and nothing in this
lane widens or narrows that.

## 3. What may not change (section 6 of the N4 contract, restated for tags)

- A tag argument, parameter or result is one `CLASS_VALUE` slot; it crosses
  `MIR_VALUE_CALL`/`MIR_VALUE_PARAM`/`MIR_RET` as its owned home, unchanged.
- `read_value`/`write_value` move a tag as one `OpLoad`/`OpStore`; no per-member
  copy anywhere.
- Member ordinals are constants: 0 is the discriminator, `1 + c` is case `c`.
- `pointer_shape`, `instruction_operands_fit`, `TargetDefs`, `Environment` and
  the environment profiles are untouched.

## 4. Acceptance record

Filled in below by this lane once the tests are green.
