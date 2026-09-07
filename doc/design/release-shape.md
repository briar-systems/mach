# The 4.30.0 / 5.0.0 release shape

The release has two published checkpoints. **4.30.0** supplies the audited
transition compiler and bootstrap seed. **5.0.0** establishes the complete
language, compiler, tooling and standard-library contract, including removal
of superseded forms. The [release coordinator](https://github.com/briar-systems/mach/issues/3112)
owns the required work across both repositories.

## The bootstrap chain

Mach is self-hosting. A compiler must understand a new source form before the
compiler or its standard library can adopt that form. This requires an explicit
bootstrap chain, rather than assuming the latest published compiler can build
any later source directly.

The current reproducible chain starts with published **4.26.5**, builds the
pinned audited compiler at `7e26667e92e279b6ad2da53bf8e9a68ca42caa49` with std
`3ee8e709a8ed7baff6e93780ce9b3582a907a91f`, then builds the final 4.30.0 source
with its own std pin. The setup action records the seed and source identities.
Darwin bridge binaries are cross-built on Linux before native self-host checks.
Required historical source commits remain reachable until the published seed
replaces them.

At each self-host stage, the seed builds A, A builds B, and B builds C. B and C
must be byte-identical. A may differ because it was produced by the older
compiler. Final verification checks the exact versioned compiler source and std
pin in both optimization profiles on the required native hosts.

Published 4.30.0 becomes the starting seed for v5. The new tagged-value and
failure-control features must be implemented in a usable compiler before its
own source and std migrate. Their design issue specifies the pinned intermediate
bootstrap and migration order. Publishing 4.30.0 does not by itself make that
compiler understand new v5 syntax.

## What is on each side

Additive, in 4.30.0:

- the declassification cast `:>T`, beside the older `:^` and `:^T`
  spellings ([secrecy.md](../language/secrecy.md#downgrade-with-t)).
- identity-keyed dependency tables `[dep.<id>]` and the flat root-owned
  closure, with migration notes for alias keys and ignored nested realization
  ([dependency-model.md](dependency-model.md)).
- `default = true` on targets, profiles, and artifacts, beside the
  first-declared fallback (which warns).
- artifact requirements (`need` naming artifacts and globs), the
  `timeout_seconds` surface on `mach test`, `mach run`, and build steps,
  the `env` target key, and the other additions in the changelog.

Removed, in 5.0.0:

- the `:^` and `:^T` spellings.
- alias dependency keys, nested realization, and `mach.lock`.
- the first-declared target, profile, and artifact fallbacks.
- the `sysv` ABI alias and the withdrawn `mos6502` target.
- an `#[embed]` path that escapes the project root (a warning in 4.30.0).
- the five deprecated manifest keys: `[project] name`, `description`, and
  `mach`, plus `[profile.*] emit_ir` and `emit_asm`.
- `$project.name` and `$project.description`, with their manifest keys
  ([migration issue](https://github.com/briar-systems/mach/issues/3226)).
- the implicit `lib.mach` entry for a dependency that declares no artifacts.

V5 also includes checked `tag` values, canonical `res`/`opt`, expression-level
`try` with explicit failure exits, query ownership and persistent compilation
caching, the approved manifest and command changes, and the coordinated std
migration. The linked issues contain their acceptance criteria. Remaining syntax
and representation choices are design work, not declarations of implemented
features.

Dependency version selection remains an explicit decision in the coordinator.
A tested dependency commit is evidence of testing, not an automatic promise of
compatibility with every later release. The earlier SemVer proposal is not an
accepted implementation contract.

Compatibility in 4.30.0 means preserving the seed path and these specific
migration forms. It does not mean accepting every project 4.26.x accepted.
The audited changes already remove `-O1` and `--verify-ir`, reject ignored CLI
options, and withdraw MOS 6502 from ordinary build planning. Soundness and
validation fixes also reject previously accepted input: address-of requires
a place and refuses temporaries, arithmetic and non-shift bitwise binary
operators require operands of one type, and manifest-controlled paths must
stay within the project root. The [changelog](../../CHANGELOG.md) lists the
immediate changes and fixes.
The listed 5.0.0 removals remain deferred until the new seed is published.

## Landing and release gates

Work lands directly on `dev`, with one cohesive commit per fix or change.
Implementation issues close when their fixes have landed and their acceptance
evidence is complete. Verification retries do not create additional issue or PR
stacks. `main` takes release integration merges from `dev`.

Every issue in the v5 required set must finish before publication, including the
4.30.0 and audited std prerequisites. Consolidation transfers unfinished work to
its named owner. Additional RV32 dynamic output, embedded E-base machines and
specialized integer matmul are tracked in post-v5 milestones. Android expansion
and MOS 6502 are withdrawn.

Release artifacts and tags identify the validated source. Historical bootstrap
and proof references are preserved before obsolete branches are removed.

## Documentation ships with the code

Supported public contracts and worked examples must agree with the compiler.
Language references, command help, manifest documentation and ownership/error
contracts have explicit coverage under the documentation issue. Internal
comments explain non-obvious invariants without a docstring quota for every
internal public declaration.

General source deprecation diagnostics are required for v5. An experimental-mode
feature is not part of that requirement. The targeted migration notes and
warnings described above ship in 4.30.0.
