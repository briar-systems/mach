# The 4.30.0 / 5.0.0 release shape

The changes that reshaped the compiler in 2026 ship as two releases with a
fixed division of labour: **4.30.0** is the compatible transition and
**5.0.0** carries every removal. This page records why it has to be two, and
why the split falls where it does.

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

- **4.30.0** adds every new form beside the old one and removes nothing. The
  4.26.x seed builds it, because nothing in its source or manifest is a form
  the seed rejects. Where a 4.30.0 feature would help the compiler's own
  source, the source waits: the compiler is written against the seed's
  dialect, not its own.
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
  spellings ([secrecy.md](../language/secrecy.md#downgrade-with-t));
- identity-keyed dependency tables `[dep.<id>]` and the flat root-owned
  closure, beside alias keys and nested realization
  ([dependency-model.md](dependency-model.md));
- `default = true` on targets, profiles, and artifacts, beside the
  first-declared fallback (which warns);
- artifact requirements (`need` naming artifacts and globs), the
  `timeout_seconds` surface on `mach test`, `mach run`, and build steps,
  the `env` target key, and the other additions in the changelog.

Removed, in 5.0.0:

- the `:^` and `:^T` spellings;
- alias dependency keys, nested realization, and `mach.lock`;
- the first-declared target, profile, and artifact fallbacks;
- the `sysv` ABI alias and the withdrawn `mos6502` target;
- an `#[embed]` path that escapes the project root (a warning in 4.30.0);
- plus the SemVer proposal on `mach dep update`, which is additive but
  belongs with the finished dependency model.

The rule for which side a change lands on is mechanical: if a manifest or a
program that 4.26.x accepts stops working, it is 5.0.0. Nothing in 4.30.0
breaks an existing project.

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
what and how; `doc/design/` states why; the changelog states when.
Documentation never changes generated code, and that is checked: objects
built with `debug = false` are byte-identical across a documentation change.
The supported surface is the editor API, the command line, and the manifest
schema, source-stable within a major; everything else under `src/` is
internal and carries no promise. Deprecation notices and experimental
marking as compiler machinery were deferred as low-priority additive work;
5.0.0 removes drift directly.
