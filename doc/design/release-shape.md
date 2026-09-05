# The 4.30.0 / 5.0.0 release shape

The changes that reshaped the compiler in 2026 ship as two releases with a
fixed division of labour: **4.30.0** supplies the transition forms and seed,
and **5.0.0** removes the legacy forms retained for migration. This page
records why it has to be two, and why the split falls where it does.

## The seed forces it

Mach is self-hosting: the compiler's own source is compiled by an installed
**seed** compiler, and the result compiles itself again. Every release is
therefore built, first, by the release before it. A language or manifest
change the seed does not understand cannot appear in the compiler's own
source until a compiler that understands it *is* the seed.

That is the whole constraint. A release that both added the new forms and
removed the old ones could not be built by the seed that only knows the old
ones, because its own manifest and source would already have to use the new
forms. So:

- **4.30.0** adds the replacement language and manifest forms while retaining
  the specific legacy forms listed below. The 4.26.5 seed builds it, because
  nothing in its source or manifest is a form the seed rejects. Where a
  4.30.0 feature would help the compiler's own source, the source waits. The
  compiler is written against the seed's dialect.
- 4.30.0 is then published as the seed.
- **5.0.0** removes the old forms, with a migration diagnostic naming the new
  one. The 4.30.0 seed builds it, because 5.0.0 source uses only forms
  4.30.0 accepts.

The two-generation invariant follows. Until the seed catches up, the
seed-built compiler (call it A) may legitimately differ from the compiler A
builds (B), because A carries the older code generator. The fixpoint that is
checked is that B and the compiler B builds (C) are byte-identical. Release
verification runs three generations for that reason.

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
  ([#3128](https://github.com/briar-systems/mach/issues/3128)).
- the implicit `lib.mach` entry for a dependency that declares no artifacts.

5.0.0 also adds the SemVer proposal on `mach dep update`. It belongs with the
finished dependency model, though it is an addition rather than a removal.

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

## Why one squashed commit

The branch that carried this work was reviewed against a specification kept
beside the code, and that specification, its conformance ledger, and its
execution record were working documents: normative while the work was in
flight, and wrong the moment the code moved past them. They do not ship.
The branch lands on `dev` as a single squashed commit whose content is
4.30.0, with the working documents absent from that commit and their history
preserved nowhere, by ruling. What is still open at the landing is filed as
issues carrying the relevant clause text, so nothing depends on the
documents surviving. The residual design decisions that are worth keeping
are these pages under `doc/design/`.

## Documentation ships with the code

No release is cut from a tree whose supported public surface lacks
docstrings or whose `doc/` pages contradict the compiler. Docstrings state
what and how, `doc/design/` states why, and the changelog states when.
Documentation never changes generated code, and that is checked: objects
built with `debug = false` are byte-identical across a documentation change.
The supported surface is the editor API, the command line, and the manifest
schema. Their intended stability within a major has the explicit transition
exceptions described above. Everything else under `src/` is internal and
carries no promise. General deprecation attributes and experimental marking
as compiler machinery were deferred as low-priority additive work.
The targeted migration notes and warnings described above ship in 4.30.0.
