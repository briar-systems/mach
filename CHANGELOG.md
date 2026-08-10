# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [4.18.3] - 2026-08-10

### Fixed

#### Exported dependencies keep every symbol claim for a shared library (#2975)

The dependency cascade deduplicated link requirements by source, loader name,
and logical library, but discarded the later requirement in full. When two
native dependencies imported disjoint symbols from the same system library,
only the first dependency's claims reached the linker. The missing claims then
surfaced as unattributed dynamic imports even though both manifests declared
the library correctly. A Windows application combining GLFW and WASAPI exposed
the defect through their shared `kernel32.dll` requirement.

Exact duplicate requirements now retain a stable union of their symbol claims.
Each claim array remains independently owned while dependency manifests are
released, and allocation failure leaves the retained requirement unchanged.
The PE cascade regression covers both root-to-dependency and
dependency-to-dependency collisions on shared DLLs.

## [4.18.2] - 2026-08-09

### Fixed

#### SPIR-V copy-only merge values keep their declared type (#2939, #2969)

Phi destruction replaces an SSA phi with one `MIR_MOV` into its result on every predecessor edge. Those moves are sized for a physical register, not a typed value: an immediate `i32` edge may use four bytes while a vreg edge uses the eight-byte machine word. The SPIR-V emitter skipped copies while finding type authorities, propagated what it found, then fell back to each move's width. A component made entirely of copies had no authority to propagate and could therefore allocate one merge variable as `i64` while the value stored or compared through it remained `i32`. Mach either wrote a module `spirv-val` rejected or caught its own mismatch and refused a valid shader.

Each source-derived MIR vreg now retains the raw, module-local IR type id of its parameter or instruction result. The metadata is inert for machine targets. SPIR-V consults it only for copy destinations still untyped after concrete definitions and typed sources have propagated, so a comparison result remains a SPIR-V boolean while a copy-only phi component takes its declared numeric type. A second propagation types any synthetic temporary introduced while sequentializing a parallel-copy cycle; move width remains only the last fallback for genuinely synthetic values.

The exact loop-carried integer shader from #2969 now builds, and the three corpus cases quarantined by #2939 are restored to SPIR-V structural validation and golden coverage.

#### The darwin corpus legs could never have run (#2965)

`test-main.yml`'s darwin legs take a cross-built binary from the seed job rather than building in place, so they never run the `mach dep pull` a from-source build does, and their checkout has no `dep/`. Every other leg gets that pull for free from its own build, which is why the gap existed only on the two darwin rows and only became visible when those rows started running the codegen corpus at all.

It surfaced as a red leg rather than a quietly smaller run because the corpus refuses to start when a dependency it needs is absent, instead of skipping the cases that need it. That is the behaviour that made a missing step legible as a missing step.

The 4.18.1 release itself was unaffected: CD builds and publishes on the tag and completed with all five binaries, and this is the release-cadence test workflow that runs on the push to `main` beside it.

## [4.18.1] - 2026-08-09

### Fixed

#### A SPIR-V function body truncated its parameters instead of refusing (#2923)

Emitting a function body filled the per-block register file with `for (i < param_count && i < 16)`, so a function reaching that point with more than sixteen parameters had every parameter past the sixteenth silently dropped and read an empty register slot instead. It was unreachable only because `emit_function` refuses first, which made a load-bearing correctness guard look like defensive clamping: any change to that refusal turned it into wrong code with no diagnostic. It now refuses on its own terms.

The bound was also stated in four places rather than one, which is the defect #2748 closed reappearing at the site that fix did not reach. The cross-module call path carried a bare `> 16` rather than the named constant, so the two agreed by construction rather than by checking.

**The constant's own documentation was wrong, and that is what sent the issue at the wrong layer.** It said sixteen came from the emitter assembling signature operands into fixed arrays, so removing the bound read as a buffer resize. The real reason is the argument register bank. `abi/spirv.mach` is an identity convention where the argument at position `i` is classified into register `i`, since SPIR-V passes composites natively and has no frame to spill into, so a parameter position is a physical register id on this target. The shared MIR lowering caps those at `MAX_GP_ARG_REGS`, also sixteen. SPIR-V's own universal limit is 255, and reaching it means decoupling a parameter from the register model rather than widening a buffer.

That shared cap turned out to be unguarded in a way no target owner would expect. `abi_gp_arg_reg` reads a convention's bank into a fixed `[16]isa.Register` and range-checks the index **after** the fill, so a convention publishing a wider bank overruns that array before anything rejects it, from a one-line edit in a target's own file. A test now walks every registered convention against a deliberately oversized scratch and fails if any publishes a wider bank, and it refuses to pass if it checked nothing.

### Changed

#### A cross-platform executable needs one artifact per extension convention (#2959)
An artifact's `out` is one literal string and every target in `targets` resolves it the same way, so an executable that ships on Windows and anywhere else cannot use `targets = ["*"]`. `bin/app` gives Windows a file it will not run until someone renames it, and `bin/app.exe` gives linux and darwin a binary called `app.exe`. The answer is two stanzas with disjoint `targets` lists, which works today and needed no compiler change: mach does not append an extension behind the author's back, and a template variable for it was considered and rejected as more magic than it buys.

`doc/manifest.md` now says so on the `out` row and in a section that shows the two-stanza form with what it costs. The stanzas differ only in `out` and `targets`, so a `link` or `need` added to one and not the other diverges on Windows alone. Every new target has to be added to the right list by hand, since neither can use `*`. And the artifact name differs between them, so `$bin.name` reads `app` on most platforms and `app-windows` on Windows.

mach's own `mach.toml` had the defect it now documents. It declared `out = "bin/mach"` with `targets = ["*"]`, so a Windows build from the repository's own manifest produced an extensionless `bin/mach`. CI never caught it because every Windows leg passes `-o` explicitly, so the manifest's `out` was never what named the Windows binary and the project worked around its own bug. The manifest is now split, and `mach build . --target windows-x86_64` with no `-o` lands at `bin/mach.exe`.

Splitting mach's own manifest is what found #2961, and is deliberately not part of this change. `mach build` enumerates artifact-by-target cells and filters them, so each platform gets its own stanza with nothing named on the command line, but `mach test` links the whole source tree and answers "which artifact is primary" separately, in more than one place, and only the planner asks whether the artifact supports the target. On a host the first-declared stanza does not declare, a test run refuses. The documentation states that limitation where it recommends the split, and the repository keeps its single artifact until the selection has one owner, rather than shipping a shape its own test command cannot run or the flags that would hide it.

#### The integration suite is retired, and a linking suite replaces the part the codegen corpus cannot reach (#2903)
`int/` and `tools/` are deleted. The unified codegen corpus at `test/` answers what the compiler computes - 91 cases through structural validation, golden disassembly against an external decoder, and differential execution against a C reference at both pipelines on every target with an engine - and 57 of the 176 integration cases were asking that same question in a per-project shape that cost a directory tree and minutes of CI each. The suite held 176 case directories under 174 distinct names, since `shell` and `step` each existed once under `regression` and once under `surface`.

**It catches what it was built for, demonstrated rather than asserted.** `convert/f2u_high` converts floats above 2^31 and 2^63 to unsigned, which is the range the rest of the convert group deliberately stayed below while #2909 was open. Reverting that fix's encoder change and rebuilding makes the case disagree with the C reference at both pipelines while `convert/f2i` stays green, which is the coverage gap and its closure in one measurement. The boundary itself is not probed for evidence: at exactly 2^31 and 2^63 the x86 integer-indefinite value equals the correct answer, so it agrees with a broken lowering and is kept only as a control beside the values above it.

