# The dependency model

This page records why the dependency model is shaped the way it is. What the
model *is* — the manifest keys, the commands, what a build verifies — is in
[manifest.md](../manifest.md#depid) and [cli.md](../cli.md#mach-dep). The
model was settled in a design review in September 2026 that replaced two
earlier ones, and every rule below is a consequence of five statements:

> Pins are gitlinks. The root owns its flat transitive closure. The manifest
> key, the directory under `dep/`, and the module namespace are one name, the
> dependency's project id. The build validates and never resolves. Selection
> is an `update`-time policy.

## Pins are gitlinks, and there is no lock file

The previous model kept dependency state twice: a tracked `mach.lock`
recording each dependency's resolved commit, and an ignored `dep/` holding
the checkouts. Two records of one fact drift, and the drift was the recurring
defect: a checkout that no longer matched the lock, a lock edited by hand, a
lock left behind by an older tool and still read as an authority. The
repairs were all of the shape "detect the drift and re-resolve", which is
exactly the work a build must never do.

Git already has a first-class record of "this directory is at this commit":
the gitlink a superproject commits for a submodule. Making the gitlink the
pin removes the second record entirely. A dependency's commit is whatever the
root repository committed for `dep/<id>`; `.gitmodules` is generated from the
manifest, never read for pins; and nothing generated is an authority. The
cost is that a project's source distribution is a git clone — a tree without
`.git` does not build, and verification never degrades in its absence. That
cost was accepted deliberately: a tarball release that carried a generated
pin record would reintroduce the second record, so it is a separate ruling if
it is ever wanted.

## The root owns a flat closure

A package-owned, nested model (each dependency realizing its own `dep/`)
looks compositional and is not. The same identity reached along two paths
lands in two directories, so one program links two copies of one library;
initialization recurses to unbounded depth; and a fix in a deep dependency
has to be re-pinned at every level between it and the root.

The flat model puts every identity in the transitive closure directly under
the root's `dep/`, one gitlink each, exactly one level deep. A consumed
dependency's own `dep/` is never initialized: its gitlinks are still readable
(they are its tested floors, below) but they are inputs to the root's
closure computation, not checkouts. A package cloned on its own is a root and
gets its own flat `dep/`. The root manifest declares only direct dependencies
and overrides; everything transitive reaches `dep/` through closure
computation and never needs a declaration in the consumer.

## One name

An earlier form let the manifest key (an alias), the directory, and the
project id differ: `[dep.mach-std]` at `dep/mach-std/` exposing `std.*`.
Three spellings of one thing is three places for a mismatch, and a reader of
`use std.x;` could not find `std` in the manifest by its name. The model
collapses them: `[dep.<id>]`, `dep/<id>/`, `<id>.*` in source, and the build
checks that `dep/<id>/mach.toml` declares that id. Aliases are gone rather
than tolerated because an alias changes spelling without changing identity,
and identity is the thing every rule below keys on. 4.30.0 accepts an alias
key with a note naming the rename; 5.0.0 rejects it.

## One commit per identity, and the consumer resolves a clash

Identity is the project id — not the alias, not the URL. Source never
carries a version: `use std.x` is the only spelling. From that follows the
rule that shaped the most discussion: one identity resolves to exactly one
commit per build, with no exception for majors.

The alternatives were all considered and rejected: two majors of one
identity coexisting in one build, per-consumer import resolution,
`dep/<id>@<major>` directories, version-suffixed identities (`std2`, `/v2`).
Each of them makes "which `std` is this" a question with more than one
answer inside one program, and each of them turns a linker into an arbiter
of which copy a symbol came from. A clash is instead reported as a clash: the
diagnostic prints both requiring chains and the exact root declaration that
resolves it. The root resolves by declaring the identity with a `ref`, which
may point at upstream or at a fork carrying the same id.

That last clause is why identity is not the URL. Responsibility for an
upgrade sits with the consumer: when an upstream is unresponsive, the
consumer forks it, and the fork slots in without any change to consumer
source or to the manifests of the packages that required it, because they
required the id, not the URL. URL disagreement between requirers is
therefore a mirror, not a conflict: the root's declared URL wins, else the
first declaring path's, and verification compares commits, never URLs. Two
unrelated packages claiming one id is the one case that is a collision, and
it is rejected.

## The build validates and never resolves

A build never fetches and never writes under `dep/`. It checks, offline,
that every `dep/<id>` is a clean checkout at its committed gitlink (or
tracked path content), that its project id equals its directory name, that
the closure computed from the realized manifests equals the set of
directories (nothing missing, nothing extra), that every requirer's exact
selector for an identity the root does not declare is satisfied by the
realized commit, and that there are no cycles. A `branch/` selector is never
a verify fact: it is what `update` moves. The verifier reads the git index
rather than `HEAD`, so a freshly realized dependency is verifiable before
the commit that records it; that ruling was made during execution when the
alternative forced a commit before the first build of every new project.

Realization is likewise git's, not the build's: `mach dep pull` checks each
committed gitlink out at its commit, initializing in place the empty
directory a plain clone leaves for a submodule, and no git command ever runs
against a gitlink that is not yet a checkout of its own. A consumed
dependency's gitlinks show up as empty directories under its own `dep/`; they
are read as its tested floors and are otherwise ignored.

A project root is identified by its own `mach.toml`, not by an enclosing
repository, and `dep/<id>` resolves relative to that root. This was not
obvious until the link-test cases, which are projects nested inside the
compiler's own repository, failed by the hundred on the first cut: a
verifier that anchored at the repository root looked for the wrong `dep/`.

## Selection is an `update`-time policy

Resolution is deliberately not part of a build, so it has to live somewhere:
it lives in `mach dep update`, the one command that moves a pin. For an
identity reached by more than one path the manual rule is: the root's
selector wins if declared; otherwise agreement among the requirers is
taken; otherwise stop, print both chains, and name the root declaration that
would decide. A consumed dependency's own gitlink for an identity is its
tested floor — the commit that package was tested against — and it is
readable without initializing that package's `dep/`.

5.0.0 adds a SemVer proposal on top of the manual rule, not in place of it:
among tagged releases at or above every tested floor and within one major,
propose the highest; candidates spanning two majors are a clash under the
one-commit rule. The proposal never bypasses a root override. It is a
separate commit after the manual rule because the manual rule is its
fallback, and the fallback has to work first.

Selectors are `branch/<name>`, `tag/<name>`, and `commit/<full-object-id>`,
and nothing else: a bare name that git would guess at is exactly the
ambiguity a pin exists to remove.

## Sources, scopes, and what was left out

Exactly one of `git` and `path`. A `path` dependency is copied into
`dep/<id>` as tracked files, without its own `dep/`, contained (no escaping
symlinks), keeping its `git` source in its manifest so `update` can swap it
back to a submodule. Symlinked path dependencies were dropped with the lock
file: a symlink is a second record of where the files are.

Dependencies declare sources only. Artifacts, tests, and steps declare which
ids they consume; there is no scope vocabulary (no "dev" or "test"
dependencies), because a scope is a property of the consumer, and the
consumer already says what it consumes. Public entry is the module a
dependency's artifacts share. In 4.30.0, a dependency with no artifacts still
gets the legacy `lib.mach` entry under its source directory. 5.0.0 removes
that fallback and requires an explicitly defaulted library artifact.

Three things fit the five statements without changing them and are named as
future slots rather than built: owner-prefixed (dotted) ids if the flat id
namespace ever collides in practice; workspaces (several packages in one
repository sharing one `dep/`); and cross-package artifact requirements.

## Release shape

4.30.0 accepts the older forms (alias keys, nested realization, a stray
`mach.lock`) beside the new ones, each with a migration note; 5.0.0 rejects
them. The reason is the seed compiler that builds mach itself — see
[release-shape.md](release-shape.md).
