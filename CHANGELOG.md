# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [4.14.0] - 2026-08-07

### Added

#### A field descriptor's `f.type` is accepted as a generic type argument (#2691)
A reflection walk could descend one syntactic level per hand-written `$each`, because there was no way to re-enter a generic at a field's type. `eq[f.type](...)` was a parse error, `eq(...)` resolved against the template instead of instantiating, and `$type_of(x)` yields a comptime type rather than a value. All three doors were shut, so a recursive derive had to be a hand-unrolled ladder with a depth cap.

`f.type` in a generic argument list now becomes a node carrying the **loop variable's name** rather than a resolved type, because which field it names is decided per iteration by the unroll. A walk is written once and reaches any depth:

```mach
fun eq[T](a: *T, b: *T) bool {
    $each f in $fields(T) {
        $if ($is_record(f.type)) {
            if (!eq[f.type](?a.[f], ?b.[f])) { ret false; }
        }
        $or { if (a.[f] != b.[f]) { ret false; } }
    }
    ret true;
}
```

**Termination is structural rather than bounded by a counter**, and this was verified against the compiler rather than argued from the spec: direct, mutual, and generic-growing-by-value self-containment are all refused as "recursive type has infinite size". A self-reference must go through a pointer, and `$is_record` does not select one. Note `rec Grow[T] { p: *Grow[*T]; }` is accepted and has unboundedly many instances reachable through its pointer -- the by-value walk never follows it, but termination stops being structural the moment `$pointee_of` lands, which is recorded as a precondition on #2693.

If the argument were ever wrong the failure is a diagnostic naming the derivation chain, not a hang: `MAX_CT_INSTANCE_DEPTH = 32` reports "a generic instantiates itself at a growing type argument".

#### A vector literal is built in one instruction where the ISA can (#2640)
A vector literal round-tripped through an array alloca -- four stores and a frame slot to materialize a constant -- which SPIR-V's logical addressing cannot express at all. `OP_VEC_BUILD` takes N lane values and produces the vector, lowering to `OpCompositeConstruct`.

The capability is read at lowering (`has_vec_build`) rather than legalized afterwards, so **no target ever sees an `OP_VEC_BUILD` it cannot select**. Native targets keep the storage path unchanged and their emitted assembly is byte-identical. A malformed build is refused at construction with no flags required, not only by the opt-in verifier.

#### Condition-flag effects are modeled in the asm effect model (#2460)
`#[oblivious]` inline asm refused every conditional branch on x86-64, because the model had no notion of what an instruction does to FLAGS. It now carries five facts per grammar row -- `writes_flags`, `defines_flags`, `opaque_flags`, `reads_flags`, `flags_stated` -- with defaults inverted so the security-relevant surface is 18 rows rather than 60.

A branch on flags derived from a secret is still refused; a branch on flags an instruction fully **defines** from public operands is not. The `defines_flags` distinction is load-bearing and subtle: `inc` and `dec` write ZF but **preserve** CF, so treating "writes flags" as "defines flags" would have accepted `cmp {secret}` / `inc` / `jc`.

**The facts are measured against the CPU, not inferred.** `int/surface/ct-flags-hardware` presets RFLAGS both ways and reads back RFLAGS and the destination register, and its golden is the measurement. Three rows (`popfq`, `iretq`, `syscall`) print `NOT MEASURABLE` -- `popfq` passes the probe as "defines both", which is correct on its own terms and wrong as a classification, because the value comes from the stack.

`setcc` laundering FLAGS taint into a general-purpose register is closed by `reads_flags`. General memory laundering is not, and is unchanged by this work: see #2706.

### Changed

#### mach-std advanced to 0.25.0 (#2710)
Brings `exec.resolve` / `exec.resolve_in`, the `std.derive` recursive tier, and riscv64 library coverage. The manifest already tracked `branch/main`; the lock had simply not been advanced.

## [4.13.0] - 2026-08-07

**Two source-compatibility changes.** A type may no longer be declared with a vector form's name, and the comptime shape predicates no longer see through `^`.

### Added

#### A reflection walk can re-enter itself at a field's type (#2691)

