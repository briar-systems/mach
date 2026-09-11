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
| transitive source | every loaded module's path and full text plus the dependency edges and emission order. std modules are loaded modules, so they are here too. | keyed, coarse: any edit misses the whole cell; the absolute path is in the key, so a relocated checkout misses too |
| resolved generic or inline bodies | covered by the whole-cell source hash, not by an instance graph | keyed, by construction |
| std and dependency content | dependency manifest identity (alias, id, source, ref, content, version, direct aliases) plus the dependency modules' text through the class above | keyed |
| target / ABI | request semantic hash (target, profile, artifact, goal, opt, debug, pie, simd, vectorize, float_reassoc, include_deps), `target.fingerprint`, os/isa/abi/platform names, project id, version and binary name, `[define]` names | keyed. The candidate hashes names by interner id (`fp_u32(id)`); ids are process-local, so the same names can produce different bytes across processes. It replaced them with `fp_name` (spelling). Ported. |
| environment | the whole inherited process environment, sorted, whenever the project has a dependency or a step (dev's `write_step_environment` for #3146) | keyed, over-broad: a build from another shell misses even though objects only depend on the environment through steps, which the step chain already keys. Phase 2 narrows the snapshot to the identity part of the configuration. |
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
- **`Q_LINK_CONFIG` keys `include_referenced`** (in-process fix the candidate
  carried; independent of the persistent cache, separate commit).
- **Not ported:** the candidate's std pin bump and the `mach fmt` era
  `replace_source` helper; the CHANGELOG entry is rewritten for the opt-in.

## Controls

Recorded with the measurement in the follow-up commit on this branch: a cached
and an uncached build of one corpus case produce byte-identical objects and
binaries; each class marked keyed above has a change-one-input test in
`src/lang/driver/cache.mach` (snapshot axes) or `src/lang/driver/tests.mach`
(end to end: second process hits, edited source misses); and the codec, store
and identity units carry their own fault-injection tests.