Bringing the corpus up to current `dev` moved six goldens for reasons the compiler changes own: the x86-64 float-to-unsigned expansion (#2909) in `convert/f2i`, `float/special_f32` and `float/special_f64`, the rv64 conditional float merge (#2917) in `vec/vec_scalar_mix`, and comptime floats evaluating at their declared width (#2918) in `comptime/ct_runtime_agree` and `convert/f2f`.

It also found two defects nothing else had. `cmp/branch_nest` built and validated for spirv before 46cd5558 and does not after, and `float/cmp_f32` and `float/cmp_f64` clear the #2910 boundary they were skipped for only to stop at the same merge-slot fault (#2939). `mem/rec_layout` reaches a refused pointer-to-record parameter (#2940) now that #2914 no longer stops it earlier. Twenty spirv skips came out because the defects they named are closed, so 78 of the 91 cases carry that column against 58 before. The four `cmp/cmp_*` cases go back to brace-literal arrays, which they had been written around while #2911 was open, so the mach source and its C reference are line for line again.

The other 65 are not codegen facts and no checksum reaches them: an import table, a GOT or IAT slot, a Mach-O load command and section type, a `PT_GNU_RELRO` span, an archive member selection, a `.symtab`, a raw flat image, and an object, archive or import library that clang, gcc, llvm-ar or mingw produced. Those move to `test/link/`, which asks that one question and refuses everything else.

**The rule the split was made against**: an integration case earns its place only where the property cannot be observed by building and looking on a developer's own machine. Cross-compilation, linking and foreign toolchains qualify. A front-end diagnostic, a comptime fold, a reflection walk, an inline-asm body and an optimizer decision do not, and cases for those are gone rather than moved. All 111 that did not move to `test/link/` are classified by disposition in the pull request: 57 the corpus now answers, 37 that are front-end or middle-end facts the compiler's own tests assert, 14 driver and toolchain checks that are simply gone because building and looking observes them, and 3 that are coverage genuinely lost rather than replaced. Those three are the x86-64 flags probe, the darwin and linux syscall conventions an inline-asm `noreturn` body depends on, and the console write `syscall-inline-asm` measured. #2944 restores them, together with the dudect timing harness that lived under `int/ct/` and was never a case.

**One registry.** `test/engines.conf` is now the single source of truth for both suites and for CI. It grows a `cadence` column, and the workflow leg set is generated from it, so registering a target is one row and no workflow edit - which the file's header already promised and CI was the last place to make false. A CI leg is a runner rather than a target row, because the corpus driver already selects every target a host can serve: the five `ubuntu-latest` rows are one job covering all five, not five jobs rebuilding the same compiler. A runner whose rows disagree about cadence is a refusal rather than a silent pick.

**A leg is a machine and a target is what a case builds**, and those are separate axes because a cross-compilation case exists precisely where they differ: `pe-import-claim` runs on `x86_64-linux` and builds `x86_64-windows`, and reading its import table needs no Windows runner. Rows with `engine none` - spirv, mos6502, riscv32 - are targets and never legs. There is no flag that overrides an engine: a run claiming it exercised a leg has to have exercised it, which the old suite's `--runmode` override made negotiable.

**Two lanes that could stop running now cannot.** Checking that every case declares the `[target.<t>]` block each leg builds, and that a pinned run can resolve every git dep any case declares, were separate scripts a workflow step invoked. #2353 and #2729 are each a year of a lane believed to be running that had never once started. Both now run inside the driver before it builds anything.

**The pin source collapses to the repo's own `mach.lock`.** The second lock existed for one dependency of one case that cloned a sibling repository over the network, and an upstream rename in that repository turned every open pull request red (#2831). That case is not in the new suite, and the driver refuses a pinned run naming a dependency the root lock does not record.

**A pin nothing can install is a wish, and every leg proved it.** The runner images carry llvm 18 and spirv-tools 2025.1 on ubuntu and llvm 20 and no spirv-tools on windows, against pins of llvm 22 and spirv-tools 2026.3, so all three legs refused to start (#2948). The refusal was right and stays. `tools.lock` gains a `source` row per pinned tool saying where that version is obtained, `run.sh --tools` prints the rows a given selection reaches, and `ci-tools.sh` installs from exactly that, so no workflow carries a copy of a version and none can install one thing while the driver demands another. LLVM comes from apt.llvm.org on linux, the llvm-project release on windows, and homebrew on darwin with the major it produced checked against the pin rather than trusted. SPIRV-Tools comes from Vulkan SDK 1.4.357.0: SPIRV-Tools tags no 2026.3 release and attaches binaries to none of its releases, and LunarG's apt repository is amd64-only and several SDKs behind, so the SDK tarball is the only thing that serves the pin, which `tools.lock` records along with why.

**Which leg carries a target is the registry's answer, and now it is the only one.** The corpus driver selected by host capability, and a target with `engine none` needs no execution host, so `spirv`, `riscv32` and `mos6502` were recomputed on the windows and arm runners as well as the one `engines.conf` assigns them. Two rules read one file and disagreed. The `runner` column is now the assignment and host capability is only a check: `run.sh --runner <label>` covers exactly the rows the registry gives that label, refuses loudly naming any row this host cannot serve rather than substituting a set it can, and names every row it was not assigned, so a leg a run was never applicable to and a leg it was dropped from do not look alike. Each CI job passes the label of the runner it is already executing on and no target list at all, since a workflow that names a leg is a workflow that can claim a leg it never ran. **Windows accordingly needs no spirv-tools**, and that is the reason rather than a side effect. What it removes from the windows and arm legs is an incidental cross-host check that a compiler built there emits the same module as one built on x86_64, which the corpus never claimed and which belongs in an explicit comparison rather than in a selection accident.

**The four targets no developer host can execute now have goldens.** `aarch64-linux`, `x86_64-windows` and both darwin targets were never selected on the linux host that blessed the corpus, so layers A and B had never run for them and their columns would have failed on the first green leg rather than reported. Layer B needs no execution, only the object and an external decoder, so all four are blessed from one host: 364 goldens, and the first layer A run over them is what found #2952.

Every run of either suite writes a coverage matrix naming the engine behind each cell, and every CI leg uploads both.

Three things kept their behaviour and changed address. `int/observability.md` is `doc/design/test-observability.md`, since the relationship between a defect class and the observation that can see it was never a property of a harness. `int/lib/check-determinism.sh` is `test/determinism.sh`, since it guards the incremental build path rather than integration. And `int/lib/cc.sh`, the host and cross C toolchain resolver every foreign-object case builds through, is `test/link/lib/cc.sh`.

Two measurements lost their guard and say so where they are claimed. `doc/language/secrecy.md` recorded a dudect-style timing harness at `int/ct/`. It ran on demand and never in CI, and it is gone, so the empirical claims there are now marked as measurements taken once. The x86-64 flags table in `x64/encode.mach` was transcribed from a probe run against real hardware, and nothing re-runs that probe. Restoring it belongs in a host-executed unit test, since the experiment only means anything on the ISA it executes on.
### Fixed

#### A COFF long section name was space-padded, and llvm refused the object (#2952)
The PE/COFF long section-name form is a slash followed by an ASCII decimal offset into the string table. mach right-justified the decimal in the seven bytes after the slash and padded the left with spaces, so a name over eight bytes came out `"/      4"`. binutils reads that by skipping the spaces. llvm does not: `llvm-readobj` and `llvm-objdump` both answer `invalid section name` and decode nothing, so every `x86_64-windows` object carrying a section name that long was unreadable by the tools the corpus is grounded in while `mach build` reported success. `.rodata.cst` is eleven bytes, so this reached every object with a constant pool.

It went unnoticed because nothing external had ever read a mach COFF object. The corpus's `x86_64-windows` column has no goldens on a linux developer host, which declines the target, and its CI leg had never started (#2948), so the first layer A run on that target is what refused. That is the same shape as the ELFCLASS32-on-Elf64 writer that motivated layer A: a format only its own reader had ever checked. The field is now pinned by byte in a unit test, because two readers disagreeing about padding is precisely what stayed invisible.

#### A float constant typed through an alias says why it does not fold (#2937)

`#2918` made a comptime float carry the IEEE width its payload is at, which means the pre-type passes have to read that width from the annotation spelling. That is exact for a literal `f32` or `f64`, because a primitive wins over scope in type position, and it is not readable when the type arrives through a `def` alias: no type exists yet to read the alias through. Such a binding is therefore skipped rather than bound at f64, which would export a constant that disagrees with the one the program computes.

The skip is right. What was wrong is that it left no trace, so a use of the name at that stage reported `identifier is not a comptime constant in scope` - which is false in both halves. `X` is a comptime constant and it is in scope, and only its width is unreadable this early. A reader acting on that message goes looking for a declaration that is already there and already correct.

The pass that skips is the only place that knows why, so it now records the name and the use site reports the reason. Nothing else changes: the value still folds wherever types are known, a name that was never declared still reports absence, and a later pass that does bind the name reports nothing at all.

Following the alias by name in the loading pass would answer sooner and would be wrong: at that stage nothing says which declaration a spelling denotes, and reading a name-keyed table for the answer is the mistake #2764 documents.

#### A folded cast records the signedness of the type it casts to (#2935)

`int_ct` in the lowering fold built its `CTValue` without setting `int_unsigned`, and a `var` record default-initialises to zero, so every constant produced by a folded cast claimed to be signed - including a cast to `u64`. The field exists to tell a negative value from a magnitude above i64 range, and those two share a bit pattern, so a wrong answer there is unrecoverable by anything downstream. Its two sibling constructors, `make_int_value` and `ct_float`, each set every discriminating field they own.

It changes no output today, and that is stated rather than assumed. `int_ct` is reachable only from `convert_const_scalar`, whose single caller reads the destination type for signedness rather than the field, and every other reader of `int_unsigned` is fed by `comptime.eval`, which does not route through it. Compiling the tree with a compiler built before and after produces byte-identical objects, 225 of them, on x86-64.

The issue this closes was filed describing a different defect: that a comptime integer is not evaluated at its declared width, so `val X: u8 = 200 + 100` errors rather than wrapping. That does not reproduce. The constant wraps at its declared width and agrees with the same computation at run time, including through division and shift, which are the operations where narrowing once at the end would differ from narrowing at each step. The refusals in the comptime evaluator fire at i64 and u64 range, matching the literal-range rejection they were built beside, and a `u8` computation never reaches them.

#### The compiler was silent both times a pass dropped a value and missed a use (#2930, #2931)
`#2749` was an `i32x4` accumulator on x86-64 that came out holding the multiplier. `#2917` was a conditional float merge on riscv64 that read its register undefined. Both were silent wrong answers, both were found by running a program, and `--verify-ir` exited 0 on both reproducers. The pass mistake behind them is ordinary. Being undetectable was not.

The instruction pool keeps every instruction a pass ever created, detached ones included, so an operand naming a dropped value still resolves and reads as an ordinary use. Only the block lists say what reaches the back end, and nothing measured operands against them. The one check that could have was dominance, which walked block bodies and terminators and never phi lists, and returned early for a phi on top of that, so a phi incoming naming a dropped value was invisible twice over. It also rode the `full` flag, so it ran only in the post-DCE stages.

`VC_DANGLING_OPERAND` now builds the live set once per function from the block lists and scans every live instruction's operands against it. It needs no dominator tree and no reachability, so it is always on and holds at every pipeline stage rather than only some of them. Dominance no longer exempts a phi: an incoming is measured at the END of the predecessor the pair names, which is the point the value has to be live out of for the edge copy to read it.

Phi destruction had the matching hole and is what turned the IR defect into wrong code rather than a build failure. `lower_block_phis` searched a phi's operand pairs for the predecessor edge it was emitting and, finding none, moved on with no copy and no error, while the four other malformed inputs in the same function each fail the build. Out-of-SSA is total, so a merge register that is not defined on every path in is a malformed input rather than a case, and it now errors with the phi's block, the predecessor and the result vreg, since the pass that dropped the incoming is well upstream of MIR lowering.

Both are diagnostic. Object output is byte-identical per target across x86-64, aarch64 and riscv64.

## [4.18.0] - 2026-08-08

#### A whole `#[uniform]` or `#[storage]` block crosses a call by value (#2922, #2926)
Handing a descriptor-bound block to a helper, `o = from_uniform(block)`, was refused for `spirv`, and so was assigning one whole record to another before #2911 landed. Reading a block's members one at a time worked, which is what made the refusal look like a design position rather than the two lowering faults it was.

The first is an address materialization. A global in a value position had its symbol LEA'd into a register before the argument or return placement was made, which is a repair for a machine whose move records no relocation for a symbol operand. On a logical-addressing target it destroys the only reference the emitter can name, so `MIR_GEP` off a symbol arrived where a whole-object transfer should have. It is now gated on `MachineModel.flat_addressing`, the same axis that gates the float materialization (#2655) and the aggregate member walk (#2649), so the transformation is gated rather than the target. Every machine target's output is byte-identical, object for object, across five targets.

The second was hiding behind the first. A descriptor-bound block's storage is declared with a **Block-decorated struct carrying its member offsets**, which is a different type from the plain struct the same `rec` has in a signature or a local, and the two cannot share an id because the decorations are part of the type's identity. A load that re-derived the type from the IR produced an `OpLoad` whose result type did not match its pointer: a module `spirv-val` rejects and the compiler wrote without complaint, for a whole-block copy that already compiled. A load now reads the variable's own declared type and `OpCopyLogical` carries the value to the type its position or its destination takes, which is the operation SPIR-V defines for exactly that crossing and which is defined only between logically matching types.

One more fault came with them. The emitter's write scan lists the operand positions that only READ, and it listed them for `MIR_LOAD` and `MIR_GEP` alone, so a whole-object read through `MIR_MEMCPY` or `MIR_AGG_LOAD` counted as a store: a `"readonly"` storage block passed by value was refused for being written, and a storage block read whole lost the `NonWritable` decoration that lets a vertex stage read it without `vertexPipelineStoresAndAtomics`.

The refusal's own text is gone with the rule it stated. It said an aggregate assignment lowers to a byte copy, which stopped being true when `MIR_MEMCPY` (#2911) and `MIR_AGG_LOAD` / `MIR_AGG_STORE` (#2914, #2915) gave the target whole-object transfers, and a copy between two objects whose types do not match logically is refused by `emit_memcpy` under the reason that is actually its own.

### Changed

#### A target owns its own type definitions, and `#[spirv_op]` becomes `#[op]` (#2888)
The compiler knew what a sampler was. `src/lang/type.mach` carried a name grammar (`[i|u](sampler|texture|image)<dim>[ms][array][shadow]`), an `ImgForm` reader, a `TypeImage` payload, dimensionality and form enums, and a word list to print a handle back. None of it belonged to a language, and the consequence for a shader library was that it could not add a handle type, could not name a dimensionality, and could not add an instruction without a compiler release.

The operations half of this moved in #2891. This is the types half. `TargetDefs` grows a type-constructor table beside the operation one, with the same contract and the same two readers: the front end closes the value set so a misspelled constructor is a compile error at the declaration, and the back end mints the type from the same row. A row carries an operand LIST rather than just an arity, because a constructor may compose over another type, and an entry is either a literal word or the tag of the constructor a type operand must name. It also carries a refusal hook, which is how a target says why it cannot emit a combination its constructor spells.

A **bodyless `def` carrying `#[handle(target, constructor, operands...)]`** declares a type the owning target mints:

```mach
#[handle("spirv", "image", TEXEL_F32, DIM_2D, NO_DEPTH, NONARRAYED, SINGLE_SAMPLED, SAMPLED)]
pub def Texture2D;

#[handle("spirv", "sampled_image", Texture2D)]
pub def Sampler2D;
```

`def` is the carrier rather than an empty `rec` because it already means "this name denotes a type" and promises no fields and no storage. An empty `rec` promises storage, fields, copyability and nestability, and an attribute retracting all four would make the declaration form assert the opposite of the truth.

`Sampler2D` **names** the image it wraps rather than restating that image's operands, so there is one statement of the shape and the two cannot disagree. That required a real change to how a decorator argument is read, since an argument is an expression position and a bare type name was refused with `` `Inner` is a record type, not a value ``. The change is scoped to this directive and to an argument that names a handle: a type name in `#[align]` is still refused exactly as it was.

The rules a handle carries are fixed and closed, never varied per declaration, and each follows from the one fact the directive states, that the program does not own the representation: no fields, no indexing, no construction, no local binding, no record or union field, no pointer or array around one, and an extent the target declares. On a target that mints no such type the declaration is inert, so a library of handles still compiles for a CPU.

The four refusals the old grammar carried by name survive, re-keyed to the operand that causes each rather than to a spelling, so a depth image, a multisampled image, a `Rect` / `Buffer` / `SubpassData` dimensionality and a storage image are each refused at the declaration with what SPIR-V would need instead. Two further checks close the operand words themselves, which a grammar did not need: an operand is any comptime word a declaration folds, so a value outside the set the target emits would otherwise be written into the module verbatim.

**The name grammar is gone in the same change.** `sampler2d` and its relatives no longer exist, `src/lang/type.mach` loses 495 lines and carries a target-neutral `TYPE_HANDLE` whose payload is a constructor tag and opaque operand words it never reads, and `#[spirv_op(set, name)]` is now `#[op(target, set, name)]` with no alias. Carrying the target as a string in the source rather than in the directive's name is what leaves the front and middle ends unaware of anything but the machinery.

A textured shader emits a **byte-identical module** across the migration: the sampled-image fixture built with the old spellings and with the new declarations produces the same 3984 bytes, and `spirv-val --target-env vulkan1.3` accepts both.

### Fixed

#### A float converted to an unsigned integer is value-preserving across the whole range (#2909)

x86-64 has no unsigned truncating convert, and `encode_float_conv` ignored the destination's signedness, so `MIR_FP_TO_SI` and `MIR_FP_TO_UI` both selected the signed `CVTTSS2SI` / `CVTTSD2SI`. Every source value at or above `2^31` for `::u32`, or `2^63` for `::u64`, exceeded the signed destination range and returned the x86 integer-indefinite value silently. `(3000000000.0::f64)::u32` gave `2147483648` where `doc/language/operators.md` promises a value-preserving conversion wherever the value is representable, and it is.

A sub-64-bit unsigned destination now converts at 64-bit width and the store keeps the low bytes. A 64-bit unsigned destination takes the two-arm sequence: below `2^63` it converts directly, and at or above it converts `x - 2^63` and adds `2^63` back. The boundary is a pooled constant read by both the compare and the subtract, a scratch register holds the biased copy so an allocated source is never written, and a NaN source takes the direct arm because `UCOMIS` sets CF when unordered.

The boundary itself is the trap this hid behind: at exactly `2^31` and exactly `2^63` the indefinite value equals the correct answer, so a test probing only the boundary passes while the entire range above it is wrong. riscv64 was correct throughout, because `fcvt.wu.d` and `fcvt.lu.d` exist and were selected.

#### A comptime float is evaluated at its declared width (#2918)

`CTValue` held a float in a bare `f64` with no width, so every float constant was computed at double precision whatever its declared type, and narrowing happened only at the far end when the constant was emitted at its IR type. A constant and the identical computation performed at run time therefore disagreed: `val B: f32 = A - 16777216.0` folded to `3f800000` where the running program produces `00000000`.

The value now carries the IEEE width its payload is at, beside the signedness the integer side already had. An untyped float literal adopts the context width, so an all-literal `f32` chain is an `f32` chain from its first literal, and every operation reconciles its operands to the shared width and rounds both operands and the result. Rounding once at the end is a different and still-wrong answer: `1.0/3.0*7.0 - 2.0` at `f32` gives `3eaaaaab` rounded once, indistinguishable from rounding nothing, and `3eaaaab0` rounded at each step. Mismatched declared widths are refused rather than folded at a guessed width, since mach has no implicit widening.

#### A conditional float merge is written on both edges on riscv64 (#2917)

The vector placement stage rewrote each block's phi operands and terminator immediately after filling that block, against a replacement map that is only complete once every block is filled. A lane read's replacement is recorded when its own block is walked, so a use in a lower-indexed block kept an identifier that no longer existed, and phi destruction then emitted a copy only for the incoming it could resolve and none for the other edge. The merge register was read unwritten on the not-taken edge, at `-O2` only, on a target with no 128-bit vector unit.

The per-block rewrite is replaced by one whole-function pass after the fill loop, which is the mechanism the gap-operator stage already grew for the same defect class.

#### The IR verifier holds an aggregate bitcast to one size (#2899)

`check_convert_width` judged a bitcast by `type.bit_width`, which answers `0` for pointers, arrays, structs, unions and functions, and bailed on the zero, while the sibling class check asked only that both sides were aggregates. Between them nothing held an aggregate bitcast to one extent, so a 16-byte struct reinterpreted as an 8-byte one passed the verifier and reached codegen. The reason it had stood is that an aggregate's size is a layout answer needing a target, and the verifier was never given one: it now carries `VerifyOpts`, and the equal-size test runs before the zero-width bail so a zero-sized aggregate is judged like any other.

#### A shader built at `-O0` and was refused at `-O2` (#2921)
Three sequential array loops in one function built cleanly for `spirv` at `-O0` and at `-g`, and `-O2` refused them: `a loop whose exits do not reconverge on one block is not yet supported by the SPIR-V target`. `-O2` is what ships, so a shader that only compiles at `-O0` does not work, and a profile that reorders and simplifies must not turn an expressible function into an inexpressible one.

Two defects, both raised by the same case, and they answer different questions.

The **auto-vectorizer had no business running on a logical-addressing target**. It rewrites an element-wise loop into a runtime-guarded vector copy plus a scalar remainder, and both halves are byte-oriented: the alias guard compares the two ranges' END ADDRESSES, and the fast copy re-reads an element pointer as a packed vector. SPIR-V has neither -- its pointer names one object at exactly one pointee type -- so the rewrite produces IR the back end cannot express at all. The pass asked only `has_v128`, which spirv answers yes because it really does have vector types and vector operators. Having vector registers and addressing memory as bytes are two questions, and the pass now asks both: `flat_addressing` gates the transformation, so every machine target keeps its vectorization byte for byte and the logical one stays scalar.

The **structurizer's loop-merge search was also too weak**, independently. What the versioner produces is a guard picking between two copies of one loop, both branching straight to the block after them. That flow is reducible and every loop in it has exactly one exit, so SPIR-V can express it: neither header dominates the shared block, but a block interposed on a loop's own exit edge is dominated by that header by construction, and the loop reconverges there while the shared block stays the selection's merge. The reconstruction already had that mechanism for a merge block another construct had claimed, and simply did not reach for it here. It refused instead, which is what the reported error was.

The vectorizer's shape is the one the reconstruction met first, so fixing only the structurizer moves the refusal to the access chain rather than removing it, and fixing only the gate leaves a legal shape refused for the next pass that produces one. A gep whose whole index list is a leading `0` also stopped reporting itself as `an access chain of more than 8 levels`, which it never was.
}

#### A SPIR-V value is typed by what defines it, not by a MIR width (#2919, #2920)
Two modules the compiler wrote while reporting success, and that only the external validator refused. A `for` loop whose counter is 8 or 16 bits allocated the counter as a 64-bit variable while its increment and its bound stayed narrow, so the comparison mixed bit widths. A `:~` between two 16-byte vector types declared an `OpTypeInt 128`, which SPIR-V has no form for, beside an `OpBitcast` that was itself typed as the vector it read rather than as the one it produces.

Both reconstruct a type from a **MIR byte width** at a point where the value's real type lives somewhere else. A phi merge materializes its constant at the machine word because a sub-word write to a machine register leaves the upper bits unspecified, which is a statement about a register file SPIR-V does not have. The emitter already knew a copy is not the source of type truth and skipped register copies, but not immediate ones. A conversion reconstructed its source type unconditionally, and interning a type declares it, so a 16-byte source width put a scalar integer in the module the instruction never read. And `compute_vec_lane` describes an instruction by its first vector operand, which is right for every instruction whose two sides share a shape and wrong for the one whose operand shape is exactly what it discards.

**The emission point now refuses rather than writing an invalid module.** `type_int` and `type_float` return 0 for a width SPIR-V has no type at, on the same terms `type_vector` already returns 0 for a component count no Shader module may declare, and the arithmetic and comparison forms refuse an operand whose own type is not the one its position declares. A width guard alone could not have seen the loop counter, where every individual type was legal and only the pairing was wrong.

Every machine target's output is unchanged byte for byte: 675 objects across linux-x86_64, linux-arm64 and linux-riscv64, plus a vector-reinterpret fixture built for all three, which is the shape the shared lane descriptor touches and which mach's own source does not contain.

#### An aggregate is passed and returned by value on the SPIR-V target (#2914, #2915)
`fun sum(p: Pair) u32` was refused for `spirv` with an addressed-memory diagnostic, and `fun make() Pair` failed earlier still with a bare `spirv.emit: unreadable move source` that named no opcode, no function and no position. A shader API where a record cannot cross a call in either direction is not a finished one.

Both come from the shared ABI lowering describing every aggregate placement in **bytes**. An argument was copied into an outgoing stack frame in 8/4/2/1-byte chunks, and a return was materialized into a caller-supplied sret slot and copied out of it. SPIR-V has no frame to build the first in and no pointer to follow for the second, and it never needed either: a composite is passed by value natively, as an `OpFunctionParameter` of the record's own type, and returned by value as an `OpReturnValue` of it.

The lever is the calling convention, which is where every other aggregate placement is already decided. `abi.CLASS_AGG` says the object travels as **one value** in an argument or return position, at its own type, and the spirv convention returns it for an aggregate. It is deliberately not a machine-model property: the addressing model does not settle a calling convention, and sysv64 and win64 already disagree about aggregates on one machine. A future target that passes composites whole declares the class in its own convention file and inherits the whole path.

MIR carries the transfer whole through `MIR_AGG_LOAD` and `MIR_AGG_STORE`, the calling-convention siblings of `MIR_MEMCPY` (#2911). Neither carries a byte count, unlike `MIR_MEMCPY` and `MIR_MEMZERO`, and that is the contract rather than an omission: those two are expanded into a sized run on a flat target, there is no expansion of these anywhere, and a byte count is exactly the fact the class exists in order not to use.

The unlocated internal error was a diagnostic bug in its own right, so it is fixed independently of the feature. Every refusal the SPIR-V emitter raises from inside instruction emission now goes through `mir.instr_refusal`, internal errors and declared unsupported features alike, so each one names its opcode, its function and its source position.

Every machine target's output is unchanged byte for byte: 224 objects each on linux-x86_64, linux-arm64, linux-riscv64 and windows-x86_64, and 225 each on darwin-aarch64 and darwin-x86_64.

#### A comparison result crosses a call and a return boundary on the SPIR-V target (#2910)
`mix(h, x == x)` compiled for `spirv` handed the `OpIEqual`'s `%bool` straight to an `OpFunctionCall` whose callee declares `%uchar`, and `ret x == x` from a `u8` function returned one out of a `%uchar` function. `spirv-val` rejected both. Neither is float specific: an integer comparison did it too.

A SPIR-V comparison yields an `OpTypeBool`, which is abstract and has no storage layout, so it cannot be an argument or a return value where a byte is declared. Every other target computes a comparison into a 0/1 byte in a register, where the value widens by zero extension and nothing has to be reconciled. The emitter already converted at every position that knows the type it is converting to: a store to a variable, a store through a pointer and a store into a record member all emitted the `OpSelect` that turns the boolean into the declared type's 0 and 1.

The two positions that did not are the two that go through the ABI pre-coloring's nominal register file, and they share one cause. That file is written by moves emitted at the machine word, which cannot type the value, so it already carries an immediate unmaterialized and mints the constant at the use site with the width the declaration asks for. A boolean now rides the same way: the register slot records that it holds one, and the use site - the `OpFunctionCall` that knows the callee's parameter types, or the `OpReturnValue` that knows the function's result type - widens it, or leaves it alone where the declaration is itself a boolean. Comparisons are still typed as booleans, so `OpBranchConditional` keeps receiving the real `%bool` the specification requires there.

#### `$length_of` folds in a comptime gate, and a gate folds the same way in both passes (#2875)
`$if ($length_of([4]f32) != 4)` was refused, in a function body and at module scope alike, with `` `$offset_of` has no value in a comptime condition `` against an operand that is plainly a fixed array. `$size_of` and `$align_of` had folded in a gate since #2857, so the one layout intrinsic that counts elements was the odd member of the set.

Lowering walks a `$if` chain by evaluating every gate again, so a layout gate is measured twice: once by sema, choosing which arms are type-checked, and once at lowering, choosing which arms become code. Lowering's layout resolver answered `$size_of` and `$align_of` and returned "not mine" for `$length_of`, and the evaluator renders that as `$offset_of`'s refusal. It now answers the count from the operand's `element_count` over the sema type, which is where an element count belongs: how many elements a type holds is a property of the spelling and is the same on every target, unlike a size, which is measured through the IR against the target's layout.

That exposed the second half. Lowering reads the operand's type out of the slot sema stamps on it, and the ordinary sema fold, the path a plain layout gate takes, folded the gate without typing it first, unlike the two folds beside it. So the operand carried no type at lowering, where `$length_of` read no count and `$size_of` silently measured **zero**: `$if ($size_of([4]f32) == 16)` type-checked its true arm in sema and then lowered its false one, with no diagnostic from either pass. A gate sema folds is now a gate sema typed, so the two passes read the same measurement and select the same arm.

#### `--pie` and dynamic linking on a loaderless OS are refused by name (#2898)
`mach build --pie` on a freestanding ELF target failed with `elf: PIE program headers exceed the one-page header reservation (too many load segments)`. That was wrong in both clauses. There were four load segments, not too many, and the reservation was not a page: freestanding declares `page_size = 1` because bare metal has no paging, so the check refused whatever the image contained. On riscv32 a third answer arrived first, the ELF32 dynamic-image message. None of the three named the reason.

The reason is one sentence. A position-independent executable exists to be relocated by a program loader when it maps the image, a dynamically-linked one to have its imports resolved by one, and a shared library to be loaded into something else. `os = "freestanding"` has no loader at all, so each of those is a category mistake in the request rather than a layout that did not fit, and someone passing `--pie` to a bare-metal build should be told which mistake they made rather than shown a segment count:

```
error: link: a position-independent executable needs a loader to relocate it, and os = "freestanding" has none
error: link: a dynamically-linked executable needs a loader to resolve it, and os = "freestanding" has none
error: link: a shared library needs a loader to map it, and os = "freestanding" has none
```

The refusal reads `os.OsVTable.mapped_by_loader` (#2895) in the linker, at the one line that picks the writer family, so the fact stays in one place and any future no-loader OS inherits it without an object-format writer being touched. It also settles the underflow #2895 left standing: `header_segment_vaddr` frames the header block a page below segment 0, which at a base address of 0 wraps to the top of the address space, and the PIE and dynamic ELF writers still computed it unconditionally. They were kept off it only by the reservation check happening to fail first. With the request refused before a writer is chosen, neither writer is reachable on a loaderless target at all, so the precondition holds by construction rather than by arithmetic accident.

Every linux, darwin and windows image is unchanged byte for byte, `--pie` included, as are freestanding `of = "elf"` and `of = "raw"` static images.

#### A freestanding target can select `of = "elf"` and link, on every architecture (#2895)
`os = "freestanding"` declares `page_size = 1`, and that is deliberate: bare metal has no paging, so segments carry no alignment obligation and 1 packs them tightly in a flat image. The ELF writer read that length as the size of the one-page reservation the ELF header and the program-header table have to fit inside, so the test was `64 + n*56 <= 1` and every freestanding ELF image was refused no matter how few segments it had. The objects were written correctly; only the link step failed, on x86_64, aarch64, riscv64 and riscv32 alike.

Behind the refusal sat a second fault that the refusal was hiding. `header_segment_vaddr` frames the header block one page **below** segment 0, which at a base address of 0 and a page of 1 is `0 - 1` - an underflow to the top of the address space. Relaxing the check alone would have traded a loud link error for a leading `PT_LOAD` mapping the header block at `0xFFFFFFFFFFFFFFFF`: an image that links and is malformed, which is worse.

Both come from applying a loader construct to a target with no loader. The leading `PT_LOAD` covering the header block exists so a **program loader** can read the program headers back out of the mapped image - `PT_PHDR`, PIE self-relocation, dynamic linking. A bare-metal image has none: the reset vector enters at the entry point and nothing parses a header at run time. So a loaderless image now emits no header segment at all, and the header block stays file metadata that is never mapped. That is also the only shape available at base address 0, where there is nothing below segment 0 to map it in.

The gate is a fact the OS states about itself - `os.OsVTable.mapped_by_loader`, carried to the writer through `of.ImageOptions` - and deliberately not `page_size == 1` read as a sentinel. A page size is a length in bytes and whether an image is loader-mapped is a property of the platform; spelling one as the other is the same unit confusion that produced the bug. Every linux, darwin and windows image is unchanged, byte for byte, as is `os = "freestanding"` with `of = "raw"`.
#### A cast that converts nothing emits nothing (#2881)
`lower_cast` chooses a conversion from the source and target pair and fell through to a bitcast whenever the two shared a width, so a cast whose operand already had the destination type emitted a reinterpretation of a type as itself. In SPIR-V that surfaced as `OpBitcast %ulong %ulong_2`, an instruction with no effect, which is how it was found.

It was never only cosmetic. An identity bitcast is an opaque SSA definition, so the value behind it stops being a constant: `~(0::usize) / 0xFF` in `std.memory.raw_fill` - the byte-splat every `raw_fill` performs - lowered to a runtime `div` on every target, because `0::usize` on a literal already typed `usize` put a bitcast between the constant and the fold. Seventy such divisions disappear from the compiler's own image.

The fold is keyed on the two IR types being EQUAL rather than on their widths matching, which is the distinction that makes it safe: a same-width cast that genuinely reinterprets - an integer and a float, where the bitcast is what carries the general/float register-bank crossing to the backend - has two different IR types and still emits. It sits in the lowerer's conversion selection rather than in `builder.emit_bitcast`, because choosing no instruction is a selection decision and the builder's callers, including the IR verifier's own width test, are entitled to emit exactly what they ask for.

Secrecy is unaffected. `OP_BITCAST` is classified a secret move, so an identity bitcast was a link in the taint chain the `#[oblivious]`-required check reads, but sema forbids `::` and `:~` from adding or dropping `^`, so a cast and its operand always agree on secrecy and the operand's own definition is already stamped. A secret compute reached through an identity cast is still required to declare `#[oblivious]`.

#### An RV64-only mnemonic in an `asm riscv32` body is refused rather than assembled (#2864)
`ld`, `sd`, `addw` and the rest of the `*W` group do not exist at XLEN 32, and an `asm riscv32` body naming one assembled it anyway. The image carried an instruction the hardware does not implement, so the mistake surfaced as an illegal-instruction trap at run time instead of a diagnostic at build time - a wrong answer through a green build, which is the failure mode this target class exists to catch.

The cause was a split authority. One mnemonic table served both machines and the rows carried no width, so the assembler knew the spelling and the encoder knew the machine and neither asked the other. The fix states the fact once, on the instruction: every `MachOp` declares the register widths it exists at, and both the assembler and the encoder read that. A mnemonic is refused because the instruction it names does not exist here, which is why an alias, a pseudo spelling and a row the suffix decoder synthesizes all inherit the answer without being listed anywhere - `sext.w` and `amoadd.d` are refused on rv32 for the same reason `addiw` is.

The set is per instruction rather than a minimum register width, so an RV32-only form is expressible when one arrives rather than a reshape of the layer. A form the module does not classify admits nothing, so appending an instruction without stating its machines refuses it loudly instead of admitting it everywhere quietly, and a unit test walks the shipping table and the decoder's whole synthesized range to hold that.

The shift-amount field went the same way: it is as wide as the register it shifts, so `slli a0, a1, 32` is refused on rv32, where its sixth bit would have landed in `funct7` and decoded as a different instruction. `slli a0, a1, 31` still assembles, and every RV64-only spelling still assembles under a `riscv64` tag.

`doc/language/asm.md` no longer tells the reader to gate these spellings themselves.
#### A CI leg no longer fails because an unrelated apt repository is down (#2885)
`apt-get update` exits non-zero when any configured repository is unreachable, including repositories the job never installs from. The runner image carries Microsoft's and azure-cli's lists preinstalled, and a 403 from them failed `int pinned (linux)` while every Ubuntu repository the job actually reads fetched fine. It cleared on a rerun, which is worse than a hard failure: it teaches everyone reading CI that a red `int pinned` is probably noise.

Package installation moved into one composite action the four sites call. The update's verdict is no longer the step's, and the install's is - a package a leg genuinely needs and cannot find still fails it, loudly, which is the signal the update was standing in for. Dropping the unused third-party lists first would work today at the price of a list of other people's repositories to maintain against a runner image nobody here controls; the install is the check that needs no such list. It lives in the action rather than at each call site so a leg cannot be added under the old shape.
#### A wide global is executed and value-checked on the rv32 reference core (#2883)
The rv32 harness loaded `.text` alone out of the unlinked object image, so no program touching a global could be executed: `.data` was not in the core's memory at all, and a global reference on rv32 is an `auipc` / `lw` pair against `%pcrel_hi` and `%pcrel_lo` that the linker patches, so even with the data present the address would have been zero. That is why the #2867 repair - one native access per lane at successive addends on the same symbol - had no in-tree value check and had to be verified against an external interpreter.

The harness now runs the build through the shipping linker, the same call `mach build` makes for a flat-image format, and loads the whole linked image. The probe is entered at the image base, where a flat image begins execution and where the linker places the entry symbol, so the harness still lays out the arguments itself rather than reaching the function under test through a compiled wrapper that would agree with the compiler by construction. The image loads at the harness's own base rather than the link's, which works because rv32 reaches both code and data pc-relatively.

A wide global is loaded, stored, added and subtracted over the lane-crossing value sweep, with every answer checked against host arithmetic, and with the 32-bit sign boundary held in a second global. The store is checked three ways so a wrong lane addend cannot cancel itself out: read back in the same call, read again in a later call so the value had to reach memory, and followed by a re-read of a neighbouring global that the store must not have damaged.

The remaining eleven probes still run from the unlinked `.text`, because a flat image carries no symbol table and so admits exactly one entry point. That is #2892.
#### An ELFCLASS32 object is now Elf32 all the way down, not a 32-bit class byte on 64-bit bytes (#2861)
The ELF writer took one thing from the target's pointer width - the class byte in `e_ident` - and then wrote Elf64 structures regardless. Every size, every field offset and every field width after byte 4 was a 64-bit literal. A 4-byte-pointer target therefore produced a file that announced ELFCLASS32 and then contradicted itself in its own header, which is worse than refusing: a reader trusts the class byte and misparses everything after it. The reader half had the same shape, walking Elf64 offsets without ever consulting the class byte it had just read.

The geometry now lives in one record. `ElfLayout` states one class's structure sizes and alignment, `layout_for` derives it from the pointer width on the write side and `layout_from_class` from the class byte on the read side, and a set of `write_*` / `read_*` helpers own where every field goes - callers pass values and never offsets. So the class byte and the geometry come out of one call and cannot disagree.

Two of those fields **move** rather than narrow, which is the part a "write a pointer-sized field" rule would have got wrong while looking right: `Elf32_Phdr` puts `p_flags` after `p_memsz` where `Elf64_Phdr` puts it right after `p_type`, and `Elf32_Sym` puts `st_value` / `st_size` before the three small fields rather than after. A segment header with correct addresses and its permissions written at the 64-bit offset loads with no access at all and faults at the first instruction, nowhere near the mistake. `r_info` is a third case again - neither a width change nor a reorder, but a different packing, with eight bits for the relocation type instead of thirty-two. A type that does not fit is now named before any output is allocated rather than truncated into a different, valid relocation.

DWARF had the same defect one layer up. `address_size` was already derived from the pointer width and `emit_addr` already wrote 4 or 8 bytes, but every relocation attached to those fields was a hardcoded `RK_ABS64` - an 8-byte write over a 4-byte field, corrupting whatever followed it. The kind now comes from `of.abs_kind_for_pointer_width`, the same call the data-pointer path already used, so the relocation width and the field width are one decision.

`riscv32` is registered on the ELF format as a result: an rv32 relocatable object and an rv32 static executable are both accepted by `readelf -a`, `llvm-readobj --all` and `llvm-dwarfdump --verify` with no errors. The three dynamic writers - PIE, shared objects, dynamically linked executables - lay their tables out in ELF64 throughout and now refuse an ELFCLASS32 target by name, because an Elf32 dynamic image needs `Elf32_Dyn`, a 4-byte GOT slot, rv32 PLT stubs and the rv32 dynamic relocation types, none of which exist. That is #2894, and the refusal says so rather than emitting an image that looks valid and is not.

Output is byte-identical on `linux-x86_64`, `linux-arm64` and `linux-riscv64` at debug, `-g`, `--pie` and release.

#### An `i64` global compiles on rv32 instead of crashing the compiler (#2867)
A wide global feeding wide arithmetic on riscv32 killed the compiler with SIGSEGV and no diagnostic, and the same construct without the arithmetic produced a refusal. Both came from one place. Lowering folds a global straight into the access as a symbol operand, so a `MIR_LOAD` or `MIR_STORE` on one carries no memory operand at all, while `legalize` entered its memory path on the opcode alone and then indexed the memory operand it assumed was there. The index was `-1`: one shape read a wild pointer and faulted, the other read one slot past the operand array and refused off whatever it found. The refusal was never a decision.

The pass now decides that a memory access is one by finding the address, not by reading the opcode, and it recognizes the two forms an address takes: a memory operand walked by displacement, and a symbol walked by addend. So a wide global is no longer refused either - it is legalized like any other fixed base, one native access per lane at successive offsets on the same symbol, which is what a 64-bit value on a 32-bit machine has always cost. An access whose address is neither form falls through to the named refusal for its opcode rather than into an expansion that cannot describe it.

mos6502, the other target that legalizes to a narrower ALU, is unaffected: an `i64` there needs a frame slot and the pass refuses the wide `MIR_ALLOCA` before any access is examined.

### Added

#### A declaration-scope `$if` that declares nothing can ask about a type (#2876)
`$if ($size_of(MeshUniforms) != 64) { $error("MeshUniforms must be 64 bytes"); }` at module scope was rejected: the gate was folded while names were being resolved, and no type has a layout yet at that point. The layout check people actually want to write - a wire format, a uniform block, two records that share a slot - had nowhere to live except a runtime test that only fires if someone runs the suite.

A declaration-scope `$if` now runs at one of two times, and what its arms contain picks which. A chain some arm of which **declares something** is decided while names are resolved, because what it decides is which declarations exist, and every later stage reads that set - a genuinely circular question to ask a type about, since the type universe is built from the declarations the gate is choosing. A chain no arm of which declares anything contributes no name and no type whichever arm wins, so nothing depends on deciding it early: it is decided during type checking, where its gate measures a type, queries one, or compares one. The measurement runs through the same path a function-body gate uses, so a size measured in one position and the same size measured in another cannot disagree.

Which time applies is decided **syntactically, over every arm** - `$if`, every `$or`, and a trailing `$or {}` - before any gate is evaluated. One declaring arm anywhere keeps the whole chain at the earlier time. A per-arm rule would make the stage that runs the gate depend on which arm the gate selects, which is not knowable at the stage that would have to choose. Because a `use` is a declaration, conditional imports - by far the commonest declaration-scope `$if` - are untouched, as is the refusal a declaring chain gives a type question, now naming the arm that declares as the reason.

The cost is stated rather than left to be discovered: `$if` runs at one of two times depending on its contents, written into `doc/language/comptime-control.md` and onto the deferral itself. The one visible consequence is ordering - a reached `$error` under a deferred chain is reported during type checking, so an unrelated resolution error in the same module is now reported before it rather than after.
#### The riscv32 float path is executed rather than only emitted (#2874)
A histogram of a broad rv32 corpus shows `fld`, `fsd`, `fadd.d`, `fsub.s`, `fmul.d`, `fdiv.d`, `fmv.w.x`, `fcvt.w.d`, `fcvt.s.d`, `fcvt.d.w` and `fcvt.d.s`. Not one of them had ever been executed. The reference RV32 core modeled RV32I and M and refused a float **by name** - honest about its coverage rather than skipping what it did not model, which is what made the gap visible instead of silently green - so every float assertion this target had was that codegen emitted something, and none was about what the emitted bytes compute. That is the same shape that let four integer defects sit in rv32 through a passing suite.

The core now models the closed RV32F and RV32D set from the manual: the float register file with its NaN-boxing rule, the loads and stores, the arithmetic, the compares, `fclass`, the sign-injection family, `fmin` and `fmax`, the whole `fcvt` family and the four fused multiply-adds, each written out as the number the manual gives next to the mnemonic it gives, importing nothing from the encoder. The ten RV64-only float opcodes are refused by name, so the core can **say** it met an `fcvt.l.d` on a 32-bit target rather than computing an answer the hardware would have trapped on.

IEEE 754 is the deliverable rather than the instruction count. NaN-boxing is enforced on both sides, so an improperly boxed single-precision operand reads as the canonical quiet NaN as the manual says - the FLEN-versus-XLEN asymmetry that already produced two frame defects on this target. The float-to-integer conversions follow the RISC-V rules rather than the host's, which are undefined exactly where RISC-V is defined: NaN converts to the destination maximum, out-of-range saturates, and a value in (-1, 0] is zero and is not out of range.

An executed sweep runs the conversions, the arithmetic, the compares and the precision changes over both NaNs, both infinities, both zeros, a subnormal, the saturation boundaries and a tie that rounds to even only if the rounding is right, with every answer checked against the host's own float unit and never against mach's riscv64 target. It runs on `ilp32`, `ilp32f` and `ilp32d`, because where an `f64` rides differs across the three, and the probe's own interface is entirely integer so the harness can lay out arguments identically for all of them and still reach the float unit inside.

#### riscv32 converts between a 64-bit integer and a float, open-coded (#2860)
RV32 has no instruction for it. `fcvt.d.l`, `fcvt.lu.d`, `fcvt.s.l` and the rest of the family name a 64-bit GP operand through their `L` selector, and no RV32 register is that wide, so `i64 -> f64`, `f64 -> u64` and their six siblings had no encoding and were refused by name. `be.codegen.legalize` could not supply one either: it splits a wide integer into native lanes and threads carries between them, and a conversion is a **single rounding of the combined value**, not two lane conversions recombined afterwards, so the model the pass is built on does not apply to it.

The conversion is now open-coded from 32-bit conversions and float arithmetic, rather than lowered to the soft-float helpers (`__floatdidf`, `__fixdfdi` and their twins) a C toolchain would call. The helpers are correct by construction and are a fraction of the code, and they were rejected because they introduce a **runtime dependency this backend has nowhere else on this target**: a freestanding RV32 image, which is the configuration mach's RISC-V targets exist for, would then need a libgcc-shaped runtime linked before an `i64 -> f64` worked at all. The conversion would be missing on exactly the target that wanted it most. The tradeoff is recorded where the expansion is written, because the price of the choice is that a rounding mistake is a silent wrong answer instead of a failure.

Each direction is an identity that rounds exactly once. `i64 -> f64` is `f64(hi) * 2^32 + f64(lo)`: each lane converts exactly, the scale is a power of two, so the closing add is a sum of two exact values. `f64 -> u64` is a truncating split whose two scalings and one subtraction are all exact, and which inherits the saturation and NaN rules **from the native 32-bit `fcvt` in each lane** rather than testing for them, so `NaN` lands on the destination maximum and a negative input on zero without a compare in sight. `f64 -> i64` converts the magnitude that way and then clamps and negates, which is what puts `NaN` and `+inf` on `INT64_MAX` and `-inf` on `INT64_MIN`.

The `f32` forms may **not** take the `f64` result and narrow it. That is a double rounding and it is not innocuous here: a 64-bit integer can round in `f64` to exactly an `f32` midpoint without being one, and ties-to-even then breaks the wrong way. The value is first rounded to odd at a fixed 11-bit granularity when it needs more than 53 bits, which leaves the `f64` carrier exact and makes the narrowing the only rounding. A sweep over values constructed to sit on that midpoint catches the naive version at every exponent from 2^54 up, and passes as written.

The four arms fire only where the conversion's float side rides a float register bank, so mos6502 - one register class, floats in integer lane groups - keeps its refusal, and every 64-bit target is untouched: x86-64, aarch64 and riscv64 objects are byte for byte what they were.

#### A storage binding nothing writes is emitted `NonWritable`, and can say so (#2879)
A `#[storage(set, binding)]` buffer that a stage only reads was emitted with no `NonWritable` decoration, so every consumer had to enable `vertexPipelineStoresAndAtomics` to read one from a vertex stage. The read-only case is the common one - a joint palette, an instance buffer, a light list - and it is the case where the decoration is free and its absence costs a hardware requirement.

The emitter now decides it from the module. A storage binding no emitted body stores through is decorated `NonWritable`, over the same emitted closure every other whole-module rule here is stated about, so a store from a drawn-in library body counts exactly as a local one does. The analysis is a **proof rather than a search**: two positions read - a load's address and an access chain's base, whose result is then subject to the same question - and every other appearance of the binding or of a pointer derived from it counts as a write. A missing decoration is therefore possible and a wrong one is not.

`#[storage(set, binding, "readonly")]` states it explicitly, and a store through such a binding is a compile error naming the line that wrote it, on every target rather than only on a SPIR-V build. That is what the qualifier buys over the inference: an accidental write does not produce a wrong module, it produces a correct one that quietly widens the pipeline's requirements, and the qualifier turns it into a diagnostic. The emitter refuses a write it can still see after lowering - an address handed somewhere it cannot follow - so the decoration cannot become a promise the module breaks. The memory qualifier rides after the descriptor pair rather than in a second decorator name, so a further qualifier is one accepted spelling and no new interface role.

Everything else is unchanged: of boom's seven shaders, the six with no storage binding are byte-identical, and `skinned_vert` differs by exactly the one decoration.

#### A layout intrinsic can be measured inside a comptime `$if` (#2857)
`$size_of` and `$align_of` are constants, but they could not appear in a `$if` condition, so the thing most worth asserting about a layout - a **relationship** between two of them - could only be a runtime test. Two records a renderer writes into one slot at a fixed offset must be the same size; if they diverge, one shader reads its colour out of the middle of a matrix, with no diagnostic at any layer. A test catches that only if someone runs the suite. A `$error` catches it in the build that introduced it.

`$if ($size_of(A) != $size_of(B)) { $error("..."); }` now compiles inside a function body, as does a comparison against a literal and an intrinsic folded into a larger expression.

The measurement is not new and deliberately is not duplicated: the comptime evaluator gained a resolver seam, and sema answers it through the same `fold_layout_intrinsic` a type position already used, so a size measured in an array length and the same size measured in a `$if` cannot disagree. Lowering installs its own resolver beside the type-query one it already had, because a chain gated on a comptime parameter or inside a generic body has its arm selected at monomorphization and never reaches sema's.

Resolve reaches the position through a mechanism that was already there: `bind_comptime_if` defers a chain it cannot fold yet, binding every arm and leaving arm selection to a later pass, which is what a type comparison has always done. A layout gate joins that list rather than introducing a second rule.

### Changed

#### A refused layout intrinsic now says which position refused it, and why (#2857)
`a layout intrinsic has no value in this position: it measures a type, which only a type position resolves` was true of every position and is now true of one. A **declaration-scope** `$if` still cannot measure, because it selects which declarations exist and that is decided before any type is laid out, so it says that instead and points at the position that does work. `$offset_of` is named apart, since it is unavailable for a different reason - a field offset is decided at lowering - and a message about type checking would be a stale explanation the moment the resolver exists.

Two limits are filed rather than left implicit: `$length_of` still does not fold in a gate (#2875), and no declaration-scope gate can see a type at all, which is shared with type queries and comparisons (#2876).

### Changed

#### A target now owns the operations it defines (#2888)
The compiler knew what a SPIR-V instruction was. `src/lang/spirvop.mach` sat outside the target holding 41 hand-written string comparisons that mapped an instruction name to a number, and the front end read it directly. Adding an instruction meant editing the compiler, so a shader library could not add one at all.

The table now hangs off the ISA vtable and the target fills it. It reaches the front end as data on the comptime context beside `pointer_width`, for the reason stated there: it is a target machine fact the frontend needs, and the frontend has no target. So `fe` names no back end, and unlike before it holds no SPIR-V numbers of its own. `src/lang/spirvop.mach` is gone, and the IR carries the set tag and opcode under target-neutral names.

An instruction's arity is now checked, which nothing did before. The decorator's contract is that operands come from the parameters in order, so a declaration whose parameter list is the wrong length assembled the instruction with the wrong operand count. The family looks uniform and is not: `Reflect` takes two operands where `Refract` takes three, and `FMin` two where `FClamp` takes three.

Emitted output is unchanged. Every machine target is byte-identical, 224 objects each across x86-64, arm64 and riscv64, and the SPIR-V fixtures emit the same bytes.

This is the first piece of #2888. Type constructors, the `#[handle]` and `#[op]` declarations, and the bodyless `def` are not here.

### Fixed

#### A shader indexing with a `u32` no longer requires `shaderInt64` (#2878)
A SPIR-V module whose arithmetic was entirely 32-bit declared `OpCapability Int64`, because every array index was widened to the target's pointer width before it reached the emitter. That is right for a byte-addressed machine, where an index is scaled by the element size and added to a base address and so must fill a machine word. SPIR-V addresses logically: an `OpAccessChain` index selects a member, nothing scales it, and the widening bought nothing.

It cost a device requirement. `shaderInt64` is an optional feature, so a shader that performed no 64-bit arithmetic was refused outright by `vkCreateShaderModule` on hardware without it, and was invalid usage on hardware with it unless the application had enabled the feature. Downstream this was worked around by requesting the feature unconditionally and refusing to start without it, which narrowed the hardware the application ran on.

The index width is now taken from `MachineModel.flat_addressing`, the same axis that already gates float address materialization and aggregate member walks (#2655), so the transformation is gated rather than the target. A genuinely 64-bit index still declares `Int64`, since that is what the program asked for. Every machine target's output is byte-identical.


### Added

#### RV32 is a target (#2778)
mach compiles for 32-bit RISC-V. `isa = "riscv32"` with `abi = "ilp32"`, `"ilp32f"` or `"ilp32d"` resolves a full machine description, and `$mach.arch.riscv32` and the three `$mach.abi.ilp32*` tags exist so a body can be guarded for it.

Nothing about the RISC-V backend was duplicated to get there. One `build_riscv` fills either arch's vtable, register machine, relocation seam and model from the register width it is handed; the register numbering, the psABI roles, the selection pack, the encoder, the printer and the attribute derivation are shared unchanged. On the convention side the three ilp32 members differ from their lp64 twins in exactly one declared fact, `xlen_bits`, and a test builds each pair and compares every other field rather than asserting that in prose.

The width facts are the machine's and are read as such. `max_alu_width` is 4 on rv32, which puts an `i64` above the ALU and sends it through the same lane-splitting `legalize` pass that serves the 6502's 8-bit one. FLEN stays 64 because rv32gc has the D extension, so a float register is twice the width of an integer one - the pair that a single "word size" gets wrong, and the one the frame layout now reads as two separate strides.

**What rv32 does not do yet, and says so.** The ELF writer stamps ELFCLASS32 from the pointer width while emitting Elf64 structures, so `of.elf` deliberately does not cover this arch and an rv32 linux tuple is refused at resolve time rather than producing a file whose class byte contradicts its layout (#2861). rv32 composes with the `raw` flat image today. Converting between a float and a 64-bit integer has no RV32 instruction and no lowering, so both the encoder and `legalize` refuse it by name with the issue number rather than emitting an RV64 opcode the hardware would trap on (#2860).

#### A shader can use an interface variable declared in another module (#2843)
#2823 let a shader call a function defined in a shared module, but the shader could not use an **interface variable** one declared. A library that owned anything beyond pure math had nowhere to put its descriptor set: a drawn-in body naming its own module's `#[uniform(...)]` found nothing, and the access was refused as computed addressing.

The cause was that `Emit.root` was the module being compiled and every global resolved against it, so `declare_interface`, `iface_slot_of`, `global_var_id` and the block-layout helpers could only ever see the root's. Globals now flatten into one unit-wide table exactly as functions did, `GlobalEnt` carrying the `(member, index)` pair, and every per-global array is indexed by a table index. A global's type ids are read against **its own module's** type table, which is what the `(member, index)` pair exists to make possible - reading them against the root's would resolve each id to whatever type sits at the same index there, which is a wrong type silently rather than a failure. An extern shadow is never entered in the table for the same reason: it is minted against the consumer's type table, so its type id means nothing outside the module that made it.

A drawn-in module contributes globals by **reachability, not membership**, the rule the function table already stated. A shared module may declare a whole descriptor set, and a shader importing it for one helper does not inherit the rest. The root is exempt: its interface list is the module's pipeline contract, declared whole. Existing single-module shaders emit byte-identical modules.

The driver half closes the module set over cross-module **global** references, not only function ones. A shader that reads a shared module's uniform and calls nothing in it must still draw that module in, and before this it could not resolve the variable at all.

### Fixed

#### riscv64 images claimed an instruction set they did not have (#2828)
`.riscv.attributes` carried an arch string taken from a per-ISA constant. A constant describes what mach's encoder emits, and an image is not what its encoder emits - it is that plus every object linked into it. A mach image containing a clang object built for `rv64gc` claimed no compressed extension and none of the z-extensions it actually held, so a disassembler configured from the image decoded its compressed float loads as `<unknown>`. This is the same class of false claim `e_flags` carried before #2813, one section over.

Nothing states a string now. An image's claims are derived from the two facts that decide them - the resolved ABI and the emitted bytes - and a linked image's are merged from its inputs. `BuildAttributes` no longer has an arch field at all, so a constant is not merely unused but inexpressible.

The merge rule is the arch's, because a union is right for the ISA string and wrong for everything else. Stack alignment must agree or the inputs cannot be linked. Unaligned access is a permission, so the image permits it if any input needed it. A privileged-spec disagreement leaves the image unable to state one, so it states none. A tag mach does not model survives only while every input that states it agrees. No shared layer can know which tag takes which rule, so the seam hands the arch the whole attribute body and reads none of it; the ELF layer keeps only the container, which the ARM EABI defines identically.

`obj.rehome_remap` copies the body rather than aliasing it. Rehoming exists precisely because the source allocator is going away, so a field holding a pointer into it needs a copy and not an assignment - the note on that function now separates the two failure modes, since a missed scalar reverts to zero on the parallel path while a missed pointer segfaults somewhere else entirely.

#### Two modules could claim one descriptor set and binding with no diagnostic (#2843)
Within one module a repeated `(set, binding)` is a typo its author can see. Across modules it is neither: two shared libraries can each declare `#[uniform(0, 0)]` without either author seeing the other, and the shader importing both is where the conflict first exists. `spirv-val` does not catch it, because two variables at one pair are well-formed SPIR-V and the conflict is with the pipeline layout, which is not in the module.

The pair is now checked over the set one entry point reaches, across the descriptor roles rather than within one, since a `#[uniform(0, 0)]` and a `#[storage(0, 0)]` are two descriptor types claiming one slot. The Location overlap check widened the same way, and both diagnostics name the declaring **module** when it is not the one being compiled - a message naming two `camera`s tells the one person who can act on it nothing.

## [4.17.0] - 2026-08-08

### Fixed

#### mach's riscv64 objects were not psABI-conformant, so no foreign linker would consume them (#2828)
A `R_RISCV_PCREL_LO12_I/_S` is required to name a **label at the paired `auipc`**, not the symbol being addressed. mach named the symbol, so `ld.lld` refused every object outright, one error per pair, before resolving anything: `relocation points to a symbol '.Lconst.0' in a different section '.rodata.cst'`. Nothing was wrong at runtime, because mach's own linker and its own producer agreed with each other. That agreement was the problem, and it is the same self-consistency that hid #2797 until a foreign object was used.

riscv64 codegen now mints one `.Lpcrel_hi.N` label per `auipc` and points every low half at it, which is the spelling the psABI states and the only one a foreign linker accepts.

The pairing normalization moved out of the ELF reader and became a **declared seam**: `isa.RelocSeam` gained an optional `normalize_image` hook that the linker runs over every input once, before anything reads a relocation, skipped for a relocatable link. That matters more than where the code sits. The reader only ever saw objects arriving from disk, so an image handed straight from codegen never passed through the chase, and once the low half named a label with a zero addend an unnormalized image would have resolved to a flat zero - reintroducing #2797 from the opposite direction. One rule now applies to every riscv64 image entering the linker regardless of provenance, and the reader's two spellings collapse into one.

Two places fabricated a symbol type rather than reading one, both true only while every text symbol happened to be a function entry: `pack_symbols` OR-ed a function flag into every mark, and the ELF writer fell back to the section kind. A mark now states its own type and an untyped symbol writes `STT_NOTYPE`.

The change also exposed a defect in shared code that had been latent because its precondition was rare. Dropping a duplicate weak function refused whenever **any** symbol sat inside its interval, so the moment every `auipc` in such a body carried a label, no duplicate weak body was ever dropped again, and a `-g` link described a discarded body with a second contradictory DIE. The rule is now that a local nothing names cannot keep anything alive: a local resolves by index rather than by name (#2520), so only a relocation in its own module can reach one, and a debug-sourced relocation does not count (#2572).

Measured rather than estimated: 31 mach objects plus a clang probe went from **228 `PCREL_LO12` errors and no binary** to zero errors, a linked binary, and correct values read back. Objects grow **8056 bytes, 3.00%**, for 215 labels at 37.5 bytes each. The labels are local and never reach an image, so a linked binary carries none of them.

The `.riscv.attributes` section is still stamped from a constant describing what mach's encoder emits rather than what the linked image holds, so a compressed float load can still fail to decode from it. That is the same class of claim one section over, and it is tracked on this issue rather than closed with it.

#### `--emit-asm` printed two different global references identically (#2839)
`&g.lo` and `&g.hi` printed the same line while the objects relocated against `g` and `g+8`. A reader comparing the printed assembly against the object had no way to see the difference, which is the same failure as #2821 arriving through a different field.

The addend is now carried once, on `isa.Operand` beside the symbol id, and read by the printers. No encoder reads it, exactly as no encoder reads the symbol id, which is what makes the change provably byte-neutral: 1344 files across three targets at both profiles, none differing.

The field holds **what the line must spell**, which is not the operand's offset and not the relocation's addend. Those coincide nearly everywhere and diverge at two real sites, both load-bearing. riscv64's `%pcrel_lo` relocates with its own distance back to the `auipc` folded in, so the pair's constant belongs to the `%pcrel_hi` and the low half must spell none, or it doubles. aarch64's GOT pair always relocates with addend zero, because a GOT relocation cannot carry one and the offset rides a trailing `add` instead, so `:got:g+8` would name an instruction that assembles to different bytes than the object holds. Each such site carries its reason inline.

The ambiguity was demonstrated rather than asserted. With the fix reverted and the new tests in place, every expectation was flipped to the old output and the whole suite passed, which proves the two states genuinely printed the same string.

#### A sub-128-bit vector stored to a global was written from the wrong register bank on aarch64 (#2838)
`g = a + b` for a global of a vector type narrower than the vector register emitted `str x2` for a sum that was sitting in `v2`. The two registers share an index and nothing else, so the global received whatever the general-purpose register happened to hold, which reads back as stack-address noise rather than as the lanes. Local and stack-record-field destinations were correct throughout, and x86-64 compiled the same source correctly, so the defect was aarch64 with a global destination and nothing else.

**It never shipped.** The direct sub-width vector access added in #2716 was the first lowering to send a vector value to a global destination, so the regression is unreleased and 4.16.0 is unaffected. That is also why it survived review: a fixture built with the released compiler cannot reproduce it.

The cause was a resolver that did not check what it was resolving. `req_gpr` accepted any `MIR_OP_PREG` and returned `isa.regid_index`, which strips the class tag off a composite register id, so a vector operand came back as the general-purpose register that aliases its index. `req_vec` was the same function with different error text. That defeats the entire purpose of the composite id, which exists so that the encoder can read a register's class straight off the operand and a float value can never be silently encoded as a move of the GP register sharing its number. **x86-64 already carried the equivalent check**, in `encode_mem_operand`, with that rationale written out beside it. This was two targets failing to meet a standard the third had already set, which is why the guard now lives in all four resolvers on both targets rather than at the call site that happened to expose it. riscv64 has no caller that reaches it today, and the check is what keeps that a fact rather than a coincidence.

The ordering fault that exposed it is worth recording alongside the fix, because both directions had it and only one was dangerous. `emit_store_mem` and `encode_load` each place the destination or source kind branch above the register class dispatch, so a symbol operand skips the bank decision. The store miscompiled in silence. The **load** direction failed the build outright with `encode: expected a memory operand`, refusing a valid program that x86-64 accepts, because `resolve_mem` validates its operand kind before using it. One resolver checked and one did not, and that is the entire distance between a build error a user reports in an hour and a wrong value that reaches production. Both arms now dispatch on the value's register class, and the folded-global load and store select the SIMD and FP form family, so a vector value reaches memory as `str d2, [x16, :lo12:sym]` and a global vector source loads as `ldr d0`.

Emitted bytes move on aarch64 only, and only for a sub-width vector reaching a global. The compiler's own source contains no such construct, so its objects are byte-identical on all three targets before and after.

The regression test keys on the **destination kind** rather than on a vector shape, since destination kind is the axis the dispatch was missing from and every lane count fails and passes together. Its observable is bit 26 of the load and store word, the V bit naming the register file, because the miscompiled store is frameless and is a genuine `str`, so neither a frame-presence assertion nor a mnemonic could tell it from the correct one. That is precisely how `int/surface/vector-subwidth-direct` stayed green over a live miscompile, and `int/surface/vector-subwidth` could not see it either, because it only ever builds local and stack-field destinations.

One existing assertion had to be corrected in the process. `access_width_refusals_emit_nothing` required `emit_global_store` to refuse a 16-byte store while passing it a vector operand, and it refused only because the bank was being ignored. `fp_access` carries a Q row and `RK_LDST128_LO12` exists, so that store is legal and now encodes. The refusal assertions were rewritten against a general-purpose value, where the absence of a 16-byte integer form is the real rule, with the accepted vector arm beside them.
#### `mach build -v` timed the wrong regions, so `codegen` read as microseconds for work that took seconds (#2816)
The phase readout printed `codegen 222 modules 343us` for a phase that actually took 670 ms under a thread pool, and 3.7 s on a serialised build. `optimize` had no row at all and had not had one since `c785ca39` folded lowering and optimization into a single query. The rows covered 51 % of the wall on a pooled build and 48 % serialised, with no statement anywhere that they were a partition or that they were not.

A wrong number is worse than a missing one, and this is the shape of why: an absent `optimize` row invites the question, while a confident `343us` answers it falsely and survives unexamined. `codegen` was timing the **cache adoption** that follows the pool rather than the pool that does the work, from `b558af4b`.

Spans now bracket where work is **dispatched** rather than where a pass function happens to sit, so `lower_phase` covers lowering and optimization together and `codegen_phase` covers the pool and its adoption. Because those two cannot both be plain wall spans, the readout grows one primitive: a span attributed to one phase and deducted from its host, which makes the two rows exactly disjoint and together exactly the whole. An item now carries a `Duration` rather than a start instant, so a worker times its own module and the main thread replays that measurement after the join - the only place a pooled module's cost exists. A phase declares its concurrency and the row prints it, so items that deliberately sum past their row say so instead of implying a partition that does not hold. A recap prints an `other` row for time no phase claimed, so rows plus `other` equal the total exactly and an untimed region appears rather than vanishing.

Six further figures were wrong and are corrected with them: `-vv` codegen items summed to 0.21 ms under a 640 ms row, `link` included writing 222 object files, `emit` was folded into `link` rather than named, and an `--all-targets` recap was cumulative, so the second unit reported the first unit's time as its own.

#### `mach build -v` reported the modules that survived the load phase rather than the modules it loaded (#2850)
Building `dep/mach-std` printed `load 93 modules 96ms`, and at `-vv` printed one hundred and eleven item lines directly beneath that header. Both numbers described the same phase and they disagreed on the same screen. Eighteen of the one hundred and eleven were uncounted, which is 16.2% of the modules and 11.0% of the phase's own measured item time, so the per-module rate a reader derives off the row reads as 1.03 ms where the truth is 0.86.

**This one shipped.** The call dates to `dd267534` on 2026-07-18, so 4.16.0 has it, unlike most of what landed alongside. It reproduces on every unit of `--all-targets`, and it appears for any library carrying per-platform modules, which is to say for the standard library that nearly every project builds against.

The cause is a phase reporting a different population than it processed. `run_load_phase` closed with `p.topo_len`, the front-end work set, while a target-gated orphan is a module the phase reads, lexes, parses, times and prints an item for, and only then deliberately keeps out of that set because its load-time `$error` guard fired under this target. The row's duration and its items covered the loads. Its count covered the survivors.

The count is now what the phase loaded, and the excluded subset is named rather than left to be noticed: the row reads `load 111 modules 98ms (18 target-gated, not carried forward)`. Reporting the larger number alone would have traded one puzzle for another, since a reader would then watch eighteen modules vanish before `resolve` with nothing to explain it. That suffix does the job the existing `(16 threads)` suffix does, which is to stop a number that is true from supporting a conclusion that is false.

**Not a regression from #2816.** That work moved spans to where the work is dispatched and left this count untouched. The two are separate claims about the same readout: one about which region a row times, this one about which population a row counts.

The audit reasoning is the part worth keeping, because it is reusable. #2816 recorded row counts as correct on the grounds that every counted module really is processed. That is sound, and it establishes one direction of a biconditional. The direction that fails is the unstated converse, that every processed module is counted, and nothing in the table marked which half had been checked. The fixture then hid the difference: every measurement in that work was taken on the compiler's own self-build, whose tree contains no target-gated module, so `load` and `resolve` both read 228 and matched perfectly. A fixture where two candidate rules return the same number cannot distinguish them, and it validates whichever one is written. That is the same shape as `narrow-stack-args`, where an argument at offset zero validated any scheme that drops the offset entirely.

So the regression test builds a tree that contains a target-gated module, asserts the relationship between the counts rather than any literal, and asserts that the drop is non-zero, so the fixture cannot quietly decay back into the case where the two rules agree. Every other phase was checked the same way, by comparing each row's count against the items printed under it on a build that has target-gated modules. `resolve` and `sema` report the topo set and walk exactly it, `lower`, `optimize` and `codegen` report the emit set and walk exactly it, and `emit` reports the objects it wrote. All of them match, and the test now pins that property for each of them rather than only for the phase that was wrong, so a phase added later has a standard to meet instead of a precedent to break by accident.

#### Every duration mach measured on darwin was zero, so `mach build -v` printed `0ms` for every phase (#2854)
`mach build -v` on darwin reported `0ms` on every row, `0ms` for `other`, and `0ms` for the recap total, and the test runner reported the whole suite as `(0ms)`. Nothing was slow and nothing was wrong with the readout. The clock underneath it was stopped.

mach-std's darwin backend called syscall 427 for `clock_gettime`. XNU has no `clock_gettime` syscall at any number, because the public `clock_gettime(3)` is implemented in userspace over `mach_absolute_time` and the commpage rather than as a trap, so hunting for a number was never going to work. 427 is `fsgetpath`. The call failed on every invocation, the wrapper zeroed its output, and both clocks returned the epoch. It has been that way since mach-std `f19a4ce` on 2026-03-11, which replaced a working `gettimeofday` implementation - carrying a `TODO` correctly stating that darwin has no such syscall - with the one that does not exist. **It shipped in 4.16.0.**

The fix is mach-std 0.26.0, which routes darwin's clocks, sleep, threads and filesystem through libSystem, verified natively on both macOS arches rather than cross-compiled and assumed.

Nothing caught it for five months because no test read a duration back and asserted on it. #2816's two readout row tests are the first that do, and darwin runs only as the `dev` to `main` release gate, so this is the first time the two ever met. That is also why the failure arrived as two opaque bound violations rather than as a statement about the clock, and why a test now asserts the clock advances before the rows that rest on it.

**Darwin executables now link `/usr/lib/libSystem.B.dylib` and carry `LC_LOAD_DYLINKER`, `__STUBS` and `__GOT`, whether or not `--pie` is passed.** Previously a darwin image reached the kernel through raw traps and so happened to be `MH_NOUNDEFS` with no dynamic loader at all. `--pie` has never selected whether dyld runs and does not now; the import list does. The `macho-framing-static` case was named for that coincidence and its comment asserted it as a rule, which the case's own observable never checked, since it reports segments and the entry command and no load command. Both goldens are updated and both comments now say what actually separates the two layouts, which is the rebase stream and with it `__DATA_CONST`.

#### mos6502 reserved eight-byte stack slots and spill slots on a one-byte machine (#2818)
Shared backend code decided argument-slot granularity, spill-slot size and spill-slot alignment from a literal `8` rather than from `MachineModel`. On the three shipped 64-bit targets that literal is right, so this looked like latent cleanup ahead of a 32-bit target. It was not: mos6502 has a one-byte machine word, so every outgoing stack argument consumed an eight-byte slot it never had, and every general-purpose spill reserved eight bytes for a move that writes one. Fixing it moves that target's ABI, and both halves of the compiler agree on the new layout, verified by executing emitted bytes on a reference 6502 core.

Twelve width decisions now read the model. Three of them turned out not to be the machine word at all, and two of those sit in one function: `stack_arg_align` aligns a by-reference argument to the **pointer** width while the granularity floor three lines below is the machine word. One literal, one function, two different questions that coincide today. The third, a vector register width in the register allocator, is left literal and documented rather than parameterised without a way to verify the byte movement.

Float operations are also gated out of width splitting. The existing gate keyed on a vector flag that was only ever a proxy for the register bank, so gating on the class subsumes it rather than sitting beside it, and the gate is applied at the lane planner too - the planner and the gate must read one authority, or the plan describes a split the rewriter never performs.

The 6502 test harness had to change with it, and that is the more interesting half. It held its own `UT_MOS_ARG1 = 8`, with a comment asserting that the mos6502 convention rounds a stack argument to an eight-byte slot. No such rule existed. The harness carried a second copy of the compiler's constant and agreed with it by construction rather than by checking it, which is the same defect one level up, inside the thing meant to catch it. It now derives the offset.

#### `--emit-asm` printed lines that name a different instruction than the object holds (#2821)
The three assembly printers render the `isa.Inst` an encoder built, so a field the printer omits is a field an assembler fills from the form's default - and a default is not necessarily what the encoder chose. On riscv64 the float-to-integer converts are emitted round-toward-zero and printed with no rounding mode, so `llvm-mc` assembles the printed line to `d3 75 05 c0` while the object holds `d3 15 05 c0`. The line named a different instruction. Every float form the encoder emits was checked against the assembler's default for that form rather than assumed, and exactly one family diverged.

On x86-64 a `REP`-prefixed string move printed identically to the unprefixed one. That form is encoded but nothing currently selects it, so it is stated as latent rather than dressed up as a live defect.

Every field of every operand kind was given a verdict per ISA, including the ones that were already correct, and the ambiguities were demonstrated rather than asserted: with the fix reverted and the new tests in place, each expectation was flipped to the old output and the suite still passed, which proves the two states genuinely printed the same string. The remaining gap is named rather than patched - a secret taint and the other metadata that leaves no trace in the bytes cannot be shown by a surface that renders the encoder's output, and belongs to the IR validator instead.

#### A constant aggregate's pointer leaf recorded a fixed 64-bit relocation regardless of the target's pointer width, refused loudly on mos6502 and silently wrong wherever a narrower-pointer target's applier does not check (#2835)
`pack_agg_relocs` recorded `of.RK_ABS64` for every pointer leaf inside a constant aggregate initializer - a `str` literal, an address-of-global, a function pointer - with a doc comment asserting the kind is "format-independent, so no target is needed." Format-independent it is; *width*-independent it is not. mos6502 (2-byte pointers) refuses the kind outright: `error: relocation 'abs64' not supported` on a module-level `val` holding one function-pointer leaf, while the identical source builds a clean object on every 64-bit target. The width ladder already existed - `RK_ABS64`, `RK_ABS32`, and `RK_ABS16` (added by #2280 for exactly mos6502's pointer width) - the emitter just never consulted it.

`of.abs_kind_for_pointer_width(w)` is the one authority now: it maps a pointer width in bytes to the `RK_*` kind whose patched field is exactly that wide, and its inverse `of.is_pointer_abs_kind(kind, w)` answers whether a relocation already in hand is the target's own pointer-abs kind. `pack_agg_relocs` calls the former, keyed on `isa.IsaVTable.pointer_width`, instead of hardcoding `RK_ABS64`. Four `be.linker` predicates that matched `r.kind == of.RK_ABS64` to mean "an in-image absolute pointer" - DWARF code-address classification, DWARF tombstoning, a Mach-O in-place import bind, and PIE base-relocation collection - now call `is_pointer_abs_kind` instead, so narrowing the emitter could not turn a loud build error into a silent wrong address on some future narrower-pointer target whose relocation applier does not refuse an oversized write the way mos6502's does (the asymmetry #2835 named: a shared RISC-V reloc seam across XLEN would write eight bytes over a four-byte field on `rv32`/`ilp32` and say nothing).

No emitted byte changes on any of the three 64-bit targets, verified by comparing object output byte-for-byte between the unpatched and patched compiler: every one has `pointer_width == 8`, so the selector returns `RK_ABS64` exactly as before and nothing moves. mos6502 now builds the reproducer from a fresh compiler and produces a correctly 2-byte-patched pointer field - hand-verified by inspecting the emitted flat image, since `int/` carries no mos6502 case (it is neither in `mach.toml` nor `int/targets.conf`) and none was added under the freeze. `mach.lang.target.of.abs_kind_for_pointer_width` and `mach.lang.target.of.is_pointer_abs_kind` pin the width ladder and the predicate's agreement with it directly.

#### riscv64 ELF headers claimed a soft-float non-compressed ABI for images that were neither (#2813)
Every riscv64 object and image mach wrote carried `e_flags = 0x0`, which on RISC-V means *soft-float ABI, no compressed extension*. Both halves were false: the back end passes floats in `fa` registers under `lp64d`, and a linked image holds whatever compressed instructions its inputs held. Five ELF writers each wrote a literal `0` into the field, which is not a defaulting bug but **one fact stated five times**, and all five statements were wrong.

The cost was real rather than cosmetic. `ld.lld` refuses to combine a mach object with a clang one at all (`cannot link object files with different floating-point ABI`) before it looks at a single relocation, so the header alone made mach's riscv64 objects unusable by any foreign linker. And a soft-float/hard-float mismatch produces *exactly* the row of believable zeros that #2771 and #2797 both wore, so a header that lies about it is actively expensive to debug around.

Both bits now come from the only honest source each has. **The ABI bits are the ABI's**: `isa.machine_flags` derives them from the resolved convention's own `float_arg_bits`, which the riscv family already sources from each member's `flen_bits`, so there is no second table of convention names to drift from the first. **The feature bits are the bytes'**: this encoder emits no compressed instruction, so an object mach writes honestly claims none, while a linked image ORs the bit across its inputs the way a real linker accumulates it. An arch that declares no derivation has no flags word and carries 0, which is the truth for every arch but riscv64.

The linker now also **checks its inputs against the target** instead of writing a header that contradicts half its own bytes, naming the object that disagrees: `object 'probe.o' was built for a double-precision hard-float ABI, but this link targets 'lp64', so every float argument crossing that boundary would read back as garbage`. That is the same disagreement `ld.lld` reports, caught where it can still be explained rather than where a float reads back as 0. The ABI-bit mask is asked of the arch rather than written into the linker, so a variant added later needs no edit here, and any bit outside that axis is carried through rather than judged.

One field on `of.ObjectImage` carries it, set from the ABI when the back end builds an image, parsed off the header when the reader loads one, combined across inputs when the linker writes one. The parallel-codegen path re-homes an image field by field onto the session allocator, so it copies the word explicitly - a field added to the record that nobody adds there is silently dropped for every parallel build while a serial one keeps it, which is how this defect first hid during development.

Verified by hand across all three riscv ABI variants, object and image, using the same `e_flags` reader `int/lib/cc.sh` already applies to the clang side: `lp64` reads `0x0` soft, `lp64f` reads `0x2` single, `lp64d` reads `0x4` double, and a mach+clang image reads `0x5` (double-float plus the RVC bit its clang input contributed). All three still run. A mixed mach + clang `ld.lld` link no longer stops at the float-ABI header check; it now proceeds to the separate psABI relocation-spelling problem tracked as #2828.

#### A constant-folded integer feeding an integer-to-float conversion failed to compile on riscv64 and arm64 (#2812)
`si_to_fp` / `ui_to_fp` reached the riscv64 and arm64 encoders with an immediate integer source whenever it was provably constant - `fcvt.{s,d}.{w,l}[u]` and `SCVTF` / `UCVTF` both read a GP register and have no immediate form, so both refused it outright with `error: encode: expected a register operand`. Found on a release build (`me.pass.constfold` proving a runtime sum constant); it turns out the shape is reachable from `debug` too, whenever the IR builder itself can already prove the source constant at build time - the defect was never tied to `-O2`, only to whichever path first proves a source constant. x86-64 never noticed, because it happened to resolve the source through code that materializes an immediate as a side effect nothing wrote for that reason - the only backend that worked, worked by luck.

The operand SHAPES a generic MIR opcode admits are now a stated part of the shared MIR contract (`mir.operand_requires_reg`) rather than a fact three encoders each had to rediscover: a slot no register backend has an immediate form for is one the shared lowering fills with a register, the same treatment a symbol operand already got. x86-64's own output moves as a result - one extra register-to-register move per conversion, because `R11` is a reserved, non-allocatable scratch register and can no longer receive an immediate baked in at encode time - and that is accepted as the fair cost of removing the accidental-correctness path rather than worked around.

A deeper, related gap is out of scope here and left as a note rather than a new issue: a provably-constant integer-to-float conversion should arguably fold to a float constant outright at IR build (`si_to_fp f64 100: i8` becoming `100.0`), in which case neither the encoder refusal nor the extra x86-64 instruction would exist on any target.

#### A failed `#[library]` pin said the library was not in the link when it was (#2800)
`import 'X' pinned to library 'glfw' not among the link's dependencies` was reported for a `glfw` the manifest **did** provide. The linker is handed loader names for the link's dynamic half and bare paths for its static half, so a `source = "local"` entry's logical name never reached it at all, and the only check it could run reported the half it could not see as absent.

The message cost a day of investigation before anything was fixed. It named the manifest, so the report went at the export cascade - which was never broken, and is now pinned by a regression case that passes on the compiler that introduced it. The actual fault was a misspelled entry point (`glfwGetInstanceProcAddr` for GLFW's `glfwGetInstanceProcAddress`), which the message said nothing about.

Which message a user saw was decided by accident, too: the same broken program read `undefined symbol` on a link with no shared library in it and the false `not among the link's dependencies` the moment any unrelated one joined. Two different statements about one program, one of them untrue, selected by something neither of them mentions.

The driver's single walk over the link inputs now records each entry's logical name and whether it resolved static or dynamic, and the linker reads that one account rather than inferring the link from the half it was handed. A failed pin resolves to one of two truthful statements - the library is in the link **as a static input**, which defines symbols rather than importing them, so the symbol is undefined; or the library is genuinely nowhere in the effective link set, in which case the refusal stands and now lists what the link does provide. Both paths that can reach a failed pin call one decision function, so the wording no longer depends on the link's shape: the two int cases that differ only by an unrelated dynamic dependency assert **byte-identical** goldens, and a third holds the genuine refusal so the fix cannot become a blanket permit.
#### A type mismatch between two same-named types from different modules said `expected P, found P` (#2288)
A nominal type is interned by `(origin, decl)` and rendered by its bare name, so `rec P` declared in two modules produced exactly this:

```
error: type mismatch: expected P, found P
```

The two types are genuinely different and the diagnostic could not show it - the message names the defect and tells the reader nothing about it. That is the failure this epic exists for, arriving in a diagnostic rather than in a dump.

The disambiguation is a property of the **pair**, not of either type, so it lives at the reporting site rather than in the type renderer: qualifying every nominal at every site would make every ordinary diagnostic noisier to fix a case that is rare. When the two renderings collide, the note names each side's defining module instead of the cast advice, which could not apply between two unrelated records anyway:

```
error: type mismatch: expected P, found P
  = note: these are two different types that share the name `P`: the first is declared in `dcase.main`, the second in `dcase.lib`
```

Every diagnostic that names two types routes through **one** decision point, so this is not a patch at one message site. It reaches the assignment mismatch, the `:~` size mismatch (which collapsed the same way - `P (16 bytes) vs P (1 bytes)`), and `incompatible operand types`, whose collision is unreachable today only because aggregate arithmetic and `==` are refused before the message is built. A generic instance is disambiguated too, since `Box[u8]` from two modules renders identically for the same reason.

Found while auditing the surfaces #2288 lists, by producing both sides of each distinction and diffing rather than by reading the printer. The rest of the type renderer came out clean under the same treatment: secret against public (`^u32` / `u32`), generic arguments, vector lane signedness (`i32x4` / `u32x4`), aggregate against scalar at equal width (`[8]u8` / `i64`), packed against unpacked, over-aligned against natural, and function arity all render distinguishably.

Alongside it, `ut_diag_note_has` - the note-reading sibling of the test harness's `ut_diag_has`, which scans a diagnostic's `message` only. A note is a separate field, so until now **no test in the tree could assert anything about a note's text** and every note the compiler attaches was unpinned. That is the same asymmetry #2297 found between `ut_lower_ok` and the sink-reading helpers, arriving through a different door, and it is worth stating plainly: two arms of this change's own test, written against `ut_diag_has` with a bare module name as the needle, passed against the unpatched compiler because some unrelated note in the same compile mentioned that module. They assert the surrounding phrase now, and all five arms fail without the fix.
#### The IR verifier now checks that a conversion is well-*typed* for its own opcode, not merely well-*formed* (#2808)
The verifier constrained a conversion's **arity** - `OP_TRUNC` / `OP_SEXT` / `OP_ZEXT` / `OP_BITCAST` all sit in the "exactly one operand" bucket - and `VC_TYPE_AGREE` covered operand agreement for binary ops, branch conditions, load/store/gep bases and `ret` against the signature. Nothing related a conversion's **result width** to its operand's, so it checked that a conversion was well-formed and never that it meant what its opcode says. A width-changing `BITCAST`, a `TRUNC` whose result is *wider* than its operand, a `SEXT` / `ZEXT` whose result is *narrower*, and any of the three at *equal* widths were all accepted.

`instruction.mach` states each of these relations on the opcode itself - `OP_BITCAST` is documented as a "same-size bit reinterpretation" - so every one of them is a self-inconsistency in the IR's own type model. #2373 shipped exactly that: `bitcast i32 %p0: i8`, a four-times width change, with `--verify-ir` exiting 0 on the reproducer, reaching users as a `^i8 -1` widening to `255` and a widened `^u8` reading uninitialized register residue.

New violation class `VC_CONVERT_WIDTH`, and it is **always on** rather than gated behind `--verify-ir`: the widths ride on the instruction and its operand with no dataflow needed, and a conversion is exactly where a front-end classification error lands, so it names the site (function, and `path:line:col`) instead of letting the defect reach codegen.

A width the type table alone cannot state is deliberately **not judged**. `type.bit_width` answers 0 - meaning *unknown*, not zero-width - for a pointer, a function type, an aggregate and void, and a pair with an unknown side is skipped. This is not laxity, it is the arm that makes the rule shippable: a `::` cast between a pointer and an integer lowers to a `BITCAST` whose pointer width is the *target's*, which the verifier does not carry, and guessing it turns correct programs into violations. Removing that guard fails the standard library's own `raw_fill` on the first build, which is how it was confirmed rather than assumed.

Because the rule is always on, its blast radius is proven by a clean self-host plus the whole int corpus on all three register targets, not by a fixture set. Each relation is separately mutation-tested: the backwards direction refused, the right direction clean, varying only the widths.

#### riscv64 never read a foreign object's pc-relative target off its paired `auipc` (#2797)
A `R_RISCV_PCREL_LO12_I/_S` does not name the address it loads. The psABI has it name a **label at the paired `auipc`**, and the target is read off the `R_RISCV_PCREL_HI20` sitting at that label. mach never followed that indirection: it took the low half's own symbol as the target - which for this spelling *is* the `auipc`'s address - and measured from `patch_va - 4`. The displacement collapsed to `auipc_va - (patch_va - 4)`, a number about where the two instructions sit with the target nowhere in it.

**This broke every foreign object unconditionally, at every hi/lo distance.** Byte-adjacent halves - exactly the layout the `patch_va - 4` rule assumed - resolve to a flat `0`, which is not the assumption holding but the target having dropped out; halves one instruction apart resolve to `-4`. Neither is the address the pair names. The earlier reading of this as a spacing problem ("a newer compiler puts nothing between the halves, so it is fine") is wrong and worth correcting explicitly, because it implies newer toolchains are unaffected.

**Silent, and it read as zero.** The wrong address held a zero, so a pooled scale factor loaded as `0.0` and every product with it came out exactly `0` - a whole row of believable values rather than a crash. `int/surface/narrow-stack-args` and `int/surface/c-variadic` failed this way on `linux-riscv64` in CI while passing on a developer host, and the difference is **fixture shape, not toolchain vintage**: the newer clang builds that probe's `10.0` with `lui`+`slli` and emits no relocation at all, so it has no pair to resolve wrongly, while clang 18 pools the constant and gets one. A probe that pools under *any* clang fails under all of them. Two earlier fixes chased this from the C compiler's flags instead (#2771's `-mabi=lp64d` pin is separately necessary and stays), and #2797 itself read the identical low-12 immediates on nine consecutive `fld`s as a shared `auipc` - they are simply equal displacements, since the constants and the pairs are both eight bytes apart, and each `fld` has its own `auipc`.

The pairing is now a **per-instruction fact carried on the relocation** rather than a rule re-derived from instruction adjacency. The producer folds this low half's own distance back to its `auipc` into the addend, so `S + A - P` names the `auipc`'s pc for a low half exactly as it names the patch site for a high half, and the applier assumes nothing about where the `auipc` sits. `push_pcrel_lo` states it at emission time by finding the high half it pairs with and carrying that half's own addend forward (so one of the two owns the constant, and they cannot drift apart); the ELF reader states it for a foreign object by following the psABI label, at the boundary where that spelling is the only spelling. A low half with no `auipc` to pair with is refused rather than encoded against a guessed one. This also fixes mach's own inline-asm `%pcrel_lo`, which was equally wrong for any pair with an instruction between the halves.

`int/surface/riscv-pcrel-pair` builds the shape deliberately and is non-trivial on every axis a pairing can silently get right by luck, because `int/surface/narrow-stack-args` - the case this was found through - is trivial on all of them: its one pooled constant sits at section offset 0, with a zero addend on the high half, four bytes from its low half, so it proves the chase happens and nothing about what the chase carries. The new case's pooled scale is the SECOND constant in its section (a dropped symbol offset shows in one row and not the other), its two stores make clang put a non-zero addend on the high half (`kl + 10`, `kd + 18`, invisible whenever an addend is 0), its pairs sit 4, 6 and 8 bytes apart, and the stores reach `R_RISCV_PCREL_LO12_S` where the loads reach `_I` - one resolution path, but a shared path is only covered where something exercises it. It rather than relying on a probe written for something else still containing one, and reads four doubles, four longs, and a pooled scale factor back **by value** - it fails on the unfixed compiler with `doubles n=4 0 0 0 0` and `stored longs=0 doubles=0`. Beside the values it reports three emitted facts, each named for what it actually measures: that the fixture's own object still contains a psABI-spelled low half at all (`label-pairs`, since clang folds a `const` array into immediates and the shape disappears); that no load anywhere in the linked image resolves to an address off its own access width (`misaligned`, which reports 8 and 4 on the pre-fix builds and is what catches this on the image alone); and that none of those addresses lands in the text range (`text-relative`, a true invariant that reads 0 both before and after this fix, kept because it fails closed for a pair resolved onto code and stated separately rather than folded in as though it were doing the catching).

#### A PIE executable and a shared object carried no section headers at all, so `.symtab` (#2772) never reached the two image kinds that need it most (#2807)
`readelf -h` reported 0 section headers on every PIE executable and every shared object, on every ELF target, at both profiles, with or without `-g` - confirmed independently of the #2772 verification pass that added `.symtab` to a static, no-flags executable. A shared object is the image a profiler most needs to symbolize, since the interesting frames in a mixed process are usually in libraries; PIE is the default for ordinary distribution on most modern targets. #2772's own justification - `perf` printing raw addresses instead of names without a `-g` rebuild - applied unchanged to both, and neither got any of it.

The cause was narrower than "PIE and shared never got the feature": the shared section-header-table machinery (`debug_sht_plan`/`debug_sht_write`) all three of `emit_pie_exec`, `emit_shared`, and the real-dynamic-link branch of `emit_dyn_exec` already call had no symtab parameter at all, so there was no trigger that could ever turn the table on for a `-g`-less build - only `debug_count > 0` could, and a release image has none. The existing suppression when nothing needs naming (no debug sections, no allocated sections worth naming alone) **is deliberate** - a release image stays byte-lean by design, and that predates `.symtab`; the fix adds a real symbol table as an independent, second reason for the table to exist, leaving the original suppression's rationale (an empty table for its own sake) untouched.

`debug_sht_plan`/`debug_sht_write` now take the same `symtab`/`symtab_count` pair `emit_exec` has taken since #2772, and `linker.mach` builds `.symtab` entries unconditionally for every executable link (static, PIE, and real-dynamic) and for a shared object, rather than only when writing a static executable. `of.DynExecFn` and `of.SharedFn` grew the same two arguments `of.ExecFn` already carries, so COFF and Mach-O's writers accept and ignore them for conformance, exactly as they already do for `ExecFn`'s pair.

A `.symtab` entry's `st_shndx` names its containing load segment by index (`build_symtab_bytes`, #2772), which `emit_exec`'s own bespoke section table has always registered a `PROGBITS` header for; the shared `debug_sht_plan`/`write` path never had to before, because `emit_pie_exec` and the real-dynamic branch of `emit_dyn_exec` passed no allocated sections in at all. Adding a symbol table without also registering the segments it points into would have shipped a table `nm` scanned into `A` (absolute) rather than `T` (defined text) - structurally present, functionally useless to `addr2line -f`. Both writers now emit one `PROGBITS` header per load segment plus a `.dynamic` header (closing a `readelf -h`/`-l` cross-check `emit_shared` already satisfied), gated on a symtab actually being present so a debug-only (`-g`, no symtab is never the case in practice, but the writers are still called that way directly in their own unit tests) build keeps its pre-#2807 shape.

`.symtab`/`.strtab` sit strictly after every `PT_LOAD`'s file extent, the same byte-additivity `debug_sht_plan` already held for debug sections and #2772 measured for the static writer - carrying a symbol table changes zero loaded bytes and zero runtime footprint on either image kind. Two new integration cases (`int/surface/symtab-pie`, `int/surface/symtab-shared`) reuse `int/surface/symtab`'s own fixture and producer unchanged, asserting `addr2line -f` resolves a mid-function address - not just a function's entry point, the fact `st_size` and not just presence proves - to the right name, per ELF ISA, at both profiles, without `-g`; each fails against a compiler built before this fix with exactly the symptom above (`symtab_present=no`). The shared-object producer's own `readelf -sW` scan is now careful to read from `.symtab` specifically: a shared object also carries `.dynsym` naming the same functions with `st_size` always 0 (that table has never had a size writer, its own separate gap and out of this fix's scope), so a scan that did not distinguish the two tables would silently pass by matching the wrong one's zero size.

#### Every static ELF carried two PT_LOAD segments claiming the same address (#2795)
The static writer mapped the ELF header and program-header table **at** `segs[0].vaddr`; the PIE and dynamic writers mapped them one page **below** it. So every static executable mach has ever produced had two `PT_LOAD` segments covering the same virtual address - the header block and `.text` both at `0x400000` - on all three ELF targets.

**The image runs correctly**, which is why nothing caught it: the loader maps segments in order and the later mapping wins. It is wrong only to a reader, and mach's linked executables carry no section headers, so program headers are all a reader has. `gdb 17.2` aborts outright at process start; a crash reporter or symbolizer doing the same derivation would have been **silently wrong** instead, which is the worse failure and why this is critical rather than cosmetic.

The two paths computed one fact in two places and agreed only by accident. The header-segment base is now a single definition all three writers call, and `header_fits_reserved_page` guards the static path too - it never needed the guard before only because it had no reserved page to overrun. The regression test asserts the general property that **no two load segments overlap in virtual address**, over the emitted headers rather than about one specific pair, so it fails closed for whatever the next segment-layout change does.

### Added

#### `int/observability.md`: what a behavioural golden can never observe, and which surface can (#2822)
A behavioural test builds a program, runs it, and diffs its output, and every such test is blind in the same ways. Nothing said what those ways were, so the blindness was rediscovered one defect at a time. The document names each class and the surface that can see it, and every entry is a defect this repo already paid for rather than a piece of testing advice.

Two overlapping `PT_LOAD` segments at one address (#2795) could never change any program's output, because the loader resolves the ambiguity and the program is correct - and gdb aborting was luck, since a profiler doing the same derivation would have been silently wrong. A pc-relative pair resolved without chasing its label (#2797) needed a **foreign** clang-18 object, because mach's own writer and reader shared the assumption and every mach-only case agreed with the bug. A float access substituting another width's form (#2766) was settled by byte comparison against `llvm-mc`. Eleven mach-std failures on darwin existed for months behind a green CI that cross-compiled and executed nothing, one of them a wrapper whose `if (rc < 0)` error check could never fire because `pthread` does not use errno, so a failure returned as success.

The sharpest section is the one asking which of those defects genuinely needed an integration test. Four of seven did not: #2795's guard is a property assertion over the ELF writer's own emitted headers and needs nothing linked or run, #2766 is settled by an external assembler in a unit test, `e_flags` is a field assertion, and the `pthread` wrapper is checkable from the pthread contract alone. Two genuinely did - Apple's arm64 stack-argument rule is stated only by Apple's toolchain and hardware, and what a current kernel does is not derivable from our source. So most of these were not missed because a behavioural golden is blind, they were missed because **nothing looked at the emitted artifact at all**, which is a unit-test-shaped gap. An integration test earns its place only when the fact it asserts is owned by something outside this repo.

It also records the degenerate-fixture trap, generalized from `int/surface/narrow-stack-args`, which is described and deliberately left unchanged: **offset zero validates any scheme that drops the offset**, because adding nothing is indistinguishable from adding correctly. Alongside the two operating rules already in force - demonstrate a new case failing against the unpatched compiler before adding it, and assert values and effects over emitted shape, goldens over shape being right for `--emit-asm` and encoder output and wrong as a general-purpose assertion.

The document describes the defects and the observation surfaces rather than the harness, so it survives the harness. It is input to whatever replaces `int/`, not a manual for what is there now.

#### A shader can call a function defined in another module (#2823)
`error: a call to a function defined outside the module is not yet supported by the SPIR-V target` for every call leaving the entry module, so a shader project could factor nothing with a body into a shared file. A 4x4 transform written over vector columns — the thing every vertex shader in a project needs — had to be copied into each one.

The gap was not a loop that stopped early. The emitter was **given** one `ir.Module` and had no way to reach another, so widening it is a change to what the driver hands the whole-module back half, and that is where the fix sits. `codegen.EmitModuleFn` now takes a `unit.Unit`: the module being compiled plus the modules it draws called bodies from, lowered. Membership is the driver's call — it owns the module graph — and what is emitted from each member is the emitter's, because it owns the call graph. A register machine is unaffected and is handed nothing: it has a linker, and lowering a module it will not emit would be waste.

The emitter flattens every defined function in the unit into **one table**, and every id, reachability mark and call resolution keys on a table index. A same-module call and a cross-module call are then the same code path, with the boundary visible in exactly one place: a callee's signature is read against the type table of the module that **defined** it, since an IR type id means nothing against anyone else's — the referencing module carries only a signature-only shadow whose parameter list is empty.

What is emitted is decided by reachability, not by membership. A shader module emits the closure of its **entry points**, so importing a large shared module costs the helpers actually called and nothing else, and two entry points reaching one helper emit it once. A library module has no entry points, so its own functions are its roots and it publishes exactly what it did before. Every whole-module rule — the recursion check from #2748, the per-stage interface rules — now runs over that same emitted set rather than over a second, quietly different one.

The refusals that remain say **which** thing they are instead of sharing one message. Recursion drawn in across a boundary names the chain, qualified with the module it came from (`case.helper.step -> case.helper.step`) so the reader is sent to the right file. A foreign `ext` import says it is foreign and that no mach body exists to draw in — a different fix from a different message. A callee no module in the build defines says so, and names where the emitter looked.

`spirv-val` could not have caught any of this: a helper duplicated per entry point, an imported module's unreachable half dragged along, a call resolved to the wrong same-shaped helper, a drawn-in copy exported under a name its owner already publishes are all modules it accepts. So the tests read the **emitted instruction sequence in process**, off the object codegen returns — the functions a shader carries and the calls between them. A shader's internal helpers have no name in the module, so the fixtures give every function in the tree a distinct arity and an arity is then a name: "no five-parameter function here" is exactly "the unreachable helper did not come along", said in the only vocabulary the module has.

#### aarch64 emits the B and H views of a V register, so a two-byte vector moves in one instruction (#2716)
`vec_mem_widths` declared 4, 8 and 16 on aarch64 while the architecture also has B and H views of the same V register. The gap was not neutral: declaring 1 and 2 anyway turned an `i8x2` copy into `internal compiler error: aarch64 has no float memory access form for this width`, because the model claimed a capability the encoder did not have. That is what an aspirational declaration produces, and it is why the field describes **what the back end emits** rather than what the ISA has.

The encoder emits them now — `ldr`/`str`, `ldur`/`stur` and the register-offset forms at B and H, every word byte-exact against `llvm-mc -triple=aarch64` and re-checked by disassembling the emitted object — so the declaration grew to match. That order is the rule, never the reverse. `copy_i8x2` goes from a stack frame, a 16-byte scratch slot and a chunk walk to `ldr h0, [x2]` / `str h0, [x1]`.

They join the plain `ldr`/`str` mnemonic rows rather than taking `ldrb`/`ldrh`: those name a W destination and have to say which part of it they wrote, while `ldr b0, [x1]` transfers exactly the register named. The width-refusal test moved with the forms — 1 and 2 are now asserted **accepted** at their exact words, and 0, 12 and 32 stay refused, which is the property that test exists for.

#### A vector of any lane count compiles on every target, whether or not a vector register can hold it (#2727)
`f32x5`, `f32x8` and `f32x16` were refused **by name** at the front end on every target: `vector `f32x8` is 256 bits wide, more than this target's 128-bit vector width`. That read a realization fact as a legality rule, and it was incoherent besides — rv64gc has no vector unit at all, scalarizes every vector it is handed, and still refused `f32x8` on the ground of a 128-bit SIMD width it does not have.

The surface now admits `TxN` for any element and any `N` from 2 to 65535, identically on every target. The only refusals left are the degenerate one-lane vector and a lane count past what the compiler represents, and neither mentions a target. `vector_bits` keeps its real job — deciding **how** a shape is realized — and has no say in whether it can be spelled, so a target that gains ymm/zmm/SVE gets better code rather than new spellings.

Realization is decided per value by one authority, `isa.fits_vector_register`. A shape that fits a vector register keeps exactly the codegen it had. One that does not is placed in memory and worked a lane at a time by `me.pass.scalarize` — which stopped being a per-target pass and became a per-value one, so a target with no vector unit (every value claimed) and a target meeting a shape too wide for its register (only that shape claimed) are now the same rule rather than two mechanisms. An `i32x4` next to an `f32x8` still selects one packed instruction.

Layout: `$size_of` is lane-derived at every width (`f32x8` is 32 bytes), and `$align_of` gains a third rung — an over-width vector aligns to the **vector register width**, 16 rather than 32, because it is placed as several register-width pieces and there is no 32-byte vector load for a larger alignment to serve. Every convention (SysV, AAPCS64, win64, lp64) gives an over-width vector the memory class: a hidden pointer in, the indirect-result pointer out.

Verified by lane values rather than by sizes, on all three ELF legs and both profiles: `i32x8`'s eight lanes checked individually after an add (a sum cannot tell a rotation from a correct result), sub/and, the non-power-of-two `i32x5` and `i16x7`, `f64x4`, a **loop-carried** `i32x8` accumulator through a phi, guard fields on both sides of a wide vector and around `i32x5`, `[3]i32x8` strides, and an over-width vector passed to and returned from a call. `f32x8` also compiles and validates on SPIR-V, where the same authority routes it to the `OpTypeArray` realization an over-**count** shape already took. The two fixed 16-lane buffers that the lane-count bound had made safe (`MAX_GAP_LANES`, `MAX_VECTOR_LANES`) are now allocations.

Two diagnostics were wrong in passing and are fixed with it. The build note read `scalarized N vector operations (target has no SIMD)` on x86-64, sending the reader to check a capability the target plainly has — scalarization has been per-operation since #2726 and is now per-shape too, so it reads `with no packed form on this target`. And `simd = "require"` said `target packs no more than the lanes its vector form holds at this lane width` for both a lane-count ceiling and a width one; it now says which of the two causes it was.

#### A sub-width vector crosses to memory in one instruction where the ISA has one, instead of always going through a scratch slot (#2716)
`i32x2` and `f32x2` cost a stack frame, a 16-byte scratch slot, a whole-register spill and a chunk walk through a GP register to move eight bytes. `copy_i32x2` on x86-64 was 13 instructions and a 32-byte frame; it is now 5 instructions and **no frame**, one `movsd` each way.

Which widths can cross between a **vector register** and memory in one instruction is a declared per-ISA capability (`vec_mem_widths`), read only through `isa.moves_vector_memory`. x86-64 has `movss`/`movsd`/`movups` at 4, 8 and 16; aarch64 has `ldr`/`str` at the S, D and Q views. A width listed there also promises that a load of it **zeroes the lanes above it**, which is what lets the direct load keep the guarantee the scratch round trip provided: the lanes a type does not have read as a defined 0 rather than as frame residue.

The 12-byte shapes stay on the scratch path, and that is the finding rather than a gap: the issue proposed gating on lane alignment, a predicate that is **true for every shape mach can spell** (every lane width is a power of two no greater than 8, so the greedy 8/4/2/1 walk is lane-aligned by induction — its `i16x7` counterexample is arithmetically wrong, the 4-byte chunk at offset 8 covering lanes 4 and 5 exactly). A vacuous guard would have licensed a chunked direct access that neither baseline can encode: `pinsrd`/`pextrd` are SSE4.1 against an SSE2 baseline, and aarch64's single-structure `ld1 {v0.s}[2]` is a form this encoder does not emit yet. Both are pure additions later; until then the round trip covers those shapes, on a declared capability rather than by accident.

Asserted on the emitted code (`int/surface/vector-subwidth-direct`, frame verdicts per shape per leg), because the lanes were already right — a run-and-compare case cannot see this at all.
#### An adversarial vector-correctness sweep across axes nothing else reaches
Vectors have shipped three silent miscompiles in recent memory - `f32x3` shipped with no operation ever run through it, `vec_lane_count` carried a 128-bit assumption no CPU backend reads, and #2749 (merged tonight) had both vector-operator expansions substituting a loop-carried phi's uses one block at a time, missing the back-edge operand - and CI was fully green through every one, because the 42 `vector_runtime` cases all cover the ten 128-bit shapes in straight-line code.

`int/surface/vector-carry-and-boundary` puts real lane values through the combinations neither that suite nor the existing `vector-subwidth` / `regression/2749-gap-op-loop-carried` fixtures reach: sub-width and float-lane loop-carried accumulators (including `*`, #2749's own operator, at a shape its fixture never tried), bitwise and comparison-into-mask operators in the carried position, a sub-width vector through two call boundaries as both argument and return, a struct containing a vector returned BY VALUE rather than merely constructed locally, two adjacent vector struct fields with no scalar guard between them, a runtime-indexed array of vectors, and four vector arguments live in one call. Every lane is hand-computed and checked individually, never compared against another target's output - #2749's own bug had two targets agreeing with each other and both being right for reasons unrelated to the defect.

The sweep found no new defect. Only `mul_loop3` (`*`, loop-carried, at `i32x3`) is an actual regression pin against #2749, and only on `linux` (x86-64) - verified by running the case against a compiler built from the commit immediately before #2749's fix merged, where it reads wrong there and correct on `linux-arm64` / `linux-riscv64` (for reasons unrelated to the defect, the same shape #2749's own i32x4 case had, with which targets are "right for the wrong reason" swapped). Every other probe reads correct against that same pre-fix compiler: real coverage of untested combinations, not additional regression pins, and the case's own comments say so rather than implying otherwise.

Over-width shapes (256-bit+) are refused at the front end on the commit this was written against and are not attempted here; #2805 / #2806 are expected to make them legal, and whoever lands that should extend this case's probes to a wide shape rather than add a new one.

#### A shader can bind and sample a texture (#2794)
The GPU decorator set covered `#[input]`, `#[output]`, `#[builtin]`, `#[uniform]` and `#[storage]`, and none of them declares an image. A shader that reads a texture could not be written at all, which is the remaining blocker for porting boom's shaders off GLSL - every one of them samples a texture.

`#[sampler(set, binding)]` binds an **image handle**, at the same descriptor addressing `#[uniform]` uses, so nothing new appears at the descriptor level and a host binds one the way it binds a block. A handle type is a **form** rather than a fixed list, on exactly the terms a vector spelling is: `[i|u] ("sampler" | "texture" | "image") <dim> ["ms"] ["array"] ["shadow"]`, plus a bare `sampler`. `sampler2d` is a combined sampled image; `texture2d` and `sampler` are the separately-bound halves; `1d`, `3d`, `cube`, `array`, and the `i` / `u` sampled scalars are each one row in that grammar and nothing else.

The type carries every `OpTypeImage` operand rather than baking in a 2D combined-sampler assumption, which is what makes the awkward members cost a row apiece instead of a rewrite. The separately-bound form is the proof of it: a handle there is two descriptors and the sampled value is computed rather than loaded, and it needed no new machinery.

Sampling is a `#[spirv_op(...)]` declaration, not a new language form, because a sample IS one SPIR-V instruction the way `sqrt` and `dot` are - `#[spirv_op("core", "OpImageSampleImplicitLod")] fun sample(s: sampler2d, uv: f32x2) f32x4;`. The decorator's existing contract (result type from the declared return, operands from the parameters in order) admits `OpSampledImage` unchanged too, whose operands are two *different* handle types.

Several well-formed spellings are **read and refused by name**, so no member of the family is half-supported or silently miscompiled, and none can be claimed later by an unrelated declaration: `...shadow` (depth-comparison sampling, which needs a reference operand this cut has no form for), `...ms` (fetched per sample rather than sampled), `rect` / `buffer` / `subpass`, and `image...` (a read-write storage image). Each error names the member it refused. A handle behind a pointer, inside an array, in a local binding, wearing another interface role, or with no `#[sampler]` at all is refused with its own sentence.

The observable is the emitted instruction stream, not the validator's verdict: an `OpTypeImage` whose `Dim` came out 2D where the source wrote `sampler3d`, whose `Arrayed` came out 0 where it wrote `array`, or whose `Sampled` came out 2 rather than 1 is a module `spirv-val` accepts describing a different image. `int/surface/spirv-sampled-image` names all seven operands of every image type, the descriptor each handle is bound at, and the handle *type* each sample instruction reads.

#### A generic instance can be named as a value, so a generic can be stored, passed, returned, and addressed (#2342)
Type arguments were spellable only in **callee** position. `ident[i64](x)` worked, but `ident[i64]` in a value slot was read as the index expression `ident[i64]` and died in resolve with `unresolved identifier \`i64\`` - accurate for the parse that actually happened, and misleading about the intent. So a generic function could not be put in a `fun(i64) i64` binding, passed as a callback, returned, or placed in a table, and `?ident[i64]` was not the address of anything. The only way to get a function value out of a generic was to wrap the call in a non-generic function and take *its* address.

`f[T, ..]` with no call after it now denotes the monomorphized instance itself and types as the **instantiated** signature, so all four uses work and `?f[i64]` is that instance's address. The instance is reached through the same `enqueue_instance` path a call site uses, so the two spellings enqueue the identical instance under the identical linkage name and neither can reach the bare template - the predicate #2302 depends on is satisfied by construction rather than by a second author. A bare `f` is still not a value and still has no address (#2305): a generic is a template, not code, and only an instance denotes a function.

The parse is the same two probes the callee form already ran, with the same four outcomes, so there is no second disambiguation to drift. That does mean **every** bracket in the language now goes through them, where before only `x[...](...)` did, and the reading that was always a subscript has to stay one. Resolve decides the value form with a deliberately *narrower* predicate than the callee form's: only a name resolving to a function **declaration** takes type arguments in value position. The callee form's predicate accepts any imported symbol, which is harmless when a `(` follows and wrong without one - it would rule `mod.ARR[i]`, an imported array subscripted by a name, a generic instance. Both directions are pinned, in one module and across modules, and the whole int suite plus the self-host fixpoint is the acceptance for the blast radius.

Where the payload is a type spelling and nothing else (`arr[*i32]`), there is no subscript reading to fall back on, and the error is now reported against the object - `type arguments in value position name a generic instance, but this does not name a generic function` - rather than as a parse failure on the `*`. A payload that is a value and nothing else (`f[0]`) is untouched and keeps #2305's `not indexable: a function has no elements`.

#### `#[inline]` crosses a module boundary, so an atomic costs its instructions and not a call as well (#2231)
Every caller of `std.sync.atomic` is in another module, and until now a dependency's function body never reached an importer at all. A `fetch_add` was a `call` to a function whose whole content is four instructions, and #2258 had already removed the stack-slot staging around it, leaving the call as the entire remaining cost the issue named.

The body of a dependency's function flows into an importer only when the function is a **template** - generic, comptime-param, or pack-tailed - because those have no single concrete body the defining module could emit. `#[inline]` is now the fourth member of that same axis, and the first one an author asks for rather than the type system forcing: it declares that the body is part of the function's public surface, so an importer may materialise it. The machinery is the one monomorphization already used - the instance worklist, the drain, and `append_function`'s upgrade of the call site's extern into the definition - with zero type arguments.

**The linkage is what keeps the function one function.** The copy is emitted `PUB | WEAK` under the origin's own mangled name, not as a private clone, so the defining module's strong definition wins at link time and every reference in every object binds to that single address. A private clone would give `?atomic.load` a different answer per module, which is a silent wrong answer rather than a missed optimisation.

**The incremental firewall gained a correspondingly shaped hole, deliberately.** `Q_LOWERED_SURFACE` (#1859) excludes regular function bodies from its fingerprint so an impl-only edit in a dependency does not re-lower its importers. An `#[inline]` body is now **included**, because an importer holding IR built from the old body is holding stale code - a miscompile that appears only on an incremental build and never on a clean one. The test that used to pin the absence of cross-module inlining, and said in its own comment that it must flip when the feature landed, now asserts the invalidation instead.

Measured on x86-64, release, a loop of 3M iterations over store / load / fetch_add / fetch_sub with six values live across each barrier, interleaved runs: **25.0 ms -> 21.1 ms (-15.5%)**, identical output. The staging that remains is inherent to a `{name}` binding, which denotes a frame slot, so removing it is a different feature (a register-constrained asm operand) and not this one.

Carrying an `asm` body across the clone is the other half, and it is the design from draft PR #2678 rebased onto current `dev` and re-measured: `ir.clone_asm_payload` copies an `OP_ASM`'s `Function.asm_blocks` entry onto the clone, remapping the map key and each `{name}` binding's alloca, and deep-copying the bindings array since `module_dnit` frees it per `OP_ASM`. `instruction:asm_is_at_least_as_conservative_as_call` pins the property the whole change rests on: inlining removes the CALL and leaves the `OP_ASM`, so the ordering an atomic relies on survives only because `is_pure` excludes both.

Two new integration cases, because one of them cannot see the feature: `2231-cross-module-inline-asm` checks every value at both profiles on all three ISAs and passes identically against a compiler that does no cross-module inlining at all, which is the point. `2231-cross-module-inline-shape` reads the **emitted call count** per function through a new `call-shape` producer, and against `dev` it reports `calls=1` on exactly the two lines that are the feature while `#[noinline]` and undecorated callees keep their calls. Memory orderings are untouched and remain sequential consistency everywhere.
#### BREAKING: `riscv64` is a family of ABIs, `lp64` now means soft float, and the linux default is `lp64d` (#2777)

Mach declared exactly one RISC-V calling convention, named `lp64`. It did not implement lp64. It implemented **lp64d**: a scalar `double` was placed in `fa0`, which is the hard-float rule. The name said soft float and the emitted calls were hard float, so nothing you could read in a manifest described the code that came out, and any judgement about interop made from `abi = "lp64"` was made from a name that was wrong. The riscv64 leg never caught it because it runs under qemu against mach's own code and links nothing foreign, so caller and callee were consistently wrong together.

`abi/lp64.mach` is replaced by `abi/riscv.mach`, which implements the psABI family once. A member is a `RiscvAbi` descriptor of declared facts (XLEN, FLEN, the integer register file size, the argument-register counts) and a single shared classifier reads them, so there is no per-member branch anywhere and adding a member is a descriptor plus a `register` call. Three members ship:

| abi | float arguments |
|---|---|
| `lp64` | soft float, every float in an integer register |
| `lp64f` | `f32` in `fa0`-`fa7`, `f64` in the integer bank |
| `lp64d` | `f32` and `f64` in `fa0`-`fa7` |

The whole float-argument rule is one comparison against the member's declared FLEN (`width * 8 <= flen_bits`), so soft float is that rule at zero rather than a second code path, and `lp64f` — the member a hard/soft boolean cannot describe at all — costs nothing. `lp64e` and the `ilp32` family are described by the same contract and deliberately **not** registered: each needs a base ISA that does not exist yet (a 16-register RV64E file, or `ARCH_RISCV32`, #2778), and a convention registered without an ISA that can back it would compile and emit calls the hardware cannot make.

**The linux default for riscv64 is now `lp64d`**, which is what `riscv64-linux-gnu` means by "riscv64". This is the interop-correct default: every riscv64 Linux distribution is built on lp64d, so a mach object under any other member passes a double in `a0` where every system library expects it in `fa0` — a mismatch that produces wrong values rather than a link error. Freestanding also defaults to `lp64d`, matching the psABI its ISA declares; a bare-metal image that leaves the FPU off should now spell `abi = "lp64"`, which is finally a real convention rather than a name that emitted hard-float code anyway.

An ABI that needs a float extension the target lacks is now **refused when the target resolves**, naming both sides and both widths, rather than emitting code that cannot execute. The ISA declares what it has (`MachineModel.flen_bits`) and the convention declares what it needs (`AbiVTable.float_arg_bits`), and the gate is those two declared widths compared — so it covers a new convention or a new ISA with no table of names. `lp64q`, which needs the Q extension rv64gc does not have, is refused on this path; `lp64d` composes on the same one.

Manifests spelling `abi = "lp64"` for riscv64 keep building, but now select **soft float** and emit different code. Change them to `abi = "lp64d"` to keep the previous behaviour; every manifest in this repository was updated.

C-variadic calls are **refused on every RISC-V member** rather than placed. The psABI's unnamed-argument rule differs per member (a tail `double` takes `a0` under lp64d where a named one takes `fa0`, and the soft-float members differ again), so implementing it would multiply the family's descriptions rather than add to them, for a call form with no user in mach or mach-std. `RiscvAbi.variadic_float_bits` is declared and carries a sentinel so the rule can be filled in later alongside the others; a refusal is exact and cheap to replace, a wrong placement rule is neither. mach's own `va: ...` parameter pack is a different mechanism, expands at compile time, and is unaffected.

### Fixed

#### A scalar `f32` passed in an integer register read back as `NaN` on riscv64 (#2777)

`capture_slot_reg` took the move width for a value captured out of an incoming argument or return register from the machine word rather than from the value, so an `f32` arriving in `a0` was reinterpreted with `fmv.d.x` (a raw 64-bit move) instead of `fmv.w.x`. RISC-V requires a single-precision value in an `f` register to be NaN-boxed, and the raw move does not box, so every subsequent `fmul.s`/`fadd.s` on it produced `NaN`. The placement side already took its width from the value, so the two directions were asymmetric — which is why `f64` in an integer register was correct (its width happened to equal the machine word) and `f32` in an `fa` register was correct (it never took the integer path).

Only reachable once a soft-float member existed, so it was latent rather than a regression. It is the first defect found by the width-versus-register-width split that #2778 exists to stress.

#### A release build now carries a real ELF `.symtab`, so `perf` and a crash reporter can resolve a function name without a `-g` rebuild (#2772)
Mach emitted no `.symtab` in any build, release or `-g`: `nm` reported nothing, `perf` could not symbolize a profile, and a crash address from a shipped binary mapped to a function only by rebuilding with `-g` and doing a DWARF lookup against the exact matching build - two ordinary workflows a shipping game needs, neither possible before this.

A static ELF executable (`emit_exec`) now carries `.symtab` + `.strtab` with every defined function's mangled name, final virtual address, and byte size, collected by `build_symtab_entries` from the same merged-symbol data `build_exec_functions` already reads for PE unwind tables, and serialized by `elf.build_symtab_bytes`. Function symbols only, not globals: functions cover profiling and crash triage, and a data symbol table costs more file size for less of either. Sizes are real, not zero-filled - `nm` alone cannot see a `st_size` gap, but a profiler attributing an address *inside* a hot loop can, so this repo's own `int/surface/symtab` case asserts a mid-function address resolves correctly, not just a function's entry point.

Trails every `PT_LOAD` segment exactly like `int/surface/debuginfo`'s DWARF sections already do (`readelf -SW`/`-lW` confirm `.symtab`'s file offset never precedes a loaded segment's end), so a release image's runtime bytes are unchanged - the loader ignores non-allocated sections and a release build stays byte-for-byte the same program, just no longer anonymous to a debugger or profiler reading its container. This is a real, measured trade against the previous byte-identical *header-less* release image, though: a build that carried no section-header table at all now carries one whenever it carries a symbol table (which is every static build now), and the compiler's own release binary - a large, real program with 8,408 function symbols - grew by about 611 KiB (7.3%) for it. `perf record`/`perf report` on a release build built with this change resolves `[.] main` by name where it previously showed a raw address; `addr2line -f` on an address in the middle of a function's body (not just its entry) resolves to that function, proving `st_size` is correct and not merely present.

Scoped to ELF's plain static executable writer (`emit_exec`) for now: a PIE/dynamically-linked executable (`emit_dyn_exec`) and a shared library (`emit_shared`) do not carry a symbol table yet, and neither does Mach-O (`LC_SYMTAB`) or PE/COFF (its own, mostly-obsolete-under-PDB symbol table) - both accept the same `of.SymtabEntry` channel for `of.ExecFn` conformance and ignore it. `of.SymtabEntry` (name, final vaddr, byte size, STB_LOCAL/STB_GLOBAL linkage) is the format-neutral shape a future writer for either reads from, so extending coverage is wiring a serializer, not redesigning the data.
### Changed

#### BREAKING: linkage names are dotted and readable

Every symbol Mach emits changes spelling. The mangled name is now the source FQN as the source spells it, with generic arguments after a `$`:

```
_M3std5types6string7str_lenN7str_len   ->  std.types.string.str_len
_M3std5types6optionN12unwrapI3ptrE     ->  std.types.option.unwrap$ptr
_M4mach4lang6internN26"intern.roundtrip"  ->  mach.lang.intern."intern.roundtrip"
```

**What this breaks.** Object files, static libraries and shared libraries built by 4.16.0 or earlier do not link against ones built by this release: every non-`ext`, non-`#[symbol]` symbol has a different name. Rebuild the whole closure. Anything that named a mangled Mach symbol from outside the compiler breaks with it — a C declaration bound to `_M...` by hand, an inline-asm reference to a mangled name, a linker script or `--wrap` naming one, a symbol allowlist, a profiler or crash-report filter matching the `_M` prefix.

**Who should care.** Almost nobody. `ext fun` foreign symbols and `#[symbol("...")]` names are literal and always were, so the entry point, the `_rt_*` runtime symbols, every C import and every hand-written asm target are all untouched. If you never wrote `_M` anywhere, a rebuild is the whole migration.

**The scheme.** The module path and the bare name are joined by `.`, exactly as the source FQN reads, and there is no prefix — a mangled name always contains a `.` and a C identifier never can, so the old `_M` reserved-space guard bought nothing. A generic argument is introduced by a run of `$` whose **length is its nesting depth**, so nothing needs a closing bracket and a shallow instance stays short:

```
f[Map[Vec[i64], str], u8]  ->  m.f$m.Map$$m.Vec$$$i64$$str$u8
```

Types spell themselves: `p$u8` is `*u8`, `sec$u32` is `^u32`, `arr4$u8` is `[4]u8`, `fn$$i64$$u8` is `fun(u8) i64`, a record is its own dotted origin FQN, a comptime value is its literal (`tag$7`, `tag$n3`, `true`, `"text"`), and a variadic pack instance carries a `pack` marker before its element list. `.` and `$` are both accepted in an inline-asm symbol name, so every emitted symbol stays nameable from `asm` — which the old scheme also managed, and which a readable scheme has no excuse to lose.

Names did **not** get longer. Over the compiler's own release build — same tree, same profile, 11660 distinct symbols either way — the longest goes from **108 bytes to 105**, and the count over the **63-byte inline-asm operand limit** falls from **1543 to 668**. A length prefix costs one or two digits per path segment where a dot costs one character, and the `_M` goes away, so a dotted name is shorter despite being readable. (That cap is a pre-existing limitation with a misdirecting diagnostic — an over-long operand is refused as "malformed" — and is not touched here.)

Fixing the scheme fixes every reader at once — IR dumps, diagnostics, `--emit-asm` headings, linker messages, `objdump`, `perf`, `DW_AT_linkage_name` — rather than teaching a dozen display sites to substitute a friendlier name one at a time, and the places with no source name to substitute are covered too.
#### An inline-asm body that cannot return no longer claims it does (#2791)
`std.system.panic`'s `panic_os` issues `SYS_exit_group` and then traps. Its own comment says the trap "is unreachable and kept only as a guard; it is NOT the terminator." Lowering ran the block on past it into an epilogue, so the function's CFG asserted that control returns from `exit_group`, and everything reasoning over the CFG inherited that. It was invisible while such a body was never inlined - the dead epilogue sat at the end of its own function - and became a caller's problem the moment a body like it could be spliced in.

**The fact is derived from the mnemonic row, and cannot be declared.** `hlt`, `brk` and `ebreak` are modeled instructions with table rows, so the compiler already holds it. The inline-asm effect model says a modeled instruction's facts come from its mnemonic row and cannot be overridden, and a `:: noreturn` clause would be exactly that override - on a claim the compiler *acts on*, where the standing rule is that a declaration buys allocation quality and never verification. So there is no declaration to get wrong and nothing rests on the author. The first person who wants a clause here should read this paragraph instead.

**Conservatism runs the opposite way from every other fact in the model**, and the direction is the whole design. A wrong `writes` costs allocation quality; a body wrongly judged non-returning makes lowering **drop everything after it**, which deletes live code. So an unreadable body, a raw `.byte` payload, a body defining a numeric local label, an ISA wiring no analyzer, and a whole-module emitter with no register machine all report that control returns - the behaviour every target had before, never worse.

**Stated limit:** a body that ends the thread of control *without* a trap - a bare `exit_group` syscall with nothing after it - still reads as returning. That is unchanged behaviour rather than a regression, but "cannot return" now has a machine meaning, and the gap between it and an author's intent is where the next surprise lives. The trap is what makes the fact checkable, and mach-std already writes one on all three ISAs.

Nothing downstream needed teaching: `lower_stmt_list` already stops at a terminated block, and the inliner only wires a continuation from a callee's returning paths.

**This is not the fix for the gdb abort that led here.** That was an overlapping-`PT_LOAD` defect in mach's own ELF writer (#2795), unrelated to control flow.


### Fixed

#### `--emit-asm` names the symbol an inline-asm statement actually references, instead of a different real one (#2788)
On x86-64, `--emit-asm` rendered every symbol operand inside an `asm` block wrongly. A memory operand naming a symbol printed a **different, real symbol from the same module** - `[rip + _rt_argc]` came out as `[rip + dbggdbcase]` - and a direct `call symbol` printed a bare `call` with nothing after it. The emitted object was correct in both cases; only the printed text was wrong.

That matters more than it looks, and it cost real investigation time. `--emit-asm` is what someone reaches for when debugging inline asm, which is exactly the code where falling back on reading the source and assuming the obvious is not available. A bare `call` is at least visibly broken; a plausible wrong symbol name gives a reader no signal at all that the output is untrustworthy - an agent chasing an inlining defect followed the printed operand and had to discover independently that the text disagreed with the object.

The cause was one missing thread, in two places. The inline-asm path interned the referenced symbol's name only at *relocation* time, after the instruction had already been built and encoded, so the `isa.Inst` the printer renders from carried no symbol at all: a rip-relative operand went out with `sym_id` 0 (which resolves to a real interned string, hence a real symbol name), and the direct-transfer form was built with no operand whatsoever. The name is now interned once, before the instruction is built, and the same `StrId` reaches both the Inst the printer renders and the relocation the linker reads - so the printed name and the relocated name cannot be two different strings. Naming the direct transfer's target costs no bytes, because `encode_call` / `encode_jmp` write a placeholder rel32 on the direct arm regardless, and it makes the inline-asm `call sym` exactly the `isa.Inst` the compiler-selected one already built - which is why *that* one always printed its target.

aarch64 and riscv64 were already correct; both already threaded the interned id into the printer notification. That is precisely why the new `int/surface/asm-symbol-operands` case covers all three ELF legs rather than the one that was broken: the printers are separate per ISA, so a fix in one says nothing about the others, and the two that were right now have that pinned. The case pairs the symbol names the assembly text prints with the ones the object's relocations name, in file order, so a printer that regresses to naming a *different real symbol* fails it, not merely one that names nothing - and it runs the program too, since a reference that reached the wrong symbol would be a wrong answer as well as wrong text. Objects are byte-identical across the fix, checked directly.

Two existing unit tests asserted the built instruction had **no** operand for `call main` / `jmp main`. Those pinned the defect, and are now the assertions that it names its target.

#### A C `[step]` cross-builds for its leg instead of silently assuming the host toolchain targets it (#2741)
`int/surface/narrow-stack-args` and `int/surface/c-variadic` shelled out to the bare host `cc` to build a probe object, which happens to target the leg on every CI runner (each builds natively) but not on a developer cross-building locally - `mach --target linux-arm64` on an x86-64 host still ran the host's x86-64 `cc`, silently linking an x86-64 object into an aarch64 image. That surfaced as `error: relocation 'plt32' to 'float_tail' overflows` on narrow-stack-args and a qemu `Illegal instruction` on c-variadic, neither of which names its actual cause.

Both cases now build their probe object through `int/lib/cc.sh`, a general contract for any case with a C `[step]`: it uses the host's own `cc` when the host already targets the leg (preserving the point of these cases, which is to catch a *real* toolchain's ABI diverging from mach's model of it - Apple arm64's stack-argument rule, for one), and falls back to `clang -target <triple> -c` otherwise, since mach does the link itself and a cross-compiled object needs no sysroot or cross-linker to satisfy it. A leg cc.sh has no route for (no host match, and clang is missing) fails the step loudly, naming what's missing, rather than surfacing downstream as an opaque link error.

This also lets both cases build for `linux-riscv64` for the first time, previously exempted only because the host-`cc` gap made it impossible; narrow-stack-args passes there cleanly. c-variadic's riscv64 leg stays exempted, but now for a real reason instead of a stale one: every variadic float/double argument comes back as `0`, a genuine mach ABI bug in riscv64's variadic lowering that this cross-build gap had been hiding coverage of (#2771).

`int/run.sh` also now prints a `SKIP <case> [<target>] (exempt, see case.conf)` line for a case's `exempt:` legs, which previously left no trace in the run's output at all - a leg a case never applies to and a leg it was silently excluded from looked identical.

`m`, the bootstrap output `mach build . -o m` (CONTRIBUTING.md's own instruction) produces, is now gitignored.

#### A whole-variable load from a shader interface variable is typed by the variable (#2794)
The SPIR-V emitter typed a loaded value from the move's byte width and register class rather than from the object it read. That agrees with the variable's own type for every shape that has a width, which is why it went unnoticed; it cannot express one that does not, and an opaque image handle has no width at all. Inferring one gave a sampled-image load an integer result type - an `OpLoad %ulong` from a sampled-image variable, which a validator rejects and the compiler was happy to write. The store side had always read the variable's type; the load side now does too.

#### A comptime identifier names the declaration it resolves to, not the spelling (#2764)
The comptime environment is keyed by **name**, and `comptime.eval_ident` looked a bare identifier up in it directly. Resolution identity was never consulted, so a binding was matched by what it is called rather than by which declaration it is. A block-local `val N` shadowing a module-level `val N = 9` therefore read back the **module** constant: `var xs: [N]i64;` under a runtime local `N` compiled as `[9]i64`, and the out-of-bounds diagnostic it later raised named a length the source never wrote.

The evaluator now decides each identifier on the symbol the resolver bound to it. Every pass that owns a name-resolution map installs the same hook, and all of them answer from **one** function, `resolve.symbol_is_runtime`.

That collapses the two rules that existed to compensate for the old reading. `resolve.check_gate_shadowing` is **removed**: it caught a case the evaluator would otherwise get wrong, and there is no such case left to catch. The constant index bound (#2751) no longer walks its operand for runtime leaves before folding, because the fold now fails on them by itself. `sema`'s `$if` gate walk keeps its per-kind diagnostics — a reader needs to hear "parameter" for a parameter and "value" for a binding — but delegates the decision.

Consolidating them surfaced a disagreement the copies had been hiding. An `$each` loop variable is a **comptime** binding that resolve declares as a `SYM_VAL` bearing the comptime flag, and a rule keyed only on module scope classified it as runtime. The constant index bound consequently declined to decide any index driven by one, so `xs[e]` over an element out of range compiled clean and read past the array. It is now refused. `$` marks a comptime binding whatever its kind, and the rule says so once.

A comptime binding that shadows a module constant still resolves innermost-first, which is the half of this that had to be preserved rather than changed: a `$param` or an `$each` variable named after a module constant denotes the inner one.

#### A `#[uniform(...)]` block's offsets are checked against the SPIR-V layout rules (#2746)
A descriptor block's member offsets were emitted straight from mach's own record layout, and exactly one of the rules Vulkan imposes on that layout was checked. A block whose members disagreed emitted a module a consumer rejects, with no diagnostic — a `rec` inside a block followed by a scalar produced **overlapping members**, and a `rec` inside a block is the natural way to write a camera or a material.

std140 gives a struct member a base alignment of 16 and rounds its extent up to a multiple of 16; mach aligns a record to its largest member and does not round. A vector narrower than the vector register aligns to one lane in mach, where both std140 and std430 align it to two or four, so an `f32x2` or an `f32x3` member was a reachable disagreement in the **storage** posture too, which had been thought correct by construction.

Every emitted offset is now held to the two conditions the specification states — a multiple of the member's base alignment, and past the end of the member before it — and an array stride to the element's extent rounded up to the array's base alignment. The two postures differ only in the 16-byte roundings std140 applies to an array and to a struct.

A disagreement is **refused, never repacked**, which is the posture the one existing check already took and the reason it gave is the one that matters: the block's layout is the host's contract, and repacking would move the members out from under a host writing by `$offset_of`. The refusal names the member, the offset mach gives it, the offset the rules require, and which of the two conditions failed.

#### A fragment stage's integer or 64-bit input carries its `Flat` decoration (#2747)
Vulkan requires `Flat` on any fragment-stage `Input` of integer or 64-bit float type, because such a value cannot be interpolated. The emitter declared no `Flat` decoration at all, so every one of those was an invalid module. The decoration is now decided over the set of variables each stage can **reach**, which the per-stage reachability pass already computes.

Vulkan also **forbids** `Flat` on a vertex-stage `Input`, so one module-scope variable read by both a vertex stage and a fragment stage has no valid decoration and is refused rather than decorated for one of them. The two readings are not the same value anyway: a vertex stage's input is a vertex attribute and a fragment stage's is the interpolated output of the stage before it.

A `Location` was emitted verbatim and never checked. mach knows each variable's type and so knows how many locations it consumes, and two whose spans overlap within one stage are now refused naming both, rather than surfacing as a validator message naming an `OpEntryPoint` operand index.

#### Recursion and a 15-parameter signature are refused rather than emitted (#2748)
There was no whole-module well-formedness stage, so a rule about the module — its call graph, its id validity — had nowhere to live, and two reachable from ordinary source emitted invalid modules silently.

A SPIR-V execution model has no call stack, so a cycle in the static call graph is refused before any body is emitted, naming the chain and saying why. This is checked rather than left to `spirv-val`, whose cycle rule is scoped to entry points: a **library** module carrying a cycle validated clean and would have failed in whatever consumer linked it into a pipeline. One had been sitting green in the integration suite.

The parameter limit was two independent literals that disagreed: `emit_function` refused above 16 while `type_fn`'s fixed operand buffer failed above 14 and returned a 0 nobody read, which reached `OpFunction` as the reserved never-valid id. The limit is now stated once as `types.MAX_FN_PARAMS`, both sites read it, 15 and 16 parameters build and validate, and a 0 from `type_fn` is reported instead of passed on.

#### A dead local's stale value survives past its real live range at opt0, and a `ret` statement's line-table row duplicates at function entry (#2779)
Two opt0-only debug-info defects `int/surface/debugger-gdb` (#2756) caught by driving a real `gdb` session, neither visible to structural DWARF validation because both artifacts are internally consistent, well-formed DWARF that simply describes the wrong thing.

**A variable's `DW_AT_location` was scoped to its lexical extent, not its true live range.** The opt0 allocator reusing a genuinely-dead local's storage is correct codegen (confirmed by checking the runtime value with no debugger involved), but the DWARF location for that local was never bounded to end where the reuse happens, so a debugger stopped after that point read the new occupant's value as though it were still the old variable's. Root cause: `gather_vars` built a variable's location ranges from its own bindings only, so a variable bound once and never rebound kept a stationary location valid to the function's end regardless of what physically happened to its storage afterward. `bound_storage_reuse` now truncates a range to the point some OTHER variable's binding, in the SAME inline site, claims the identical physical home - a stationary home a debugger could no longer trust past that point now reports unavailable instead. Scoping the search to the same inline site is load-bearing, not an optimization: at opt2 with heavy inlining, two variables in unrelated inline instances can land in the same register with no real relationship, and comparing across sites produced exactly that false positive while building this fix's own regression coverage.

**A `ret <expr>;` statement's line-table row was duplicated at the function's own entry address.** `lower_phis` (out-of-SSA phi elimination) runs once, after every block in the function has already been lowered, and stamped its synthesized merge-copy instructions from `ctx.cur_loc` - a cursor left stale at the function's LAST lowered statement by the time phi elimination ran, for every predecessor block's edge regardless of where in the function that predecessor actually was. `break file:22` on a function whose `ret` genuinely lives at line 22 could silently resolve to its entry instead, with no ambiguity warning, reading whatever was left in an uninitialized register. Fixed by stamping from the predecessor block's own terminator location instead, which was already correct when that block was lowered and never went stale.

`int/surface/debugger-gdb` gained two debug-profile-only probes proving both fixes together, asserted as a pair per each bug's own shape: a variable that genuinely dies mid-function now reports unavailable, while a sibling that stays live keeps reading its real value (a fix that reported "unavailable" for both would pass either probe alone); a second breakpoint on a real `ret` statement resolves to its own address and reads the correct value rather than silently landing at the wrong one. Both bugs are specific to opt0's own location and line-table construction and do not reproduce at opt2, so the case's golden is now per-profile (`expect.debug.txt` / `expect.release.txt`) rather than the one `expect.txt` every other `gdb-session` case still uses.

#### A loop-carried vector accumulator through an unpacked operator came out holding the wrong value (#2749)
The gap expansion (#2726) replaces a vector operator the target has no packed form for with per-lane scalar work and an assembled vector, then points that operator's uses at the assembled value. It rewrote those uses one block at a time, immediately after expanding that block's body. That is correct for every use that follows its definition, and wrong for the one that does not: a loop header's phi names the value the **latch** defines, so its back-edge operand is a forward reference no block order removes.

The phi kept naming the deleted operator. MIR lowering turned the phi edge into a copy from a virtual register nothing defines, and the accumulator came out holding whatever that register happened to be. Measured on x86-64, a loop-carried `i32x4 * i32x4` returned the **multiplier** where the product belonged. aarch64 was right throughout for a reason unrelated to the defect - NEON has `MUL.4s`, so nothing expands there at all - which is why an entire suite of straight-line vector cases passed on every leg while this was live. The lane-access expansion had the identical defect one pass over.

Substitution is now a whole-function step in both passes, after every block has been expanded, because the ordering it needs does not exist.

#### A float or integer memory access refuses a width the target has no form for (#2766)
A memory access takes a byte width and picks a machine form from it. On aarch64 and riscv64 that pick was a boolean selector with a trailing default, so a width the target has no form for silently got some other form's bytes rather than a diagnostic, the same shape #2733 removed from the float move, negate, arithmetic, compare and conversion encoders, one family over.

riscv64's `fp_mem_funct3` answered 4 with `flw`/`fsw` and **everything else** with `fld`/`fsd`. RV64D has exactly those two float access forms and no vector extension is enabled, so a 16-byte float access had nothing to encode to at all, and it encoded to `fld`, moving eight bytes and losing the other eight. aarch64's `emit_fp_mem` handled 16 in its own arm and then fell through to `use_d = width == 8`, so a 32-byte access took the **S** form and moved four bytes of a slot the surrounding code had already sized at 32.

The fix is a contract change on the access family, not a check at today's callers. The defaulting helpers were shared with the **integer** path, aarch64's `ldst_lo12_kind`, `width_shift` and the three `ldst_base_*` base-word tables were five parallel ladders that each named the narrow widths and each ended in the doubleword row, so validating only the float arms would have left the identical shape live next door under a different name. Each family now comes off **one** form table: aarch64's `int_access` / `fp_access` return a row carrying all three addressing-mode base words, the imm12 shift and the `:lo12:` relocation kind together, and riscv64's `int_mem_funct3` / `fp_mem_funct3` return the funct3 or an error. A width with no row is refused. `emit_mem_access`, `emit_fp_mem` and `mem_access_needs_no_scratch` report rather than emit, and the Q arm that used to sit ahead of the aarch64 default is now the same three lines as S and D, because the difference between them was only ever the row.

The census found the same defect on **x86-64**, which the issue did not name: `emit_float_load`, `emit_float_reg_or_slot_load`, `emit_float_store` and `float_const_operand` each read `mov_op = MOVSD; if (w == 4) { mov_op = MOVSS; }`, so every width other than 4 took the eight-byte MOVSD. It was inert only because `encode_fp_mov` returns its vector arm above them. Nothing in the helpers said so, and `emit_float_load` is reached separately by the arithmetic and seed paths. aarch64's `emit_fp_const` carried the same `use64 = width == 8`.

Every refusal is **driven from a unit test** with a width it must refuse, and the assertion is that the output buffer stayed **empty**, not merely that an error was returned. That distinction is load-bearing here: both `emit_mem_access` implementations materialize an out-of-range offset into the scratch register before the access word, and a folded global emits its `adrp` / `auipc` high half before the low one, so a check placed at the emission point rather than at the top would leave stranded instructions behind the error. Every legitimate width still encodes byte-exactly against `llvm-mc`, including aarch64's Q form at 16 and the sub-word integer forms at 1 and 2 that share the same helpers.

#### An inline-asm operand longer than a register-name scratch buffer is no longer reported as malformed (#2789)
Each ISA's inline-asm operand parser copied the operand token into a fixed stack buffer before deciding what kind of operand it was, and reported a **well-formed** operand as malformed when the copy did not fit — `[64]u8` on aarch64 and riscv64, `[128]u8` on x86-64. The copy's only consumer was the register-name lookup on the next line; every other branch (a relocation modifier, an address, an immediate, a bare symbol) already read the source region directly and never touched it. Register names are three or four characters; a mangled symbol is not, and dotted mangling's own measured longest symbol (105 bytes) already exceeds the aarch64/riscv64 buffer today.

The buffer is gone, not enlarged: `asm_reg_index` (riscv64), `a64_reg` (aarch64), and `x64_reg` (x86-64) each now compare the register-name table in place against the source region, mirroring the in-place symbol check (`iasm.is_sym_region`) already sitting one branch below them. x86-64's `[symbol]` memory-operand parser had a second, narrower instance of the same shape one layer down: its no-star bracket-content parser copied into a `[32]u8` meant for a base register, and a bare rip-relative symbol reference (`[rip + <symbol>]`, #2788's own repro shape) passed through the same copy, because x86-64's grammar makes a base register and a bare symbol genuinely ambiguous at that position until one or the other is ruled out. Fixed the same way. aarch64 and riscv64 were checked and do not have this second problem: `off(base)` and `[Xn, ...]` both require a register in the base position syntactically, and a bare symbol reaches a structurally different form (`:mod:sym`) that never touches a register buffer.

Verified per ISA with a symbol well past every old limit: it encodes, and encodes **byte-identically** to a short-named operand (instruction bytes never depend on a symbol's name, only its relocation does), with the relocation itself carrying the full, untruncated name. A genuinely malformed operand still refuses, checked alongside the long-operand case so the two cannot be confused.

### Changed
#### The realization authority answers a lane-count ceiling as well as a lane width (#2749)
`isa.packed_width` answers a question keyed on the lane **width**, and a SPIR-V Shader module's constraint is a lane **count**: `OpTypeVector` takes 2, 3 or 4 components at every element type. For 8-bit lanes the authority reported a 128-bit packed form, meaning sixteen lanes, which no Shader module may declare - so the authority and the emitter gave different answers about the same shape, and the emitter refused `i8x16`, `u8x16`, `i16x8` and `u16x8` by name.

Under #2488 a vector type and a lane-wise operator are legal on **every** target and only the realization varies, so a refusal was the wrong answer. The machine model gains `max_vector_lanes`, folded into the authority so no caller consults two facts and picks the wrong one: every machine target declares no ceiling and its answers are unchanged, asserted rather than assumed; SPIR-V declares four, so 8-bit lanes report a 32-bit packed form and a wider shape takes the scalar expansion. The four by-name refusals are deleted rather than left unreachable, and a vector past the ceiling is realized as an `OpTypeArray` of its lanes - the logical-addressing analogue of a machine target splitting a vector across registers.

`simd = "require"` now names the lane count as well as the lane width, because a target can pack the same operation at one count and scalarize it at another.

The vectorizer takes its lane count from the same authority instead of `16 / element-bytes`, so it cannot form a shape the target would immediately take apart again.

#### Sub-word integer arithmetic on SPIR-V produced a module no consumer accepts (#2749)
The 4-byte ALU floor is a **register machine's** contract: a sub-word operation leaves the bits above its width undefined, and a spill saves the whole word, so computing narrower would persist residue. SPIR-V has no register file, and its arithmetic opcodes are typed - so computing an 8-bit add at 32 bits emitted an `OpIAdd` whose operand and result types disagree, which `spirv-val` rejects. Any `i8` or `i16` arithmetic reached it.

The floor is now a declared model fact (`alu_min_width`) rather than a constant with two derivations layered on it: 4 on a register machine, 1 on the 8-bit 6502 whose word is one byte, and 1 where there is no register file at all. Machine-target output is byte-identical.

A scalar comparison feeding arithmetic is likewise reconciled: SPIR-V's comparison yields a boolean with no numeric representation, which per-lane mask construction needs as a 0/1 value. That is the scalar counterpart of the `OpSelect` widening a lane-wise compare's mask already used.

#### A float literal narrowed to `f32` is folded at its own width (#2752)
A float literal is carried as a comptime `f64` bit pattern from the lexer onward, so an `f64` constant narrowed by an explicit `::f32` reached the back end as a double-width materialization plus a runtime convert. Two instructions, and on x86-64 an FP scratch register, for a value known at compile time. `me.pass.constfold` now folds an `FP_TRUNC` whose operand is a `VAL_CONST_FLOAT` into the constant.

The fold is a **retype**, not a value change. A `VAL_CONST_FLOAT` carries its `f64` pattern whatever its type says, and the back end reads the width off the type: `lower_value` narrows through `f64_to_f32_bits` at width 4, exactly as `pack_globals` already did for a module-scope global. So this moves *when* the round-to-nearest-even happens, not what it rounds, and nothing downstream can double-round it either, because every float fold in the pass is gated on `is_f64` and an `f32`-typed constant is inert there.

The **widening** is deliberately not folded, and a test pins that. An `f32`-typed constant still carries the unrounded `f64` pattern, so retyping it to `f64` would produce the double nearest the literal where the machine produces the double nearest the `f32`. That is a value change, and folding it correctly needs a narrow-then-widen round trip through a single-to-double conversion `float` does not have.

Measured on a six-kernel `f32` micro-benchmark covering the ways a narrowing can arise, release profile, `#[noinline]` on every kernel:

| target | kernel instructions | constant-pool symbols |
| --- | --- | --- |
| x86-64 | 30 to 24 | 4 to 3 |
| aarch64 | 33 to 30 | 2 to 1 |
| riscv64 | 33 to 30 | 3 to 1 |

`int/surface/float-emit` moves on every target, and the movement is the residual its own commentary already named: `main` goes 2 to 0 on x86-64, which is exactly the `CVTSD2SS` and the scratch load feeding it, and 6 to 4 on aarch64 and riscv64, the same pair spelled as a constant materialization into the FP scratch plus the `FCVT` that read it.

Two things the issue expected turned out not to need changing. The front end already types a suffixed or contextually-typed literal at `f32` directly, so `x * 0.1f32`, `x * 0.1` in an `f32` context and `val k: f32 = 0.1` never produced an `FP_TRUNC` at all; only an explicit `::f32` of an `f64`-typed value did. And `fe.comptime` needed no matching fold, because a `val` initializer, an array element and a record field are already narrowed at their own width when the global is packed, which the emitted `.rodata` bytes confirm.

This also makes the constant pool's four-byte `CONST_F32` entry reachable from source for the first time.


#### The FNEG sign mask and aarch64's `fmov` immediate reach the constant pool (#2754)
Two constant materializations the pool did not reach, both because the ISA form that would consume one was not modelled by the encoder. They are independent and both landed here.

**The x86-64 FNEG sign mask.** An IEEE-754 negation is a sign-bit flip, and the mask it XORs against is one of two values in every program that negates a float. It was rebuilt at every negation through the GP scratch, `movabs r11, mask` then `movq xmm15, r11` then `xorps`: three instructions, twenty bytes and a general-purpose register, for a constant. `encode_xorps` gains the memory-source form it lacked (the same `0F 57 /r` opcode with a ModRM memory operand) and the mask is interned once per width per module, so a negation is one `xorps work, [rip + .Lconst.N]`.

The entry is a **16-byte, 16-byte-aligned** vector, and that is forced rather than chosen: XORPS is the aligned 128-bit form, so it reads sixteen bytes and faults on a misaligned operand. An entry at the float's own width would be wrong twice over. The high lane is zero because XORPS touches it and a negation must leave every bit outside the sign exactly as it found it. This is the pool's `CONST_VEC` kind and its alignment rule getting their first real consumer, and the test reads the shape out of the pool rather than asserting it from the interning call.

**aarch64's `FMOV` scalar immediate.** A64 can build a float constant in one instruction, with no bank move and no memory reference, for the set `±(1 + m/16) × 2^e` with `m` in 0..15 and `e` in -3..4, which is the set numeric code writes most often. The form was not modelled at all, so `1.0`, `2.0`, `0.5` and `-1.5` each cost two instructions. `fmov_imm8` is the `VFPExpandImm` decode read backwards, and `emit_fp_const` takes that arm ahead of its siblings because nothing below it can match one instruction.

The predicate checks both conditions, not just the exponent. A pattern whose mantissa has a bit below the four the field carries looks encodable from the exponent alone, and the form would silently drop those bits. The test pins the boundary at the edges rather than in the middle: one exponent step below the floor, one above the ceiling, and a fifth mantissa bit all fall through to another form, and the two in-range ends encode.

Measured on a kernel of three negations and five constant uses, release profile, `#[noinline]` on every kernel:

| target | negation instructions | constant-use instructions | .text | .rodata |
| --- | --- | --- | --- | --- |
| x86-64 | 18 to 9 | 20, unchanged | 442 to 406 | 40 to 72 |
| aarch64 | 9, unchanged | 25 to 20 | 332 to 280 | 0, unchanged |
| riscv64 | 9, unchanged | 25, unchanged | 380, unchanged | 32, unchanged |

The x86-64 read-only growth is the two 16-byte mask entries, one per width, shared by every negation in the module: at three negations that is close to a wash on size and a clear win on instruction count and register pressure, and it improves with every further negation. riscv64 does not move on either half, which is correct: its negation is a single `fsgnjn` that needs no mask, and it has no immediate float move form.

`int/surface/float-emit`'s `maxabs` moves 4 to 2 on x86-64, which is exactly the mask's bank move and the XOR that read it. The two that remain are the `t < 0.0` comparison materializing its zero, a different constant and one the pool deliberately never takes.

Also fixed while here, both without a separate issue: `encode.intern_const` refused to check its interner and dereferenced a null one, crashing inside `intern` rather than naming the encoder that asked, and now reports instead.

And aarch64's memory-access family no longer carries a **width-0 pseudo-width**. Every entry point in it used to open with `if (w == 0) { w = 8; }`, a convention from when a MIR pseudo could reach an access, which #2766 left in place as the one substitution it did not remove. Nothing reaches it: routing 0 into a refused width instead left the whole aarch64 surface green, the unit suite, every `int` case on `linux-arm64` in both profiles, and the compiler's own source cross-built for `linux-arm64` and `darwin-aarch64` at both optimisation levels. A default nothing exercises is not a safety net, it is a silent doubleword waiting for the first caller that means something else by 0, so 0 is now refused like any other width the target has no form for and the refusal is driven from a test that fails with the fallback restored. `is64` keeps its own 0 rule, which selects an ALU register form rather than a memory form and which a pseudo with no value width legitimately reaches.

## [4.16.0] - 2026-08-08

### Added

#### A real gdb session drives the DWARF the compiler emits (#2756)
`int/surface/debuginfo` proves the `-g` image is structurally valid (`llvm-dwarfdump --verify` clean, `addr2line` round-trips); it cannot prove the DWARF describes the right thing, because a location list can be valid and still name a register the value has already left. Nothing in the harness drove an actual debugger, so that half of debug info was never checked.

`int/surface/debugger-gdb` now does: one `gdb --batch` session per `opt` level breaks on a call inlined at release only (`#[inline]`, which the debug pipeline's no-inlining-pass makes a real call there instead) and reads its parameter back from whatever register or stack slot the location list names, breaks mid-loop on a local that stays register-resident for the whole loop and reads a running sum this case's own case.conf computes by hand, and confirms a local that is written and never read carries no location at either profile - "optimized out" is the correct answer there, not a gap. A single `step` off a loop body line is asserted to land on the condition check, then back into the body, by source line rather than instruction count.

Scoped to the `linux` leg only: gdb is what a Linux runner carries (no lldb here), and this is the one leg whose result was read by hand against the source. `linux-arm64` runs gdb natively in CI and could plausibly carry this case too, but that leg's session was never exercised or hand-verified, so it - along with `linux-riscv64` (qemu-user has no ptrace story a host gdb can attach through), `windows`, and both `darwin` legs - stays explicitly unverified rather than assumed to work.

#### An integer vector multiply works on every target (#2726, #2721)
`i32x4 * i32x4` was refused at the language level on **every** target, citing x86-64's SSE2 baseline, where `pmulld` is SSE4.1. That refused it on aarch64, whose NEON hardware has the instruction, and on riscv64, which has no vector unit at all and would have lowered it to four ordinary scalar multiplies.

The legality of a vector operation is a **target-independent** fact; only its realization is target-dependent. The rule lived in three places and now consults one authority, `isa.packed_width(model, op, is_float, lane_bits)`, which answers the width of the target's packed form and zero where there is none. Sema no longer asks the question at all, because it is not a legality question. Where a packed form is missing the operation takes the existing per-lane scalar expansion, and `simd = "require"` reports it naming the operation and lane width.

The authority is keyed on **lane width**, never lane count or total width, so splitting a wider-than-register vector into chunks does not change the answer. It declares **gaps rather than presence**, because a presence table would be roughly eighty rows per target and the row that mattered would be the missing one.

Also pinned as part of this: aarch64's NEON integer-multiply arrangements are now byte-exact against the ARM ARM, and a 64-bit lane arrangement reaching the selector is a defined encoder error rather than an emitted `mul v.2d`, which is unallocated. The word's comment previously claimed it was "legal only on .8h", which was wrong in both directions.

#### `$is_secret(T)`, so a reflection walk can act on secrecy instead of refusing it (#2694)
Every comptime predicate asks about the shape **under** a `^`, so all of them answer false for a secret. `$is_record(^u64)` and `$is_record(u64)` were therefore the same answer, and a `std.derive` walk could only ever meet a secret field as a fallthrough it had to refuse. That is a refusal, not a decision, and it blocked all three things a derive actually wants: a formatter that redacts a secret field, a hash that refuses one (a data-dependent fold is a leak in the shape of a digest), and an equality that selects the constant-time comparison rather than the early-out whose timing is the secret.

`$is_secret(T)` is the positive question, and it joins the predicate family exactly: comptime-only, one type operand, gate position, answered per instantiation inside a generic. It is the family's one exception to the stripping rule, and the exception is coherent rather than special-cased — the other three ask about the wrapped storage, this one asks about the wrapper. The doc's stripping table names it as such.

The contract is **outermost only**, the same line the shape predicates already draw, so the four cannot disagree about what a type is. `^*u8` is true (the pointer is the secret), `*^u8` is false (a public address to secret storage), `[N]^u8` is false, `^^T` is true because `^^T` collapses to `^T`, and a record with a secret field is false — the field is secret, and `$is_secret(f.type)` is where a walk meets that.

There is deliberately no transitive "contains a secret anywhere" query. Folding it in would make the common case wrong: a formatter gating on a transitive answer would redact a whole record over one field, and could not tell which. Where the transitive question is genuinely wanted through a reference it composes instead, `$is_secret($pointee_of(f.type))`. `$pointee_of(^*U)` stays refused, which is right — `$is_secret` has already answered true there and a walk should stop.

A walk that skips a secret field is pinned by the flow rules rather than by convention: reading one into a public accumulator does not compile, so a gate that answers wrongly is a compile error rather than a silent disclosure. The regression case relies on exactly that.

#### A constant pool, so a float constant is loaded rather than rebuilt (#2248, #2700)
A constant wider than an ISA's immediate forms has to live in memory somewhere. Every back end here instead rebuilt one from instructions at every use, inside loop bodies included: x86-64 spent `movabs r11 ; movq xmm, r11` (two instructions, 15 bytes, and a general-purpose scratch) because SSE has no immediate form at all, aarch64 spent up to four `movz`/`movk` words plus a bank move, and riscv64 spent `emit_li`'s recursive shift-and-add - five to seven words for an arbitrary `f64`, since its widest immediate is 32 bits.

The pool lives in the **shared encode state**, not in a back end. It interns a constant under its full identity - kind, width, alignment, and every byte - so two uses share one entry while constants that merely resemble each other do not, and bit patterns are compared rather than the values they denote, so `-0.0` and `+0.0` stay distinct. Each entry is packed into its own read-only section with a local symbol and marked coalescable, so byte-identical entries from **different modules** collapse to one placement at link time, reusing the machinery `#[embed]` dedup already had.

Alignment is decided in one place and carried to the section unchanged. No encoder re-derives it, which is what will keep an aligned load legal at a width **wider than a register**, where an entry is read in register-width pieces at power-of-two offsets from its own base.

Pooling is a decision, not a reflex, and the targets disagree on purpose. x86-64 pools every non-zero constant, because with no immediate form the alternative is never cheaper even at a single use, and folds the entry straight into the operation that reads it: `mulsd xmm, [rip + .Lconst.0]`. aarch64 and riscv64 pool only what they cannot build in fewer instructions than a load takes - an all-zero constant reads the zero register in **one** instruction, and a pattern one `movz` or one `lui` builds stays in registers rather than touching memory for the same two instructions.

Measured, on programs that run:

| | before | after |
|---|---:|---:|
| five-term Horner loop body, x86-64 | 31 instructions / 164 bytes | 22 / 109 |
| the same program's wall time | 60.1 ms | **41.9 ms** (1.43x) |
| a four-constant kernel, x86-64 | 17 instructions | 9 |
| the same kernel, aarch64 | 25 | 13 |
| the same kernel, riscv64 | 41 | 13 |

The compiler's own code is not float-heavy, so it barely moves: compiling one source tree with both compilers, `.text` falls 354 bytes for 24 bytes of new `.rodata`, and every one of the 58 float-constant rebuild pairs in its objects is gone. The win is in numeric code, and this is what it is worth there.

### Changed

#### The `#[packed]` vector refusal now waits on one thing, not two (#2728)
The refusal is sequencing rather than policy, and it was waiting on two conditions: evidence that an unaligned vector access is correct on real silicon, and #2687's aggregate-layout half. The first is now settled.

`int/surface/unaligned-vector` stores and loads `i32x4`, `f32x4`, `i32x3` and `f32x3` at deliberately misaligned offsets. A store probe reassembles every lane from four byte loads and a load probe writes the bytes by hand, so neither reads back through the access it measures, and a sentinel in the bytes on both sides of each extent catches the truncated and over-wide stores a lane check alone cannot see. It is correct in both profiles on every **native** leg: `linux-arm64` on `ubuntu-24.04-arm`, which is the row that matters because aarch64 has 128-bit forms with alignment requirements, plus `linux` and `windows` on x86-64. Nothing faults, no lane is dropped, no neighbour is disturbed. `linux-riscv64` passes and is deliberately not counted: it runs under qemu-user, and riscv64 declares no 128-bit vector support, so the access there is a scalar expansion rather than a vector access.

The refusal itself is unchanged. Its diagnostic and `doc/language/decorators.md` now name what actually remains.

### Fixed

#### A constant out-of-bounds index is refused, on every target (#2751)
`var xs: [4]i32; ret xs[9];` compiled cleanly on x86-64, aarch64 and riscv64, and on SPIR-V emitted an `OpAccessChain` with `%uint_9` into a four-element array that `spirv-val` did not catch either. The length is part of the type and the index is a constant, so the violation was decidable with no analysis at all. It had simply never been asked.

Sema now asks it, keyed on `type.element_count` — the one definition of "how many elements a type holds", the same answer `$length_of` gives. Keying on the count rather than on the syntax is what makes the spellings free: a `def` alias, an array field of a generic instance, an array nested in a record or in another array, and a `^`-qualified array all name the same interned array type, and all get the same refusal. A vector's lane count is a statically known length too and now takes the identical rule and the identical message, replacing the vector-only wording it had before.

The diagnostic names the index, the type and the length: ``index 9 is out of bounds for `[4]i32` of length 4``.

Scope is deliberate. Only a **constant** index against a **fixed** length is checked, which needs no dataflow. A runtime index is not checked, and a pointer is not checked at all — `*T` carries no length to measure against.

Folding the index now decides on **resolution identity** rather than on name text. The comptime environment is keyed by name, so a block-local `var n` that shares its name with a module-level `val n` used to fold to the module constant. That was already visible on the vector path, where `v[n]` with a local `n = 1` was reported as an out-of-bounds lane against a module-level `n = 9`; it now reports the dynamic-lane refusal it should always have given, and an array indexed by such a local is correctly left alone rather than refused. The runtime-binding rule the `$if` gate check already carried is now one shared definition.

#### The debug-info validator's warnings are now an observable, not a wall (#2755)
`int/surface/debuginfo` read `llvm-dwarfdump --verify`'s exit status, which is zero on warnings. So "no diagnostics at all" and "no errors, plus a standing wall of about 200 warnings on every `-g` image" both rendered `dwarfdump_verify=clean`, and the golden recorded the second while claiming the first. That is how a validator stops validating: the next real warning arrives into a stream nobody reads.

The case now reads the stream. It reports `errors`, `warnings` or `clean`, and lists each residual warning text with only the per-CU section offset elided, so a failure names itself in the diff. Exactly one class is filtered, with its reason written beside it, and every other warning of any class now fails the case where before none could.

That filtered class is the duplicate itself, and it is deliberate. DWARF 5 §6.2.4 numbers file entries from 0 and §6.2.2 still gives the line state machine's `file` register an initial value of 1, and a single-file compile unit cannot satisfy both. binutils resolves it the way §6.2.2 says: measured on binutils 2.47, `addr2line` reports `mangled line number section (bad file number)` on every compile unit of a table whose only entry is slot 0, including clang's own `-gdwarf-5` output. gcc emits the duplicate and llvm-dwarfdump warns on gcc's output identically. `dwarf.mach` now records that, because the obvious repair trades a cosmetic warning from one validator for a real decode failure in every libbfd consumer. It is also validator-version dependent - llvm-dwarfdump 18 does not report it and 22 does - so leaving it in the observable would have made the golden a statement about the runner's llvm package rather than about the compiler.

#### An unencodable float move produced a wrong move instead of a diagnostic (#2733)
All three register machines answered a float-move width they do not implement by **substituting 8** and encoding anyway. `if (w != 4 && w != 8) { w = 8; }` sat in x86-64's `encode_fp_mov`, aarch64's `encode_fmov` and riscv64's `encode_fmov` - the same line in the same role on each - so a move the compiler could not encode became an 8-byte move that dropped everything above the low lane, with no error anywhere. A defensive fallback that produces **wrong output** is worse than no fallback: it converts "the compiler was asked something it cannot do" into "the program computes the wrong answer", which is the worst failure a back end has.

It had already cost real debugging. During #2687 three layers each answered one fixed-size ladder, and the middle layer's honest refusal was **masking** this one - widening that refusal alone compiled cleanly and then silently dropped the top lane of every stack-passed vector. That refusal was the only thing standing between this default and a shipped miscompile.

Every float encoder on every register machine now refuses a width it does not implement, and the census turned up two more sites than the issue named. x86-64 carried a **second** silent narrow in `encode_float_const_mov`, where a wrong width pools an f64-sized image of a constant that is not an f64 - and the constant is the value the program computes with. And the float **conversions** on all three targets carried the same substitution in a shape no search for the ladder finds: they read `dw == 8` / `src_w == 4` and took the other form for everything else, with no ladder to grep for. Arithmetic, negate and compare already refused on all three, and are now pinned by the same tests.

**The refusals are driven, not argued.** Each one is called directly from a unit test with a width it must refuse, and the assertion is that the byte sink stayed **empty** - "returned an error" and "emitted nothing" are different properties, and only the second distinguishes a refusal from an emit-then-error. Arguing that no caller reaches a site is exactly what let this sit. x86-64's `encode_fp_mov` reports its backend bugs rather than ending the process itself to make that checkable at all; the process-terminating decision moved to its single caller, unchanged in behaviour.

The aarch64 calling convention's two `make_slot_hfa(reg, 1, 16, size)` literals are on the same path and are now written as what they are: the **V register** width, not the vector's. A `f32x3` is twelve bytes and rides a whole register, so "correcting" the literal to `size` breaks it - which is measured, not supposed. What the literal did hide is the vector too wide for one register that #2727 makes legal, which would have been handed V0 and a piece describing half of it; those now spill by value and return indirectly.

#### `--emit-ir` shows a float constant, not its truncated integer magnitude (#2753)
Every float constant in an IR dump rendered as the integer part of its magnitude, so the surface could not show any distinction living below the decimal point, at the top of the range, or at the bottom of it. `1.5` and `1.75` both printed `f1`. A positive infinity printed `f9223372036854775808`, which is also what the finite value 2^63 printed, so an infinity read as a plausible finite number and the two were indistinguishable. The smallest denormal printed `f0`.

The middle end never lost any of this: `me/pass/cse.mach` value-numbers float constants by bit pattern and `me/pass/algebraic.mach` declines to fold `x + 0.0`, precisely so the IR keeps IEEE distinctions apart. Only the view collapsed them, which matters because a dump is what someone reads when they already suspect a float bug and the fraction is the evidence.

Constants now render through `std.format.write_f64` in shortest round-trippable decimal, so `f1.5`, `f1.75`, `f5e-324` and `f9.223372036854776e18` are each themselves, and the non-finite classes spell themselves as `finf`, `f-inf` and `fnan` rather than borrowing an integer. A NaN whose payload is not the canonical quiet one renders its full bit pattern as `fnan:0x…`, since a payload is a distinction the middle end keeps too. Signed zero still renders `f-0` and `f0`, which #2274 established.

The reason recorded in the code for the old rendering - that std had no float formatter - had gone stale; `printer.mach` already imported `std.format`. The unit test that pinned `-1.5` to `f-1` and `2.5` to `f2` was enforcing the defect rather than any intended behaviour, and now pins the real rendering across the IEEE classes, built from bit patterns rather than decimal literals. Objects are byte-identical: this moves nothing but the dump.

#### A riscv64 `-g` local is described where it actually lives (#2759)
Every `-g` variable in a frame slot was described at the wrong address on riscv64, by exactly the bytes the prologue reserves at the top of the frame. The debug-info producer recorded the raw slot offset while the encoder biases every slot access down past the ra/s0 record and the callee-saved GP and FP areas, so a debugger read `s0 + slot.offset` where the machine had written `s0 - reserved_top + slot.offset`. In a function with callee-saves live that lands in the save area: measured on a fixture with six callee-saved GP and three FP registers, the reservation is 88 bytes, the variable is at `s0 - 104`, and `DW_OP_fbreg -16` pointed at the saved `s0`. Silent, because the value has the variable's type and a plausible magnitude.

One layout fact was derived twice, and the two derivations agreed on x86-64 and aarch64, whose reservation is zero. That is why it went unnoticed, and it is the same shape as #2715, #2725 and #2735.

There is now one derivation. `MirFrame.fp_dist` - the bytes between the frame pointer and the slot region's base - is stamped once by the target's own encoder alongside `base_dist`, its stack-pointer-side twin, and both the encoder's own slot accesses and the `DW_OP_fbreg` offset read it. `frame.slot_offset` no longer takes a caller-supplied bias, so there is no longer a reader that can forget to apply it.

`int/regression/2759-riscv64-fbreg-bias` pins it by crossing the two halves against each other: every `DW_OP_fbreg` offset must name an address the same function's emitted code addresses from the register `DW_AT_frame_base` names. Reading the offset alone cannot see this, since a producer with its own derivation is self-consistent under the bug. The case runs on all three ELF legs, and against the unpatched compiler it is green on x86-64 and aarch64 and red on riscv64 alone.

`.text` is byte-identical with and without `-g` on every target, and `.debug_info` is byte-identical before and after on x86-64 and aarch64.

#### An over-aligned stack local is over-aligned at run time (#2735)
`#[align(32)]` and above worked on a global and did nothing on a local. The slot's offset was a correct multiple of the requested alignment, but it was measured from the frame pointer, and the only alignment that reaches the frame pointer is the ABI's 16-byte call boundary. So the address was a multiple of 32 exactly when the process stack happened to start on one, which depends on the environment block and therefore changes between runs of the same binary. `$size_of` already reported the padded size, so the storage was sized for a promise the placement did not keep.

A function whose frame contains an over-aligned slot now gets a prologue that masks the stack pointer down to the largest alignment that frame needs, and its locals, spills and callee-save area are addressed from there. The frame pointer stays exactly where the ABI put it: incoming stack arguments and the frame record hang off it and belong to the caller, and after a mask only one of the two pointers is still a fixed distance from the caller's stack. Debug info follows the same split: such a function's `DW_AT_frame_base` names the stack pointer, which is the register its recorded offsets are measured from.

The decision is recorded once, on `MirFrame`, and every encoder reads it. So is the distance from the stack pointer to the slot region's base, which aarch64 and riscv64 had each been re-deriving for inline-asm `{name}` operands since #2689.

The cost falls only on functions that ask: one masking instruction and up to `N - 16` bytes of frame. A function with nothing over-aligned emits a byte-identical prologue to before.

#### A sub-width vector could not reach a SPIR-V shader's interface, or be declared as a zeroed local (#2744, #2758)
`#[input(0)] var a: f32x3;` could not be read or written at all. Neither could an `#[output]` or a `#[builtin]` of `f32x2` or `u32x3`, and neither could a local declared without an initializer - `var v: f32x3;` was refused at every **read** of the slot while `var v: f32x3 = a;` beside it was fine. `f32x4` and `f64x2` worked throughout. A vec3 vertex attribute is one of the two live needs #2687 names, and a vertex attribute that cannot be read is not one.

Both were the same lowering. A vector whose memory footprint is narrower than its register cannot cross between the two in one access on a byte-addressed machine, so #2687 routed every such move through a register-width scratch slot walked in 8/4/2/1 chunks - and applied it on **every** target. Under logical addressing there is no register and no byte footprint: a value is one value at one type. Running the round trip there costs three things the target cannot express at all - a scratch slot the vector is not typed at, byte-sized chunk accesses of it, and a reinterpreting load back out - and it destroys the direct symbol reference the emitter reaches an interface variable through.

It is now gated on `MachineModel.flat_addressing`, the same lever #2655 used for the float address materialization that made scalar `f32` varyings unreachable for the same kind of reason. **No machine target changed**, asserted rather than assumed: `int/surface/vector-subwidth` builds byte-identical on linux-x86_64, linux-arm64 and linux-riscv64 in both profiles.

The refusal that reported this was worse than the defect it reported. It keyed on the address's origin and then sent three of the four origins to one sentence about module-scope interface variables, so a program holding no module-scope variable at all was told that a module-scope variable is reachable only when it carries an interface role - sending two readers to a role they had already written. Every origin has its own arm now, and the one for an interface variable that *does* carry a role says what is actually wrong: the access reached the back end as computed addressing rather than as a direct reference, which is a lowering that ran where it should not have and not a limit of the declaration.

#### The `uvec3` compute built-ins were refused on a rule #2687 deleted, and no built-in's type was checked (#2745)
`#[builtin("global_invocation")]`, `local_invocation` and `workgroup_id` were refused by name, on the ground that SPIR-V specifies them as three-component unsigned vectors and every mach vector is exactly 128 bits, so `u32x3` could not exist. That stopped being true when #2687 landed, which left the diagnostic a false statement about the language and a compute stage unable to know which invocation it is.

The wider hole was that **no** built-in's declared type was checked, at any of the eight. `#[builtin("position")] var p: i32;` was accepted by the emitter and rejected by `spirv-val`, and nothing covered it.

So the refusal inverts into the check its own comment said it wanted. One table gives every built-in the type SPIR-V specifies for it, and a variable declared at anything else is refused by a message naming both the declared type and the required one. Adding a built-in is a row in `builtin_selector`, a row in `builtin_storage` and now a row here, and it cannot be added without the third. Signedness is not checked because nothing can distinguish it: IR carries an integer's width and nothing else, and the emitter writes `OpTypeInt n 0` for both spellings.

`GLSL.std.450 Cross` becomes a row alongside it. It is defined only for three-component float vectors, so it was cut pending exactly this ABI work.

#### Every SPIR-V operation on a sub-width vector was mistyped (#2743)
A vector narrower than 128 bits reached the SPIR-V target correctly as a type and was computed on as if it had four lanes. `fun add3(a: f32x3, b: f32x3) f32x3 { ret a + b; }` built with no diagnostic and emitted `OpFAdd %v4float` inside a function declared `%v3float`, which the Khronos validator rejects. Arithmetic, bitwise, comparison-to-mask and lane writes were all affected, on every shape that is not exactly one 128-bit register, and a vector literal was the single spelling that failed loudly - as an internal error with no source location, because it is the one site that counted operands.

The MIR lane descriptor carried an element class and an element byte-width and **derived** the lane count as `16 / element-bytes`. That is the 128-bit assumption #2687 removed from the language, still standing in the one representation it did not reach. It is sound for the ten shapes that are 128 bits and wrong for every other one.

The descriptor now carries the lane count as a supplied fact. It is a `u32` packing the class, the lane width and the count, with sixteen bits of count rather than the four a 128-bit register would need, because the hardest member is the **over**-width vector of #2727 rather than the sub-width one at hand. Its constructor takes the count positionally, so a descriptor cannot be built without one, and the IR type accessor that feeds it now reports the count alongside the class and width for the same reason - a caller holding the width but not the count is exactly the shape that invented one.

**No machine target changed.** x86-64, aarch64 and riscv64 select an instruction from the lane class and width and write a whole register, so none of them ever read a lane count, and all three were correct on the bug. That is why it survived: SPIR-V is the only target that must *name* a vector type, so it is the only place the count is observable, and the eleven `int/surface/spirv-*` cases were 128-bit shapes by construction.

`int/surface/spirv-vector-narrow` covers `f32x2`, `f32x3`, `i32x2` and `u32x3` through arithmetic, bitwise, compare-to-mask, lane read, lane write, construction, control flow and calls, validated by `spirv-val`. The unit test over the descriptor changed too: it asserted `16 / element-bytes` back at itself, which is how a derivation pins its own defect.

#### The pinned integration lane can run (#2729)
`int/run.sh --deps pin` stopped on the first SPIR-V case on every target, because no lock recorded a commit for `mach-shader`. The lock was not stale, it was the wrong set: the root `mach.lock` is written by `mach dep pull` from the **root manifest**, so it can only ever cover the compiler's own dependency closure, while `int` builds a larger one. Two cases declared a dep the root has no reason to.

So the pin now reads two sources with disjoint coverage. The root `mach.lock` stays authoritative for anything the compiler also builds against, which is what keeps `mach-std` to one place to bump, and the new `int/deps.lock` covers only the difference. `int/lib/update-deps.sh` writes it from what `mach dep pull` resolves, so no ref-to-commit mapping is reimplemented outside the compiler.

The lane failing was the smaller half. `--deps pin` exists because a case floating a ref can change verdict with zero changes in this repo (#2592), and it is what a release gate and a bisect run, yet its inputs were only ever validated by running it, and the one pinned run in CI is the main-cadence darwin gate, whose leg set contains neither case that declares the missing dep. `int/lib/check-deps.sh` now checks the whole condition statically on every PR - coverage, that the two locks stay disjoint, that no pin outlives the case that asked for it, that cases declaring one dep agree about it, and that a branch ref is `branch/main` - and a pinned linux run of the full suite joins the PR lane. Every run also prints the mode and, when pinned, every lock entry it consulted, so a floating run cannot be read afterwards as a pinned one.

#### A sub-width vector can cross a function boundary (#2687)
4.15.0 shipped `f32x3` working inside a function and refused at every call boundary. Passing or returning one reported `CLASS_FP scalar of unsupported size`, which made the shape unusable in practice: a shader-math or vertex-building library is nothing but functions taking and returning these.

The cause was the same one #2697 fixed one layer down, and there turned out to be **three** layers of it. A value's register width and its memory footprint are different numbers that agreed for every vector until lane-derived layout made them differ, and each layer answered both questions with one fixed-size ladder:

- `op_width_of` fell through to the general-purpose word, losing lanes (fixed in 4.15.0)
- `fp_move_width` refused outright, which was honest and total
- `encode_fp_mov` **silently narrowed** a 12-byte move to 8

The middle one was masking the third. Widening it alone compiled and then dropped the top lane of every stack-passed vector with no diagnostic. So the ABI now asks the two questions separately: a register move uses the target's vector width, and every register-to-memory access uses the value's own extent through the scratch round trip, because no ISA here can issue a 12-byte access and the encoder quietly narrows one that claims to.

Classified per target from `MachineModel` facts rather than assumed to follow x86-64, and **nothing is refused**. Apple aarch64 is the case that breaks the obvious fix: it gives a fixed stack argument its natural size, so the slot is exactly 12 bytes with the next argument packed against it.

The SPIR-V half of #2687 remains open.

## [4.15.0] - 2026-08-07

### Added

#### A vector narrower than the vector register works (#2697)
`f32x3` and every other sub-register-width shape were refused by name, because codegen could not carry one. `op_width_of` answered a **single number** for two different questions, a value's register width and its memory footprint. Those coincide while every vector is exactly 16 bytes, and stop coinciding the moment a lane-derived layout makes one 12. A 12-byte vector truncated to 8 on a move, and carrying it at the register width instead overran its slot and clobbered the next field.

The two questions are now separate. The hole was never "non-power-of-two" as first supposed: it is any size **strictly between the target's general-purpose register width and 16**, which is a target-dependent range rather than a property of the number.

So `rec Vertex { pos: f32x3; uv: f32x2; }` is genuinely **20 bytes**, and `[N]Vertex` is a real vertex buffer rather than a padded array indistinguishable from one built on `f32x4`.

**A sub-width vector cannot yet cross a function boundary.** Passing or returning one is a clean refusal naming the ABI classifier, not a miscompile. #2687 carries that half.

#### A project can declare the toolchain it needs (#2714)
A manifest could not say which compiler it required, so a project using a feature added last week failed on an older toolchain with an ordinary parse error pointing at the feature rather than at the version. `[project].mach` takes a semver minimum, validated when the manifest is read:

```toml
[project]
mach = "4.15.0"
```

Checked for the root project and for every dependency, so a dependency that outgrows the running compiler says so in its own name rather than failing somewhere in its source.

#### `#[packed]`, a record laid out with no padding (#2715)
`#[align(N)]` only ever raises alignment, and nothing did the inverse, so any shape whose layout is decided elsewhere — a C struct, a file header, a wire frame, a vertex whose stride the buffer fixes — could not be described as a record at all. `#[packed]` on a `rec` or `uni` places every field immediately after the previous one, adds no tail padding, and takes no alignment from its fields.

It **composes** with `#[align]` rather than conflicting: packed decides padding, align decides the record's own alignment. It is **not transitive**, matching C — an inner record keeps its own internal padding and is merely *placed* without any, which is the rule that composes and the one a reader is most likely to assume wrongly.

Three refusals, and one of them turned out to be two:

- **The address of a packed field.** `?r.b` would yield a `*u32`, which states alignment 4 to everything downstream while the storage has none. The access is fine; the pointer *type* is what is untrue, and it travels. The predicate walks the whole access chain, so `?r.arr[0]`, `?r.inner.x` and `?p.b` through a `*Packed` are refused too. `?r` on the whole record stays legal — a `*Packed` describes an align-1 pointee correctly. Fail-closed on purpose: refusing is relaxable once alignment can ride in a pointer type, permitting is not tightenable.
- **Atomics on a packed field**, which needed no rule of its own. Atomics are not a language surface in mach — `std.sync.atomic` is ordinary functions over `*i64` — so a pointer is the only route an atomic has to a field, and the address-of rule already refuses to hand one out. Pinned as an `int` case so the coupling fails loudly if atomics ever become an intrinsic over an lvalue.
- **Vector fields**, including through an array or a nested record. The reason is evidence, not arithmetic: an unaligned *scalar* access is measured on hardware and that measurement is what `#[packed]` rests on, while an unaligned *vector* access has no equivalent measurement and is the case where an emulator is least trustworthy. #2687 carries the aggregate-layout half. A sequencing decision, not a permanent rule.

Also refused on a `#[uniform]` / `#[storage]` interface block, whose member offsets are fixed by std140 / std430.

**riscv64 gets a written claim rather than a green tick.** Unaligned access there is permitted-but-may-trap and Linux emulates a trapping one in the kernel, so a packed field access is expected to be correct and pathologically slow. That cost cannot be measured under qemu-user, which emulates the misaligned load directly and never takes the kernel path. The decorator docs say so rather than leaving a passing correctness leg to be read as a usability result.
#### `$length_of`, and a type operand may be a value binding (#2536)
A program could not ask a value how long it is. `$size_of` took a **type**, and a binding's type has no spelling -- `val LOGO: [_]u8` really is a concrete `[7194]u8` that is simply never written -- so an `#[embed]` could be indexed and passed around and never **iterated**. The only way to learn an embed's length was to declare it as an explicit `[N]u8`, which defeats the inferred form for any asset whose size is not fixed by contract.

Two changes, and they are separable on purpose.

**`$length_of(T)` counts elements where `$size_of(T)` counts bytes.** For `[N]u8` the two answers are equal, and for everything else they are not. Making the caller divide by an element size is exactly the silent-arithmetic error this codebase keeps finding, and the equality at `u8` is what hides it during development, so the distinction belongs in the surface rather than in the caller's head. A vector's length is its **lane count**, which is decided rather than incidental.

Only a fixed array and a vector have an answer. Everything else is **refused rather than guessed**, naming the type: a pointer (`str` included) has a length the compiler does not know, and a record has a field count rather than an element count.

**A value binding is accepted in a comptime intrinsic's type-operand slot**, denoting that binding's type:

```mach
#[embed("assets/logo.qoi")]
val LOGO: [_]u8;

var i: u64 = 0;
for (i < $length_of(LOGO)) { ... }     # 7194 elements
$size_of(LOGO)                         # 7194 bytes
```

This is the **operand slot's** rule rather than a per-intrinsic one -- "does this name a type" is the slot's question, and every intrinsic that takes one asks it identically -- and it is *only* that slot. A name written where a type is expected and resolving to a value is still a mistake in every other position, and accepting it there would turn a typo into a silently-typed binding. The slot is marked where it is produced, by the parser, rather than inferred later from context.

The issue's premise that "a value name and a type name occupy separate registries" does **not** hold: both live in one scope chain, so a name is either a type or a value and never both. There is no ambiguity to resolve, which is why the scoping is by position rather than by lookup order.

A local binding's operand resolves the binding's **annotation node** rather than reading the parallel `decl_type` array, because the operand is reached from `resolve_all_types`, which runs before that array is filled -- reading it would answer TYPE_ERROR for every binding declared after the measurement. Pinned by a case that measures a binding declared later in the file.

The `#[embed]` limitation note on the decorators page comes out with this.

#### `$pointee_of(T)` makes a reference field traversable (#2693)
`$is_pointer(f.type)` told a reflection walk that a field was a reference, and that is where the walk stopped: there was no way to ask what a typed reference points at. `std.derive` refused every reference field at comptime as a result, which blocked the whole family that wants one -- deep equality, owning clone, a hash over pointed-to contents, and a formatter that renders a `str` field, since `str` is `def str: *char` and so a reference field like any other.

`$pointee_of` is a type **constructor**, in the same family as `*`, `[N]` and `^`, rather than an intrinsic call. That is what makes it nest, and nesting is the requirement rather than a nicety: the descent a walk actually performs is `$fields($pointee_of(f.type))`, an operand inside an operand, and a call cannot appear where a type is required.

```mach
$each f in $fields(Node) {
    $if ($is_pointer(f.type)) {
        $each g in $fields($pointee_of(f.type)) { total = total + (@(n.[f])).[g]; }
    }
    $or { total = total + n.[f]; }
}
```

**Everything that is not a typed reference is refused, and the refusal names what it was handed**: the raw `ptr` with its own cause (it is untyped and carries no pointee), `^*U` because `$is_pointer` answers false for it and descending would put the intrinsic back into the disagreement with its own gate that #2692 closed, and any other type by name. A plausible wrong answer here flows into a `$fields` walk that then reports about the wrong record, which is the failure class a refusal exists to prevent.

**`$pointee_of(**U)` is `*U`, one level and not all of them.** An implementation that peeled to the innermost non-pointer passes every single-level case and is wrong here, so it is pinned by value rather than by compiling.

**A latent memo bug surfaced with it and is fixed.** `resolve_type_ref` skipped its memo for `f.type`, whose answer changes per `$each` iteration -- but the rule was written as "this node IS `f.type`" when the real rule is "this node's answer is not fixed by its source text". The two agreed only because `f.type` was spellable at two leaf sites and nowhere else. `$pointee_of(f.type)` is a composite containing one, so the memo would have handed every iteration after the first the first field's pointee. Pinned by a record with two reference fields at *different* pointees, where a stale answer is a clean compile with a wrong field count.

Termination is the caller's problem and is stated as such: unlike the by-value walk of #2691, following references does not terminate structurally, since `rec Grow[T] { p: *Grow[*T]; }` has unboundedly many instances reachable through its pointer.

### Changed

#### Vector operation legality is target-independent, so an unpacked lane op scalarizes instead of being refused (#2726)
`i32x4 * i32x4` was refused at the language level, citing x86-64's SSE2 baseline (`pmulld` is SSE4.1). That refused it on **riscv64** — a target with no vector unit, which scalarizes `i32x4 + i32x4` one line up and would lower the multiply to four ordinary scalar multiplies — and on **aarch64**, whose NEON `MUL` supports `.4s` in hardware. The rule refused work on the targets least able to benefit from the refusal.

The legality of a vector type and of a vector operation are now both target-independent; only the **realization** is target-dependent. Integer `*` is legal at every lane width on every target, and the backend picks the packed form where one exists and the per-lane scalar expansion where one does not.

The rule's three homes collapse onto one authority. `isa.packed_width` answers a realization question — "how wide is this target's packed form for this operation at this lane width, or 0 for none" — declared per ISA beside the other capability facts and reached through the single mid-end door `me.vecform`. **Sema stops asking entirely**, because it was never a legality question. It returns a *width* rather than a bool so #2727 has the number to split by, and it is keyed on the lane width and never the lane count, so a vector wider than the register splits into chunks that each get the same answer.

**`simd = "require"` now applies per operation on every target**, not only where there is no vector unit, and names the lane width:

```
error: simd = "require": vector multiply on 64-bit lanes in '_M4case4mainN5mul64' would scalarize on aarch64
       (target has no packed form for this operation at this lane width); set simd = "scalarize" to build with the scalar expansion
```

A project that set the lever precisely to refuse a silent scalar expansion was previously told nothing about the ones x86-64 and aarch64 perform.

The auto-vectorizer consults the same authority, so it forms a vector multiply only where the target packs one rather than forming one the realization stage would immediately take apart.

### Fixed

#### An inline-asm `{name}` operand ran out of reach in a large frame (#2689)
A `{name}` binding was addressed frame-pointer-relative, and on aarch64 the addressing form's immediate range gave it a reach of **sixteen** slots. A function with a larger frame, or one an inliner had grown, refused to encode. riscv64 had the same limit at 128 slots. x86-64 had none, which is why it went unnoticed.

The reach was not the real defect. The interface itself said *frame-pointer-relative*, freezing one target's addressing choice into the contract every target reads, when which base a `{name}` uses is part of the addressing decision and only the target knows which of its forms reaches. aarch64 and riscv64 now measure from the stack pointer, x86-64 keeps the frame pointer, and asm-bound slots moved to the bottom of the slot region so what separates a `{name}` from its base is the callee-save and outgoing-argument areas, bounded by the ABI rather than by the function.

**New refusal:** an inline-asm body that binds a `{name}` may not write the stack pointer, on targets that measure from it. The displacement is taken once at block entry, so the pointer must still be where it was. The whole block is judged rather than a prefix, because a backward branch reaches an earlier `{name}` again with the moved pointer. Move the stack adjustment out of the block, or drop the `{name}` and stage the address into a register.

#### The NEON integer-multiply arrangement rule was wrong in both directions (#2721)
`arm64/encode.mach` declared the `MUL (vector)` base word as "legal only on .8h (size 1)". NEON encodes that word at sizes 0/1/2 as `.16b` / `.8h` / `.4s` and leaves size 3 **unallocated**, so the comment forbade three arrangements the hardware has and omitted the one real constraint: there is no `mul v.2d`.

The encoder was not wrong yet, because nothing selected the word at another size. That is exactly the #2037 shape — a base word plus a size field that *looks* general, with nothing pinning what each arrangement encodes — and a widening that trusted the word's apparent generality would emit `0x4EE29C20`, an invalid encoding, rather than refuse.

Each permitted arrangement is now pinned byte-exact against the encoding definition, and `neon_3same_size_ok` makes the hole a fact the compiler enforces: a 64-bit integer lane multiply reaching the selector is a defined error emitting no bytes, proved by driving the selector directly rather than by arguing no caller reaches it.

#### Field stores went through a second layout policy that no decorator could reach (#2715)
`mir.lower_ir`'s `struct_field_offset` re-derived every field offset with its own `round_up` over each field's alignment, rather than calling `ir_type.byte_offset` like every other offset consumer. So a `#[packed]` record reported correct `$size_of`, `$align_of` and `$offset_of` — those go through the shared layout policy — while the code that actually **read and wrote the fields** placed them at the **natural** offsets, and nothing errored. A record whose fields fit its packed size wrote its last field past the end of its own storage.

Found by an `int` case whose golden is a **byte image** rather than a set of intrinsics. Pinning `$size_of` / `$offset_of` proves only that the compiler agrees with itself; the bytes are what a C struct or a wire frame is actually compared against. The duplicate walk is deleted — there is now one layout policy, so a future layout decorator cannot be taught to half the compiler.

#### A secret spilled to memory no longer launders past the `#[oblivious]` asm walk (#2706)
The `#[oblivious]` inline-asm walk carried taint as a register bitmask with **no memory domain**, so a secret stored to the stack and reloaded came back in a register the model believed public. That defeated all three leak checks at once - a branch condition, a memory address, and a variable-latency operand:

```
mov rax, {r}
mov [rsp], rax
mov rbx, [rsp]
mov rcx, [rbx]     # a secret-derived address, accepted
```

The direct form was always refused, which made the gate look sound while an ordinary spill walked through it. Memory is now the third taint domain beside the register set and the flags bit, shared across x86-64, aarch64 and riscv64 as the walk itself is.

The domain is one bit for all of memory, and monotone. Per-slot tracking is not sound at this layer: `[rsp + k]` and `[rbp + j]` can name the same byte with nothing relating two base registers, and a slot key is an address rather than an extent, so overlapping accesses of different widths compare as different slots. The failure direction is over-refusal, never acceptance, and a body only loses by it if it stores a secret, reloads some other public value, and then leaks through what it reloaded.

Note for anyone extending the walk: "does this instruction store" cannot be read from the `writes` mask, which names register operands. aarch64 `str` and riscv64 `sd` both declare `AW_NONE`, so a memory domain derived from that mask fires on x86-64 and silently passes every store on the other two targets.

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