`$fields(f.type)` (#2454) let a walk descend into a record-typed field, but only **one syntactic level per `$each` someone wrote**. Closing the loop needs the walk to re-enter itself at the field's type, and the only spelling for that is a generic call — which did not parse, because a generic argument list reads with the type grammar where `f.type` is indistinguishable from a qualified `module.Type`:

```mach
fun eq[T](a: *T, b: *T) bool {
    $each f in $fields(T) {
        $if ($is_record(f.type)) {
            if (!eq[f.type](?a.[f], ?b.[f])) { ret false; }
        }
        $or { if (a.[f] != b.[f]) { ret false; } }
    }
    ret true;
}
```

Neither of the two alternatives worked either: there is no inference from the argument, so `eq(?a.[f], ?b.[f])` resolves against the template rather than instantiating, and `$type_of` is not a type operand. All three doors were locked, which is why `std.derive` shipped a hand-unrolled ladder with a documented depth cap.

#2454's one-token lookahead now extends to the generic argument list. `f.type` there is a `TYPE_KIND_FIELD_TYPE` node carrying the loop variable's name rather than a resolved type, because which field it names is decided per iteration by the unroll.

**Termination is structural**, so there is no depth limit and none is needed: each descent instantiates at a field's own type, a record's fields are finite, and a record cannot contain itself by value — the compiler already refuses that as a recursive type of infinite size, verified for the direct, mutual, and generic-growing forms. A self-reference must go through a pointer, which is a different type and which `$is_record` does not select.

Two implementation notes worth carrying:

- **A `f.type` node is never memoized.** `resolve_type_ref` caches a syntactic node's resolved type, which is sound only because every other spelling's meaning is fixed by its source text. This one is not: a single node serves every iteration, so caching the first answer gives the second field the first field's type. Pinned by a case with two record fields of different arity, which is a wrong count rather than a silent pass.
- **Both phases resolve it through one helper.** Sema resolves it during its own `$each` unroll; lowering resolves it during the unroll it runs for a walk whose `$fields` operand was an unbound generic parameter at template sema — which is the shape a generic derive actually has. `comptime.field_type_of_binding` is the single implementation, going through the same installed field resolver as the `f.type` value projection, so a descriptor read as a value and the same descriptor read as a type cannot land on different fields.

The lookahead does not absorb real mistakes: a genuinely wrong operand in the same position (`g[nosuchmod.T]`) still reports against the type grammar, a name that is not a `$each` loop variable is reported where it is written, and `type` stays contextual so an ordinary record field called `type` is untouched.



### Changed

#### `^` is stripped only where the question is about storage (#2692)

**This is a contract change, and the contract it changes was unsound.**

`comptime-intrinsics.md` documented `^` as stripped before a shape predicate answers, so `^Pair` was a record. `$fields(^Pair)` refuses, and the doc's own descent idiom pairs the two:

```mach
$if ($is_record(f.type)) {
    $each g in $fields(f.type) { ... }   # descend
} $or { ... }
```

The rule and its counterexample were nine lines apart in the same section. For a `^`-wrapped record field the gate answered yes and the intrinsic it gates then refused the same operand, so a walk written exactly as prescribed failed with `$fields requires a record type` — a message asserting the operand is not a record, immediately after a predicate said it was. No gate could prevent it, because no predicate distinguished `^Inner` from `Inner`.

`$is_record` / `$is_union` / `$is_pointer` now answer about the **outermost** constructor, and `^` is a constructor. All three answer false for `^T`, so a reflection walk refuses a secret rather than descending into secret storage. `$type_name` stops stripping too and spells `^Pair` — its stated purpose is to be the spelling diagnostics print, and a diagnostic prints `^Pair`, so stripping was a drift on the one qualifier where drift matters most.

That leaves one rule for the whole surface: **`^` is stripped only where the question is about storage.** `$size_of` / `$align_of` / `$offset_of` strip, because storage is exactly what they ask. Nothing else does.

Outermost means outermost, which is `type.is_secret` and not `type.carries_secret`: `^*u8` is a secret pointer and answers false, while `*^u8` is a public pointer to secret storage and is still a pointer. `Box[^u64]` is a record, because the instance is not itself secret — its field is, and the field is where a walk meets the question.

**Why this direction rather than making `$fields` accept `^Rec`.** Under that alternative a walk descends into a secret record and each leaf action meets the secrecy gates on its own: formatting is refused at the variadic boundary, comparison is refused as a branch on a secret, hashing is refused as a cast that drops `^`. All true — and a memberwise **copy** is accepted, correctly, since secret to secret is what the lattice permits. Safety would then be a property of which leaf actions happen to exist rather than of the contract, and the next walk whose leaf touches no gate descends into a secret with nothing to stop it. Refusing at the gate puts the guarantee in the contract.

Reasoning by analogy to #2297 (field resolution) and #2373 (cast width) is what produced the old rule. Those are storage questions — a secret occupies its base type's storage and has its base type's fields — and stripping is right for both. A predicate that gates a walk is not a storage question, and carrying the analogy across is the defect.

A walk still has to classify **positively** for this to refuse rather than skip: gate on the shapes you handle and refuse the fallthrough, which is what `std.derive` does and what the docs now prescribe. Because all three predicates answer false for `^T`, "nothing classifies it" is a usable secrecy signal on its own.

Blast radius: no user-level consumer in this repo. In mach-std every consumer is `std.derive`, where a `^`-wrapped record field now fails with derive's own written message instead of the raw intrinsic error.

#### A type may not be declared with a vector form's name (#2687)
`rec u8x16 { a: i64; }` used to compile clean and be silently unreachable: the builtin won every use, and the only symptom was a later error pointing at the wrong thing. `uni` and `def` behaved the same. It is now refused as a name collision.

The guard keys on the spelling rather than on the target's vector width, so `f32x3` and `f32x7` are reserved everywhere. Otherwise a name would be claimable on a 128-bit target and collide on a wider one from identical source.

**Values are unaffected.** A vector spelling is read in type position only, so `val f32x4` remains legal and is not shadowed by anything.

### Added

#### GLSL.std.450 extended instructions for shader stages (#2688)
A `#[spirv_op(set, name)]` decorator declares which SPIR-V instruction a function *is*, over two sets: `"core"` and `"GLSL.std.450"`. 33 extended instructions plus core `OpDot`, covering the common-value, trigonometric, exponential, range/interpolation and geometry families. The extended set is imported only when used.

The compiler names no library: it knows how to read the decorator and emit the instruction, nothing more. `briar-systems/mach-shader` provides `shader.math` over these, and anyone may declare their own.

Deliberately shader-only. A decorated function may carry a body, and that body runs on every non-spirv target; a bodiless one called from a CPU build fails at link naming the symbol. A native fallback would make `sh.sqrt` and `math.sqrt_f32` the same function on a CPU and a different one on a GPU, which is the only outcome that never errors.

#### A vector is read as the form `TxN`, bounded by the target's vector width (#2687, partial)
Vector types were a table of ten hard-coded spellings. They are now read as `<element>x<lanes>` and bounded by a new `MachineModel.vector_bits`, which is 128 on every current target.

The bound is a width rather than the `has_v128` capability: rv64gc and mos6502 declare no vector unit and still admit every 128-bit spelling today through scalarization, so gating on the capability would refuse names that compile correctly, and a boolean cannot express a wider future target at all.

A vector's extent is now lane-derived rather than a flat 16 bytes. Size is `lane_size * lanes`. Alignment is the full size when the vector fills the vector register and one lane otherwise -- a vector that fills the register aligns to its whole size because the machine's vector load requires it, and a narrower one is a packed aggregate. All ten former spellings are exactly 128 bits and are unchanged in both size and alignment.

**`vec3` is not delivered yet.** A vector narrower than the vector register is refused by name, because codegen cannot carry one: `op_width_of` answers a single number for both a value's register width and its memory footprint, which is invisible while every vector is 16 bytes and truncating once they are not (#2697). The form, the bound, the layout rule and the documentation are in, and lifting the restriction is deleting one check.

### Fixed

#### A darwin link failed when load commands exceeded one page (#2690)
The Mach-O header reservation was hardcoded at one page, so an image whose load commands exceeded 4096 bytes was refused outright with `macho: dyn-exec header exceeds the one-page reservation`. Nothing in the format caps them there -- `sizeofcmds` is a `uint32`.

The reservation is now computed from the actual load commands and rounded to the target's page size. It grows **inside** `__TEXT`, so `__TEXT` still maps at the image base and `__PAGEZERO` is unchanged; widening the gap below the first segment instead would have slid both down, which is #2599.

Reachable by any objective-c consumer rather than only unusually large images: #2606 stopped coalescing input sections, and every surviving section costs an 80-byte `section_64`. Sixty `#[section]` globals cross the cap on section count alone.

#### SPIR-V access chains, local allocations, and a dead shuffle constant (#2649)
A GEP's index list now survives to a logical-addressing target as member ordinals rather than being folded into a byte displacement, so a struct member read lowers to `OpAccessChain` instead of being unrepresentable. Local allocations and a dead shuffle constant were fixed alongside it.


## [4.12.0] - 2026-08-07

**Contains a critical correctness fix. Anyone on 4.11.0 or earlier should upgrade.**

### Fixed

#### A bit reinterpret confused values with addresses, in both directions (#2670)
`OP_BITCAST` did not distinguish an operand that *is* a value from one carried by its storage address. An aggregate's rvalue is its address; a scalar's or vector's is the value itself. So `:~` across that divide changed the type tag without changing the representation:

- **`a:~i64` over a `[8]u8` evaluated to the array's own stack address.** It does not fault. It is a silent wrong answer that also places a stack address into ordinary integer data, readable and printable with no indication anything is wrong.
- **`x:~[8]u8` produced an aggregate-typed value from a register**, which the store then dereferenced as a pointer. It faults only when the operand's bytes do not happen to form a mapped address, so the observable was "sometimes SIGSEGV, sometimes silent corruption".

Both directions, every target, both profiles. **This predates the 4.9/4.10/4.11 series** — it reproduces on 4.7.1 — so there is no regression window to bisect.

`lower_reinterpret` now materializes the crossing: a **load** when the source is an aggregate, an **alloca plus store** when the destination is. Within one class nothing is materialized, because aggregate-to-aggregate is a pointer retag and value-to-value is the genuine reinterpret the opcode means. The fix is at the producer, so the bad shape never enters the IR.

A new always-on IR verifier invariant, `VC_BITCAST_CLASS`, refuses a bitcast whose operand and result sit on opposite sides of the divide. Always on rather than behind the `repr` gate, because this shape is individually coherent at both ends and wrong only in combination — what review misses and only an invariant catches.

The documentation asserted the property that caused the bug: `mir.mach` said a `:~` was a plain `MIR_BITCAST` "in every direction", reasoning entirely about register banks, with no notion of an address-carried side. Corrected (#2679).

- **The `:~` size check read every `rec` and `uni` as 0 bytes** (#2672), so no record reinterpret compiled at all while `$size_of` reported the correct size. That is why records were absent from #2670's evidence: unreachable rather than working. `::` was affected identically through the same call. Sequenced deliberately after #2670, since fixing it alone would have enabled records straight into the miscompile.
- **An inline-asm `ldr`/`str` offset outside both aarch64 immediate forms clobbered IP0** (#2667), silently destroying a live value in an author's own asm body. A compiler-emitted access may borrow IP0 because it is reserved from allocation; an author-written one may not. Frame-size dependent, so invisible in small tests.
- **Inline-asm `{name}` slots are now laid out nearest the frame pointer** (#2674). They are the one slot kind that cannot be legalized when the offset is out of range, and the reach is smaller than it looks: slots are below the frame pointer, aarch64's scaled `imm12` is unsigned-only, so the whole reach is the unscaled `imm9`'s -256. Thirty-three ordinary locals ahead of an asm block was enough.
- **128-bit vectors reached codegen under `simd = "scalarize"`** (#2654), so nothing using `std` built for riscv64. The scalarizer had no case for a vector-to-vector `MIR_BITCAST`, which the signed-select idiom in `std` produces. mach-std's full suite now runs on riscv64, having never executed there.
- **A manifest `symbols = [...]` claim keyed on the source spelling** while `#[library]` keyed on the link name (#2636), so on Mach-O the two occupied disjoint key spaces and a claim attributed nothing.
- **A target-conditional `#[library]` bound to the arm the target does not take** (#2639).
- **`#[library]` attributions were projected from every loaded module** (#2657) rather than the compiled set, which #2539 had made a live problem rather than a latent one.

### Changed
- **Float loads and stores no longer materialize the address unconditionally** (#2655), gated on a new `MachineModel.flat_addressing`. Native output byte-identical across three targets and both profiles.
- **A structural GEP keeps its index list on a logical-addressing target** (#2649, enabling half), **`MirSlot` carries its allocated type** (#2673), and **`memzero` survives as a whole-object operation** (#2669). Three erasures where MIR discarded structure for a machine reason a logical-addressing target cannot reconstruct. Each proven not to change machine-target output: the entire compiler binary is byte-identical across two of them.
- **The backend's unsupported-instruction refusal names the opcode, the function and a position** (#2656), the last instance of the complaint #2538 fixed for the IR verifier.

### Added
- **spirv**: cross-wired refusals corrected and `OpTypeArray` with `ArrayStride` (#2659), **storage buffers via `#[storage(set, binding)]`** (#2665). A uniform block is std140-shaped and a storage buffer std430-shaped, so the array shapes a uniform must refuse are exactly the ones a storage buffer accepts. Refused rather than repacked, because every offset comes from the compiler's own layout so a host writing by `$offset_of` sees what the shader reads.
- **A vector is allocated at its own type** (#2640, first stage), which also fixed a real **under-alignment on every target**: `types.md` declares a vector's `$align_of` to be 16 and an `[4]f32` slot is 4-byte aligned.

## [4.11.0] - 2026-08-07

Closes out the import-attribution work, makes `mach build` and `mach test` agree about whether a project compiles, and stops the linker emitting debug info it has invalidated.

### Fixed
- **`mach build` silently skipped modules unreachable from an artifact entry** (#2539), so a module under `src` that nothing imported was never checked and a broken one stayed green indefinitely. `mach test` roots its load at the whole source tree and did check it, so the two commands disagreed about whether the project compiled, which is the part that erodes trust in a green build. Resolve and sema now walk every module under `src`; lower, codegen, emit and link still walk only the reachable prefix, so an unreferenced module costs no object code and does not enter the artifact. The cost falls on the front end, which is the cheap half.
- **A `[step]` writing into the module object tree silently clobbered compiled modules** (#2532). `{project.out}/obj/<project.id>/` is where a project's own modules land, and a vendored library whose sources share those names overwrote them, or was overwritten, with the link proceeding on whatever survived. It produced a runtime failure that looked exactly like a miscompilation. Declared step outputs are now validated at manifest load, and undeclared writes into the tree are caught after the step runs.
- **A foreign Mach-O object's DWARF was merged without remapping its unrelocated cross-section offsets** (#2540). The damage was worse than shifted offsets: `.debug_abbrev` is deduplicated across contributors, which is correct where every module's table is byte-identical, but for a foreign unit it substitutes a **different producer's table**. The linker now leaves out the debug sections of a module that does not reference its own DWARF through relocations, keyed on that property rather than on the format, so a foreign ELF object still merges on its own evidence. Two hand-built fixtures turned out not to be representative of a real mach object and were corrected rather than the rule being loosened around them.
- **A target-conditional `#[library]` attribution bound to the arm the target does not take** (#2639). Attributions were projected before any arm was selected, so the last arm walked won. Loud when the losing arm's library is absent, and silent when both are present, where the import is simply attributed to the wrong library.
- **A manifest `symbols = [...]` claim keyed on the source spelling while `#[library]` keyed on the link name** (#2636), so on Mach-O the two occupied disjoint key spaces: a claim attributed nothing, the link failed with the very error the claim exists to prevent, and the claim-vs-decorator conflict check could not fire at all. Found by the darwin release gate.

### Added
- **spirv: interface variables and descriptors** (#2571, partial) — `Input`/`Output`/`Uniform` storage classes with `Location`, `BuiltIn`, `DescriptorSet` and `Binding` decorations, validator-clean. A vertex stage reading a `Location` input and writing `BuiltIn Position` works end to end. Reading a **member** of a uniform block is refused: MIR folds an aggregate walk into a byte displacement and `OpAccessChain` needs the ordinal, which an offset does not determine because unions overlap and nested aggregates share offsets. Filed as #2649, and #2571 stays open on it.

### Changed
- mach-std advanced to **0.24.2** (#2651, #2652). `O_DIRECTORY` had been given `O_DIRECT`'s value on riscv64, so `fs.read_dir` could not open a directory there at all, which is what blocked #2539: front-ending every module makes the compiler enumerate a source tree, and the riscv64 self-host case is the only place a riscv64-hosted compiler runs. The first correction over-generalised and broke aarch64, which swaps the two flags relative to asm-generic; **this repo's native aarch64 leg caught that immediately**, and 0.24.2 restores it.

## [4.10.0] - 2026-08-07

Repairs two defects shipped in 4.9.0, one of them a regression, and lands a large `Result` codegen win, declared clobbers for raw inline-asm encodings, and the first three layers of the SPIR-V backend.

### Fixed

#### Shipped in 4.9.0
- **A target-conditional `#[library]` attribution was rejected outright** (#2630, regressing #2529). The conflict check added in 4.9.0 treated the two arms of a declaration-scope `$if` chain as contradicting each other, so the idiomatic way to write a cross-platform `ext` binding stopped building where 4.7.1 accepted it. Each declaration now carries the comptime-arm path it sits under, and a conflict is reported only between declarations no `$if` chain separates. The rule is decided from the source alone rather than by evaluating conditions: this pass runs before sema, and evaluating here front-runs sema's own diagnostics for a `$size_of` in a declaration-scope condition. A separate pre-existing defect found while fixing it, that such an attribution binds to the arm the target does **not** take, is filed as #2639 and deliberately untouched.
- **The Mach-O section type was dropped across the link** (#2643, completing #2606). 4.9.0 made input section names survive, but an emitted `__DATA,__mod_init_func` carried `S_REGULAR` where its input carried `S_MOD_INIT_FUNC_POINTERS`. dyld finds an image's initializers by scanning for that **type** and never consults the name, so a module initializer was never going to run and nothing reported it. `of.SEC_FLAG_INIT_FUNCS` now carries the fact from parser to writer, stated about the content rather than about Mach-O's encoding of it, and section properties are part of a group's identity so a typed and an untyped same-named section can never share one output section and one flags word.

  The darwin leg's SIGSEGV on `macho-named-sections` was **not** this defect. That fixture's `__objc_classlist` entries were markers rather than addresses, unmapped inside `__PAGEZERO`, and an image declaring `__objc_imageinfo` has every class-list entry dereferenced by libobjc's `map_images` walk, so `ld64` would emit the same crashing binary from the same input. That case is structural only now, and a new `surface/macho-mod-init` links a genuine initializer and asserts the section type on the **linux** leg, so this class of defect no longer needs a darwin runner to be seen at all.

#### Debug info and linking
- **A foreign Mach-O object's DWARF was merged without remapping its unrelocated cross-section offsets** (#2540), producing an executable `llvm-dwarfdump --verify` rejects. The damage was worse than shifted offsets: `.debug_abbrev` is deduplicated across contributors, correct for mach where every module's table is byte-identical, but for a foreign unit it substitutes a **different producer's table**, which is what `invalid abbreviation 115, valid abbreviations are 1-15` was. No offset arithmetic repairs that. The linker now leaves out the debug sections of a module that does not reference its own DWARF through relocations, keyed on that property rather than on the format, so a foreign ELF object still merges on its own evidence. It warns rather than dropping silently.
- **Nothing checked that a DWARF code-address addend stays inside its anchor function** (#2620). #2583's soundness argument rested on an invariant the linker never stated or checked; it is enforced now rather than assumed.
- **An import whose loader name is an import-address alias** is refused (#2527), and **the unattributed-import diagnostic names both attribution routes** (#2528). Both landed after the 4.9.0 branch was cut.

### Added
- **codegen: a `Result` no longer round-trips through the stack** (#2315). SROA never saw `Result` at all: `splittable_at` requires the whole type tree to reduce to scalars, `Result[T, E]` lowers to `{i8, uni{T, E}}`, and `agg_field_count` returns 0 for a union by design, so the candidate was refused at the type test before escape analysis ever ran. A union field is now **one** slot rather than one per member, so every variant still aliases byte for byte and the reinterpretation a union exists to express is preserved exactly. Re-admitting the *shape* is not re-admitting the *layout*, which is what keeps #2239 fixed.

  `result_chain` goes from **19.52x to 2.50x** against `cc -O3` (754 us to 97 us), its kernel from 93 instructions to 61 against C's 19. Whole-suite retired instructions fall 13.5%, and a per-function diff shows only three functions changed anywhere in the benchmark, so the whole gain is this shape. It is **not at parity**: what remains is a boolean materialized and then tested, and the tag compared a second time inside an inlined panic guard. Neither is aggregate traffic. `rec_particle` does not share the cause, settled by its emitted code being byte-identical across the change rather than by argument.
- **asm: a raw encoding can declare the registers it writes** (#2594), as a `::` clause on a data directive: `.byte 0xee :: writes()`, `.word 0xd4000002 :: writes(x0, x1, x2, x3)`. A failed parse degrades the clobber mask to the whole register file, so the escape hatch that makes the surface complete is also what defeats the allocator. The design is **monotone**: a clause may only follow a data directive, and a modeled instruction's effects still come from its mnemonic row and cannot be overridden, so a declaration can only ever narrow the maximally conservative default at exactly the statements where the compiler had no information. It can never contradict a fact the compiler derived, and a wrong one's blast radius is the single statement carrying it. An absent clause keeps today's behaviour byte for byte.
- **spirv: scalar float arithmetic, comparisons and conversions** (#2568), **vector types, lane-wise arithmetic and lane access** (#2569, partial), and **`OpEntryPoint` with execution models via `#[stage(...)]`** (#2570). #2569 stays open on purpose: a vector literal lowers through an array `alloca` read back as a vector through its pointer, and SPIR-V's logical addressing model gives a pointer exactly one pointee type with no pointer bitcast, so no SPIR-V back end can express it. Filed as #2640, whose fix also removes a stack round-trip for constant vector literals on every native target.
- **link: `#[embed]` content is deduplicated across modules in one artifact** (#2541), completing #2507's acceptance. Ordinary globals are untouched, because their distinct addresses are observable.

### Tests and CI
- **int: a floating dep resolved once per case rather than once per run** (#2619), so a suite spanning a mach-std merge could compile some cases against one standard library and some against another while reporting one verdict. Landed after the 4.9.0 branch was cut.
- Four `#[library]` attribution tests asserted a symbol's source spelling, which is the link name only on a format with no symbol prefix. They passed on linux and windows and failed on the darwin release gate, twice, in two separate changes. `ut_names_import` and `ut_import_attribution` now state that rule once so the next test in this area cannot repeat it. The divergence they exposed, between the decorator and manifest-claim attribution keys, is real and is filed as #2636.

## [4.9.0] - 2026-08-07

Three silent-wrong-answer defects, a class of debug info that standard tools rejected, and a run of work on the checks themselves. The recurring theme is coverage that reported more than it verified: several fixes here are for defects that shipped green, and the corresponding tests were the harder half of each change.

### Fixed

#### Silent wrong answers
- **abi: Apple arm64 gave every stack argument an 8-byte slot** (#2598). Apple's arm64 ABI gives each stack-passed **fixed** argument its natural size and alignment; the AAPCS64 base standard rounds every one up to 8 bytes. Mach applied the AAPCS64 rule on every aarch64 target including darwin, so a `darwin-aarch64` call past eight GP arguments with a narrow tail wrote each argument where the callee does not read it. No link error, no crash, just a different argument's bytes. The rule is now resolved from the `(os, isa)` pair by `target.select`, the same seam #2575 established, and the **variadic** tail correctly keeps its 8-byte minimum on both targets, because Apple genuinely has two different rules. The stack walk was 8-byte granular in three places (slot alignment, slot footprint, and the memory access width) and all three had to move together: leaving the width alone would have had a machine-word store overwrite the argument packed against it.
- **link: an import named `__imp_X` was emitted into the PE import table** (#2527). `__imp_` is PE's spelling for the address cell the import table itself synthesizes, so no library exports a symbol under that name. The image linked clean, passed CI, and failed only when Windows loaded it. Refused now, against the format's own prefix so it stays inert on ELF and Mach-O.
- **link: duplicate `#[library]` attributions resolved last-writer-wins** (#2529). Two modules declaring the same `ext fun` under different libraries were settled silently by module order. #2510 had made the claim-vs-claim and claim-vs-decorator cases hard errors and its docs already described "claimed only once" as a global invariant, so the prose asserted a symmetry the code did not have. Decorator-vs-decorator is now an error naming both declarations, and the adjacent unreachable silent-drop branch is an internal error.

#### Debug info
- **debuginfo: `addr2line` rejected the line table** (#2582). DWARF 5 gives the line state machine's `file` register an initial value of 1, which a consumer materializes before any `DW_LNS_set_file`. The encoder interned files from slot 0, so a single-file compile unit declared no slot 1 and binutils rejected the section with `mangled line number section (bad file number)` on every CU. Slot 0 now holds the CU's primary source file, as gcc and clang both emit. **`llvm-dwarfdump --verify` passed the whole time**, which is the more important half: `int/surface/debuginfo` now also runs `addr2line` and takes its stderr as part of the observable, so a structural check can no longer stand in for a consumer.
- **link: DWARF addresses into a discarded weak body were never tombstoned** (#2583). Tombstoning tested `r.addend == 0`, but DWARF names a body's interior as `anchor + inner_offset`, so every inlined-scope range, line row, and location entry reaching a discarded duplicate carried a nonzero addend and resolved against whichever copy survived. On a release `-g` self-build that was 4220 references, 709 of them wrong and 149 resolving past the end of the winning body. The test is now on the effective location, and the sentinel survives addend application. Tombstone count on the same build went from 4353 to 8573, a delta of exactly the 4220 measured.

#### Object formats
- **link(macho): non-PIE darwin x86_64 images were malformed** (#2599). `__PAGEZERO` was a page short of 4 GiB, `__TEXT` was based a page below `DARWIN_BASE_ADDR`, and no load segment carried any section commands at all, so `llvm-objdump -d` produced nothing for the image. The header-page reservation is a property of the object format rather than of position independence, and is now declared as one (`of.header_in_first_segment`); both writers share one framing routine and the two layouts differ only in the entry load command.
- **link(macho): input section names were discarded** (#2606). The merge collapsed loadable sections by kind and dropped their names, so anything a runtime finds **by name** did not exist in the output even though its bytes were mapped. Objective-C was the immediate casualty: libobjc scans `__DATA,__objc_classlist` and `__DATA,__objc_imageinfo`, mach emitted neither, and a statically vendored ObjC library aborted on its first class use. Merging is now grouped by `(kind, name)`, so same-named inputs come out as one contiguous correctly aligned run rather than several sections sharing a name. Canonical names round-trip, so plain code and data still merge and still come out spelled `__text` / `__data` / `__const`. `section_64.align` was written as 0 for every section and now carries the run's real alignment.
- **link: `R_X86_64_REX_GOTPCRELX` (42) and `R_X86_64_GOTPCRELX` (41) were rejected** (#2534). `-fPIC` is the distribution default, so any project vendoring C hit `elf: unsupported relocation type 42`. Both are the *relaxable* spellings of GOTPCREL and carry its arithmetic unchanged, so both are honoured as written. Relaxation is permitted by the psABI, never required, and rewriting the instruction at the patch site is silent when wrong for the sake of one elided load per site.
- **coff: `SizeOfCode` omitted the `.itext` import-stub section** (#2588) while `SizeOfInitializedData` counted every synthesized section. All three content aggregates are now summed off the emitted section table, keyed on each header's own characteristics, so there is no second rule to drift from.

#### Codegen
- **codegen: a record- or f64-returning function ending in a bare `for { }` failed IR verification** (#2523). Lowering closed the fall-through edge with a scalar zero stamped with the function's return type, which is accidentally well-typed for an integer or pointer and ill-typed for an aggregate or float. Two independent defects produced the one symptom: a provably unreachable edge is now left to `unreachable`, and a genuinely reachable fall-through goes through the return type's own zero-init path.

### Added
- **inline asm: aarch64 `hvc`, `smc`, `wfi`, `wfe`** (#2585), and **x86-64 `cld`, `pushfq`, `popfq`, `iretq`, `swapgs`, register and memory operand `jmp`, and `mov` to and from a control register and from a segment register** (#2590). `hvc` and `smc` declare the SMC Calling Convention's x0-x17 rather than a procedure call's caller-saved set, so a PSCI conduit is genuinely cheaper than a `bl`. A control register is not a member of any file the compiler models, so it rides the instruction as the number it encodes rather than as a register id, which is what keeps `mov cr3, rax` from claiming a write it does not perform.
- **inline asm: explicit `byte` / `word` / `dword` / `qword` on a memory operand** (#2595), with a prefix that contradicts the instruction refused rather than ignored. `movzx eax, word [rcx]` no longer needs a wide load and a mask.
- **legalize(mos6502): the six wide comparison opcodes** (#2495), as two algorithms. Equality is lane-wise and order-independent; an ordered relation is a lexicographic walk run from the least significant lane up, which is what makes it branchless in a pass that cannot emit control flow. Signedness touches the top lane alone and `<=` the bottom lane alone, so both surcharges are constant rather than per-lane.
- **build: `--bin X` settles the target when the artifact declares exactly one** (#2604). The second flag carried no information: the manifest had already said it. Several declared targets keep host resolution; several with no host match are refused naming what the artifact does declare.

### Changed
- **diag: IR verifier failures now carry a location** (#2538). They named the check that fired and nothing else, which made a middle-end error close to undiagnosable: localizing #2523 in a 900-line module took a scripted bisection of about forty compiles. The issue expected per-instruction locations to require the debug-info foundation, but that plumbing had landed in the meantime, so this is the full fix rather than the proposed interim. A check that genuinely has no instruction to point at now says so explicitly instead of reading as though it forgot.
- **diag: the unattributed-import error named the one remedy that cannot work** (#2528). A symbol from a static archive member has no declaration to annotate with `#[library]`, which is why #2510 added the manifest `symbols = [...]` key. Both routes are named now, with the symbol's own name spelled into the manifest form.
- **docs: `mach help build` advertised `[bin.<name>]` / `[lib.<name>]`** (#2600), tables the manifest parser rejects. It was the last place in the shipped binary naming the V1 schema, and it read as corroboration for it. The same spelling had propagated into source comments and `doc/language/comptime.md`.
- **docs: the secrecy page named `&T` as address-of** (#2601). Mach spells address-of `?` and dereference `@`; `&` is bitwise-and only.
- mach-std advanced to **0.24.0** (#2626), which settles the OS-layer filesystem contracts: windows `rename` now replaces an existing destination, and `getcwd` honours the caller's capacity on darwin and reports one contract across all three backends.

### Tests and CI
This release changed the checks about as much as the compiler, because several defects above shipped green.

- **int: an executed case reported crash, dyld rejection, and wrong-value exit identically, and discarded the program's output** (#2593). In #2586 the third case was what happened, the message said "execution failed", and the investigation went looking for a loader defect while the program had printed the answer on stdout and the harness threw it away. All six producers that run a built binary now report the exit status, distinguishing signal death from a deliberate nonzero return, and the captured output.
- **int: no fixture pinned mach-std** (#2592), so every case resolved `branch/main` fresh and an integration case, including the darwin release gate, could change verdict with zero changes in this repo. This produced a false lead during the v4.7.1 cut. Runs now record the resolved commit in `int/out/deps.txt` as an artifact, and `--deps pin` resolves against this repo's own `mach.lock` for release-gate runs, so a tag is cut against a known dependency.
- **int: a floating dep resolved once per case, not once per run** (#2619), so a suite spanning a mach-std merge compiled some cases against one standard library and some against another while reporting a single verdict. The first case resolves fresh and every case after it is handed that commit. The harness still never maps a ref to a commit itself, it reuses what `mach dep pull` wrote.
- Two `#[library]` attribution tests asserted the source spelling of a symbol, which is only the link name on formats without a symbol prefix. They passed on linux and windows and failed on the darwin legs. They now leave the prefix out of the match. The divergence they exposed between the decorator and manifest-claim attribution keys is real and is filed as #2636.

## [4.8.0] - 2026-08-07

Adds C-variadic `ext fun`, so a C function taking `...` can be declared and
called correctly on every supported target — including Apple arm64, whose
variadic tail goes on the stack rather than in registers. Also repairs build
steps on Windows, where they had never worked at all.

### Added
- lang: an `ext fun` may declare a C-variadic tail with a trailing `...`, and a
  call through it lowers per target. Apple arm64 is the case that forces the
  feature to be a real ABI decision rather than a parser change: AAPCS64 passes
  named arguments in registers, but Darwin's variant places every variadic
  argument on the stack, so a fixed-arity declaration of `printf` is silently
  wrong there in a way that no diagnostic catches. The rule is pinned to the
  (os, isa) pair rather than to either alone (#2575, #2584).

### Fixed
- driver: build steps run on a native Windows host. mach prefixed each step
  command with `cd /d "<root>" && ` and passed the result as a single argv
  element, which the argv joiner escapes with the CRT convention — an embedded
  `"` becomes `\"`, which `cmd.exe` does not implement. `cmd` stripped the outer
  quote pair, took the rest literally, and `cd` reported `Path not found.` The
  prefix was unconditional, so **every** step failed, and a project root
  containing a space is the common case on Windows. The root now leaves the
  command string entirely: `exec.run_shell` (mach-std 0.23.0) resolves the
  interpreter from `%ComSpec%`, applies the working directory in the child, and
  owns the per-platform quoting, so the two conventions have nowhere left to
  disagree. The POSIX path's matching exposure to a root containing `'` goes
  with it (#2578, #2587, #2602).
- int: the harness drops the step stamp cache before each case build. mach skips
  a step whose stamp matches and whose outputs exist, neither of which depends
  on the compiler, and the harness cleared only `out/int` — so a stamp from an
  earlier run made later runs skip the step engine outright, and a case
  asserting on step output kept passing against a compiler that could not run a
  step at all. A fresh CI checkout has no stamps, so only local iteration saw
  it, in the direction that reports green (#2602).
- int: the x86-64 GOT fixture reads its local GOT slot through both loads, so
  the case asserts the signature it was written to assert (#2586, #2596).

### Verification
- The Windows `[step]` fixtures run on `windows-latest`, **native**, on every
  PR, and were confirmed by name in the job log rather than inferred from a
  green leg. Both the Windows and POSIX cases were demonstrated failing against
  unfixed code before being accepted.
- The C-variadic lowering was checked against clang's own output on all four
  conventions, and against a C object built by the runner's own `cc` on both
  macOS legs.

### Known issues
- Non-PIE darwin x86_64 images are malformed — `__PAGEZERO` is one page short of
  4 GiB, `__TEXT` is based one page below `DARWIN_BASE_ADDR`, and the load
  segments carry no section commands. The `--pie` path, which arm64 darwin and
  every `--pie` x86-64 build use, is unaffected (#2599).

## [4.7.1] - 2026-08-07

A correctness release for the toolchain itself. Debug information no longer
changes the code that is generated, the CI seed resolves from a published
release rather than whatever the client considers newest, and the Windows
resource case that had never built now executes on real Windows.

### Fixed
- link: relocations sourced from debug sections no longer feed the atom
  liveness index, so a DWARF reference reaching across a duplicate weak
  function body can no longer keep that body alive. Building with and without
  `-g` now emits identical `PT_LOAD` segments (#2572, #2577).
- infra: the CI seed resolves through the latest published release, so a draft
  or partially uploaded release can no longer become the seed for every lane.
  Seed resolution lives in one composite action instead of nine call sites
  (#2573, #2574).
- int: `surface/pe-resources-native` declares the required `need` key. The case
  had never built, so PE resource emission shipped in 4.7.0 with no executing
  coverage. It now runs on the Windows leg (#2579, #2580).

### Known issues
- `[step]` build steps cannot execute on a native Windows host. Program
  resolution and `cmd.exe` argument quoting are both wrong (#2578, #2587).
- the darwin x86_64 integration case for GOT indirection fails, because the
  case's own hand-written assembly dereferences a non-relaxable GOT slot once
  where two loads are required. The linker's GOT emission is correct at every
  relocation form and its output is byte-identical once the case is corrected,
  so this is a test defect rather than a compiler one. It does mean the Mach-O
  x64 GOT support added in 4.7.0 has not yet been executed on darwin (#2586).

## [4.7.0] - 2026-08-06

Adds compile-time file embedding, Windows executable resources, and PE subsystem
selection, and makes linking foreign C objects correct on every object format —
the static vendored-C path a real application needs. The foreign-object
relocation core was re-keyed after mis-relocating section symbols, and the
Windows target now reads import libraries, the MinGW static CRT, dllimport
indirection, and x64 unwind tables; Mach-O links real clang output end to end.

### Added
- lang: `#[embed("path")]` embeds a file as a typed byte array at compile time,
  with no runtime I/O (#2507, #2518).
- link: PE executables carry `.rsrc` resources — icon, version info, and an
  application manifest — validated and fingerprinted for warm relinking
  (#2509, #2561).
- link: PE subsystem selection for Windows executables (#2508, #2519).
- link: symbol→DLL import mapping is pinnable per static archive on PE
  (`symbols = [...]`), attributing each import to its loader library (#2514).
- link: COFF short import records are read from import libraries, so archives
  derived from real Windows import libraries link without hand-written symbol
  lists (#2525, #2542).
- link: PE dllimport indirection — `__imp_X` references address X's loader-
  filled slot directly (#2526, #2549); duplicate `__imp_X` references to an
  in-image X share one local pointer cell (#2552, #2555).
- driver: dependency step outputs root at the dependency's own output directory
  instead of clobbering module objects (#2558, #2560).

### Fixed
- link: foreign C objects no longer mis-relocate — relocations key on symbol
  index rather than symbol name, repairing section-symbol resolution across
  ELF, COFF, and Mach-O alike (#2520, #2537).
- link: the MinGW static CRT archives link as vendored Windows CRT (#2530,
  #2551); `mach test` satisfies its generated entrypoint's `main` before
  archive selection, so an unrelated CRT startup member stays unselected
  (#2562, #2564).
- link: foreign x64 unwind tables (`.pdata`/`.xdata`) publish into PE output
  (#2535, #2545), and COFF section-relative debug relocations apply (#2553,
  #2554).
- link: BSD ar extended member names decode (#2543, #2544).
- macho: foreign clang objects link on Mach-O (#2521, #2533) with x86_64
  SIGNED_1/2/4 immediate-store relocations parsed (#2546, #2550), x64
  GOT/GOT_LOAD relocations resolved through synthesized, dyld-bound slots
  (#2557, #2559), and absolute 64-bit references to imports bound in place by
  dyld — the ObjC class-metadata shape (#2563, #2565).
- elf: the object reader admits x86_64's SHT_X86_64_UNWIND `.eh_frame` (#2531).
- codegen: weak-function DWARF entries retain their winning addresses (#2384,
  #2556).

## [4.6.0] - 2026-08-05

Opens inline assembly to the privileged and system instruction families on every
target, adds comptime type reflection, and validates inline assembly inside
`#[oblivious]` rather than refusing it. Also builds every artifact a manifest
declares for the selected target, and legalizes the remaining wide integer
operations on mos6502.

### Added
- asm: the privileged and system instruction families are reachable from inline
  assembly on every target — x86-64 port i/o (`in`/`out`), `cli`/`sti`/`lidt`,
  and `rdtsc`/`rdmsr`/`wrmsr` (#2468); AArch64 `mrs`/`msr`, by register name or
  by `s<op0>_<op1>_c<crn>_c<crm>_<op2>` encoding (#2480); and the RISC-V CSR
  family — `csrrw`/`csrrs`/`csrrc`, their immediate forms, and the
  `csrr`/`csrw`/`rdtime` pseudo-forms, by CSR name or numeric address (#2481).
  A name table can never be complete, so every target accepts the numeric
  escape alongside its names.
- asm: raw-encoding data directives (`.byte`, and the half/word/quad forms as
  each target spells them) are available on every target, so an encoding the
  assembler does not know can still be emitted inline (#2479).
- comptime: type predicates (`$is_record`, `$is_union`, `$is_pointer`, and
  siblings) and `$type_name`, taking a field-descriptor type operand so a
  `$fields` walk can branch on each field's type (#2128, #2454).
- ct: inline assembly inside `#[oblivious]` is validated rather than refused. A
  taint walk over the parsed assembly rejects a secret reaching a branch, a
  secret reaching an address, and a variable-latency instruction on a secret,
  and fails closed on any construct it cannot model (#2230).
- sema: a type-dependent comptime directive is evaluated per instantiation, so
  `$fields` and the type predicates answer for the instance rather than the
  template (#2465).
- build: every artifact a manifest declares for the selected target is built,
  not only the first. `-o` is refused by plan shape — when the plan resolves to
  more than one unit — rather than by target count (#2474, #2484).
- legalize: wide multiply (#2344) and wide sign-extension (#2345) on mos6502,
  built from the lane primitives the width-legalization pass already has and
  costed honestly rather than approximated.

### Changed
- build: per-module optimization runs in parallel across cores. In a controlled
  Linux x86_64 measurement, release `--jobs 16` self-host went 15.45s -> 8.89s
  (-42%) and debug -11%; serial builds are unchanged (#2100).
- arm64, riscv64: an identity conversion selects to a move, so the register
  allocator can coalesce the copy away (#2443, #2392).

### Fixed
- ct: constant-time classification keys on the instruction's code *and* its
  flags. AArch64 `b.<cond>` shares its code with the unconditional branch, so a
  secret-dependent conditional branch was validating clean under `#[oblivious]`
  (#2477).
- asm: the assembly printer is notified from the system-register and directive
  emitters, and the sink from a `.byte` payload, so `--emit-asm` renders what
  the encoder actually emitted (#2467, #2493, #2501).
- sema: a global initialiser that references another global in the same module
  folds (#2486); a type query over an unbound generic parameter is refused
  instead of answering for the template (#2464); type layout resolves on demand,
  so a layout intrinsic folds in a type position (#2442).
- mangle: a type with no mangling encoding fails the compile instead of silently
  writing `'v'`, which collided distinct symbols (#2303).
- regalloc: a same-vreg copy is dropped at every width, on every target (#2395).
- encode: x86-64 encodes an indirect `JMP` through memory rather than taking the
  fallthrough path (#2400); an unnamed relation is refused instead of defaulting
  to one (#2435).
- legalize: `MIR_BITCAST` has a mos6502 arm, and its encoder retag (#2483).
- mos6502: the JSR callee relocation applier is named (#2280).
- link: the output is renamed over the target instead of truncated in place, so
  relinking cannot corrupt a running binary (#2475).
- driver: an absolute `[dep.*]` path is used verbatim, never joined onto the
  manifest directory (#2470); an unresolved path dep names both the dep and the
  fix (#2471).
- lower: the unknown-escape diagnostic carries a location (#2472).
- codegen: the "var does not zero" claim is corrected and the redundant clears
  it justified are dropped; `AbiVTable` and `MirVReg` construction is visible to
  the census (#2335).

## [4.5.0] - 2026-08-03

Retires bmos as a built-in OS in favor of the freestanding + platform-tag model,
and lands a flat image's entry symbol at its base regardless of link order.

### Removed
- target: `os = "bmos"` and the `$mach.os.bmos` selector. A BareMetal target is now
  a `freestanding` target with a `base` load-address override and a `platform =
  "bmos"` tag; the kernel-call machinery lives in the external `mach-bmos` package,
  gated on `$mach.build.platform`. bmos as a built-in OS never composed with the
  standard library (#2408); the platform-tag replacement shipped in 4.4.0. The
  mach-std pin advances to 0.21.0, which drops its bmos arm (#2426).

### Fixed
- link: a raw flat image's entry symbol is placed at the image base regardless of
  module link order, rather than only when the runtime module links first (#2409).

## [4.4.0] - 2026-08-03

Adds an open `platform` tag and a load-address `base` override to the target
manifest, letting a bespoke bare-metal environment be expressed as
`freestanding` plus a platform tag rather than a hardcoded OS in the compiler.
Also preserves Darwin framework and dylib link semantics through the driver.

### Added
- manifest: `[target.<name>]` accepts an optional `platform` string tag and a
  `base` load-address override. `platform` is surfaced to comptime as
  `$mach.build.platform` — an open string (empty when unset) that libraries key a
  backend on — folded before the `$mach.build.<name>` define lookup so a define
  cannot shadow it. `base` overrides the first loadable segment's virtual address
  at link time, deferring to the OS default when 0 (#2426, #2427).

### Fixed
- link: Darwin framework and dylib link semantics are preserved end to end —
  frameworks and named dylibs resolve to stable logical `#[library]` identities,
  dynamic requirements carry typed manifest attribution, Darwin runtime search
  paths are retained, and duplicate static/loader inputs are deduplicated (#2424).

## [4.3.5] - 2026-07-30

A compiler-output and vectorization patch. In controlled Linux x86_64 release
self-host measurements, omitting production test bodies reduced the compiler
from 11,177,984 to 8,884,224 bytes; pruning losing weak bodies reduced it again
to 8,200,192 bytes.

### Added
- vectorize: register-only guarded diamonds in integer loops can be predicated
  into packed mask/select operations on x86_64 and AArch64. Conditional memory,
  float lanes, and unsupported guards remain scalar (#2348).

### Changed
- build: normal production IR no longer contains test bodies or test-only
  generic/comptime instances. Tests remain semantically checked, discovered,
  and emitted by `mach test` (#2320).
- repository: the root `dep` checkout is ignored whether it is a directory or
  symlink, without hiding nested `dep` paths (#2422).

### Fixed
- verify: `-g --verify-ir` accepts aggregate locals that SROA made unavailable
  to debug metadata, while retaining the runtime aggregate-address check
  (#2419).
- link: manifest requirements retain their `local`, `system`, and `framework`
  source semantics through direct and cascading links. Darwin resolves
  version-independent framework paths and `.dylib` install names for native and
  cross-target plans, retaining resolved `@rpath/` search directories in Mach-O
  executables, and the optional `[link.X].library` identity makes
  `#[library]` attribution portable across platform loader names without
  order-dependent identity collisions (#2424).
- linker: final executable links discard proven losing weak function bodies and
  their dead relocations across ELF, COFF, and Mach-O; ELF shared links do
  likewise. Relocatable output and target-specific relocation semantics remain
  unchanged (#2386).

## [4.3.4] - 2026-07-30

The release that actually closes #2327. `v4.3.3` was tagged at `4d482ca0` but
never published: its `x86_64-darwin` PIE change exhausted memory during the
native self-host, so CD produced no release. The merge was reverted and the tag
deleted. This release supersedes it with the two root-cause fixes while keeping
`x86_64-darwin` on its fixed-layout executable path.

### Fixed
- darwin: **`mach init` resolves the working directory on macOS.** Advances
  mach-std to 0.20.2, whose Darwin `current_dir` path uses `F_GETPATH` rather
  than the unsupported raw `__getcwd` syscall.
- macho: **debug executables page-align their `__DWARF` segment.** XNU checks
  every segment file offset before skipping a zero-`vmsize` segment, so the old
  8-byte alignment made some small debug executables malformed. The writer now
  uses the target's Mach-O page size: 4 KiB on x86_64 and 16 KiB on arm64.

### Changed
- dep: mach-std 0.20.1 → 0.20.2.

## [4.3.2] - 2026-07-27

A patch release fixing macOS execution and project scaffolding issues (#2327). Advances standard library dependency to **mach-std 0.20.1**.

### Fixed
- darwin: **`getcwd` path length on success.** Fixes `mach init` on macOS where raw Darwin `__getcwd` syscall returning 0 caused working directory resolution to error out.
- macho: **`__DWARF` section layout formatting.** Fixes malformed Mach-O executable headers when compiling with debug profile enabled on macOS.

## [4.3.1] - 2026-07-26

A dependency move and one flat-image fix. The standard library advances to
**mach-std 0.20.0**, which carries the BareMetal arm — `std.system.bmos`'s kernel
call table, `std.runtime.bmos`'s entrypoint, and a `std.system.panic` arm that
terminates. Together with 4.3.0's `bmos` target and indirect `call`, a BareMetal
program can now be written in Mach against the standard library.

**The pin move adds nothing to a hosted compiler.** mach-std's new modules gate
their declarations on `$mach.build.os == $mach.os.bmos`, so a linux, darwin or
windows build links none of them: the compiler binary is **byte-identical at both
pins** over the same source. That is the gating working, stated as a measurement
rather than an expectation.

### Fixed
- raw: **a flat image stores its zero-fill.** The writer sized the image by the
  last *file* byte and its header said bare-metal startup would zero `.bss`
  itself - but the linker exports no `__bss_start` / `__bss_end`, or any symbol
  naming a section boundary, so no startup written in Mach could find the region.
  A 64-byte zero-initialized global landed at the first address *past the end* of
  a 48-byte image, and the loader, which copies the file and nothing else, left
  whatever the memory held. The image is now sized by its last *memory* byte, so
  the zeros are part of it.

  This affected **every `of=raw` target**, `freestanding` included, and went
  unnoticed because no fixture declared a zero-initialized global - which is also
  why the image-size delta on all four existing raw fixtures is exactly zero. The
  cost is the bss size and nothing else: a 4 KiB zero-initialized array grows its
  image by 4096 bytes. Non-raw formats are untouched (#2402).
- vectorize: the float reduction identity is spelled as the `-0.0` literal rather
  than a bit pattern. A bootstrap workaround, load-bearing until a released seed
  carried #2274's fix - 4.3.0 is that seed, and `int/lib/seed-tripwire.sh` failed
  the build the day it became removable, which is what the tripwire exists for.
  The tripwire's last entry left with it, taking the script and its CI step
  (#2284).

### Changed
- dep: mach-std `0.19.0` → `0.20.0`.

## [4.3.0] - 2026-07-26

124 merged pull requests. **Three of its fixes are wrong-answer defects present in
released 4.2.2** — narrow integer arithmetic that skipped its truncation, every
signed *secret* operation emitted in its unsigned form, and a field-wise `$each`
compare that read every field at the last field's type. All three are silent: no
diagnostic, no crash, wrong values. If you are on 4.2.2 read
[Affects 4.2.2](#affects-422) below and check whether your code has the shape.

Around them: **`bmos` joins linux / darwin / windows / freestanding** as a target
operating system — ReturnInfinity's BareMetal x86-64 exokernel — the middle end
gains loop-invariant code motion, scalar replacement of aggregates and three more
vectorization forms, SPIR-V grows structured control flow and calls, mos6502 grows
wide scalars and legalized shifts, and inline assembly becomes one parser across
all three ISAs with `#[naked]`, `#[noinline]`, aarch64 `msr`/`mrs`, and the
x86-64 indirect `call` forms. The standard library moves to **mach-std 0.19.0**.
Built with mach 4.2.2.

(**The constant-time support in this release remains an experimental preview and
its guarantee is not complete.** Several of the disclosures 4.2.2 carried are
closed here — the generic reflection-projection leak (#2168), the deferred
`$each` re-validation gap (#2177, #2174), the secret address gate and literal
shift count (#2195, #2196), and secret field/index resolution (#2297) — and the
signed-opcode defect below was itself a constant-time correctness hole. The `^` /
`#[oblivious]` surface still has not been audited end to end. **Do not build
production cryptography on this version.** Epic #1643 stays open.)

### Affects 4.2.2

All three were verified by execution against the shipped 4.2.2 compiler, not by
reading the diff — each is a program whose output carries the answer, run on both
profiles.

- **Narrow (`u8` / `u16`) arithmetic kept its full 32-bit register content
  instead of truncating to the declared width.** Any later operation that reads
  the high bits then computed on bits that are not part of the value — `>>` and
  `<<` are the ones that do.

  ```
  (0 - a) >> 7   at u8,  a = 1   want 1   4.2.2 gives 255
  (0 - a) >> 15  at u16, a = 1   want 1   4.2.2 gives 65535
  (a << 1) >> 1  at u8,  a = 255 want 127 4.2.2 gives 255
  ```

  Wrong on **both profiles**. An explicit `::u8` cast did not save it; an
  explicit `& 0xFF` mask saved it at `opt=0` but not at `opt=2`. The value was
  truncated only when it crossed a store, a call argument, a return into a narrow
  slot, or a widening cast — which is why code that merely stores narrow results
  never saw it, and why it surfaced from byte-width comparison work in
  `crypto.ct` rather than from the test corpus (#2357).

- **Every signed *secret* operation was emitted in its unsigned form.** The three
  classifiers that answer a type's width, signedness and float-ness read the raw
  type without stripping the `^` qualifier, so `^i32` reported *0 bits, unsigned*
  — and `is_signed_type` is what picks the opcode for shifts and comparisons.

  ```
  ^i32 >> 4        on -64      want -4  4.2.2 gives 268435452   (shr_u for shr_s)
  ^i8  >> 2        on -64      want -16 4.2.2 gives 48
  ^i32 < ^i32      on (-64, 1) want 1   4.2.2 gives 0           (cmp_lt_u for cmp_lt_s)
  ^i32 <= ^i32     on (-64, 1) want 1   4.2.2 gives 0
  ```

  **No dirty-operand precondition — wrong on every input, on both profiles.**
  The same omission made every secret-to-secret `::` cast a *width-changing*
  bitcast, which is malformed IR: `^i8 -1` widened to 255, and a widened `^u8`
  read uninitialized register residue, so that face was non-deterministic across
  builds. `--verify-ir` did not catch it, because the verifier constrains a
  conversion's arity but never relates its result width to its operand width
  (recorded on #2288).

  **Shipped `crypto.ct` was not corrupted by this** — it contains the malformed
  cast in four places, but has no signed secret types at all, and an `& 1`
  immediately before each cast is emitted at register width with an immediate
  that clears every high bit. That is a property of the current lowering, not a
  guarantee the source expresses (#2373).

- **A `$each f in $fields(T)` body read every field at the LAST field's type**, so
  a field-wise compare reported two **identical** records as unequal. The typing
  arrays hold one stamp per expression node and the body is walked once per
  field, so whatever the last field left behind is what every emitted copy read.

  Reading identical bytes at the wrong type still compares equal for almost every
  type, which is why this survived so long. **It becomes visible exactly when the
  wrong type turns the bit pattern into a NaN**, since `NaN != NaN`:

  ```
  rec M4 { i: i32; u: u16; f: f64; g: f32; }
  ```

  `M4`'s first field holding `i32 -2` (`0xFFFFFFFE`) read as `f32` is a NaN, so a
  record compared against **itself** reported field 1 unequal, and every differing
  pair reported field 1 whichever field actually differed. The same record ending
  in `f64` instead reports nothing — `0xFFFFFFFE` inside an `f64` is a denormal,
  not a NaN.

  In released 4.2.2 this affects the **concrete** unroll — a `$each` over a named
  record — on both profiles. The **generic** unroll (`$each` over a type
  parameter) answered correctly in 4.2.2; it regressed later on `dev` via #2286
  and is fixed here by the same change, so it never reached a release. Both
  unrolls are pinned by `int/regression/2376-fields-each-per-element` (#2376).

### Added
- **`bmos`, a new target operating system**: ReturnInfinity's
  [BareMetal](https://github.com/ReturnInfinity/BareMetal) x86-64 exokernel. A
  real OS with its own load contract rather than a spelling of `freestanding` —
  a container-free flat image, loaded at a fixed `0xFFFF800000000000`, entered at
  its first byte with a `call`, and exited by returning. x86-64 only; any other
  instruction set is refused at composition. The stack is not 16-byte aligned at
  entry and `.bss` is not zeroed — both belong to a startup shim, and both are
  documented in `doc/manifest.md` (#2396).
- **A joint (os, isa) capability on the target vtable.** Each operating system
  declares the architectures it runs on, alongside the object formats it can
  load. The object format could not stand in for this: an isa-agnostic flat image
  covers every architecture, so an OS defaulting to `raw` would otherwise be
  advertised on every machine with an encoder (#2396).
- **x86-64 indirect `call`**: `call rax`, `call [0x100018]`, `call [rax + 8]`.
  The absolute form (`ff 14 25 <disp32>`) reaches a fixed-address ABI, whose
  entry points are addresses rather than symbols. `.byte` also gains its own
  bound — one directive now carries a whole sequence rather than four values,
  and a value that does not fit in a byte is refused instead of truncated
  (#2398).
- **`#[naked]`** — a function with no prologue or epilogue, whose body may hold
  only inline asm — and **`#[noinline]`**, the inverse of `#[inline]` (#2198,
  #2200).
- **One inline-asm parser across all three ISAs.** x86-64, aarch64 and riscv64
  now share the statement grammar, the effect model and the numeric-local-label
  scope; each supplies only its mnemonic table and encoder. aarch64 gains
  `msr`/`mrs`, so `PSTATE.DIT` is settable from Mach source (#2243, #2253,
  #2258, #2352).
- **Middle end**: loop-invariant code motion, scalar replacement of aggregates,
  conservative copy coalescing, and leaf frame elision (#2204, #1940).
- **Vectorization**: module-scope arrays and loop-invariant scalars, affine loop
  indices, and fast-math-gated float reduction (#2312).
- **SPIR-V v2**: structured control flow and function calls, and
  `freestanding-spirv` builds a `.spv` end to end (#2120).
- **mos6502**: wide scalars built on a narrow-register target, and a constant-count
  wide shift plus a runtime public-count shift legalized as a masked barrel
  (#2217).
- **`--emit-asm` renders aarch64 and riscv64 from the encoder's instruction
  stream**, as x86-64 already did, so the `.s` is what was emitted rather than a
  second printer's account of it.

### Changed
- **The standard library moves to mach-std 0.19.0** (`eff1e8e` → `fad0355`),
  which carries the string / io / filesystem overhaul and the `crypto.ct`
  constant-time primitives.
- `x86_64-darwin` is a released target again, on native Intel runners rather than
  Rosetta (#2104, #2327).
- Internal structure, no behaviour change: `IsaVTable` split by target family
  (#2213), the relocation applier became a contract the ISA declares, both type
  universes fold through one layout policy (#2289), and every `MirInstr` and
  machine operand is built through a constructor rather than field by field
  (#2215, #2358) — the change that makes a dropped field a compile error instead
  of a silent miscompile.

### Fixed
- **Secrecy and constant-time**: a generic instantiated inside a variadic-pack
  `$each` body is re-validated (#2177, #2174); reflection projection inside a
  generic no longer erases a secret field's secrecy (#2168); the secret address
  gate, the literal shift count, and an unsound placement comparison are
  corrected (#2195, #2196); `^` is stripped before resolving a field or index of
  a secret container (#2297) and before classifying a type's machine shape
  (#2373); `#[oblivious]` is refused on a whole-module-emitter target (#2189).
- **Generics and comptime**: a pack instance is keyed on its generic type-args
  (#2251) and its whole body re-typed (#2254); a generic union's variants are re-checked
  at the instance and a union instance lays out as a union (#2225, #2239);
  comptime type gates and module-scope vals fold against what they actually name
  (#2257, #2206); instantiation depth and count are bounded (#2163); a generic
  template no longer emits a bare body (#2302); a `rec`/`uni` that contains
  itself by value is rejected instead of segfaulting the compiler (#2355, #2356);
  a generic type named without its type arguments, and the address of an
  uninstantiated generic, are both rejected (#2311, #2305).
- **Codegen**: an indirect scalar is returned by value (#2195 was a real
  miscompile, not a validator over-taint); multi-block and memory-tested loops
  rotate; x86-64 scalar float ops take the two-address form; vector lane access
  has a register-resident form; `-0.0` is spellable; an 8-byte scalar reports its
  width as 8 rather than as the machine word (#2346); same-register copies the
  machine says write nothing are deleted (#2275).
- **Diagnostics**: a type node resolves once per pass, so a diagnostic fires once
  instead of two or three times (#2334); a layout depth bound is given to the
  caller and names the real failure cause rather than reporting the wrong one
  (#2368); a comptime intrinsic's type operand is read with the type grammar
  (#2178); an unbound identifier reports the invariant breach instead of falling
  through (#2144).
- **Windows**: `std.system.panic` on Windows no longer compiles darwin syscalls
  (mach-std#397); COFF gains `/bigobj` support.

## [4.2.2] - 2026-07-24

The correctness release. Two of its fixes carry it: on mos6502 a secret integer
`==` did not merely leak through a timing branch, it answered **wrong for every
input** — 65280 of the 65536 ordered byte pairs — and three build phases
returned success while an error already sat on the diagnostic sink, so a program
the compiler had rejected still produced an artifact. Around them, a Windows
image stops carrying two `.rdata` sections, `--emit-asm` renders what the
encoder actually emitted rather than a second printer's account of it, and CI
gates the `dev` → `main` release merge on darwin. Built with mach 4.2.1.

(**The constant-time support in this release is an experimental preview and its
guarantee is not complete.** The flow typing, the codegen taint contract, the
translation validator, and the timing harness are all in, and #2158 and #2159
close below — but proven secret-disclosure paths remain open. A generic
instantiated inside a variadic-pack `$each` body is never constant-time
re-validated, and the program prints the secret with zero diagnostics (#2177).
`$fields` reflection projection inside a generic erases a secret field's secrecy
(#2168). A secret pointer dereference is gated only by the validator, which runs
inside `#[oblivious]` functions alone, so anywhere else it compiles silently
(#2191). Deep secrecy placement still over-rejects differing-shape aggregates
that lack field offsets (#2167). Two new false positives join them, both failing
safe — they reject a correct program rather than accept a leaking one: the
validator over-taints a wide secret on a narrow-ALU target and rejects a
public-count shift as a secret memory address (#2195), and a literal shift count
coerces to `^T` and inherits secrecy, tripping the constant-time shift gate
(#2196). The `^` / `#[oblivious]` surface has not been audited and is not sound
today. **Do not build production cryptography on this version.** Epic #1643
stays open.)

### Fixed
- mos6502: **a secret integer `==` / `!=` compiles branchlessly, and answers
  correctly.** The emitted sequence read the compare's zero flag after
  `LDA #$00` — and `LDA` sets `Z` from the byte it loads, so `Z` was always 1,
  the branch was a constant, and `==` returned 1 while `!=` returned 0 for
  **every** input: 65280 of 65536 ordered byte pairs wrong on both relations.
  A leak and a miscompile in one sequence. Every relation now lands in the carry
  and is read out through fixed-cycle immediate / zero-page / accumulator forms,
  with no branch opcode anywhere in the image. Both enforcement layers had
  passed the old form — the compile-time constant-time gate and the translation
  validator each read pre-selection MIR and trust the encoders to be
  timing-preserving — so it surfaced only by executing the emitted bytes, not by
  reading the lowering. The end-to-end shift-gate compile-reject test #2149 asked
  for lands with it. Every other target is byte-identical (#2158, #2149).
- build: **`codegen`, `emit`, and `link` no longer return success with an error
  on the diagnostic sink**, so a program the compiler has already rejected can no
  longer produce an artifact. The check moved to `run_phase` — the single
  dispatch point every phase flows through — where it covers all eight phases
  uniformly and a phase added later inherits it, rather than being restated in
  each; `sema_phase`'s own copy folded into it. A warning-severity diagnostic
  still does not fail the build (#2173).
- coff: **a Windows image carries one `.rdata` section, not two.** The PE exec
  writer named each linker load segment from its permission bits alone, so a
  plain rodata segment and a RELRO segment — both read-only, neither writable
  nor executable — collapsed onto the same name, and every image built since
  #2126 shipped the duplicate. Sections are now named by kind: RELRO-ness is
  derived from whether the base-relocation stream touches the segment, and that
  segment is named `.data.rel.ro`, matching the COFF object writer's own
  convention so the two writers agree. That name is 12 bytes and does not fit
  COFF's inline 8-byte name field, so exec-writer segment names now take the
  same `/<offset>` string-table indirection the debug names already used —
  closing a latent overrun into the next header field that the permission-only
  naming could never reach. The loader keys sections by table entry rather than
  name, so this is cosmetic to it; it misrepresented the image to anything that
  groups sections by name. ELF and Mach-O output are byte-identical (#2176).
- asm: **`--emit-asm` renders the encoder's own instruction stream.** The `.s`
  came from a second, independent MIR-level printer walking the post-regalloc
  MIR beside the encoder, and the two disagreed at scale: on x86_64 the old
  artifact accounted for 58,798 of the object's 69,976 instructions, with all 26
  modules of the cross-check corpus diverging, and 767 printed prologues carried
  0 epilogues. Worst on vector code — a module whose object holds **858 SIMD
  instructions** rendered **zero** of them, its vectorized loops appearing as
  scalar `mov` / `add` / `cmp.eq`, so the artifact read as if auto-vectorization
  had fired nowhere. The text now falls out of the encoder itself through a
  notifying sink, under a totality guard that fails the build naming any opcode
  whose bytes went unrendered: a printer that silently skips an instruction is
  no longer expressible. The guard earned its keep immediately, finding three
  encoder paths that emitted two instructions under one notification and five
  that named an opcode rather than the form emitted — including a dropped `LOCK`
  prefix that made an atomic read-modify-write read as a plain one. Emitted
  objects are byte-identical, with and without the flag; this is artifact-only
  (#2187).
- ci: **the darwin lane gates the `dev` → `main` release merge.** It ran only
  from a tag push or a manual dispatch before, so nothing in the merge path
  invoked it and #2172 reached a pushed `v4.2.0` tag through a 17/17-green
  release PR. The lane is now a reusable workflow called by both CD and a
  `ci.yml` job conditioned on a `main` base, and CD's release job asserts its
  archive set against the build matrices themselves, so an incomplete publish
  cannot go out green either. Merges into `dev` schedule no macOS runner
  (#2179).

### Changed
- ct: **`#[oblivious]` is refused when the target's back half emits a whole
  module** — SPIR-V today — rather than accepted where nothing enforced it. That
  path forks to the emitter right after MIR lowering and returned before the
  translation validator ran, so the compiler reported success on a contract it
  had not checked. Running the validator there would have certified an artifact
  nobody executes: a vendor driver recompiles the module under no constant-time
  contract, onto hardware whose timing channels this leakage model does not
  describe. Refusal is the rule already applied to inline assembly — reject the
  unverifiable construct rather than trust it. A secrecy-transparent function
  still compiles for SPIR-V, and secret-free code is untouched (#2159).
- asm: **`--emit-asm` now fails with a diagnostic on riscv64 and aarch64**
  instead of writing a misleading file and reporting success. The riscv64
  artifact accounted for 49,380 of 102,503 instructions — 48% complete — and
  aarch64 wrote a 0-byte file. The refusal names the issue that restores each:
  #2193 for riscv64, #2194 for aarch64. x86_64 is unaffected. Anything scripted
  against the flag on those two targets now stops rather than reads a wrong
  answer (#2187).

## [4.2.1] - 2026-07-24

The release that delivers 4.2.0. Its entire payload — auto-vectorization, loop
rotation, fallthrough elision, bounded recursion peeling, loop-carried phi
coalescing, in-memory linking, real Mach-O DWARF, parallel `-g` codegen, and the
constant-time preview — reaches users for the first time here, so the
`## [4.2.0]` **Added**, **Fixed**, and **Changed** sections below apply in full
to this release. On top of them sits one fix: a mach-built aarch64-darwin
executable execs again. Built with mach 4.1.0.

(v4.2.0 was tagged but never published: its CD run caught the cross-built
aarch64-darwin seed being SIGKILLed at exec, so `publish release` skipped and no
artifact and no release object ever left the workflow — the full ledger is
#2172. This release supersedes the tag; the payload is otherwise identical.
Upgrading from 4.1.0 lands the whole 4.2.0 feature set at once.)

(**The constant-time support in this release is an experimental preview and its
guarantee is not complete.** The flow typing, the codegen taint contract, the
translation validator, and the timing harness are all in — but proven
secret-disclosure paths remain open. A generic instantiated inside a
variadic-pack `$each` body is never constant-time re-validated, and the program
prints the secret with zero diagnostics (#2177). `$fields` reflection projection
inside a generic erases a secret field's secrecy (#2168). Both are joined by
#2158, #2159, and #2167. The `^` / `#[oblivious]` surface has not been audited
and is not sound today. **Do not build production cryptography on this
version.** Epic #1643 stays open; #2177 extends the holes enumerated under
4.2.0's Added below.)

### Fixed
- of: **a mach-built aarch64-darwin executable execs again.** The Mach-O writer
  emitted one `LC_SEGMENT_64` per linker load segment, so two identically-mapped
  segments wrote the same 16-byte name twice, and every predicate keyed on load
  segments while the emitter keyed on commands. Introduced by #2126 — the
  `SK_RELRO` round-trip listed under 4.2.0 — which deleted `coalesce_kind` so an
  object round-trips RELRO separately from rodata: the link then produced two
  read-only load segments, and the writer, deriving `SG_READ_ONLY` from
  permissions alone, flagged **every** read-only segment under PIE. The darwin
  kernel refuses to exec such an image and SIGKILLs it before dyld runs, which is
  what killed the 4.2.0 seed. The writers now emit one command per run of
  identically-mapped load segments, each carrying its own `section_64`, and set
  `SG_READ_ONLY` on exactly the read-only segment the rebase stream writes into;
  un-rebased read-only content maps as `__RODATA`, leaving `__DATA_CONST` to mean
  to the image what it means to dyld. Two rebased read-only segments that no
  single command can span are now a loud link error rather than a file that dies
  at exec with no diagnostic. ELF and COFF output are byte-identical (#2172).

### Changed
- of: **a Mach-O read-only segment holding no relocated pointer now maps with
  `maxprot` `r--`**, where every read-only segment in a PIE image previously
  carried `rw-`. The segment holds nothing the loader slides, so capping it is
  the point rather than a side effect, and `__LINKEDIT` has always shipped
  `r--`. The visible consequence: an `mprotect` / `vm_protect` requesting write
  over a page holding a `val` constant now fails on darwin where it previously
  succeeded — relevant to self-patching code and to warming a constant cache in
  place (#2172).

## [4.2.0] - 2026-07-24

The auto-vectorization release. Counted element-wise and integer-reduction loops
compile to 128-bit SIMD at `-O2` on capable targets — element-wise f64 **2.07x**,
f32 **4.90x**, i64 sum reduction **1.53x** — behind a runtime alias guard that
leaves any loop it cannot prove safe scalar. Around it the optimizer gains loop
rotation, fallthrough elision, bounded recursion peeling, and loop-carried phi
coalescing; the backend links executables straight from in-memory images, emits
real Mach-O DWARF, parallelizes `-g` codegen, and splits COFF per function.
Built with mach 4.1.0.

(**The constant-time support in this release is an experimental preview and its
guarantee is not complete.** The flow typing, the codegen taint contract, the
translation validator, and the timing harness are all in — but a proven
secret-disclosure path remains open (#2168), along with #2158, #2159, and
#2167. The `^` / `#[oblivious]` surface has not been audited and is not sound
today. **Do not build production cryptography on this version.** Epic #1643
stays open; the holes are enumerated under Added below.)

### Added
- codegen: **loop auto-vectorization** — a counted, unit-stride, single-block
  loop whose dependence analysis proves independence is rewritten to 128-bit
  vector operations with a scalar remainder for the trip-count tail. Two shapes
  land: **element-wise maps** (`dst[i] = f(src0[i], ...)` over add/sub, float
  mul/div, and integer bitwise ops) guarded by a runtime alias check that routes
  overlapping arrays to a scalar fallback, and **integer reductions** (`add`,
  `xor`, `or`, `and`) via lane-count partial sums and a horizontal combine,
  **bit-identical** to scalar. Measured against a `#[scalar]` twin at N=1024,
  cache-resident: element-wise f64 2.07x, f32 4.90x, i64 sum reduction 1.53x.

  **Gates:** the release pipeline only (`opt = 1` or `2`, which `-O1` / `-O2`
  also select; the stock `release` profile is `opt = 2`), and only targets
  reporting `has_v128` (SSE2 on x86-64, NEON on aarch64). riscv64 has no 128-bit
  vectors, never enters the pass, and is byte-identical to 4.1.0. Debug builds
  are entirely unaffected.

  **Opt-outs:** the optional `[profile.X] vectorize` manifest key (absent
  defaults to on; `false` skips the pass at release), and the `#[scalar]`
  function decorator, which also declines inlining so the opt-out survives being
  inlined into an unflagged caller.

  **Float reductions are deliberately not vectorized.** Reassociating an f32/f64
  accumulator changes IEEE rounding, so they stay scalar pending a fast-math
  profile key (#2160). Integer dot/multiply-reduce and non-associative
  subtraction likewise decline — anything unproven stays scalar (#1942, #2139,
  #2140, #2141, #2142).
- language: **constant-time support — `^` secret types and `#[oblivious]` —
  reaches end-to-end enforcement, as an experimental preview.** The `^` flow
  typing shipped earlier; this release adds the layers that make it mean
  something at the machine level:
  - `#[oblivious]`, a closed-set functions-only decorator marking a constant-time
    boundary, with a secret-taint bit threaded from sema's flow typing through IR
    and MIR to the emitted instruction stream, monotonically preserved across
    every RAUW, inline clone, isel piece-emission, and regalloc copy. Secret-free
    code is byte-identical (#1646).
  - an op→CT-capability table (`mach.lang.ct`) shared by the compile-time gates
    and the validator: integer multiply is gated on the target's `ct_trust_mul`
    (conservatively false on every ISA, absent a trusted DIT/DOITM mode),
    variable shift counts on the barrel-shifter fact, and divide/remainder plus
    all floating point are always gated.
  - a **`MIR_DECLASSIFY` barrier** marking the one intentional secret-to-public
    crossing, so taint propagation stops exactly there and nowhere else.
  - a **translation validator** (`ctvalidate`) that re-derives taint over the
    lowered, target-independent MIR as a monotone dataflow fixpoint — handling
    phi joins, inlined-return copy chains, rotated-loop back edges, and
    precolored physical registers (x86-64's RAX dividend, RCX shift count) by
    construction — and rejects a secret reaching a branch condition, a memory
    address, or a forbidden variable-latency op. It is the independent
    relational backstop behind the compile-time gates (#1648).
  - an `#[oblivious]`-required check per monomorphized instance: an instance that
    *computes* on a secret must carry the decorator, while a transparent instance
    that only moves, stores, or declassifies stays annotation-free (#2136).
  - a **dudect-style timing harness at `int/ct/`** measuring a branchless
    `#[oblivious]` reference against a deliberately-leaky control with Welch's
    t-test — the leaky control lands at |t| ≈ 989 against the reference's |t| < 1.5,
    a discrimination ratio in the thousands, and the driver self-tests that the
    assertion genuinely gates. Run it with `bash int/ct/ct.sh` (#1647).

  **Known open holes — the guarantee is incomplete:** `$fields` reflection
  projection inside a generic erases a secret field's secrecy and discloses the
  secret (#2168, proven, security-blocking); secret integer `==`/`!=` compiles a
  timing branch on no-flag targets (#2158); `#[oblivious]` functions compiled to
  SPIR-V bypass the validator entirely (#2159); deep secrecy placement
  over-rejects differing-shape aggregates that lack field offsets (#2167). The
  validator covers the lowered MIR and trusts isel, legalization, regalloc, and
  encode to be timing-preserving; validation of emitted machine code is future
  work. A proof is always relative to a leakage model, and this one has not been
  audited.
- codegen: **MIR-level loop rotation** — a top-tested loop becomes a guarded
  bottom-tested one, so the steady-state body carries a single taken branch (the
  back edge) instead of two. On a branch-throughput-bound counting loop this
  roughly doubles throughput: 1.774s → 0.888s best-of-5 on x86-64 release
  (#1937).
- codegen: **branches to the fallthrough block are elided** — across the
  compiler's own self-build corpus, jump-to-next-instruction sequences fall from
  66,975 to ~10 (the residual are jumps over degenerate zero-byte blocks), and
  `.text` shrinks by exactly the eliminated x64 `rel32` bytes (#1936).
- inline: **bounded self-recursion peeling** — a directly self-recursive function
  is peeled K levels from a pristine id-stable snapshot, so the peel stays a
  uniform K deep with the residual tail calling back into the real function. On
  `fib`: retired instructions 2.72B → 1.87B (−31%), wall time 305ms → 201ms
  (1.51x) (#1941).
- regalloc: **loop-carried phi copies coalesce** — admitted only when the source
  dies at the copy, both sit in one block, and the destination is unread in the
  window. The xorshift loop tail drops 11 → 8 instructions; across the self-host
  corpus (196 modules, ~1.98M instructions) 7,386 instructions disappear, 7,378
  of them `mov`s (#1938).
- build: **executables link from in-memory codegen images** on ELF and Mach-O
  (and, since the COFF fix below, on COFF) instead of re-parsing the `.o` files
  the compiler wrote moments earlier. Gated on a declared per-format
  `OfVTable.links_from_image` capability; only genuinely-on-disk inputs (libc,
  `-l`/`-L`, archives, shared objects) are still parsed. Linked output is
  byte-identical on every target (#1828).
- of: **Mach-O `-g` emits real DWARF** — `SK_DEBUG` sections no longer collapse
  into one `__DWARF,__debug` blob but become individual `section_64`s spelled in
  Mach-O form (`.debug_info` → `__debug_info`), with the parser reversing the
  spelling so the linker's name-keyed debug merge and `.debug_str` dedup see
  canonical names. The in-binary `__DWARF` segment is framed below the code
  signature so the ad-hoc signature's `code_limit` covers it.
  `llvm-dwarfdump --verify` reports no errors on both darwin executables
  (#1701, #1702).
- build: **`-g` codegen runs in parallel** — the DWARF producer builds debug
  types into a private per-worker `IrTypeTable` used purely as a layout oracle,
  removing the shared-session-allocator race that forced `-g` builds serial.
  Output is byte-identical at `--jobs 1` vs `--jobs 8` (all 196 release objects,
  `.debug_*` included), `.text` stays byte-identical with and without `-g`, and
  the `-g` self-build goes 21.4s → 7.5s (~2.9x) at 8 jobs (#2101).

### Fixed
- of: **`SK_RELRO` survives the Mach-O and COFF object round-trip.** Both writers
  collapsed RELRO into `SK_RODATA` and never recovered it on parse, so a module's
  in-memory image was not link-equivalent to its own `.o`: the reparse merged one
  read-only section where the image had two, shifting every subsequent vaddr and
  cascading into all address-bearing bytes. Mach-O now emits RELRO to
  `__DATA_CONST,__const` and recovers it from the segment name; COFF recovers it
  from a read-only `.data.rel.ro`. Relocation addends were never the cause — they
  already round-tripped exactly. ELF untouched (#1865).
- coff: **per-function section split drops weak-body duplication.** The
  defined-weak COMDAT carrier kept the weak body in the original `.text` as well,
  so every `.o` shipped every weak body twice. Emit now tiles each source section
  with non-weak spans plus one carrier per weak body and no retained original,
  remapping every symbol and relocation. Cross-building the compiler to
  `windows-x86_64`, `.text` VirtualSize drops 7,629,330 → 6,224,082 bytes
  (−1.34 MB), and the `.o` becomes link-equivalent to the in-memory image
  (#2125).
- resolve: **the `:^` secret-strip cast binds its operand in a full build** — it
  resolved in isolation but not through the whole-project path, so declassifying
  a secret failed to compile end-to-end (#2138).
- sema: **four secret-disclosure paths closed.** A secret passed to a variadic
  pack is rejected, including a deep (aggregate-wrapped) secret and one reached
  through a lazily-materialized generic-instance field (#2162); reinterpret
  placement fails closed by default and the function-type secrecy launder is shut
  over the whole kind space (#2165); and generic secrecy is re-validated per
  instantiation, so `cmp_branch[^u32]` is rejected with the same diagnostic as
  its concrete twin despite monomorphization collapsing it onto a public sibling
  — a proven false-negative that shipped with the taint foundation (#2157).
- codegen: **an unencodable inline-asm mnemonic renders a located diagnostic**
  at the statement's `path:line:col` instead of a blank crash, and worker error
  messages are re-homed before pool teardown so they survive to be printed
  (#2153).
- codegen: **inline asm is rejected inside an `#[oblivious]` function** — a type
  system cannot check an `asm` block, so it cannot be trusted inside a
  constant-time boundary (#1648).

### Changed
- sema: **aggregate-value `==` / `!=` is rejected** with a teaching diagnostic
  rather than silently comparing representations. Scalar and address comparisons
  are unaffected; the rejection extends through generic instances (#2127).
- abi: the SysV vector-count register (AL) is staged **only for variadic calls**,
  not on every call through a function pointer (#1939).

## [3.6.1] - 2026-07-12

The build-speed and interop release. Parallel codegen takes the compiler
self-build from 25.2s to 11.7s on 16 cores with byte-identical output, win64
C interop for vectors conforms at `ext` boundaries, the inliner finally sees
generic-instance bodies, and macOS support consolidates on Apple Silicon.
Built with mach 3.5.1.

(v3.6.0 was tagged but never published: its CD run caught the parallel
compiler's worker threads crashing under Rosetta 2 on the x86_64-darwin lane,
which led to the platform ruling below — the full ledger is #2104. This
release supersedes the tag; the payload is otherwise identical.)

### Removed
- **the x86_64-darwin release artifact.** Intel Mac hardware is EOL and its CI
  substrate is gone: Rosetta 2 cannot host mach's raw `bsdthread_create`
  worker threads (the kernel entry bypasses Rosetta's per-thread
  translation-context init — proven by core backtrace), and native Intel
  runners no longer allocate. The `x86_64-darwin` target triple remains a
  buildable, unvalidated cross-target; `aarch64-darwin` is the supported macOS
  platform, now validated on macos-15 (#2104, #2105). The darwin thread entry
  additionally gained the x86_64 stack-alignment trampoline in mach-std
  (briar-systems/mach-std#377), a standalone ABI hardening found during the
  investigation.

### Added
- driver: **parallel per-module codegen** — codegen runs across a worker pool
  (`--jobs <n>`, default host CPUs), with output byte-identical to the serial
  build at every core count (proven through the self-host fixpoint): codegen
  17.7s → 3.3s, whole release self-build 2.15x on 16 cores. `-g` builds keep
  the serial codegen path for now (#2101); parallel lowering is the next phase
  (#2100) (#1826, #2099).
- abi: **win64 vector arguments marshal to the Microsoft x64 convention at
  `ext` boundaries** — a call to a declared `ext` function passes each 128-bit
  vector by reference (a 16-byte-aligned caller temp), varargs included;
  returns already conformed (XMM0). Internal calls keep mach's XMM convention
  everywhere, byte-identical on every target. A call through a function
  pointer carries no `ext` fact yet and does not marshal (#2058, #2087).
- manifest: **step template variables `{target.isa}` / `{target.os}` /
  `{target.abi}`** join the closed expansion set, and every step process
  receives `MACH_TARGET_ISA` / `MACH_TARGET_OS` / `MACH_TARGET_ABI` in its
  environment — closes RFC #1964 (#2098).

### Fixed
- sema: **an imported `pub val` constant folds in type positions** —
  `var buf: [a.CAP]u8;` with a cross-module constant compiles exactly like its
  local equivalent; an imported `var` is still rejected (#2081, #2097).
- me: **generic-instance and forward-referenced function bodies are visible to
  the inliner** — materialising a definition reuses its forward extern entry
  in place instead of orphaning every captured call against a bodiless twin,
  so eligible instance calls inline (#2064, #2096).

## [3.5.1] - 2026-07-11

The emergency inliner-correctness patch: one silent miscompile, one fix,
nothing else. Built with mach 3.5.0.

### Fixed
- codegen: **the value-substitution primitive preserves the use-site operand
  type stamp.** When the inliner wired a call result to its return value,
  `replace_uses` substituted the value wholesale, clobbering the use-site
  stamp — so a deref-copy of an inlined callee's returned pointer
  (`var x: Rec = @small_same_module_fn();`) degraded from a record copy to an
  8-byte pointer-bits store: silent wrong values at `opt >= 1`, present in
  every release of the #2035 vintage through 3.5.0. The primitive now retypes
  per use slot, matching the batched-rewrite invariant, with every other
  wholesale substitution site audited clean; in-vitro tests pin both inliner
  return arms and a release-profile end-to-end regression case pins the
  observable defect (#2092, #2093).

## [3.5.0] - 2026-07-11

The data-imports and correctness release. `ext val` / `ext var` land as the
data analogue of `ext fun` — imported data resolves through the GOT on dynamic
targets — and the defect trail exposed by the vector library build-out is
closed: two aarch64 SIMD criticals, the riscv64 scalarized compare, and the
generic-instance signature gap behind the 1.0.0-era inliner miscompile's ABI
fragility. Built with mach 3.4.4.

### Added
- language: **`ext val` / `ext var` foreign data imports** — storage-less
  declarations mirroring `ext fun` (no initializer; the `symbol` and `library`
  decorators work as for functions). On a dynamic target the reference is
  GOT-indirect (ELF `R_*_GLOB_DAT`, eager-bound); a same-artifact cross-module
  reference keeps direct addressing. Data imports in a shared object are
  exec-only for now (loud error); the executed-`dlopen` bind is tracked at
  #1885. Seed note: `ext var` parses under the 3.4.4 seed, in-tree `ext val`
  needs this release as seed (#2026, #2079).
- codegen: **the always-on call-argument typing verifier** — every direct
  call's fixed arguments are checked against the callee's declared signature
  at build time, making the #2035 mis-typing class structurally unshippable;
  vector parameters accept the scalarized by-address form (#2063, #2080,
  #2086, #2088).
- diag: the emitted-IR printer renders extern-decl signatures and per-operand
  types (#2065, #2078).
- test: the riscv64 vectors int case — the first end-to-end rv64 vector
  build + runtime coverage (qemu leg); it caught #2077 and #2086 before it
  merged (#2075, #2076).

### Fixed
- codegen: **generic, comptime-value, and pack instance callee references
  carry their true signature** instead of a bare pointer, so a direct call
  classifies its ABI from the declared parameter types rather than per-operand
  stamps — the structural fix behind the #2035 miscompile class (#2063, #2080).
- codegen: **aarch64 same-bank vector bit-reinterpret (`:~`) materializes the
  register copy** — it previously selected a GP move that never wrote the
  vector destination: garbage lanes or SIGSEGV on every float/signed
  select-via-mask since v3.4.0 (#2082, #2083).
- codegen: **an aarch64 128-bit vector live across a call keeps its upper
  lanes** — AAPCS64 callee-saves only the low 64 bits of V8-V15, so the
  allocator now spills a vector around a call instead of parking it there;
  modeled as a per-ABI fact, win64's full-width XMM6-15 preservation untouched
  (#2085, #2089).
- codegen: **riscv64 scalarized signed i8/i16 vector ordering compares
  sign-extend their lanes** before the register-width compare — `-1 < 0` no
  longer evaluates false under scalarization (#2077, #2084).

## [3.4.4] - 2026-07-11

The vector compare/merge patch — two more shipped miscompiles caught by the
new runtime suite's first CI runs, plus the suite itself landing as permanent
coverage. Built with mach 3.4.3.

### Added
- test: the colocated runtime vector per-op suite — 36 self-checking blocks
  (every table operator on every shape with discriminating lane values, ABI
  round-trips, spill pressure) executing natively on the x86_64 and aarch64
  legs (#2038).

### Fixed
- codegen: **aarch64 float vector ordering compares** (`< <= > >=`) selected
  the unsigned integer byte compare instead of the float ordered compare —
  silent wrong masks on every float ordering since v3.4.0 (#2069).
- codegen: **a by-value vector returned through a control-flow merge carries
  all 16 bytes** — the phi-elimination copy was hard-coded to GP register
  width, truncating vector merges to the low 64 bits (#2070).

## [3.4.3] - 2026-07-11

The inliner correctness patch, fixing a long-lived silent miscompile the repo
owner hit in the field (mach-mqtt's `brokerd`). Built with mach 3.4.2.

### Fixed
- codegen: **the inliner preserves an operand's use-site type when substituting
  a callee parameter with the caller's actual argument.** A by-value aggregate
  argument is an address the frontend retypes to the aggregate; clobbering that
  stamp made the surviving call classify its ABI from a bare pointer, so an
  inlined call site passed the aggregate as a pointer while the callee read a
  by-value stack parameter — a wrong-address read that could crash or, worse,
  silently return foreign memory. Affected every release since 1.0.0 at
  `opt >= 1` for generic calls passing by-value aggregates through an inlined
  clone (#2035). A `--verify-ir` structural check and an opt-independent
  inliner unit test now guard the invariant. Follow-ups: #2063 (instance
  callees carry real signatures), #2064, #2065.

## [3.4.2] - 2026-07-11

The win64 vector patch: closes the known limitation documented in 3.4.1. Built
with mach 3.4.1.

### Fixed
- codegen: **win64 classifies 128-bit vectors** — arguments ride XMM registers
  (stack by value on exhaustion) and returns ride XMM0, instead of being
  mis-passed in a single 8-byte GP register. This is mach's internal convention,
  consistent across all four ABIs; it deliberately deviates from the Microsoft
  x64 `__m128` by-reference argument convention, so C interop with vector-typed
  `ext` functions on windows awaits the conformance ruling (#2058). The by-value
  round-trip regression test now runs on every target (#2055).

## [3.4.1] - 2026-07-11

The vector correctness patch. The `#2038` runtime suite's first run caught a
shipped v3.4.0 miscompile: a by-value 128-bit vector argument or return whose
value was not already register-resident was captured with a machine-word move,
silently truncating lanes 2-3. Fixed at the shared capture path (a 16-byte
`CLASS_FP` value now moves full-width on both SysV and AAPCS64); scalar codegen
is byte-identical. Debug info for vectors also lands. Built with mach 3.4.0.

**Known limitation:** win64 never classified vector arguments/returns (a
distinct, pre-existing gap first exercised by this patch's regression test) —
vector-by-value calls on x86_64-windows are mis-passed until #2055 lands; the
MS x64 convention (args by reference, returns in XMM0) is specified there.

### Added
- debuginfo: vector types emit `DW_TAG_array_type` + `DW_AT_GNU_vector` DIEs —
  gdb prints vector locals lane-by-lane on `-g` builds (#2018).

### Fixed
- codegen: **by-value vector argument/return capture is full-width** — the
  caller's return capture and the callee's parameter capture no longer truncate
  a non-coalesced vector register copy to 64 bits (#2053).
- debuginfo: a struct's vector member gets its true 16-byte layout in DWARF
  member offsets (the DWARF type bridge modeled vectors at pointer width;
  runtime layout was always correct) (#2051).

## [3.4.0] - 2026-07-11

The SIMD release. Ten 128-bit vector types (`f32x4 f64x2 i8x16 i16x8 i32x4
i64x2 u8x16 u16x8 u32x4 u64x2`) land as first-class primitives on the
portable-vector model: semantic types legal on **every** target, with lane-wise
operators, full-arity literals, comptime-bounds-checked lane access, and
comparisons that produce hardware-shaped unsigned mask vectors. x86_64 lowers
one instruction per operator on SSE2 and aarch64 on NEON; riscv64 builds the
same programs through a defined unrolled scalar expansion of each operator,
reported with a build-time note. What an incapable target does is the new
`[profile.X] simd` lever — `"scalarize"` (build with the expansion) or
`"require"` (hard error naming the offending operator). Built with mach 3.3.1.

**Manifest schema change:** `simd` is a **required** key on every declared
`[profile.X]` (`"scalarize" | "require"`), per the schema-evolution rule —
additions enter as required-everywhere fields with a coordinated sweep, never
optional-with-default. Every briar-systems repo was swept ahead of this release;
a third-party manifest declaring a profile must add the key (the 3.3.1 seed
already accepts it).

### Added
- language: **the ten seeded 128-bit vector types** — spelling
  `<u|i|f><width>x<count>`, single-`x` only; 16-byte size/align;
  arrays-of-vectors and pointers-to-vectors ordinary; full-arity literals;
  comptime-constant bounds-checked lane read/write; uninitialized locals
  default to all-zero lanes; no scalar↔vector casts; the honest lane-wise
  operator table (float `+ - * /`; integer `+ -`, `*` on 16-bit lanes;
  bitwise `& | ^ ~`; all six comparisons → same-shape unsigned mask; no
  vector `%`, integer `/`, or shifts) (#1965, #2013, #2030, #2043).
- codegen: **SSE2 (x86_64) and NEON (aarch64) vector backends** — one
  instruction per table operator, fixed defined sequences where the baseline
  lacks one; SysV SSE-class and AAPCS64 short-vector ABIs; 16-byte-aligned
  vector spills (#2014, #2015).
- codegen: **riscv64 defined unrolled scalarization** — lane-exact-identical
  results with no vector unit, memory-class ABI, and the one-shot build-time
  scalarization note (#2016).
- driver: **the `[profile.X] simd` lever** — `"scalarize"` | `"require"`,
  enforced at a single middle-end gate off the shared detection; the lever is
  always the consumer's (a dependency's value is inert — no ecosystem fork)
  (#2017).
- doc: language and manifest documentation for all of the above
  (types/operators/policy/manifest) (#2020).

### Changed
- test infra: the standalone `dbg/` debug-info harness folded into the int
  framework as the `debuginfo` binary-inspection case kind (dwarfdump
  `--verify` + `-g` byte-additivity, multi-TU, all three ELF ISAs); the
  whole-compiler `-g` additivity capstone now runs inline at release cut
  (#2039).

### Fixed
- codegen: the x86_64 encoder's memory-to-memory staging register is chosen
  bank-correctly for 16-byte moves (a hardwired GP scratch aliased a live XMM
  register under vector spill pressure) (#2034).

## [3.3.1] - 2026-07-10

The seed-gate patch for the SIMD program. The manifest parser now accepts and
validates the `simd` profile key (`"scalarize"` | `"require"`) without requiring
it — the staging step that lets the next compiler parse a swept manifest tree
before the required-everywhere flip lands with the SIMD gate. Alongside it,
`.debug_str` is string-merge deduplicated across modules, and the aarch64
shared-object path is verified end to end and carries its GOT relocation
foundation. Built with mach 3.3.0.

### Added
- manifest: **the `simd` profile key parses** — accepted and value-validated
  (`"scalarize"` | `"require"`) on a declared `[profile.X]`, not yet required;
  the required-everywhere flip ships with the SIMD gate (#1965, #2013, #2027).
- link: **aarch64 GOT relocation foundation** — `RK_GOT_PAGE21`/`RK_GOT_LO12`
  mapped to `R_AARCH64_ADR_GOT_PAGE`/`R_AARCH64_LD64_GOT_LO12_NC`, with a
  unit-tested GOT-indirect encode primitive, dormant pending the imported-data
  ruling (RFC #2026); aarch64 `kind = "shared"` output verified end to end —
  PIC-clean internal references, `R_AARCH64_RELATIVE` dynamic relocations, and
  the release/`-g` byte rules (#1980, #2022, #2023).

### Fixed
- debuginfo: **`.debug_str` is string-merge deduplicated across modules** —
  406,117 → 166,072 bytes on the compiler itself, with a per-site
  content-equality guard that turns a wrong remap into a loud link error
  (#1708, #2025).

## [3.3.0] - 2026-07-10

The shared-objects and debugging-experience release. `kind = "shared"` artifacts
now link into real position-independent ELF shared objects — exporting
`.dynsym`/`.hash`, with a `DT_SONAME` taken from the output basename — `dlopen`-able
and consumable by another project through a `local` `[link.X]` entry. A release
image is program-headers-only; a `-g` image carries a full descriptive section
table, and `-g` stays byte-additive over the release image. And debugging under
optimization is materially better: local variables survive phi merges,
register-passed by-value aggregate parameters get honest frame locations, a
breakpoint on a function lands after its parameters are homed, anonymous and
generic-instance aggregates no longer emit empty type names, and `.debug_abbrev` is
deduplicated across modules — 44,968 bytes down to 345 on the compiler itself. Built
with mach 3.2.0.

### Added
- backend: **`kind = "shared"` artifacts link real ELF shared objects** —
  position-independent, exporting `.dynsym`/`.hash` with a `DT_SONAME` from the
  output basename, `dlopen`-able and consumable through a `local` `[link.X]` entry.
  A release image is program-headers-only; a `-g` image carries a full descriptive
  section table, and `-g` remains byte-additive over the release image (#1980,
  #2004).

### Fixed
- debuginfo: **local variables survive phi merges** in optimized `-g` builds,
  instead of losing their locations where control flow joins (#1904, #2005).
- debuginfo: **register-passed by-value aggregate parameters** get honest frame
  locations rather than register locations left stale after homing (#1954, #2006).
- debuginfo: **`break <fn>` lands after parameter homing** — a `prologue_end` row
  plus a `low_pc` anchor row put the breakpoint past the prologue (#2007, #2008).
- debuginfo: **anonymous and generic-instance aggregates omit `DW_AT_name`**
  instead of emitting an empty string (#1955, #2003).
- debuginfo: **`.debug_abbrev` is deduplicated across modules** — 44,968 → 345
  bytes on the compiler itself (#1708, #2009).

## [3.2.0] - 2026-07-10

The strictness release. A declared manifest table is now **total** — every field
is spelled or it is a parse error — so there are no silent defaults to memorize:
`[project]` is exactly `id`/`version`/`src`/`out`, a filter axis takes a canonical
`os`/`isa`/`abi` value or `"*"` (a typo like `"windoze"` is rejected rather than
silently never matching), `[]` says "none" out loud, and the path template set
closes to `{project.out}`/`{target.name}`/`{profile.name}`. Totality is enforced on
the manifest being **built**; a **dependency's** manifest parses permissively, read
only for its export surface, so historical commit pins keep resolving and a project
never waits on its dependencies to migrate. `static` artifacts now emit a real `ar`
archive with a symbol index at their declared `out`. The manifest and CLI
documentation are rewritten against the landed schema, and the pre-flag-day CLI
leftovers — `--release`, `--no-emit-ir`/`--no-emit-asm`, and the `[project].dep`
read — are gone. The thirteen first-party repositories were swept to the strict
shape in lockstep, so no in-house consumer breaks. Built with mach 3.1.0.

> **Breaking:** a root manifest that relied on lenient parsing must now be total —
> every field of every declared table spelled, canonical filter values, and only
> the closed template set. Dependency manifests are unaffected (they parse
> permissively for their export surface). Every first-party repository was
> converted by the ecosystem sweep (#1971, #1979).

### Added
- manifest: **`static` artifacts emit a real `ar` archive** with a symbol index at
  the declared `out`, the deliverable a consumer links as a `.a` (`shared` remains
  phase 2) (#1980, #1997).

### Changed
- manifest: **no-defaults totality on the root manifest** (the #1964 flag-day) — a
  declared table spells every field or it is a parse error; `[project]` is trimmed
  to `id`/`version`/`src`/`out`; filter axes take a canonical value or `"*"`; the
  template set closes to `{project.out}`/`{target.name}`/`{profile.name}`; and
  `emit_ir`/`emit_asm` become CLI-only (#1971, #1996).
- manifest: a **dependency's manifest parses permissively** — read only for its
  export surface (project id, `export = true` link entries, and the steps they
  demand) — so a dependency's own migration state never gates a consumer and
  historical commit pins keep resolving. Unknown keys are still rejected on both
  paths (#1971, #1996).
- doc: **`doc/manifest.md` and `doc/cli.md` rewritten** against the landed strict
  schema, and the `mach help` text corrected (#1977, #1998).
- ecosystem: the **thirteen first-party repositories** were migrated to the strict
  manifest shape in lockstep with the flag-day, so no in-house consumer breaks
  (#1979).

### Removed
- cli: **`--release`** (rejected with a `--profile release` hint), **`--no-emit-ir`
  / `--no-emit-asm`** (emission is CLI-only via `--emit-ir`/`--emit-asm`), and the
  **`[project].dep` read** (the store is the hardcoded `dep/` convention) — the
  pre-flag-day leftovers. `-O0`/`-O1`/`-O2` remain as per-invocation opt overrides
  (#1999, #2000).

## [3.1.0] - 2026-07-09

The release that makes the V2 build system work as designed. The `[step.X]`
engine is now demand-driven: a step runs only when a build needs it — a
selected, target-matching `local` link entry whose path matches the step's
`out`, an artifact's `need`, or another step's `need` — never automatically,
and a dependency's export steps run through a consumer so vendored-C libraries
finally build and link. Each step is cached by the content of its inputs and
its expanded command, so warm rebuilds skip unchanged work. `mach test` links
the union of every artifact's entries; link filters accept string and array
forms with `[]` meaning none and `""` a parse error; a duplicate dependency id
is a hard error that names the requirer chain. Project paths resolve by four
explicit rules with the path argument now **required** — a directory or a
config file, with no current-directory walk. The manifest parser matches the
RFC (optional `name`/`description`, `{project.out}`), and a pre-V2 `mach.lock`
is read transparently with its pin preserved. Built with mach 3.0.2.

### Added
- steps: **Demand-driven `[step.X]` engine** — steps run only when a link
  entry, artifact, or another step demands them (never automatically; cycles
  are an error), cached by a per-step content fingerprint (sorted inputs +
  content hashes + expanded `cmd`) so warm rebuilds skip unchanged steps.
  Dependency export steps execute from the dep checkout and home
  `{project.out}` into the consumer out tree. `in` accepts `*`/`**` globs
  (sorted, empty match errors); paths/`cmd` use the closed template set
  `{project.out}`/`{target.name}`/`{profile.name}` (#1970).
- manifest: `mach test` links the **union** of every artifact's referenced
  entries plus exported dependency entries, filtered to the native target
  (#1969).
- linking: **string-form link filters** — `os`/`isa`/`abi` accept a string or
  an array of strings — with up-front validation of every `local` link path
  (#1969, #1983).
- driver: a **duplicate dependency id** anywhere in the closure is a hard error
  naming the requirer chain (#1968, #1986).

### Changed
- driver: **project path resolution is four explicit rules and the path
  argument is required** — a directory or a config file, with no
  current-directory walk (behavior change from 3.0.x; `mach clean`'s path is
  likewise required) (#1974, #1985, #1992, #1993).
- manifest: link **filter presence semantics** — an explicit `[]` matches
  nothing and an empty `""` is a parse error — with cell-aware `local`-link
  validation (#1988, #1990).
- manifest: `[project].name` and `[project].description` are **optional**, and
  `{project.out}` resolves per the RFC (#1983).
- deps: a pre-V2 `mach.lock` `[deps.X]` section is read transparently as
  `[dep.X]` with its pin preserved; the repo lock re-pinned to a
  V2-compatible mach-std (#1975).

### Fixed
- driver: a dependency's `[step.X]` outputs are produced in the consumer out
  tree before the link, instead of failing with "cannot find link input"
  (#1970).
- doc: `mach doc` creates its output directories recursively (#1987, #1991).
- ci: the integration and debug-info fixtures were migrated to the V2 manifest
  schema, restoring a green matrix (#1976).

## [3.0.2] - 2026-07-07

Fixes transitive dependency resolution to resolve flat relative to the current root directory instead of nesting them.

### Fixed
- driver: Flat transitive dependency folder lookup.

## [3.0.1] - 2026-07-06

Updates the compiler project's self manifest (`mach.toml`) to the new V2 manifest format (`[artifact.mach]`).

### Changed
- manifest: Migrated self-manifest layout to comply with the V2 manifest spec.

## [3.0.0] - 2026-07-06

A major paradigm break and structural overhaul to v2 manifests. Overhauls parser, driver, and CLI commands to comply with the v2 spec: replaces `[bin.X]` and `[lib.X]` with `[artifact.X]`, adds `[link.X]` with target filtering, adds custom `[step.X]` support, removes `[project].module` and `[os.X]`, and replaces release/debug flags with the profile-selection parameter. This release retains the old-format `mach.toml` for bootstrapping.

### Added
- manifest: **V2 manifest layout spec parser** and target-cascading linker resolver.
- steps: Custom shell step topologies executed relative to package root.
- linking: Transitive cascading of local and system library link entries.
- CLI: `--profile <name>` parameter replacing `--release`/`--debug` switches.

## [2.15.0] - 2026-07-04

The debug-info release. `-g` produces full DWARF 5 on every ELF target
(x86_64, aarch64, riscv64): breakpoints by file:line, stepping, named and
typed frames, live parameter/local values with PC-ranged locations, inlined
frames with call sites, and composite types - structs, unions, arrays, and
typed pointers print their members in gdb. Debug info is strictly additive:
`-g` adds only `.debug_*` sections and never changes machine code, enforced
byte-for-byte in CI at both debug and release profiles. Also fixes a critical
comptime field-gate defect and the dep resolver's misleading unresolved-ref
error. Built with mach 2.14.1.

### Added

- debuginfo: **full DWARF 5 for ELF targets** under `-g` (or per-profile
  `debug = true`), format-agnostic producer behind the `DebugInfo` vtable
  seam. Source locations ride IR and MIR instructions from lowering through
  encoding; the encoder emits a PC/line row table with branch-relaxation
  re-homing (#1689, #1691, #1692, #1693, #1697).
- debuginfo: `.debug_line` line tables and compile-unit scaffolding;
  `DW_TAG_subprogram` and base-type DIEs with `.debug_str` interning
  (#1699, #1703).
- debuginfo: variable and parameter locations - `dbg.value`-style bindings
  survive the optimizer (RAUW, DCE, salvage; debug-only uses never keep a
  value alive), render as single-location expressions under the
  agree-or-omit stationary-home rule, PC-ranged `.debug_loclists` for
  re-bound variables, and computed constants (`DW_OP_stack_value`) for
  folded locals. A variable never shows a home it does not hold; unknown
  renders as absent, never garbage (#1695, #1696, #1704, #1705, #1706).
- debuginfo: inlined frames - `DW_TAG_inlined_subroutine` with abstract
  instances, `call_file`/`call_line`, exact multi-extent `.debug_rnglists`,
  nested instances under their parents, and inlined locals scoped to their
  instance. Backed by an inline-site identity channel threaded IR→MIR→encoder
  (#1707, #1924).
- debuginfo: composite types - struct/union/array/member/pointer DIEs with
  member offsets taken from the same IR layout codegen used; generic
  instances materialize concrete field tables; aggregate variables locate
  by reference (frame-homed print members; register-homed conservatively
  omit) (#1919).
- debuginfo: a dedicated CI lane (`dbg/`) verifying every PR on all three
  ELF ISAs at debug **and** release profiles: `llvm-dwarfdump --verify`,
  addr2line spot checks, byte-additivity of loadable segments, and
  falsifiable per-feature assertions (#1698).
- link: debug sections carry through all three object writers (ELF, Mach-O
  `__DWARF`, COFF `DISCARDABLE`) and merge into executables (#1694).
- link: unattributed dynamic imports are a hard error on two-level-namespace
  formats instead of a silent mis-link (#1800).
- driver: `mach init` scaffolds the target's native ABI via the OS vtable
  (#1837).

### Fixed

- codegen: `-g` no longer changes machine code - the compiler's own build
  is byte-identical with and without `-g` at both debug and release. Four
  independent causes fixed: the inliner's size gate counted debug-value
  instructions, mem2reg treated a debug-value alloca reference as an
  address escape, param homing counted a debug value as a real parameter
  use, and DWARF metadata perturbed layout via stream-resident markers
  (replaced by attached per-instruction records). Enforced by falsifiable
  CI byte-identity assertions at release (#1944, #1956, #1932).
- codegen: a zeroed source location could alias the entry module's first
  byte, misattributing a `.debug_line` row to an unrelated module -
  `FILE_NIL` is now 0 so zeroed memory is invalid by construction, and the
  x64 NEG/NOT expander stamps loc/inline-site on its expansion pieces
  (#1902).
- codegen: subprogram DIEs with no children use `DW_CHILDREN_no` leaf
  abbrevs - 60 dwarfdump warnings to zero (#1949).
- sema: a comptime `$if` gate reading a `$each` field descriptor
  (`f.name`/`f.type`/`f.offset`) defers to the unroll where the loop
  variable is bound, and sema installs the field resolvers, so only the
  live arm is type-checked - fixes bogus "no such field" errors and
  rejected `f.name` string comparisons in `$fields` loops (#1923).
- driver: an unresolvable git dependency ref now reports
  `fatal: invalid reference` instead of git's misleading pathspec error;
  slashed branch refs were already handled (#1890).
- ir: dbg-value salvage in batch erasure gates on debug-value presence
  instead of variable metadata, so a metadata-free debug value can never
  skip salvage and dangle past its dropped definition (#1958).

### Changed

- ir: `Function` construction is a single `function_new` constructor - new
  fields initialize in exactly one place (three `-g`-only segfaults this
  release traced to the old three-site pattern) (#1906).
- ir: batch instruction erasure (`erase_marked`) is the single free path
  with dbg-value salvage built in (#1690, #1895).

## [2.14.1] - 2026-07-04

Hotfix for two defects in 2.14.0.

### Fixed

- codegen: **critical** - a module-global aggregate passed by value lowered to
  its bare pointer-typed storage address, so the call ABI classified it as a
  scalar and passed the address instead of copying the record's bytes - garbage
  values in the callee, debug and release. Global aggregate rvalues now retype
  to the aggregate IR type, identical to locals (#1925).
- link/elf: `PT_GNU_RELRO` declares the actual `.data.rel.ro` extent instead of
  the image max-page-rounded size, so the header matches what the runtime maps
  and re-protects (#1885 follow-through; the startup-crash half shipped via
  mach-std).

## [2.14.0] - 2026-07-03

The incremental compilation back half and a hardening sweep. The full pipeline
now runs through the query engine (lowering, codegen, and link cache per
module), riscv64 becomes a self-hosting target with a byte-identical fixpoint
after a critical phi-width miscompile fix, and the RELRO program completes:
`.data.rel.ro` splits from read-only data, aarch64 images align to 64 KiB
max-page-size, and the mach-std runtime treats re-protection failure as fatal.
`mach check` is removed and `mach clean` added. Contains breaking CLI and
test-collection changes - see Breaking. Built with mach 2.13.0.

### Breaking

- cli: the `mach check` command is removed, including its `--format json`
  surface added in v2.13.0 — a parse-only check reports success on code that
  cannot build; use `mach build` (#1860).
- test: `mach test` collects tests from the whole `[project].src` tree instead
  of the entrypoint's import closure, so a `pub` module with no in-tree
  consumer keeps its tests; projects with previously-orphaned modules may
  collect more tests than before. Target-gated modules (a top-level comptime
  `$error` guard, or orphan modules depending on one) are excluded with a
  visible `skipped N target-gated modules` note (#1863, #1873).

### Added

- driver: **the incremental back half** - `Q_LOWER`, `Q_CODEGEN`, and `Q_LINK`
  are bound into the query engine. Lowering and codegen cache per module keyed
  on stable module identity; the link records a complete fingerprint (per-module
  images, content-hashed external inputs, output path, link-affecting flags,
  target/profile) and reports whether a re-link is needed. Cold builds are
  unchanged; warm sessions reuse every phase (#1618).
- driver: a reused cached phase replays the diagnostics it produced when
  computed, so warm-session rebuilds report the same errors and warnings as a
  cold build (#1586).
- driver: session source-overlay API - live (unsaved) buffer text shadows
  on-disk content as the query engine's source input, keyed by path; the
  byte-equality cutoff re-parses exactly the overlaid module (#1588).
- driver: a lowered-surface fingerprint firewall for `Q_LOWER` - an
  implementation-only edit to a dependency no longer re-lowers its importers;
  only material that can monomorphize into an importer (generic and
  comptime-template bodies, comptime values, type layouts, signatures)
  invalidates (#1859).
- link/elf: `.data.rel.ro` is a first-class section kind - reloc-bearing
  read-only globals split from `.rodata`, which is now mapped read-only from
  load under `--pie` instead of writable-then-reprotected; `PT_GNU_RELRO`
  covers exactly the relocated constants (#1844).
- link/elf: target-parameterized max page size - the writer's p_align, header
  reservation, and file-offset congruence come from the OS vtable's per-arch
  `page_size_for` (64 KiB on linux-aarch64, matching lld's max-page-size), so
  aarch64 images load on 16K/64K-page kernels; 4K-page targets are
  byte-identical (#1845).
- cli: `mach clean` - removes the output directory trees declared by the
  manifest's `out`/`obj`/`ir`/`asm` templates; idempotent, manifest-driven,
  no flags (#1830).
- infra: a riscv64 self-host smoke runs in CI - the cross-built riscv64
  compiler executes under qemu to compile a multi-block case, locking in
  self-host support (#1852; `self-host: true` int-case key).
- cli: `mach init` derives each scaffolded target's abi from the OS's
  self-declared native calling convention (`native_abi` on the OS vtable) and
  skips OSes with no composable tuple on the host - a riscv64 host scaffolds
  riscv64, not x86_64 (#1837).

### Fixed

- codegen: **critical** - out-of-SSA phi merge moves materialized immediate
  sources at full register width, defeating riscv64's width-4 sign-extension
  re-canonicalization; a negative i32 phi constant read positive in 64-bit
  signed compares, which miscompiled the compiler's own register allocator on
  riscv64. With the fix the riscv64-hosted compiler self-hosts with a
  byte-identical fixpoint (#1851).
- driver: test-mode `main`-neutralization no longer mutates query-owned IR in
  place; the test variant is cached under its own query key, so cached IR is
  immutable by construction (#1858).
- cli: `mach info` reports `riscv64` instead of `unknown` for the host
  architecture on riscv64 builds (#1853).

### Removed

- cli: the dead `pub fun build` wrapper in the build command (#1840).

## [2.13.0] - 2026-07-02

A follow-up hardening, cleanup, and tooling sweep over the 2.12.0 overhaul.
`--pie` images regain read-only hardening for their relocated constants
(`PT_GNU_RELRO`), the register allocator sheds redundant copies for a smaller
binary, `mach test --format json`, `mach check --format json`, and `mach info
targets` add machine-readable surfaces, diagnostics gain `= help:` lines and
secondary spans, and out-of-range integer literals now report their bounds.
Contains breaking CLI, manifest, and source-acceptance changes - see Breaking.
Built with mach 2.12.0.

### Breaking

- sema: an integer literal that overflows a signed destination is a compile
  error; it previously compiled and silently wrapped (#1804).
- driver: unknown flags are hard errors on every command; `-h` / `--help` are
  not flags (use `mach help <command>`); the `--` separator is honored only by
  `mach run` (#1810).
- cli: the `--color` and `--artifacts` flags are removed (#1784, #1829).
- target: incoherent tuples are rejected at composition - an object format that
  does not cover the ISA, an ABI that does not target it, or an OS that cannot
  load the format (`windows`+`elf`, or `raw` on any real OS - flat images are
  `os = "freestanding"`) (#1806).
- build: `-o` cannot be combined with `--all-targets` (#1831).

### Added

- link/elf: **`PT_GNU_RELRO` under `--pie`** - relocated constant pointers
  (vtables, `val` pointer globals) that live in read-only data are writable while
  the static-PIE image self-relocates at startup, then `mprotect`'d read-only
  before `main`; a write through a relocated constant now faults. The linker
  emits the page-rounded header over the single read-only reloc-bearing segment,
  and the mach-std runtime re-protects it after applying its `RELATIVE`
  relocations (#1778).
- test: **`mach test --format json`** - a versioned, machine-readable NDJSON
  event stream (`run_start` / `test` / `summary`, plus `case` under `--list`;
  `schema: 1`) for editors and CI, documented in `doc/tooling/test-json.md`. JSON
  goes to stdout and build diagnostics to stderr; strings are `ensure_ascii`
  escaped so the stream is byte-identical across platforms; labels are emitted
  verbatim; `test` events arrive in **completion order** (sort by `index` for
  declaration order) (#1792).
- check: **`mach check --format json`** - a versioned, machine-readable NDJSON
  diagnostic stream (a `diagnostic` object per diagnostic carrying severity,
  message, primary location, note, help, and the LSP-shaped related list, then a
  closing `summary`; `schema: 1`) for editors and CI, documented in
  `doc/tooling/check-json.md`. JSON goes to stdout with no human frames
  interleaved; positions are 1-based and spans half-open; it reuses the
  `mach.cli.json` emitter, which grew nested-object and array support to carry
  the model (#1815).
- diag: diagnostics gained a distinct **`= help:` line** and an optional
  **secondary span** - a duplicate definition now points at the prior binding in
  its own `--> file:line:col` frame labelled `previous definition here`, and
  did-you-mean suggestions moved from `= note:` to `= help:`. Both are structured
  fields on the shared location model, ready for the coming
  `mach check --format json` consumer (#1783).
- driver: **`mach info targets`** prints the supported `<os>-<isa>` target
  matrix, derived from per-vtable capability declarations (each object format's
  ISA relocation coverage and each ABI's ISA) rather than a curated list, so a
  new target appears by declaring its capability (#1806).

### Changed

- codegen: **regalloc copy hygiene** - a dead-def sweep and copy-source
  allocation hints in the linear-scan allocator, backed by tighter live-range
  extension, eliminate redundant register moves. On mach's own release build the
  executable shrinks ~-4.0% and redundant register-to-register self-moves drop
  -88% (#1801).
- target: **incoherent target tuples now fail at composition** - `select_of`
  enforces the joint object-format/ABI capability at selection, so a tuple whose
  object format does not cover the ISA's relocations (e.g. `windows`+`aarch64`,
  COFF being x86_64-only) or whose ABI does not match the ISA is rejected with a
  message naming the missing capability, instead of composing and failing deep in
  codegen or link (#1806).
- driver: **`mach init` scaffold target names** follow the `<os>-<isa>`
  convention (`linux-x86_64`, `windows-x86_64`, `darwin-x86_64` on the host isa),
  matching the root manifest's normalized naming. A fresh project now writes
  `out/linux-x86_64/…` rather than `out/linux/…` (#1832).
- cli: **JSON emission moved to `std.data.json`** - the streaming NDJSON emitter
  that backed `mach test --format json` and `mach check --format json` (the
  `mach.cli.json` module) now lives in mach-std, unified with the tree emitter
  behind one escape core. `src/cli/json.mach` is deleted and both consumers
  (`testing.mach`, `diagnostic.mach`) import `std.data.json`; the mach.lock
  mach-std bump carries the moved code. Both JSON streams are byte-identical
  before and after (mach-std#338).

### Removed

- cli: the orphaned **`--color` flag** and its `ColorMode` surface, dead since
  the ASCII-only diagnostic renderer (#1777) dropped the color argument -
  `--color` had parsed but no-opped (#1784).
- build/run/test: the inert **`--artifacts <dir>` flag** and its `Config`
  field - it parsed but no code ever read it, so it silently no-opped. The
  object-tree location is expressed by the manifest `obj` template
  (`out/{target}/{profile}/obj`), which expands per target/profile/artifact; a
  flat CLI override cannot and would collide every target's objects into one
  directory (#1829).

### Fixed

- driver: **`--emit-ir` dumps the final post-pipeline IR** - the dump moved from
  the lower loop to the codegen loop, beside `--emit-asm`, so the `.ir` reflects
  the optimized module the objects are built from - it now varies with `-O` and
  picks up test-mode `main` neutralization, instead of the naive pre-pipeline
  lowering (#1802).
- sema: **out-of-range integer literals report their bounds** - a literal that
  does not fit its destination now reports
  `literal <v> is out of range for <T> (<min>..<max>)` instead of a generic type
  mismatch whose cast note advised the truncating `value::Type`. A literal that
  overflowed a signed slot (e.g. `val mask: i64 = 0xFFFFFFFFFFFFFFFF;`) used to
  compile and silently wrap at runtime; it is now rejected, while `u64` slots
  keep their legitimate maximum (#1804).
- link/elf: **the ELF exec writers refuse a program-header table that overflows
  its reserved header page** - the PIE and dynamic writers extend the first
  `PT_LOAD` down exactly one page to cover the ELF header + program headers, which
  only maps segment 0 at its vaddr while the header block fits that page
  (`phdr_count <= 72`). Past that the image was silently unloadable; the writers now
  return a link error instead. Latent (real builds emit ~9 headers), owning the
  invariant before segment counts grow (#1814).
- build: **`mach build --all-targets -o <path>` is now rejected** - `-o` names a
  single artifact path, so combined with `--all-targets` every target linked to
  that one path and overwrote the previous, leaving only the last target's binary
  with no warning. The combination now errors (`-o cannot be combined with
  --all-targets`) at parse; single-target `-o` is unchanged (#1831).
- driver: **unknown flags are rejected instead of silently misparsing** - an
  unrecognized flag was never reported and hijacked positional resolution
  (`mach build --color always .` resolved `always` as the project path). Each
  command now marks the flags it consumes and rejects the first unmarked
  `-`-prefixed token before resolving positionals - `error: unknown flag '<flag>'
  for '<command>'`, exit `1`. `-h` / `--help` are not flags; use `mach help
  <command>`. The `--` end-of-flags separator is honored only by `mach run`, which
  forwards every post-`--` token to the executed program as its `argv`; on every
  other command `--` is itself rejected as an unknown flag (#1810).

## [2.12.0] - 2026-07-01

The terminal output & test-harness overhaul: framed source-snippet diagnostics,
a per-phase build readout, and `mach test` rebuilt around a single dispatcher
binary with live, parallel, deterministic reporting (epic #1788 - on mach's own
438-test suite, `mach test .` drops 44.9s -> 7.2s wall and 9.4 GB -> one 7.3 MB
artifact). Static-PIE self-relocation is now active under `--pie`. Contains
breaking CLI changes - see Breaking. Built with mach 2.11.0.

### Breaking

- build/test: the `--verbose` flag is gone; use `-v` / `-vv` (#1775).
- test: `--runner <cmd>` now launches `<cmd> <exe> <idx>` - the runner receives
  the dispatcher path and a test index instead of a per-test executable (#1789).
- test: the `tests` manifest template's `{name}` resolves to the product name
  (one dispatcher binary) rather than a per-test name (#1789).

### Added

- diag: a framed source-snippet renderer for driver diagnostics - each
  reported error/warning shows its source line in a frame with a caret span,
  file:line:col header, and severity-tagged message, replacing the one-line
  flush (#1777).
- test: a per-module roll-up readout for `mach test` - all-passing modules
  collapse to one line, failures expand with the child's captured stdout,
  `file:line`, and exit code or signal, and the run ends with a summary that
  re-lists the failures; a crashing test reports its signal and the run
  continues (#1776). Extended to a live, parallel readout later in this
  cycle (see the `Changed` entries).
- link: static-PIE self-relocation is active - under `--pie` the mach-std
  runtime relocates the image's absolute pointers at startup, covered by an
  int exec guard (#1727).
- build: `-v` prints a per-phase readout (load / resolve / sema / lower /
  optimize / codegen / link) with item counts and timing, closed by a
  `built <path>  N modules  <size>  in <time>` summary; `-vv` adds a
  per-module/file line under each phase with its own duration and a `(slow)`
  marker on the slowest item. Fixed-width ASCII on stderr, identical across
  platforms - timing via `chrono.monotonic`, durations via
  `chrono.format_duration`, columns via the `{:<N}`/`{:N}` format spec (#1775).

### Changed

- test: `mach test` runs tests **in parallel** - a sliding window of `--jobs`
  child processes (default: the CPUs available to the process; `--jobs 1`
  serializes), reaped with a blocking wait-any. Each child's stdout *and
  stderr* are captured to a per-test file under `log/` beside the dispatcher;
  a passing test's file is removed on reap, a failing test's stays (the
  expanded failure shows the first 64KB, a `full output:` pointer when
  truncated, and the exact `rerun: <exe> <idx>` command). Results render in
  collection order regardless of completion order, so the readout is
  deterministic (#1791).
- test: the `mach test` readout is **live** - each module's roll-up prints the
  moment its last test completes, and failures expand as they happen. Column
  widths are computed from the collected tests (clamped) instead of hard-coded,
  test labels print verbatim as declared (the old `<module>.`-prefix stripping
  is gone), durations right-align, and the closing summary reports the run's
  wall time. `-v` prints each test's line as it completes; `-vv` now also
  prints passing tests' captured output (it was a silent alias of `-v`);
  `mach test --help` documents both (#1790).
- test: `mach test` builds **one dispatcher executable** covering every
  collected test instead of one standalone executable per test - one link
  instead of N (on mach itself: 44.9s → 7.2s wall, 9.4 GB → 7.3 MB on disk).
  Each test still runs as its own process, spawned as `<exe> <idx>`. `--runner`
  now receives the executable path and the test index as its two arguments;
  `--filter` selects at run time, so the built executable is identical
  regardless of filter; the `tests` template's `{name}` resolves to the product
  name (falling back to the project id) rather than a per-test name (#1789).

### Removed

- build: the `--verbose` flag, replaced by `-v`/`-vv` (#1775).

### Fixed

- ci(int): the darwin-x86_64 integration leg never ran - it queued forever on
  starved `macos-13` runners and died at GitHub's 24h cap; routed to `macos-14`
  under Rosetta 2, mirroring CD (#1787).

## [2.11.0] - 2026-06-30

Position independence and multi-arch ELF dynamic linking: opt-in static-PIE
executable emission for linux, PLT/GOT writers for aarch64 and riscv64, plus
editor groundwork and a loud-error sweep over the backend's silent fallbacks.
Built with mach 2.10.0.

### Added

- link/elf: opt-in **static-PIE** (`ET_DYN`) linux executables via `--pie` /
  `$mach.build.pie` - `PT_PHDR` load-bias recovery, a synthesized `.rela.dyn`
  of `R_*_RELATIVE` base relocations, and `PT_DYNAMIC`; runtime self-relocation
  activates in the next release (#1727).
- of/elf: aarch64 and riscv64 **PLT/GOT dynamic-link writers** with byte-level
  encoding guards, completing dynamic linking across the shipped linux ISAs
  (#1741).
- editor: sema retains per-expression types on `SemaResult` for editor/LSP
  consumption (#1501).
- ci: an x86_64-darwin release lane via Rosetta 2 on `macos-14` - Intel
  runners are queue-starved (#1728); darwin integration legs run on merge to
  main rather than a schedule (#1765).
- int: structural (field) and freestanding (flat-loader) producers widen what
  the integration harness can assert (#1760).

### Fixed

- be: silent backend fallbacks now fail loudly, with the AAPCS64 edge rules
  documented where they were being papered over (#1745).

## [2.10.0] - 2026-06-29

Native arm64 macOS. mach now compiles, ad-hoc code-signs, and self-hosts
position-independent arm64 Darwin executables that run on Apple Silicon - the
first published Darwin release binary. Also a format-neutral base-relocation
foundation for position independence, project-scoped `mach test`, `mach run`
without a rebuild, a riscv64 ELF build-attributes section, and a parser
correction. No breaking changes.

### Added

- target: native **arm64 Darwin (Apple Silicon)**. mach emits position-independent
  Mach-O executables (`MH_PIE`, `LC_MAIN`, an `LC_DYLD_INFO` rebase stream, and an
  ad-hoc `CS_LINKER_SIGNED` code signature) that exec and self-host on Apple
  Silicon; CD publishes an `aarch64-darwin` archive (#1679, #1717, #1722). x86_64
  Darwin stays cross-compile-only (#1728).
- link: a format-neutral image base-relocation set - the linker collects every
  in-image absolute pointer once and each object format encodes it, the foundation
  for PIE/ASLR across targets (#1722).
- arm64: `cset` in the inline-asm assembler (#1714).
- target: a riscv64 `.riscv.attributes` ISA-string ELF section (#1673).
- run: `mach run` executes the already-built artifact without recompiling (#1482).
- test: `mach test` collects only the current project's tests by default; pass
  `--include-deps` to widen (#1556).

### Fixed

- macho: coalesce object sections by kind so modules with more than 255 sections
  link (Mach-O's `n_sect` is a single byte) (#1682).
- macho: map the mach header into `__TEXT` so the Apple Silicon kernel admits the
  signed image (#1717).
- parser: require a block body for `fin`; a bare `fin stmt;` was never intended to
  parse (#1548).

### Changed

- deps: mach-std v0.16.2 - the arm64 Darwin runtime (`LC_MAIN` entry, syscall layer).

## [2.9.0] - 2026-06-27

Darwin executables, the constant-time secret qualifier in the type checker, and
the riscv64 backend hardened against real code. macOS now has working dynamic
executables, completing both Darwin triples, and a sweep driven by compiling and
running real std under qemu closed every riscv64 codegen gap real programs hit.
No breaking changes.

### Added

- target: a macOS / Darwin OS substrate plus Mach-O dynamic executables (an
  `emit_dyn_exec` that writes `LC_LOAD_DYLINKER`, `LC_LOAD_DYLIB`, an
  `LC_DYLD_INFO_ONLY` bind stream, `LC_SYMTAB`/`LC_DYSYMTAB`, `LC_UNIXTHREAD`,
  and per-arch import stubs), completing the `x86_64-darwin` and `aarch64-darwin`
  cross-compile triples. Byte-verified static and dynamic for both (#1178).
- target: riscv64 RV64A atomic inline-asm mnemonics, `lr`/`sc`, the `amo*`
  family, the `.aq`/`.rl`/`.aqrl` ordering suffixes, and `pause`, so atomic
  read-modify-write can be expressed for the riscv64 std atomics and runtime
  (#1668).
- sema: constant-time secret-qualifier flow typing. The `^` qualifier carries a
  public-to-secret lattice (public coerces up, any secret operand taints the
  result), the type checker gates what a leakage model can observe (no secret
  branch condition, memory index, or variable-latency operand), `:^` is the only
  downgrade, and secret pointers are welded and non-launderable. Type-checking
  only, with codegen taint a later step (#1645, epic #1643).

### Fixed

- codegen(riscv64): long-branch relaxation. An out-of-range B-type conditional
  becomes an inverted guard plus a `jal`, and an out-of-range `jal` becomes an
  `auipc`+`jalr` trampoline, resolved by a per-function relaxation fixpoint.
  Large functions no longer fail to encode (#1666).
- codegen(riscv64): local frame slots no longer overlap the saved ra/s0 record,
  fixing a silent SIGSEGV for any function with an address-taken or spilled local
  (#1670).
- codegen(riscv64): 32-bit `and`/`or`/`xor` now encode full-register ops instead
  of nonexistent word-group instructions, fixing a SIGILL (#1672).

## [2.8.0] - 2026-06-26

The first working bare-metal build, plus a riscv64 codegen fix surfaced by the
new backend. The freestanding `raw` object format now has a real flat-image
build path, so an `os=freestanding, of=raw` target builds end to end.

### Added

- build: a flat-image build path for image-producing object formats. An object
  format declares through a `produces_image` predicate whether it round-trips
  through relocatable `.o` files, and an image format (the freestanding `raw`
  writer) now links straight from the in-memory codegen output, bypassing the
  per-module `.o` emit/parse round-trip. This is the last gap to a real
  `os=freestanding, of=raw` build (#1616).

### Fixed

- codegen: a riscv64 variable-amount shift with a constant value operand
  (`1 << node`) used as a call argument now materializes the constant into a
  register before the shift instead of failing to encode. The shift was the lone
  reg-reg ALU encoder not materializing an immediate operand (#1657).
- diag: the unknown-os diagnostic now lists `freestanding` among the expected
  values (#1617).

## [2.7.0] - 2026-06-26

First targets on the retargeting foundation. riscv64 lands end to end as the
register-machine validator (substrate authoring plus vtable hooks, no
shared-backend edits), Mach-O joins ELF and COFF as an object format, and the
constant-time work begins with the secret type qualifier in the front end.
Releases now ship optimized (opt=2) binaries. No breaking changes, and existing
x86_64 and aarch64 builds are unaffected.

### Added

- target: a full riscv64 (RV64) isa backend (machine model, instruction
  selection, encoder, relocations) and an lp64/lp64d ABI, composing the
  `riscv64-linux` cross-compile target. Byte-verified against llvm-mc and run to
  its exit code under qemu (#1172).
- target: a riscv64 inline-asm assembler so `asm riscv64 { ... }` blocks encode
  to RV64 bytes, including `ecall`. The cross lane now runs a qemu exit-code e2e
  (#1642).
- target: a Mach-O object format (writer, reader, static executable) for x86_64
  and arm64, keyed on the isa with no new shared backend hook. Cross-compile
  byte-verified for the darwin triples (#1176).
- frontend: the secret type qualifier `^` (`^u32`, `*^u8`, `[N]^u8`, `^MyRec`)
  and the `:^` strip-cast token. Lexer, parser, and grammar only, the
  de-risking first slice of the constant-time guarantees epic (#1643); no
  type-checking behavior yet (#1644).
- build: `[profile.debug]` and `[profile.release]` profiles with
  profile-segmented output paths, so debug and release builds no longer clobber
  each other. Releases build with `--profile release` (opt=2) gated by a
  self-host fixpoint, and a release-profile CI lane catches -O2 regressions on
  PRs (#1591, #1592).

### Changed

- link: the relocation and ELF-class seam is widened onto the substrate, so a
  new isa owns its reloc packing and object-format mapping rather than editing
  shared linker and ELF code (#1625, #1635).

### Fixed

- test: `mach test` bounds per-test link memory with a per-test scratch arena
  reset after every test. Memory was O(test count) through the session arena and
  tripped the windows runner ceiling at `coff: unwind table alloc failed` once
  the suite grew (#1653).
- codegen: riscv64 sub-word loads read the source width rather than the clamped
  width, fixing a sub-word load miscompile (#1639).
- codegen: `fp_class_index` finds the float bank by RegClass kind instead of
  assuming a >=128-bit float bank (#1624).
- docs: `doc/language/files.md` no longer misframes `main()` as compiler-known
  (#1631).

## [2.6.0] - 2026-06-26

Retargeting foundation. A compilation target is now a composition of named
substrates (isa, abi, os, object-format) carrying a by-value machine model, so a
new register-machine target plugs in as substrate modules plus vtable hooks with
no shared-backend edits. Adds the first bare-metal capability: a freestanding os
and a raw flat-image object format. No syntax changes, and existing x86_64 and
aarch64 builds are unaffected.

### Added

- target: a `freestanding` os substrate (no syscalls, no OS runtime, custom
  `_start`, image base 0) and a `raw` flat-image object-format writer that lays
  each segment at its virtual address and emits the bytes with no container,
  entered at the image base (#1613).
- target: object-format selection by name. An optional `of` key on a
  `[target.<name>]` table picks the object format explicitly (e.g. `of = "raw"`);
  omitting it derives the format from the os default, as before (#1615).
- target: a `MachineModel` record carrying the machine description (widths,
  register file and classes, addressing modes, frame model) as data, read by the
  shared backend stages (#1606).

### Changed

- target: targets compose by name through name-keyed substrate registries. The
  legacy os/arch/abi/of ids are derived from the resolved substrates rather than
  authored, and the central `canonical_*` switches are gone (#1609).
- codegen: lowering, register allocation, and frame layout read the widths, the
  register file, and the index/address width from the machine model instead of
  shared constants (#1603, #1607, #1608). Behavior-preserving for the existing
  targets.
- link: each isa owns its relocation packing through an `apply_reloc` vtable hook
  and declares its own ELF `e_machine`, replacing the `arch_id` switches in the
  linker (#1610, #1612).
- target: removed the dead `syscall_layer` os field and consolidated
  primitive-type classification behind a `prim_desc` table (#1604, #1601).

## [2.5.9] - 2026-06-24

Fixes a latent miscompile: a runtime array/pointer index narrower than the
machine word (e.g. a `u8`) was passed to the GEP at its source width, so the
back end scaled an index register carrying undefined high bits and computed a
wild address.

### Fixed

- lower: `lower_index_lvalue` widens a sub-word runtime GEP index to the
  machine-word index type (zext unsigned / sext signed) before `emit_gep`, so
  the back end scales a clean register. In-tree index sites cast (`::u32` /
  `::usize`) and masked it; an uncast sub-word index miscompiled (#1596). The
  64-bit widen lives in the target-agnostic lowering for now; #1598 tracks
  moving index normalization into the target-aware codegen.

## [2.5.8] - 2026-06-24

Incremental type-checking (#1164), completing the incremental front-end: parse,
name resolution, and now sema all run as memoized queries, so an edit re-checks
only the changed module plus dependents whose typed public surface moved.

### Added

- sema: `Q_SEMA` (owned-handle query keyed by StableModuleId) with a
  `Q_TYPED_EXPORTS` fingerprint firewall — an impl-only edit firewalls type
  dependents while a public-type change re-checks them. `Q_MODULE_NUMBER` gates
  re-check on a graph reshape that renumbers a module's nominal TypeIds.

### Fixed

- type: field tables (keyed by TypeId) are rebuilt per build via a field-table
  epoch, so a record's field-type change on a long-lived session no longer
  leaves a stale layout (latent; fresh-session builds were unaffected).

## [2.5.7] - 2026-06-23

Incremental name resolution (#1164). The front-end is now query-engine-driven
through resolve: an edit re-resolves only the changed module plus importers
whose public surface actually moved, instead of the whole closure.

### Added

- query: parsing and name resolution run as memoized queries (`Q_PARSE`,
  `Q_RESOLVE`) with owned pointer-graph handles; a `Q_EXPORTS` fingerprint of
  each module's resolved public surface is the early-cutoff firewall, so a
  private-body edit does not re-resolve importers while a public-signature or
  public comptime-const-value change does. Build-stable `StableModuleId`s key
  the cached results across rebuilds.

### Fixed

- parser: a function's `DeclId` is reserved before its body is parsed, so a body
  edit no longer shifts the function's id (which silently staled importers
  indexing it).
- driver: each module's `Q_EXPORTS` is refreshed in topological order during the
  resolve pass, so an importer is never validated against a stale dependency
  fingerprint.

## [2.5.6] - 2026-06-23

Incremental parse caching (first increment of #1164): unchanged modules are no
longer re-parsed across builds, so a one-file edit's rebuild re-lexes/re-parses
only the changed file instead of the whole reachable closure.

### Changed

- driver: parsed ASTs are cached in a session-owned `AstCache` keyed by
  `(FileId, source revision)` and reused across builds; `parse_module` skips
  tokenize+parse on a cache hit. `ModuleEntry` now borrows its AST from the
  cache (the cache is the sole owner; `dnit_project` no longer frees it), so the
  cache survives a project teardown and rebuild.
- source: `update` no longer bumps a file's revision when the new text is
  byte-identical to the old, so an unchanged file keeps its revision (and stays
  a cache hit) across rebuilds.

## [2.5.5] - 2026-06-23

`mach doc` now reads first-class documentation, so the compiler, the docstring
lint, and editor hover all share one doc source.

### Changed

- doc: `mach doc` sources documentation from each decl's `Decl.doc` span via
  `mach.lang.fe.doc` (walking the parsed AST, recursing into comptime branches)
  instead of re-scanning raw source text. `#[attr]` decorator lines no longer
  leak into rendered docs, and component whitespace is normalized. Content after
  `# ---` that is not a valid `name: desc` component is dropped, matching the
  lint and hover.

## [2.5.4] - 2026-06-23

First-class documentation. Doc-comment blocks are now captured on declarations
and validated by a docstring lint, laying the groundwork for editor hover and
`mach doc` to share one source of truth.

### Added

- fe: capture each declaration's doc-comment block as a span on `Decl`
  (attribute-aware — `#[...]` lines between the docstring and the decl are
  skipped), plus `fe/doc.mach`, an allocation-free doc-block parser exposing the
  summary and component lines with their name/description spans.
- fe: a `pub`-scoped docstring lint (`fe/doclint.mach`) that warns when a
  documented component names no real parameter/field/generic/`ret`
  (misspelled), is out of declaration order (judged over the documented subset,
  so partial docs are fine), or has an empty description. It does NOT require
  completeness — undocumented items are never flagged.

### Fixed

- fe: the doc-block parser no longer mistakes a wrapped prose continuation line
  beginning `word:` for a component head (continuation lines are indented past
  the `# name` column).

## [2.5.3] - 2026-06-23

Front-end performance and visibility. Bumps mach-std to 0.14.1, whose
linear-time `str_region_equals` removes the quadratic-parse stall that froze the
front-end of stdlib-heavy builds.

### Added

- build: `--verbose` now streams per-module `lex`/`parse`/`resolve` lines during
  the front-end, alongside the existing `sema`/`lower`/`codegen` stages (#1567).

### Changed

- resolve: hash-index the module scope for O(1) name lookups instead of a linear
  scan (#1565).
- driver: hoist the resolve dep-closure scratch out of the per-module loop,
  removing O(modules²) allocation during the resolve pass (#1565).
- deps: bump mach-std to 0.14.1 (linear `str_region_equals`).

## [2.5.2] - 2026-06-22

Patch — const-string global references now fold correctly.

### Fixed

- lower: a `str`/pointer-typed global initialized to a reference to a const-string
  `val` (`pub val B: str = A;`) emitted the string as raw inline bytes in the
  pointer slot instead of a pointer-to-rodata relocation. The global-init folder
  now routes any initializer that comptime-folds to a string through the rodata +
  relocation path, generalizing the v2.5.1 comptime-path fix (#1559).

## [2.5.1] - 2026-06-22

Patch — global initializers now fold comptime intrinsic paths (#1557).

### Fixed

- lower: a module-level `val`/`var` initialized to a comptime intrinsic path
  (`$project.version`, `$mach.*`) that folds to a string is now lowered to a
  pointer-to-rodata relocation instead of being mis-emitted as raw inline bytes,
  so a global like `pub val X: str = $project.version;` folds correctly. The
  global-init aggregate folder routes string-valued comptime paths through
  `comptime.eval`, mirroring expression-position folding (#1557).

## [2.5.0] - 2026-06-20

Minor — a breaking CLI change to project-root resolution (#1545) and a codegen
fix for field access on struct-typed `$each` pack elements (#1549). The project
commands now resolve the project root one way only: `build`, `run`, `test`, and
`doc` each take the project root as a required positional (`mach build .`); a
bare invocation with no path is a user error, collapsing the three redundant
resolution paths (bare-cwd default, positional, and `--cwd`) that had drifted
in.

### Changed

- cli: `build`, `run`, `test`, and `doc` now **require** an explicit project-path
  positional (`mach build <path>`). A bare invocation prints
  `error: missing project path; pass the project root (e.g. '.')` and exits `1`.
  `doc` previously took no positional; it now requires one like the others
  (#1545).

### Removed

- cli: the `--cwd <path>` flag. Pass the project root as the positional instead
  (`mach build .`) (#1545).

### Fixed

- lower: field access (`a.x`) or address-of (`?a`) on a struct-typed `$each` pack
  element fabricated an extern global named after the loop variable, failing at
  link with `undefined symbol`. `lower_ident_lvalue` now resolves a pack element
  to its expanded-parameter storage slot, mirroring the rvalue path (#1549).

## [2.4.1] - 2026-06-20

Patch — windows git dependency resolution (#1538) and a multi-target union-build
crash (#1540). The windows git path failed in two distinct ways, both now covered
by the `Windows (native)` CI lane, which resolves the vendored std with
`mach dep pull` instead of a manual clone.

### Fixed

- dep: `resolve_cmd` searched `PATH` with POSIX separators (`:` list, `/` path),
  so it never resolved an executable on windows — `PATH` is `;`-separated there
  and drive-letter prefixes (`C:\…`) embed a `:` that shattered every entry. it
  now selects the list/path separators by target os, so `git.exe` is found on
  windows as it is on linux. also fixes runner resolution in `mach run`/`mach test`
  (#1538).
- dep: git was spawned on windows with a reconstructed allowlist environment that
  omitted `SystemRoot`, so its winsock initialization failed with
  `getaddrinfo() thread failed to start` once git was found. the allowlist exists
  only for posix `execve` (which does not inherit and exposes no `environ`
  handle); on windows `CreateProcess` inherits the full parent environment
  natively, so git is now spawned with a nil child env there (#1538).
- driver: `walk_comptime_if_union` cached a `*ModuleEntry` (and AST-arena pointers
  into it) across a recursive load that can `reallocate` the `p.modules` array,
  so multi-target union builds spanning ≥3 OS/arch tuples dereferenced freed
  memory and crashed (SIGSEGV in tooling/LSP). the taken branches are now
  snapshotted before recursing, mirroring `walk_use_for_load` (#1540).

## [2.4.0] - 2026-06-19

Minor — Phase 2 (collapse) of the `#[attr]` decorator migration (#1526): `#[attr]` is now
the **only** decorator syntax; the backtick form is removed. Treated as pre-stable churn
rather than a breaking major — the backtick form shipped days ago (2.0.0) with ~0 external
adoption, and the change is purely surface-level (same Decorator AST, same codegen).
Byte-reproducible from the v2.3.0 seed; the vendored-std advance to mach-std 0.14.0 is
inert.

### Removed

- syntax: backtick decorators (`` `symbol(...)` ``, `` `library(...)` ``, `` `inline` ``,
  `` `align(N)` ``, `` `section(...)` ``). Use the `#[attr]` form (`#[symbol("...")]`,
  `#[inline]`, …) added in v2.3.0. A backtick at decorator position now reports a
  migration diagnostic — `backtick decorators were removed in v2.4.0; use #[name(...)]` —
  and recovers at the next declaration (#1535).

### Changed

- deps: the vendored mach-std advances to **v0.14.0** — the `#[attr]`-migrated standard
  library.

## [2.3.0] - 2026-06-19

Minor — Phase 1 of the `#[attr]` decorator migration (#1526): the parser now accepts
Rust-style `#[attr]` decorators **alongside** the existing backtick decorators. Purely
additive and backward-compatible — both surfaces produce the same declaration AST, and
the compiler's own source is unchanged (still backticks), so it stays byte-reproducible
from the v2.2.0 seed. The v2.4.0 collapse migrates all source to `#[attr]` and removes
the backtick form.

### Added

- syntax: `#[symbol("...")]`, `#[library("...")]`, `#[align(N)]`, `#[section("...")]`,
  `#[inline]` decorator forms. The lexer opens an attribute when `#` is *immediately*
  followed by `[` (otherwise `#` stays a line comment — a literal comment beginning
  `#[` must insert a space). `#[...]` and the backtick form attach identically; the
  decorator AST and sema/resolve/validate are unchanged (#1532).

## [2.2.0] - 2026-06-19

Minor — correctness and cross-arch coverage, hardening the compiler ahead of a manual
audit. This release changes how the compiler emits float comparisons (#1446), so it
**re-seeds** the self-host baseline: it is intentionally *not* byte-reproducible from the
2.1.0 seed (stage1≠stage2 by design) but converges to a byte-identical fixpoint
(stage2==stage3) on both x86_64 and aarch64.

### Added

- target/aarch64: AAPCS64 **HFA/HVA** classification — a homogeneous floating-point
  aggregate (1–4 members of the same FP type, counted recursively, including >16 bytes)
  is passed and returned in consecutive SIMD/FP registers (V0–V7), with all-or-memory
  spill when the run doesn't fit. Completes the aarch64 by-value float-aggregate ABI;
  x86_64 SysV and win64 emission are unchanged (#1174).

### Fixed

- codegen/x86_64: floating-point comparisons now follow **IEEE-754** for NaN, matching
  aarch64 — an unordered (NaN) operand yields false for `<`, `<=`, `>`, `>=`, `==` and
  true for `!=` (`<`/`<=` via operand-reversed `SETA`/`SETAE`; `==` as `SETE ∧ SETNP`;
  `!=` as `SETNE ∨ SETP`). Cross-arch NaN comparisons now agree (#1446).
- comptime: `$each f in $fields(T)` over a **generic** type parameter no longer errors at
  template sema — it defers to monomorphization (mirroring variadic packs), so reusable
  generic derive helpers (`debug[T]`, `equals[T]`, …) compile. A non-record instantiation
  is diagnosed at instantiation (#1523).
- ci/aarch64: the `Test corpus under qemu-aarch64` step is un-gated — `ut_manifest`
  gained the `[target.linux-arm64]` stanza so `target = "native"` resolves to the aarch64
  host under qemu (#1391) — and the Integration lane installs `qemu-user-static`, so the
  aarch64 integration suites (HFA register placement, `aarch64run`) **execute** under
  emulation in CI instead of skipping (#1464).

## [2.1.0] - 2026-06-19

Minor — comptime type-directed-dispatch ergonomics and multi-artifact tooling
graphs, surfaced while building the mach-std `{}` formatter and loading
multi-binary projects in mach-lsp.

### Added

- driver: the long-lived tooling union (`build_project_union`) now unions the
  import closure over **every declared artifact** (the full target × artifact
  matrix), not just one — a multi-artifact project (`[bin.*]` + `[lib.*]`) loads
  its whole module graph in tooling/LSP instead of erroring on artifact
  ambiguity. Single-artifact builds and the `mach build`/`mach run` ambiguity
  guard are unchanged (#1505).
- comptime: `$error("msg")` is now a real compile-time directive — it fails the
  build with its message when reached on a live path (an unconditional position
  or a selected `$if`/`$or` arm; a dead arm's `$error` never fires) and is valid
  in both declaration and statement scope. It was previously parsed as a silent
  no-op (#1511).

### Changed

- comptime/sema: a `$type_of`-comparison `$if`/`$or` chain now type-checks only
  the **selected** arm, pruning provably-dead arms at monomorphization.
  Type-directed dispatch over a concrete type no longer needs per-arm identity
  casts and is statically total (#1511).

## [2.0.1] - 2026-06-19

Patch — parser error-recovery and a grammar-doc fix surfaced while migrating the
mach ecosystem (blit, mach-glfw, mach-sieve, …) to 2.0.0.

### Fixed

- parser: a removed-syntax error at declaration scope — a comptime attribute
  setter (`$sym.attr = value;`) or a C-style `...` variadic parameter — now
  recovers at the **next** declaration instead of skipping it and reparsing its
  body at module scope, which sprayed a spurious "expected a declaration" for
  every non-`val` body statement. `sync_to_decl` no longer advances past a token
  that already begins a declaration; a regression suite (`tests/recovery`)
  guards both cases.

### Documentation

- grammar.md: removed the stale C-style `...` variadic **parameter** production
  (rejected since 2.0.0) — the comptime variadic pack `name: ...` is the only
  declaration form; function **pointer types** still carry `...` for FFI (#1518).

## [2.0.0] - 2026-06-19

**BREAKING.** The comptime collapse completes the v1.7 metaprogramming arc and is
the first new-only, post-migration release — hence the major version bump from
the 1.7.0 transition, which kept the legacy surfaces resolving for seed
compatibility. Code that still uses `$mach.target.*`, the top-level `$target.*`
root, `$<sym>.*` attribute setters, or C-style varargs must migrate (see Removed).
The self-host fixpoint holds — stage1==stage2==stage3 byte-identical on x86_64 and
aarch64 (the compiler binary shrinks as the legacy paths are deleted, but it
reproduces itself exactly).

### Removed

- comptime: the legacy `$mach.target.*` and top-level `$target.*` namespace
  spellings — build facts read through `$mach.build.*` and the declared target
  tuple through `$project.target.*` (#1480).
- comptime: the C-style varargs feature — the trailing `...` parameter marker,
  the `variadic` signature flag, `va_arg`/`va_start`/`va_end`, the `va_list`
  type, the OP_VA_* IR opcodes, and the per-ABI register-save callee prologue
  (superseded by comptime variadic packs in v1.7.0) (#1478).
- comptime: the `$<sym>.symbol` / `$<sym>.library` / `$main.symbol` attribute
  setters — superseded by `` `symbol(...)` `` / `` `library(...)` `` backtick
  decorators (#1476); a stray `=` after a comptime directive now reports a
  migration diagnostic (#1478).
- ci: the content-based seed-safety guard (#1486) — obsolete now that the seed is
  v1.7.0 and the vendored std legitimately uses `va:` / `$each` / `$mach.build.*`.
- abi: the vestigial VaModel register-save geometry left behind by the varargs
  removal — the 12 save-area fields, the per-ABI `va_save` helpers, and the
  never-emitted `MIR_VA_FP_SAVE` pseudo. The functional call-ABI facts (the SysV
  vector-count register and the win64 float-duplication) are retained (#1478).

### Changed

- deps: the vendored mach-std pin advances to v0.12.0 — the migrated, new-only
  std (comptime packs + `$mach.build.*`) (#1480, #1478).
- comptime: float-literal folding imports `std.math.bignum` from the vendored std
  instead of the temporary in-tree bignum port added for v1.7.0 seeding (#1483).

## [1.7.0] - 2026-06-19

Comptime rework and first-class variadics — the last big breaking change before
stability. The C-style `va_list` surface is gone, replaced by comptime variadic
packs that monomorphize per call; the `$` comptime channel gains first-class type
values; and per-declaration attribute directives move onto backtick decorators.
The compiler source uses no new syntax, so the self-host fixpoint converges
byte-identical on x86_64 and aarch64 — except that float-literal folding is now
correctly rounded, which intentionally changes the compiler's own float constants
relative to the v1.6.0 seed (this release re-seeds).

### Added

- comptime: first-class type values — `$type_of`, `$fields(T)`, field projection
  `v.[f]`, bounded `$each` expansion, and type `==` in `$if` (#1472, #1473).
- variadics: comptime variadic packs — a `va: ...` pack parameter consumed by
  `$each arg in va`, monomorphized per call-site type-list (no `any`, no runtime
  `va_list`); `va...` forward, `va.len` (#1474, #1475). aarch64 variadics work.
- decorators: per-declaration backtick decorators `` `symbol` ``/`` `library` ``/
  `` `inline` ``/`` `align` ``/`` `section` `` (codegen-only; `align`/`section`
  emit, including per-function section placement) (#1476).
- comptime: provenance-rooted namespaces — resolved `$mach.build.*`, fixed
  `$mach.{os,arch,abi,mode}.*` tag tables, and `$project.*` from the manifest
  (#1471). The legacy `$mach.target.*` spellings still resolve this release.
- comptime: correctly-rounded float-literal folding via exact bignum — large
  exponents no longer lose precision (e.g. `1.0e100`) (#1483).
- ci: a self-host fixpoint lane that byte-diffs seed→stage1 vs stage1→stage2 on
  PRs, catching uniform codegen changes the release `cmp` can't see (#1488).

### Changed

- deps: the vendored mach-std seed freeze is now single-source — `mach.lock` +
  `mach dep pull` with an immutable `commit/` pin, the redundant submodule
  removed, and a content-based seed-safety guard (#1486).

### Fixed

- dep: `mach dep pull` now finds `git.exe` on Windows (the lookup missed the
  `.exe` extension) (#1506).

## [1.6.0] - 2026-06-14

The aarch64-linux debut: mach now cross-compiles and self-hosts on 64-bit ARM
linux (AAPCS64) alongside x86_64 linux and windows. The compiler itself makes no
variadic calls, so the self-host fixpoint converges byte-identical on aarch64
under qemu exactly as it does natively on x86_64.

### Added

- target/aarch64: a complete aarch64-linux back end (#1431). The A64 fixed-width
  encoder emits 32-bit instruction words directly; instruction selection covers
  the scalar integer and floating-point paths; integer division and remainder
  lower to `sdiv`/`udiv` + `msub` with the defined divide-by-zero and
  signed-overflow trap semantics; and variable (runtime-count) shifts lower to
  the register-operand shift forms.
- target/aarch64: the AAPCS64 calling convention — argument and result
  classification across the general-purpose (x0–x7) and SIMD/FP (v0–v7) register
  banks, the x8 indirect-result register for large/`sret` returns, and
  natural-alignment of stack-passed arguments.
- target/aarch64: calls, relocations, and frames — `bl` calls and the
  `adrp`+`add`/`ldr` symbol-address idiom emitting the R_AARCH64_CALL26,
  ADR_PREL_PG_HI21, ADD_ABS_LO12_NC, and LDST\*_ABS_LO12_NC relocation kinds,
  with the leaf and non-leaf frame prologue/epilogue (`stp`/`ldp` of x29/x30).
- target/aarch64: inline-asm mnemonic support for the A64 instruction set, so the
  mach-std aarch64 runtime entry (`_start`) and OS layer assemble through the same
  per-architecture asm path as the x86_64 runtime.
- ci/release: the aarch64-linux release asset is enabled (`RELEASE_AARCH64`), with
  a qemu-aarch64 smoke-test that proves the cross-built aarch64 `mach` can both
  build and run a real project under emulation before it ships — mirroring the
  windows/wine compiler smoke. CI's aarch64 lane cross-builds mach to aarch64 and
  byte-verifies the emitted encodings; the runtime is exercised by that qemu smoke,
  the aarch64 self-host fixpoint, and the in-source test corpus run under qemu-aarch64
  (#1464).

### Fixed

- target/aarch64: f64 and f32 memory loads and stores selected the wrong register
  class — addressing a general-purpose register where a SIMD/FP register was
  required — so a float loaded from or stored to memory (a struct field, array
  element, or pointer dereference) now uses the SIMD `ldr`/`str` forms (#1459).
- parser: the bare statement keywords `cnt` / `brk` are disambiguated from
  identifiers via lookahead — they are keywords only in the bare `cnt;` / `brk;`
  statement form, so `cnt = expr;` now parses as an assignment instead of failing
  with the misleading "expected ';' after 'cnt'" (#1458). This was a prerequisite
  for aarch64: the aarch64 std runtime's inline asm uses the `brk` mnemonic
  (`asm aarch64 { brk 0 }`), which the pre-fix parser mis-parsed as the keyword.

### Known limitations

- aarch64-linux v1.6.0 ships without variadic formatting (`printf`/`format`/
  `vformat`): mach-std gates the variadic surface out of the aarch64 build, with
  the redesign deferred to v1.6.x as a cross-platform varargs effort. The
  compiler makes no variadic calls, so self-host is unaffected; only aarch64
  programs that call the variadic formatting helpers are impacted.

## [1.5.5] - 2026-06-14

Tooling-prep patch. Adds reload-friendly source handling so a long-lived session
(e.g. the language server) can rebuild its project graph on every save without
growing without bound, and completes the target-layer interner-elimination by
dropping the now-dead interner parameter from `register_all` and every target
registrar — the target vtables are now immutable singletons.

### Added

- driver/source: reload-friendly source handling for long-lived sessions. The
  session `SourceMap` now dedups by path (`source.load`): re-loading a path returns
  its existing `FileId` and swaps the text in place rather than appending, so a
  session that reloads its project graph on every save no longer grows without
  bound. `dnit_project` now also resets the session's per-build module registries
  (AST/sema/resolve/comptime/fqn and export/import maps), dropping the borrowed
  references the freed Project leaves behind so one Session is reusable across
  rebuilds. This is path-dedup + reclamation only; reusing untouched ASTs/resolve
  results across a rebuild (true incremental rebuild) is tracked separately in
  #1164 (#1389).

### Changed

- target: `register_all` and the target registrars no longer take an interner (or
  the vestigial allocator) parameter. The registrars set immutable `str` vtable
  fields and intern nothing, so the parameter had been dead since the OS-vtable
  interner-elimination — each registrar is now trimmed to exactly the registry it
  installs into, and `mach info` no longer builds a throwaway interner to call them.
  Behavior-preserving, verified by the byte-identical self-host fixpoint; completes
  the interner-elimination begun in #1402 (#1418).

## [1.5.4] - 2026-06-14

Stabilization + multi-target-prep release. Adds the multi-target union Project for
long-lived tooling (the language server now sees modules reachable only on a
non-default target), lands the post-windows compiler-stability audit fixes
(object-format parse-leak reclamation + truthful ELF symbol counts, the COFF REL32
addend convention, a distinct unknown-target-name diagnostic, and an arm64
register-id correction), and completes the OS-vtable interner-elimination — readying
the target layer for the v1.6 architectures.

### Added

- driver: `build_project_union` builds the union of every manifest target's import
  closure into one deduplicated Project, so long-lived multi-target tooling (e.g. the
  language server) sees modules reachable only on a non-default target. Each `$if`
  chain follows the first active branch of every declared target; the Project resolves
  under the default (native-resolved) target's tuple — the documented v1 default, "the
  default target's branches win" — and a module that does not fully resolve there
  records diagnostics without aborting the build (#1391).

### Changed

- target: the os/arch name↔id mapping is consolidated to one canonical table per
  dimension — `os.os_id_for`/`os.os_name_for` and `isa.arch_id_for`/`isa.arch_name_for`
  — replacing the copies that lived in `driver.mach` and `manifest.mach`. An
  unrecognized manifest `os`/`isa` name now reports a distinct "unknown target os/isa
  name '<name>' (expected: ...)" diagnostic instead of folding to `*_UNKNOWN` and being
  mis-reported as an unsupported pair (#1412).
- The `OsVTable` `entry_symbol`, `syscall_layer`, `libdir`, and `dynamic_linker`
  fields are immutable `str` constants set directly by the OS registrars, no longer
  `StrId` fields re-interned per session; the linker now interns these at the point
  of use (the entry-symbol lookup and the dynamic interpreter path), completing the
  interner-elimination from the OS vtable started in #1377. Behavior-preserving,
  verified by the byte-identical self-host fixpoint (#1402).
- target: the dead `IsaVTable.endianness` field is removed. It was set by the ISA
  registrars but read by nobody — the object-format writers hardcode little-endian —
  so the field documented a behavior that did not exist. The `ENDIAN_LITTLE`/
  `ENDIAN_BIG` enum is retained for the eventual big-endian abstraction, and the
  comments that claimed endianness was honored now state the little-endian
  assumption plainly (#1411).
- objfmt (COFF): the writer/reader resolve every relocation's target symbol through a
  name→index map built once per pass instead of a linear symbol-table scan per
  relocation, turning the two reloc passes from O(reloc × symbol) into O(reloc +
  symbol). Behavior-preserving (same indices resolved, same unresolved-symbol error),
  verified by the byte-identical self-host fixpoint (#1413).

### Fixed

- target (aarch64): `IsaVTable.fp_scratch_reg` for the (stub) arm64 backend is now
  the composite vector register id `regid_make(REG_CLASS_ID_XMM, 31)` instead of the
  raw bank index `31`, which would have read as a GP register. Dead today (the arm64
  backend has no encoder), corrected before it seeds the future backend (#1414).
- objfmt: the COFF and ELF object-file parsers allocate the section/symbol/relocation
  arrays up front but left each `ObjectImage` count at `0` until its populate loop
  finished, so an error return before or within those loops (a truncated or hostile
  object) leaked the arrays — `obj.dnit` frees nothing when the counts are `0`. Each
  count is now set to its populated size immediately after the array is allocated (the
  loops use local write cursors), so teardown reclaims the full arrays on any later
  parse error; the ELF symbol array is sized and counted to the populated count
  (excluding the reserved index-0 entry) so allocation, count, and entries all agree.
  Emitted output is unchanged (#1410).
- objfmt (COFF): REL32 relocations are next-byte-relative (`S + A − (P+4)`), but the
  abstract addend uses ELF's field-relative convention (`S + A − P`), so the writer's
  on-wire field and the reader's recovered addend were off by 4 — a foreign
  (MSVC/clang/GAS) COFF read by mach landed calls 4 bytes past target, and mach's
  emitted `.obj` was off by −4 under link.exe/lld. The writer now folds `A_coff =
  A_elf + 4` and the reader recovers `A_elf = A_coff − 4` for REL32 only; ABS64
  (ADDR64) and ADDR32NB are unaffected. A mach-emitted `.obj`'s REL32 wire field
  changes (that is the fix — a `call`'s field is now 0, the COFF convention), but a
  mach→mach round-trip recovers the same abstract addend and links to the same final
  binary; only foreign-COFF interop behavior changes (#1409).

## [1.5.3] - 2026-06-13

Correctness patch for two silent defects in shipped v1.5.2: a relocation
patch-site miscompute that corrupted out-of-`imm32` constant stores to globals,
and an extreme float-literal exponent that hung the compiler.

### Fixed

- codegen (x86-64): an 8-byte store of an immediate outside signed-`imm32` range to
  a global is legalized into `MOVABS r11, imm64` + a register store, but the PC32
  relocation was patched at the pre-legalization offset — landing 4 bytes early,
  corrupting the encoding and leaving the store pointed at `rip+0`. The patch site
  now tracks the legalized `disp32` (the same fix covers the ALU memory-destination
  and inline-asm `mov [sym], imm64` paths). No diagnostic was emitted; ordinary code
  storing a large constant to a global silently miscompiled (#1407).
- comptime: a float literal with an extreme exponent (e.g. `1.0e2000000000`) hung the
  compiler in the unbounded decimal-scale loop and could silently fold to its mantissa
  on i64 exponent overflow. The exponent accumulator is now clamped past the point
  f64 saturates, so such literals fold to `inf`/`0.0` instead (#1408).

## [1.5.2] - 2026-06-13

Maintenance patch: path dependencies materialize through the standard library
instead of host `ln`/`rm`, the target vtables become immutable `str`-named
singletons, and the windows CI gains a trailing-import-descriptor regression
guard.

### Fixed

- Path dependencies materialize via the std filesystem primitives (`fs.symlink`,
  `fs.remove_all`) instead of shelling to `ln -s` / `rm -rf`, so they no longer
  require host `ln`/`rm` — fixing path-dependency materialization on native
  Windows, where `ln` is not on `PATH`. The `git` transport is unchanged (#1392).

### Changed

- The OF/ISA/OS/ABI vtable `name` and register-class names are immutable `str`
  constants set directly by the registrars, no longer `StrId` fields re-interned
  per session; the dead `OfVTable.file_extension` is removed. Behavior-preserving,
  verified by the byte-identical self-host fixpoint (#1377; the remaining
  interner-elimination is tracked in #1402).
- CI: a `threadsync` wine integration test guards the trailing PE
  import-descriptor call-thunk against regression (#1388), and the win64 wine
  tests are bounded with a `timeout` so a faulting binary fails fast instead of
  hanging the lane (#1399, #1404).

## [1.5.1] - 2026-06-13

Native Windows lane: CI now builds `mach.exe` and runs the in-source test suite
on real Windows, not just the wine cross-compile path. Two codegen fixes complete
the windows backend end to end — per-page stack probing for frames over a page,
and per-descriptor PE import call-thunks — and the vendored standard library is
updated to its windows-complete release. The native lane passes 468/468.

### Fixed

- Windows function prologues now probe the stack one page at a time for frames
  larger than a page, instead of a bare `sub rsp, N`. Windows commits the stack
  one guard page at a time, so a single large subtraction skipped the guard page
  and the first write near the bottom of the frame faulted on reserved memory
  (`STATUS_ACCESS_VIOLATION`); Linux and wine auto-grow the stack on any in-range
  fault and so never surfaced it. The encoder now emits the inline `__chkstk`
  page-walk (mach links no runtime) when the target OS commits incrementally — a
  new `OsVTable.stack_probe` flag (true on Windows, false on Linux/Darwin) gated
  by the OS `page_size`. This was the root cause of the native-Windows exec
  failures in briar-systems/mach-std#262 (a 32 KiB `spawn_redirected` cmdline frame)
  (#1395).
- PE import call-thunks for the **last** import descriptor jumped through the
  previous descriptor's null IAT slot (a `jmp 0` access violation on the first
  call). `pe_iat_slot_rva` never advanced its dependency index, so it re-scanned
  only the first descriptor when mapping an import ordinal to its IAT slot — the
  trailing descriptor's stubs landed short of its `FirstThunk`. Reordering the
  `libs` list moved the breakage to whichever DLL was last. Now every import's
  thunk targets its own descriptor's slot, so the trailing descriptor's calls
  (e.g. advapi32's `SystemFunction036`) dispatch correctly (#1388).

## [1.5.0] - 2026-06-12

Inline-asm & foundations: carry-flag mnemonics and numeric local labels in x64
inline assembly, per-symbol DLL attribution for windows imports, the first PE
output the native Windows loader accepts, float `%` correctness, `nil`
coercion to function types, materialized `path` dependencies, declared
foreign-target runners, and session-owned target registries under the hood.

### Added

- `mach test` and `mach run` accept `--runner <cmd>`: every child exec becomes
  `<cmd> <binary> <args...>`, the declared host-side launcher for
  foreign-target artifacts (e.g. `--target windows --runner wine`). The value
  is a single command name or path (no shell-style word splitting), resolved
  on `PATH`. Absent the flag, binaries are exec'd directly and a launch
  failure is reported as a failure — no auto-detection (#1345).
- One-line install scripts: `install.sh` (Linux) and `install.ps1` (Windows),
  shipped in the repo and as release assets. They resolve the latest tag from
  the `releases/latest` redirect, download the versioned archive for the host,
  verify it against `SHA256SUMS`, and install to `~/.local/bin`
  (`%LOCALAPPDATA%\mach\bin` on Windows); `MACH_VERSION` and
  `MACH_INSTALL_DIR` override the release and destination (#1352).
- Releases now ship versioned archives — `mach-<version>-x86_64-linux.tar.gz`
  and `mach-<version>-x86_64-windows.zip`, each containing the binary and
  LICENSE — alongside the existing assets, with `SHA256SUMS` covering the
  full set (#1352).
- x64 inline assembly accepts the carry-flag mnemonics `jc` / `jnc` (and the
  `jb` / `jae` / `jnb` aliases) and `setc` (and the `setb` / `setae` / `setnb`
  aliases), reusing the existing conditional-branch and SETcc encoders. Previously
  only `je` / `jz` / `jne` / `jnz` were recognized, forcing carry-flag handling
  through `.byte` escape hatches (#1359).
- x64 inline assembly accepts NASM-style numeric local labels: a `<digits>:`
  statement defines a block-local label and `<digits>f` / `<digits>b` branch
  targets resolve to a rel32 within the function (no relocation). Every branch
  mnemonic takes a symbol or a local-label target; a backward reference binds to
  the nearest preceding definition and a forward reference to the nearest
  following one, redefinition of a number is allowed, and an unresolved or
  malformed reference is a hard compile error. This unblocks block-local forward
  branches such as the `jc 1f` / `1:` shape in std's darwin syscall wrappers
  (#1365).
- `$<sym>.library = "<dll>"` pins an `ext` import to a specific dependency DLL,
  giving the PE (Windows) import directory per-symbol attribution. Imports were
  previously all forced onto the first dependency (kernel32.dll), so extra DLLs
  in `[target.*].libs` emitted only empty descriptors; an attributed import now
  lands under its named library, an unattributed one still binds to the first,
  and pinning to a library absent from the link's dependencies is a hard link
  error rather than a silent fallback. Composes with `.symbol` — the rename sets
  the imported name, `.library` the DLL it is imported from (#1382).

### Fixed

- Float `%` now evaluates to the truncated (C `fmod`) remainder
  `a - trunc(a / b) * b`, whose result takes the dividend's sign
  (`5.5 % 3.0 == 2.5`, `-5.5 % 3.0 == -2.5`). The operator had no float lowering:
  the runtime path fell through to the integer IDIV/DIV opcodes, running an
  integer divide over the raw IEEE-754 bit patterns (a passthrough-below-divisor,
  near-zero-above shape), and the comptime fold rejected it as non-constant. Both
  now synthesize the remainder from the existing float divide / truncating
  conversion / multiply / subtract primitives, exact for `|a / b| < 2^63` (#1378).
- `nil` coerces to function types, so a fun-typed binding can be explicitly
  nil-initialised, not only default-initialised. Previously `var q: fun(u32) =
  nil` was rejected with `type mismatch: expected fun(u32), found *u8` and the
  cast spelling `var r: F = nil::F` failed lowering with `global initialiser
  must be a constant expression`, even though a fun-typed value already compares
  `== nil` and `nil::F` was accepted in argument position. nil now coerces to
  any pointer-like target — `ptr`, `*T`, or `fun(...)` — uniformly across
  globals, locals, record fields, array elements, arguments, and return slots,
  and a nil global initialiser (bare or written through pointer casts) folds to
  the null constant. nil into a non-pointer slot remains a type error (#1369).
- `mach dep pull` now materialises a `path` dependency at `<dep>/<alias>/` as a
  relative symlink to the resolved source instead of silently doing nothing —
  previously it printed "mach.lock up to date", exited 0, and never created the
  dep directory, so any later build failed to resolve the dep's modules. The
  `path` is resolved relative to the requiring manifest's directory; a stale link
  is replaced and an already-correct one left in place (idempotent), while a
  missing directory, a directory without a `mach.toml`, or a vendor location
  occupied by a real directory is a hard error. A path dep carries no lock entry —
  its manifest `path` is the record (#1370).
- Sema reports a teaching diagnostic for every symbol kind that can never be a
  value reaching value position — a record, union, or `def` type name (local
  or imported, bare or `alias.member`), a generic type parameter, and a member
  access on an expression with no value (a call with no return type) — instead
  of silently poisoning and surfacing link `undefined symbol` or span-less
  lowering errors; completes the #1343 silent-poison audit (#1348).
- The PE emitter no longer produces executables the native Windows loader
  rejects with `ERROR_BAD_EXE_FORMAT`. The image base reserved a full 64 KiB
  below the first segment, placing the first section at RVA `0x10000` while the
  headers spanned one page — a 15-page unmapped gap the loader refuses (wine
  tolerated it). ImageBase now sits exactly one header span below the first
  segment, so the first section maps immediately after the headers with no gap,
  matching the layout MSVC emits. Every `mach`-built Windows binary, including
  the published `mach.exe`, now launches natively (#1374).
- A `$<sym>.symbol` rename on an `ext fun` is no longer clobbered. The bare `ext`
  identifier was re-registered over the rename after `scan_export_directives`
  recorded it, so renames on foreign imports silently did nothing and bound under
  the mach identifier; the bare name is now only a default that an explicit
  `.symbol` override wins, order-independently (#1382).
- `mach init` validates the project id before scaffolding: a name the manifest
  grammar would reject (`.`, path separators, spaces) is refused with nothing
  written, instead of silently scaffolding an ungrammatical `[bin..]` manifest.
  The positional name is taken verbatim — no basename derivation (#1355).
- A type mismatch against a call with no return type reads `expected i64, found
  no value` (with a clarifying note) instead of rendering the misleading
  `<error>` placeholder, and diagnostics consistently say "returns nothing" /
  "no value" — mach has no `void` (#1360).
- `infer_generic_call` reports internal generic-substitution and type-argument
  allocation failures instead of silently poisoning the expression (#1361).

## [1.4.1] - 2026-06-12

Patch release clearing the open bug board: environment forwarding for spawned
user programs, the `-o` absolute-path override, `fwd` module re-export
consumption, and inline-asm comment handling.

### Changed

- Bumped the vendored mach-std to v0.6.0 (adds `std.process.env.environ()`;
  fixes the thread spawn/join deadlock and json key lookup).

### Fixed

- `mach test` and `mach run` now forward the parent environment to spawned
  user programs instead of execing them with an empty `envp`; `getenv` in a
  test or run child sees the inherited variables (mach-std#197).
- An absolute `-o` path is honored verbatim; previously its leading slash was
  swallowed by an unconditional join with the project root and the output
  landed project-relative (#1340).
- A consumer can chain through `fwd` module re-exports in expression position
  (`lib.alpha.answer()`), at any depth including a `fwd` of another library's
  `fwd`; a module alias referenced in value position now reports
  `a module alias is not a value` instead of silently poisoning and surfacing
  internal lowering errors (#1343).
- Inline-asm comments are now opaque to the instruction parser regardless of
  their bytes. A comment containing `;` was split into a phantom instruction
  and one containing `{...}` was misread as a local binding; comments are now
  stripped to end-of-line when the asm body is materialized, before any
  substitution or statement tokenization (#1297).

### Removed

- The unused test-runner write-primitive machinery (`RunnerWrite`,
  `OsVTable.runner_write`): since 1.4.0's per-test executables, test binaries
  perform no OS output — the host process reports — so the vtable hook had no
  consumers on any OS (#1292).

## [1.4.0] - 2026-06-12

The clean-break release: one manifest format, the project module, and the
test infrastructure the language was designed around.

### Added

- **The project module**: `[project] module = "lib.mach"` — a bare project-id
  `use glfw;` / `fwd glfw;` resolves to the project's declared module. Never
  inferred; module-less bare imports and dangling declarations error loudly.
- **OS link overlays**: `[os.<name>] libs = [...]` — link requirements scoped
  to one tuple component, cascading to consumers across every ISA/ABI of that
  OS (`[isa.*]`/`[abi.*]` reserved).
- **Per-test executables**: every `test` block builds to its own standalone
  program under the `tests` path template; `mach test` runs each in its own
  process — a crashing test reports `FAIL(signal n)` and the run continues —
  with `--list` and `--filter`. Test artifacts never touch the project binary
  path.
- The compiler versions itself from its manifest (`$project.version`):
  release bumps are now a one-file change.

### Changed

- **One manifest format.** The pre-1.3 manifest system is removed entirely —
  `[targets.*]`, `dir_*` keys, string opt levels, and `type/path/version`
  dep stanzas no longer parse. Projects update their manifests by hand;
  doc/manifest.md documents the format.
- `[profile.*] opt` is an integer (`0 | 1 | 2`).
- `mach dep pull` clones into an empty dependency placeholder directory
  (plain-clone installs work without `--recurse-submodules`).
- Inline-asm comments are fully opaque to the instruction parser; every
  audited diagnostic now names its subject and carries a span.

## [1.3.1] - 2026-06-11

Transitional self-seeding release. Carries the v1.4.0 feature line developed
so far and exists so the next release can drop the old manifest format
entirely: this binary reads both manifest formats; the next reads only the
current one.

### Added

- The new `mach.toml` manifest format: explicit `[target.*]`/`[bin.*]`/
  `[lib.*]`/`[profile.*]`/`[deps.*]` stanzas, path templates, `native` target
  resolution, multi-artifact matrix builds, tuple-matched cascading dependency
  libs, per-target comptime `defines`.
- The `mach dep` model: `pull`/`update` with a manifest-is-intent lockfile
  law, `git`/`path` source forms, transitive resolution.
- Comptime namespace roots `$project.*`, `$target.*`, `$bin.*`; bare `$ident`
  is now rejected with a teaching diagnostic.
- `mach info` (and `mach info --version`): compiler version, build host, and
  the registered target capability surface.
- Mixed-numeric comparisons: legal and value-correct across signedness and
  widths (comptime agrees with runtime); int↔float still requires a cast.
- A diagnostics overhaul: ~70 audited messages now carry spans, name the
  offending symbol, and say what to do.

### Changed

- Release-mode builds are ~5x faster (quadratic pass costs removed); the
  optimizer gained phi simplification, post-inline promotion, and call-graph
  aware inlining.
- The IR verifier is real (dominance, reachability, type agreement) and CI
  enforces `--release --verify-ir`.
- Relocation kinds are a closed enum; the object-format layer no longer
  captures per-session interners.
- The inline corpus grew to ~390 tests; comment content inside `asm` blocks
  is now fully opaque to the instruction parser.

## [1.3.0] - 2026-06-11

Correctness and foundations release. A full-compiler audit (125 findings,
45 confirmed bugs) was executed end to end: every confirmed miscompile is
fixed, the ABI abstraction boundary is complete ahead of new targets, and
mixed-numeric comparisons joined the language with value-correct semantics.

### Added

- **Mixed-numeric comparisons**: integer comparisons across any signedness
  and width are now legal and compare mathematical values, with results
  identical in both operand orders (`i64`/`u64` lowers to a sign-test plus
  unsigned compare). Float widths mix exactly; int↔float still requires an
  explicit cast. Comptime evaluation agrees with runtime semantics on the
  same boundary cases.
- Variable-index function-pointer dispatch: `table[i]()` now parses correctly
  (the bracket payload is resolved against the callee — generic call vs
  index-then-call — instead of guessed in the parser).
- C interop, win64 variadic, float-argument, conversion-boundary, comptime/
  runtime-agreement, and release-verifier integration suites run in CI.

### Fixed

- **Miscompiles**: FP-register interference tracking (swapped float arguments
  collapsing); win64 callee-saved XMM6–13 never preserved (with full
  `UWOP_SAVE_XMM128` unwind coverage); inline-asm clobber inference
  implemented as documented; `u32`→float converting as signed; register
  `i32`→`i64` sign-extension reading only 16 bits; SysV float-bearing
  aggregates never classified to SSE registers (C FFI divergence); comptime
  `u64`-range constants evaluating as negative.
- The program entrypoint entered every callee with an 8-byte misaligned
  stack (mach-std v0.4.2) — fatal for C callees using aligned SSE accesses.
- The IR verifier now verifies: real dominance (near-linear), reachability,
  operand type agreement, dangling-definition detection; `--release
  --verify-ir` passes on the full compiler and is enforced in CI.
- Optimization passes: constant folding no longer crashes the compiler on
  `INT64_MIN / -1`; dead phis are eliminated; phi simplification unblocks
  constant propagation across inlining; the inliner refuses call-graph
  cycles.
- The ELF/COFF writers and linker fail loudly on unresolved relocation
  symbols, unknown relocation kinds, and malformed objects; weak/strong
  symbol resolution is order-independent; section/relocation counts are
  validated before narrowing.
- IR teardown frees operand arrays and aggregate blobs by true capacity —
  correct under any allocator.
- CLI: `mach run --` argument passthrough, `mach init --name`,
  `mach build <path>`, a lockfile-writer heap overflow, `--quiet`/`--color`
  now functional, unified exit/signal handling.
- Frontend: the grammar holes vs the locked spec (multi-line strings,
  `val`/`var` forms, integer-literal overflow) are rejected with diagnostics;
  parser OOM can no longer yield a silently corrupt AST.
- Generics: cross-module arity checking, deeply nested parameter
  substitution, `fwd` module re-export, duplicate diagnostics.

### Changed

- The ABI layer is complete and arch-keyed: selection consults (isa, os),
  classifiers carry an explicit by-reference class, the variadic model is
  vtable-driven end to end, and the test runner's output primitive comes
  from the target OS — groundwork for the aarch64 and darwin targets.
- `mir` is split into per-concern modules (data model, lowering context,
  IR→MIR core, calling-convention lowering, variadic expansion) with
  byte-identical output.
- `mach init` scaffolds mach-std pinned to `branch/main` (v0.4.x); the
  vendored std is v0.4.2.
- Object-format writers share a common binary-IO layer.

## [1.2.0] - 2026-06-10

Native Windows release. `mach.exe` now builds Mach itself on the win64 target
to a byte-identical fixpoint (verified under wine), and releases ship
per-target artifacts.

### Added

- **Per-target release artifacts**: `mach-x86_64-linux`, `mach-x86_64-windows.zip`
  and `SHA256SUMS` alongside the bare `mach` seed binary; the windows artifact is
  gated on a wine smoke test in which `mach.exe` builds and runs a real project.
- CI now runs the integration suites (dynlink, extlink, opt, win64byref,
  win64fnptr) with wine on every pull request, plus a windows cross-build of the
  compiler.
- Per-function `.pdata`/`.xdata` unwind metadata on win64 executables, with
  spec-correct `UNWIND_INFO` encoding.

### Fixed

- Function-to-pointer casts (`fn::*u8`) lowered as a 32-bit truncating move,
  corrupting every type-erased function pointer above 4 GiB — the fault that
  kept native `mach.exe` from running on win64 image bases.
- win64 by-reference aggregates passed as a fifth-or-later argument: caller and
  callee now agree on the spilled hidden pointer via an explicit ABI class
  (`CLASS_STACK_BYREF`), covering sub-pointer (3/5/6/7-byte) aggregates.
- win64 unwind metadata: removed the incorrect frame-register declaration,
  fixed `UWOP_SAVE_NONVOL_FAR` to record unscaled offsets, and stopped
  misreading `[r13+disp]` stores as frame saves.
- The ELF and COFF writers now fail loudly, naming the symbol, when a
  relocation targets an unresolved symbol instead of silently emitting a
  corrupt object.
- COFF extern-function inference is scoped to foreign objects, indexed O(n),
  and propagates allocation failure.
- `mach dep`: unified dependency-root resolution across sync/add/remove,
  proxy environment variables (`ALL_PROXY`, `all_proxy`, `no_proxy`) forwarded
  to git, idempotent lockfile writes, and `mach init` no longer pre-creates
  the dependency directory.
- Release tags are verified against the manifest version before any artifact
  is built.

## [1.1.1] - 2026-06-09

Patch release for project scaffolding correctness.

### Fixed

- `mach init` now scaffolds fully buildable projects with all required manifest
  entries and `mach-std` dependency wiring.

## [1.1.0] - 2026-06-08

Tooling and cross-compilation release. Mach can now emit Windows executables,
link dynamically, manage dependencies, and answer editor queries — while the
Linux self-host continues to build to a byte-identical fixpoint.

### Added

- **Windows cross-compilation** for `x86_64-windows`: the Microsoft x64 calling
  convention (win64 ABI), a COFF/PE object and executable writer, and kernel32
  import linking. Mach builds runnable Windows `.exe`s. (Running the compiler
  itself natively on Windows is in progress for a later release.)
- **ELF dynamic linking**: link against shared libraries via `-l`/`-L` and
  `[targets.*].libs`, with a real PLT/GOT and `DT_NEEDED`/`PT_INTERP`.
- **External linking and static archives**: link prebuilt `.o`/`.a` inputs; a
  Unix `ar` archive reader.
- **`mach dep`**: git-based dependency management (`add`/`remove`/`sync`/`vendor`)
  with a `mach.lock`.
- **`mach check`**: single-file diagnostics with no project or link step.
- **Per-target optimization levels** via the manifest, overridable on the CLI.
- **Editor query surface** (`mach.lang.editor`): single-file/unsaved-buffer
  open, parse, resolve, and diagnostics for tooling and language servers.

### Fixed

- `fwd` re-exports now resolve against the dependency set correctly.
- x86-64 `imul` by a constant outside signed-imm32 range no longer truncates to
  the low 32 bits (silent miscompile of large-constant multiplies).
- A global `val` initialized from a constant cast no longer silently lowers to
  zero; a non-foldable global initializer is now a hard error.
- Several win64 codegen fixes (shadow space, variadic definitions, callee-saved
  register preservation) and a COFF weak-symbol round-trip.

## [1.0.0] - 2026-06-06

First stable release of Mach: a self-hosting, dependency-free native compiler
for the Mach systems programming language. The compiler builds its own source
to a byte-identical fixpoint and emits statically-linked x86-64 ELF directly,
with no external backend, assembler, or linker.

### Added

#### Compiler

- Self-hosting compiler that builds its own source to a byte-identical fixpoint.
- Direct x86-64 (Linux, SysV ABI) native code generation: lexer, parser,
  resolver, semantic analysis, an SSA mid-end, instruction selection,
  linear-scan register allocation, and ELF object/executable emission — with no
  LLVM and no external assembler or linker.
- Optimization pipeline: `mem2reg` (stack-to-SSA promotion), constant folding,
  dead-code elimination, function inlining, algebraic simplification, and local
  common-subexpression elimination. `-O0` runs the always-on subset
  (`mem2reg` / constant folding / DCE); `-O1` and `-O2` run the full pipeline.

#### Language

- Records (`rec`) and overlapping-layout unions (`uni`).
- Generics with bracket syntax (`T[U]`) and monomorphization.
- Compile-time evaluation: `$if` / `$or` branch selection, `$mach.*` target
  parameters, comptime value-parameter monomorphization, and value/layout
  intrinsics (`$size_of`, `$align_of`, `$offset_of`, …).
- Two cast operators: `::` (value conversion) and `:~` (same-size bit
  reinterpret).
- Pointers (including `nil`), slices, and fixed-size arrays.
- Error handling with `Result` and `Option`.
- Modules with `use`, module aliases, and `pub` visibility.
- Inline assembly (`asm`) and variadic functions.

#### Standard library

- `mach-std`: runtime, allocators, strings, collections, I/O, filesystem,
  formatting, OS/syscall bindings, and the core `Option` / `Result` types.

#### Tooling

- `mach` CLI: `build`, `test`, and `init`.
- Differential test harness (optimization-level and cross-compiler miscompile
  detection), a crash fuzzer, and a compiler compile-time benchmark harness.
