# Persistent object cache, phase 1 (#3221)

Status: audit of the archived candidate `60570613` ("feat(build): cache object
images across compiler processes", `archive/3218-candidate-era`) plus the digest
memo `ddcdb67a`, and the decisions taken for the phase-1 extraction onto dev.
Phase 2 completes the keys listed as gaps below and flips the default.

The issue's acceptance bullets are the reference. Each key class gets one of
three verdicts: **keyed** (the candidate's key changes when the input changes),
**gap** (not in the key, or in it too coarsely to matter), **wrong** (in the key
in a way that produces a wrong answer or contradicts a stated contract).

## What the candidate does

One persistent product: the `of.ObjectImage` a module's codegen produced,
serialized by `src/lang/build/cache/image.mach` and stored by
`src/lang/build/cache/store.mach` under `<project out>/.mach-cache/`. Nothing
from the front end is persisted; a hit still loads, resolves, checks, lowers and
verifies every module, then skips `codegen.codegen_module` for that module.

Keys are two SHA-256 digests (`src/lang/build/cache/key.mach`, length-framed
fields under a domain string, so field boundaries cannot slide):

- the **cell snapshot** (`driver/cache.mach:snapshot`): compiler identity,
  the build-configuration bytes (`passes.capture_build_config`), compiler name
  and version, the build request hash, the placement policy, the executed-step
  digest chain, project module and source-directory names, the project root,
  the root count, then for every loaded module its FQN, source path, full source
  text and dependency FQNs, the emission order, every dependency entry (alias,
  id, source dir, module, vendor root, source, ref, content, version, direct
  aliases), every typed module's embedded inputs (path, length, content
  digest), and the link providers (name, static or shared);
- the **object key** = domain `mach.object-image` + snapshot + module FQN.

An object entry is therefore invalidated by any change anywhere in the cell.
That is coarse (one edited file misses every module) but it is what makes the
"resolved generic or inline bodies" class trivially sound: every body any
instantiation could have come from is in the snapshot.

Hook points: `passes.prepare_persistent_cache` runs after
`acquire_codegen_inputs` (both the parallel and serial codegen paths);
`passes.codegen_reusable` consults the store when the in-process `Q_CODEGEN`
product is not reusable, staging a decoded image on the module; and
`q_codegen_compute` publishes every freshly generated image and returns a staged
image without generating. All of it runs on the main thread inside one query
operation; `project.begin_query_phase` resets the per-operation cache state and
`finish_query_phase` releases it.

## Key classes

| class | candidate | verdict |
|---|---|---|
| compiler / schema identity | SHA-256 of the running executable, computed through `/proc/self/exe`, the darwin vnode of the code mapping, or the windows module path with delete sharing denied. The codec has a magic but no version number; the executable hash covers the codec's code. | keyed (see the identity decision below) |
| transitive source | every loaded module's path and full text plus the dependency edges and emission order. std modules are loaded modules, so they are here too. | keyed, coarse: any edit misses the whole cell. The path is keyed as spelled (`mach build .` and `mach build /abs/path` miss each other); that is sound, because with `-g` the spelling reaches the line tables and the objects differ (34 of 41 hello objects), and without `-g` they are identical. Phase 2 may key the canonical path and add the spelling only when debug info is on. |
| resolved generic or inline bodies | covered by the whole-cell source hash, not by an instance graph | keyed, by construction |
| std and dependency content | dependency manifest identity (alias, id, source, ref, content, version, direct aliases) plus the dependency modules' text through the class above | keyed |
| target / ABI | request semantic hash (target, profile, artifact, goal, opt, debug, pie, simd, vectorize, float_reassoc, include_deps), `target.fingerprint`, os/isa/abi/platform names, project id, version and binary name, `[define]` names | keyed. The candidate hashes names by interner id (`fp_u32(id)`); ids are process-local, so the same names can produce different bytes across processes. It replaced them with `fp_name` (spelling). Ported. |
| environment | the candidate keys the build-configuration bytes, which sample the whole inherited process environment whenever the project has a dependency or a step (dev's `write_step_environment` for #3146) | **wrong** as a persistent key: a build from another shell missed every entry (observed: `SHLVL` and `_` differ between a script and an interactive shell). Objects depend on the environment only through the steps that ran, and the step chain keys each step's program content, argv, env, inputs and outputs. Not ported: the snapshot keys `capture_build_identity` (request, target, names, defines, declared steps) and the planner environment stays in the in-process `Q_TARGET` input. Control: `passes.capture_build_identity:excludes_the_planner_environment`, plus the cross-environment hit below. |
| artifact identity | `req.artifact` and `config.bin_name` are in the snapshot | keyed. Two artifacts of one project never share entries even for common modules; phase 2 may move artifact identity out of the module key once link inputs are cached separately. |
| profiles | profile name, opt, debug, vectorize, float_reassoc, simd through the semantic hash | keyed |
| embedded data | per typed module: embed path, length and content digest; an unavailable embed refuses the snapshot | keyed |
| link inputs | link providers (name, static or shared) are in the snapshot because import codegen depends on them. Link tokens, library directories, archives, resources and the output kind are not cached products: the link always runs against current inputs. | not cached (link products are phase 2, and `Q_LINK_CONFIG` in-process keying is separate; its missing `include_referenced` axis is fixed on this branch) |
| generated-step / tool / environment inputs | `p.step_digest` is a chain over every step the plan ran or reused: owner, step name and the step fingerprint (target, profile, program path and content, argv, env, prerequisites, output paths, input file digests) | keyed |
| nonsemantic request fields | the candidate keys `request.request_hash`, which includes `jobs`, `emit_ir`, `emit_asm`, `-o` and the link tokens | **wrong**: `--jobs 4` and `--jobs 8` produce different object keys, contradicting `request.mach`'s own semantic/nonsemantic split (`semantic_hash:omits_nonsemantic_fields`). Not ported: the snapshot keys `semantic_hash` plus the placement policy. |
| object image completeness | the candidate codec predates dev's native section and symbol metadata (`Section.native`, `Symbol.native`, `Symbol.absolute_value`, `ObjectImage.indirects`, `SEC_FLAG_COALESCE`) | **wrong on dev**: a hit would drop Mach-O and COFF native metadata and absolute symbols, so cached and uncached links would differ. The port extends the codec and its roundtrip test to every field of the current `of.ObjectImage`. |

Not in any key and not needed: `jobs`, verbosity, `-o`, `--emit-ir/asm`,
`--plan`, the cache flags themselves.

## Publication, validation and failure behavior

- **Layout.** One file per entry, named by the 64-hex object key, in
  `<out>/.mach-cache/`. Entry = 76-byte header (magic, object key, SHA-256 of
  the payload, payload length) + payload. Store limit 512 MiB of logical entry
  bytes, entry limit 64 MiB, decoded-owner budget 64 MiB.
- **Atomic publication.** `store.publish` takes the root lock
  (`std.filesystem.transaction`: `flock(LOCK_EX|LOCK_NB)` on
  `.mach-txn-lock`), runs `txn.recover` (sweeps recognized `.machtxn.*` staging
  left by an interrupted writer), claims the leaf, `prepare_bytes` writes a
  private staging file, and `commit` is one native `rename`. A reader never
  sees a partial entry under the final name.
- **Corrupt, truncated, stale.** `store.read` refuses anything that is not a
  regular file (`O_NOFOLLOW`, symlink is refused), whose size does not equal
  header + declared length, whose magic or embedded key differ, whose payload
  hash differs, or that has trailing bytes. `image.decode` then refuses
  truncation, trailing bytes, count overflows, non-canonical booleans, absent
  names, and validates the decoded object with `of.validate_owned_object`, with
  a resident budget so a hostile count cannot allocate unboundedly. Every
  refusal is a **miss** (compile normally); it is never an accepted product.
  There is no "stale" state by content: an entry is either the exact key or
  another file.
- **Concurrent writers.** The root lock is exclusive and non-blocking. The
  loser gets `LOCK_HELD`, the store reports `UNAVAILABLE`, and that process
  disables its cache for the rest of the operation (`p.cache_ready = false`).
  No retry, no waiting, no corruption. Phase 2 may add a bounded retry.
- **Cache unavailable** (directory cannot be created, root cannot be opened,
  lock held, containment violation such as an unrelated file in the directory,
  or I/O error): the build proceeds uncached and succeeds. **Compiler failure**
  (allocation refusal, `INVALID`, codec `INTERNAL`) is an error and fails the
  build. The two are distinguished by `store.Error` (internal) versus
  `store.Status{UNAVAILABLE}`.
- **Eviction.** Before a publish, `make_room` sums the directory and unlinks
  entries in directory order until the store is under the limit, under the
  same lock. Bounded, but the policy is arbitrary (not LRU): phase 2.
- **Resident bounds.** Decoded images are charged against a 64 MiB per-entry
  owner budget; hits are boxed on the module and taken by `Q_CODEGEN` exactly
  as a fresh image would be, so no live pointer or arena is ever serialized.
- **Git and fetching.** No cache path touches `dep/` or Git state; the cache
  lives under the project's own output directory.
- **Forcing an uncached build.** `--no-cache` on `build` and `test`: no object
  entry is read or written, no build-step stamp is read or written, and every
  declared step executes. One mechanism, documented in `doc/cli.md`.

## Compiler identity: the decision

Three candidates were evaluated for the compiler-identity component of the key.

1. **Linker-emitted build id.** A content hash the linker writes into the
   binary (ELF `.note.gnu.build-id` under a `PT_NOTE`, Mach-O `LC_UUID`, PE
   CodeView debug directory), read back at startup from the headers only.
   Today mach's linker emits none of these: `of/elf.mach` has no `SHT_NOTE` or
   `PT_NOTE`; `of/macho.mach` writes `LC_UUID` only under `--pie` and derives
   it from the artifact **name**, not content (`write_macho_uuid`), so it is
   not an identity; `of/coff.mach` has no debug directory. Adding it is three
   format writers, a hash-everything-but-the-note layout pass, and three
   startup readers, plus a full-hash fallback for any compiler linked by a seed
   that predates the note (the seed lag rule: a compiler built by generation A
   carries no note until A emits one). Right destination, not a phase-1 change.
2. **Per-process memoized digest** (the candidate plus `ddcdb67a`). Exact:
   the key is the content of the running compiler, on all three hosts, with
   the file identity checked so a replaced binary cannot be hashed by path.
   Cost: one 13 to 18 MB SHA-256 per process, 139 ms release / 275 ms debug
   with std's pure-mach sha256, paid once per process however many units it
   builds. The 91-process corpus lane pays it 91 times per target.
3. **Version plus schema constant.** Free, and wrong for every development
   compiler: successive from-source builds share a version string and differ
   in code, so entries produced by an older compiler would be served to a
   newer one. Rejected.

**Chosen: 2 for phase 1**, with 1 recorded as the phase-2 replacement. The
cost is confined to invocations that opt in (below), so the corpus lane pays
nothing until phase 2 lands the build id. A file-identity sidecar (device,
inode, size, mtime, ctime to digest) would remove the hash from the second
process onward but adds a second trust root; the build id is the smaller total
change and is preferred.

## Phase-1 extraction decisions

- **Default off.** The cache runs only with `--cache` on `build` or `test`.
  `--no-cache` remains the documented force-uncached mechanism and wins when
  both are given. Reasons: the keys above still carry two gaps (environment
  over-breadth, arbitrary eviction), and on a single-artifact build the digest
  (139 ms) costs more than a hit recovers (about 70 ms of object compile,
  measured on #3218), so an on-by-default cache would slow the common case.
  A build without `--cache` executes no cache code after the request flag
  check: no digest, no directory, no store I/O. The controls below prove it.
- **Snapshot keys `semantic_hash`**, not `request_hash` (the wrong verdict
  above), plus the placement policy which is a codegen input outside the
  request hash.
- **Codec extended** to the current `of.ObjectImage`: `Section.native`,
  `Symbol.absolute_value`, `Symbol.native`, `ObjectImage.indirects`, and
  `SectionId` for section references. The roundtrip test covers every field.
- **Store directory scan** uses the std pin's `os.read_dir` and the
  transaction module's public reserved names (`LOCK_LEAF`, `CLAIMS_LEAF`,
  `STAGING_TAG`); the candidate used a `DirectoryCursor` and
  `coordinator_entry` that only exist in a later std the pin does not carry.
  Behavior is the same: unrelated names refuse the directory as foreign.
- **`fp_name` in the configuration fingerprint** (spelling, not interner id),
  including dev's `config.name` which the candidate predates.
- **Configuration identity, not configuration bytes**, in the snapshot: the
  environment verdict above.
- **`Q_LINK_CONFIG` keys `include_referenced`** (in-process fix the candidate
  carried; independent of the persistent cache, separate commit).
- **Not ported:** the candidate's std pin bump and the `mach fmt` era
  `replace_source` helper; the CHANGELOG entry is rewritten for the opt-in.

## Controls and measurement

Compiler: this branch at `486c92f1` built by the 4.30.0 seed, `release`
profile (`out/release/mach`, 17.3 MB). Projects at profile `o2`,
x86_64-linux: `hello` (one file plus std, 41 modules) and the corpus case
`bits/logic_u32` in the hosted corpus project (43 modules). Host load average
6 to 14 from concurrent workers throughout; spreads are stated. `hits` is the
count of `-vv` `cached object` lines, the reuse signal; every sequence starts
with one untimed build that warms the OS file cache, and "cold" means the
output directory including `.mach-cache` was removed before the build.

**Byte identity.** The corpus case built uncached, cached-cold (publishing)
and cached-warm (43 of 43 hits): the 45 objects and binaries are identical
across the three (`diff -r`).

**Inert without `--cache`.** `strace -e openat` on an uncached hello build:
zero opens of `/proc/self/exe` and zero of `.mach-cache`; with `--cache`, one
open of `/proc/self/exe` (the memo) and 82 of `.mach-cache`. The no-flag build
is dev's build.

**Cross-environment hit.** hello published from an interactive shell, then
rebuilt under `env -i PATH=... HOME=... SHLVL=9`: 41 of 41 hits.

**Cold and warm, three repetitions, wall in ms (codegen phase in brackets).**

| project | off, cold | off, warm | on, cold | on, warm | hits |
|---|---|---|---|---|---|
| hello | 518 / 518 / 535 [99, 100, 101] | 511 / 520 / 514 [98, 106, 97] | 688 / 699 / 673 [256, 257, 255] | 602 / 597 / 604 [171, 178, 170] | 41 / 41 |
| corpus | 525 / 528 / 524 [95, 104, 99] | 534 / 525 / 519 [97, 100, 95] | 683 / 685 / 717 [261, 257, 269] | 609 / 614 / 597 [175, 174, 173] | 43 / 43 |

With 16 codegen workers a warm hit is **not faster**: about 80 ms slower than
an uncached build. The codegen phase of an uncached build is 95 to 106 ms
because 41 to 43 modules generate in parallel; the hit path replaces it with
the executable digest (about 139 ms, measured at 186 ms under `strace`) plus a
serial restore of every entry (read, payload hash, decode, re-intern: about 35
ms for 43 entries). Publication costs about 25 to 35 ms on top of the digest.
With `--jobs 1` (serial codegen, hello, warm): uncached 218 / 219 / 225 ms
against cached 193 / 189 ms after the first process at that spelling (the
first missed because the earlier entries were published under the absolute
project path), so the restore path does beat serial codegen by about 30 ms and
the digest eats the gain.

Where the remaining per-process cost is, warm corpus build at 16 workers,
about 600 ms wall: load 34, resolve 19, sema 47, lower 75, **optimize 173**,
codegen 173 (of which the digest about 139 and the restore about 35), emit 12,
link 15. Two items dominate and neither is the object compile:

1. the executable digest, addressed by the build id (phase 2, above);
2. the optimize phase, which runs on every module before the cache decision.
   The object key does not depend on the optimized IR, so phase 2 should ask
   the store before lowering and optimizing a module whose object it can
   restore; on this project that is the whole 173 ms, and with the digest gone
   a warm build would be roughly load + resolve + sema + restore + link, near
   200 ms against 520 uncached.

**Mutation controls** (each removes one keyed input or guard from the
implementation; the named test must fail; all 17 failed as required):
source text, embedded digest, step chain, dependency content, compiler
identity, configuration bytes, request digest, placement policy, link provider
(`driver.cache:snapshot_tracks_...`); restore never hits
(`driver.cache:repeated_project_operations_...`); `include_referenced` in the
link configuration (`engine.link_config:...`); native section metadata in the
codec (`cache.image:roundtrip_...`); `fp_name` by id (`query:fp_name_...`);
payload hash check in the store (`cache.store:replace_digest_...`); the digest
memo (`cache.compiler:second_digest_...`); the `--no-cache` stamp guard
(`driver:build_steps_repeat_no_cache_...`); the planner environment in the
identity (`passes.capture_build_identity:...`).

**Suite, census, corpus.** Full suite through the from-source compiler at the
final head: see the pull request for the exact count against the dev baseline
of 2732; `sh test/census.sh` 9 of 9 ok; corpus layer B on x86_64-linux 91
pass, 0 fail, 0 skip.

# Phase 2a: the linker-emitted build id

Status: replaces the per-process executable digest (identity candidate 2 above)
with candidate 1. The digest was the largest single warm-build cost measured in
phase 1 (139 ms release / 275 ms debug per process, paid before any entry can be
read). After this phase the compiler's identity is a note the linker wrote into
the image and the running compiler reads back from its own mapped headers; no
file is opened and nothing is hashed at run time.

## What the build id hashes

Every image writer computes the id after its file plan is sealed and every other
byte of the image has been written, so it is a function of the bytes the file
will contain and of nothing else: no time, path, name, environment or random
source enters it, and byte-identical inputs yield byte-identical notes. The hash
is SHA-256 over the bytes the loader maps, taken with the note payload as zero:

- **ELF**: every `PT_LOAD` file extent in program-header order, with the ELF
  header's `e_shoff`, `e_shnum` and `e_shstrndx` taken as zero. The section
  header table, `.symtab`, `.strtab`, `.shstrtab`, the debug sections and the
  build attributes are not loaded and are not hashed. The id is therefore the
  identity of the loaded program, the same whether or not debug information
  accompanies it, exactly as a stripped binary keeps the id of its unstripped
  twin. This is what the release-additivity constraint below requires.
- **Mach-O**: the file from offset zero to the code signature, which is the
  range the ad-hoc signature covers (header, load commands, every segment,
  `__DWARF`, and the `__LINKEDIT` structures before the signature), with the 16
  `LC_UUID` bytes as zero. The signature is written after the id, so it covers
  the final UUID (ld64's order). `-g` adds a `__DWARF` segment command to the
  header, so the loaded program already differs and there is no additivity to
  keep on this format.
- **PE**: the whole file with the 16-byte CodeView GUID as zero. `-g` adds
  section headers, so the same applies as on Mach-O.

The 32-byte digest is carried whole on ELF. Mach-O and PE carry a 16-byte
identifier by format, so they carry the first 16 bytes with the RFC 4122
version and variant bits forced (version 4, as ld64 and lld do), which is a
deterministic function of the content and still 122 bits of it.

## Where each format carries it

- **ELF**: a `.note.gnu.build-id` note (`namesz` 4, `descsz` 32, `type`
  `NT_GNU_BUILD_ID` 3, name `GNU\0`, 32-byte descriptor, 52 bytes in all) placed
  in the header page directly after the program headers, so it lies inside the
  first `PT_LOAD` (the header mapping) and is in memory at run time. A `PT_NOTE`
  program header names it, and when the image has a section header table an
  `SHT_NOTE` section header with `SHF_ALLOC` names it too, so `readelf -n`,
  `file`, `llvm-readobj --notes`, debuginfod and `dl_iterate_phdr` consumers all
  find it. All four ELF image writers emit it (static executable, static PIE,
  dynamic executable, shared object). A freestanding image whose headers are
  not mapped still carries the note and `PT_NOTE` in the file, with a zero
  address, since there is no mapping to name.
- **Mach-O**: `LC_UUID`, in every executable the writer produces (the static
  `LC_UNIXTHREAD` image, the dynamic image and the PIE image; it was previously
  written only under `--pie` and derived from the artifact name, which is not
  an identity). `LC_UUID` is the Mach-O build id: dyld, crash reports, dSYM
  matching and `dwarfdump --uuid` read it, and it sits in the header inside
  `__TEXT`, which `_dyld_get_image_header(0)` hands back mapped.
- **PE**: a `.buildid` section (read-only initialized data, the MinGW
  convention for `ld --build-id`) holding one `IMAGE_DEBUG_DIRECTORY` entry of
  type `IMAGE_DEBUG_TYPE_CODEVIEW` followed by the `RSDS` record (GUID, age 1,
  empty path), named by the optional header's debug data directory. A dedicated
  section with no directory entry would be invisible to every Windows consumer;
  the CodeView record is what WinDbg, symbol servers, `dumpbin /headers` and
  `llvm-readobj --coff-debug-directory` already treat as the image identity.
  The section is mapped, so the running compiler reads it through the image
  base and the RVA.

The byte layouts are defined once, in `src/lang/target/of/buildid.mach`, and
both the writers and the run-time reader use that definition.

## How the running compiler reads it back

`cache.compiler.identity` reads the note from the process's own mapped image,
without opening any file:

- **linux**: the auxiliary vector follows the environment block the runtime
  captured (`os.environ()`); `AT_PHDR`, `AT_PHNUM` and `AT_PHENT` locate the
  program header table in memory. The load bias is `AT_PHDR` minus the
  `PT_PHDR` address (zero when there is no `PT_PHDR`, an `ET_EXEC`). `PT_NOTE`
  plus the bias is the note; it is accepted only if one `PT_LOAD` maps its whole
  extent and its header reads `4 / 32 / 3 / GNU\0`.
- **darwin**: `_dyld_get_image_header(0)` is the main executable's header; the
  load commands are walked for `LC_UUID`.
- **windows**: `GetModuleHandleW(nil)` is the image base; `e_lfanew`, the PE
  signature, the PE32+ optional header and its debug data directory lead to the
  `IMAGE_DEBUG_DIRECTORY` entries, and the first CodeView entry's `RSDS` record
  holds the GUID.

Every reader is bounds-checked against the mapped image and answers "absent"
rather than failing when any piece is missing or malformed.

## Fallback and the seed-lag chain

The identity is the note when the image carries one and otherwise the phase-1
memoized full digest. It carries a source tag (`BUILD_ID` or `DIGEST`) and its
length, and the snapshot keys the tag and the length-framed bytes, so a
digest-identified compiler and a note-identified compiler cannot share a key
even in principle.

The seed lag rule holds: a compiler linked by a seed that predates this phase
carries no note and identifies itself by digest exactly as before; the first
compiler it links carries a note. The fixpoint is undisturbed because the note
is a deterministic function of the loaded bytes: B (linked by A) and C (linked
by B) are byte-identical as before, and identical bytes carry identical notes.

## Release additivity

The suite's guard (`test/link/cases/debuginfo` `g_additive`, and
`elf.emit_shared:debug_additive_nonloaded`) compares the `PT_LOAD` file extents
of the `-g` and non-`-g` images after zeroing `e_shoff`, `e_shnum` and
`e_shstrndx`. The ELF build id is a function of exactly that comparison input,
so wherever the two images were identical they still are: the note lies inside
the loaded image and is equal in both. Byte-identical inputs yield
byte-identical notes because nothing outside the image bytes enters the hash.

## What the note cannot do

The note is a link-time content identity, not a run-time integrity check. An
image edited after linking, outside the note, still reports the note it was
linked with, so such an image is served entries the unedited compiler
published. The phase-1 digest caught this incidentally, and this design cannot
by construction: verifying the note against the file would mean hashing the
file, which is the cost being removed. The cache's question is "which compiler
produced this entry", and the linker answers it; a post-link edit is outside
that contract. The digest remains the identity of any image without a note.
