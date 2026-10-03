# Dependencies

How a project's dependencies are resolved, pinned and verified. A dependency is
declared by a `[dep.<id>]` table, whose keys are described in
[manifest.md](manifest.md#depid).

## Releases and resolution

A **release** of a git dependency is a tag `vX.Y.Z` (optionally
`vX.Y.Z-pre`) together with the `mach.toml` at that tag. A tag whose manifest
does not load, as an old tag's written for an earlier manifest schema, or whose
`[project].version` differs from the tag name is not a candidate. Resolution
sets it aside and keeps looking. When nothing fits, the error lists it among the
requirements (`gl 0.1.0 is not a candidate: its mach.toml does not load: unknown
key 'name' in [project]`, or `... its [project].version does not match the
tag`), so the error never reads as one in the project's own `mach.toml`.

Resolution runs in exactly three places: `mach dep add`, `mach dep update` and
`mach dep outdated`. **Builds never resolve and never verify.** They read
what `dep/` holds (see [What a build reads](#what-a-build-reads)), and `mach
dep verify` checks it (see [What `mach dep verify`
checks](#what-mach-dep-verify-checks)). For every identity in the
closure that some manifest selects by `version`, resolution picks one release
such that:

1. every requirer's range contains it;
2. its own `[project].mach` contains the running compiler;
3. the closure its own manifest implies also resolves.

A requirer is the root, a release resolution chose, or a dependency the root
reaches by `ref` or `path`. The last declares its range in the closure directly,
so two such dependencies naming one identity by range are two requirements of
the same problem, and the error names each by its chain (`root -> c requires b
<1.2`).

Among the choices that satisfy all three, it takes the highest release of each
identity. The result is written as gitlinks, like any other pin; there is
still no lock file. `mach dep update <path> <name>` keeps every other
identity at its pinned release (the release its recorded gitlink carries) while
that release still fits, so an update moves as little as it can. `--all`
resolves from scratch. A pin the range no longer admits, as after the range is
raised past it, is re-pinned to the release resolution picks; `update` never
keeps it. The recorded gitlink is the pin, not whatever the checkout holds: a
checkout that drifted from its gitlink is moved to the chosen release and the
gitlink staged in the same run, even when the drifted checkout already sits at
that release.

When nothing fits, the error lists every requirement that took part and names
the identity the root can settle:

```
error[dep.unsatisfiable]: no set of releases satisfies every requirement:
    root requires b ^1.2
    a 1.0.0 requires b ^2.0
  the root decides by declaring the identity itself, for example:
    [dep.b]
    version = "<a range the root can use>"
```

A release that needs a newer compiler appears as one of those lines (`b 2.0.0
requires mach ^6, and this is mach 5.2.1`). Resolution never silently settles
for a lower release than the ranges allow.

What `mach dep add` writes:

- with `--git <url>` alone, `version = "^X.Y.Z"`, where `X.Y.Z` is the
  release resolution picked. The lower bound is the release actually tested
  when the dependency was added, and the caret follows the pre-1.0 rule;
- with `--range <range>`, that range;
- with `--ref <selector>`, that selector, as before.

`mach init` adds std the same way, so a new project names the std release that
works with the compiler that created it. std is an ordinary dependency, with
no std-specific command. `mach init --no-deps` still resolves and writes the
range and skips only the checkout, so it needs the network too. Offline it
fails and writes no `[dep.std]` table. A tool that needs the std for a given
compiler runs `mach init` and `mach dep pull` in a scratch project and takes
what resolution chose.

**`--offline`.** `add`, `update` and `outdated` read candidates from each
dependency's repository: one `git ls-remote --tags` per URL, and the manifest
at a release through a shallow fetch of its tag. With `--offline` they use only
the tags already present in the realized checkouts, and they say so
(`resolving from releases already fetched (--offline)`). A resolution that
needs a candidate it doesn't have fails, naming the identity. Realizing a
checkout fetches nothing either. A missing checkout is initialized from the
submodule store Git kept for it, and a pin that no local checkout or store
holds fails, naming the dependency and the commit. A dependency added over a
retained checkout or store is checked out at its selector as held there, and
one with neither is not cloned. A selector nothing local holds fails, naming
the dependency and the selector.

**`--lowest`.** `mach dep update <path> --all --lowest` picks the lowest
release every range accepts. A library's CI runs it in a scratch checkout and
then builds and tests, which proves the lower bounds it declares are honest.
Without that check, `^3.2.0` can quietly depend on something only 3.4 has. It
belongs in a release or manually dispatched job, never a scheduled one.

**`mach dep outdated <path>`** prints, for each version-selected identity, the
pinned release, the highest release resolution would pick now, and the highest
release published. A newer release held back by a range or by the compiler is
marked as such.

**No yanking.** A bad release that is otherwise compatible is fixed forward
with a new release. Nothing marks a published version as withdrawn. A
consumer that must avoid one raises its range's lower bound (`^3.2.1`).

**Forks.** Identity is the project id, not the URL. A root that declares a
fork's URL makes that fork the candidate source, so its `vX.Y.Z` tags compete
under the same ranges. A fork that wants to stay distinguishable tags
pre-releases (`v1.4.3-fork.1`), and a consumer opts in by naming the
pre-release in its range.

## Root declarations: narrowing and overriding

A root `version` for an identity **narrows**: it is intersected with every
requirer's range, and resolution and verification hold the pin to all of them.
A root `ref` or `path` **overrides**: the requirers' ranges and selectors for
that identity no longer apply. A range is therefore never widened silently, and
the escape hatch is one visible line in the root manifest.

An override is always reported. `mach dep pull`, `mach dep update` and `mach
dep add` print one note on stderr for each requirement a root declaration
replaced, whatever `--quiet` says, naming the identity, the root's winning
selection, the requirer chain and what that chain asked for:

```
note: dependency 'std': the root declares ref = "tag/v2.0.0", overriding
hedgeacme -> hedge -> std which requires ref = "tag/v2.1.0"; nothing checks that
'std' supports the root's selection
```

A requirement that asks for exactly the root's selection is no override and is
not noted. `mach dep list` shows each root declaration's winning selection
(`ref=`, `version=` or `path=`, and the recorded `pin=`), its state
(`realized`, `missing`, or, for a `version` selection whose range excludes the
pinned release, `out of range (the pinned release 7.0.2 is outside ^8.0)`) and,
under it, every requirement it overrides (`overrides hedgeacme -> hedge -> std, which requires
ref = "tag/v2.1.0"`). `mach dep outdated` names the requirements of a chosen
release that a root or a fixed dependency overrides (`root declares b by ref
"branch/main", overriding root -> a 1.0.0 requires b ^1.2`).

An override is not checked against the requirers. Because the closure is flat,
a requirer's `use b.*` binds to whatever the root selected, even a major that
requirer was never built or tested against. Nothing proves the requirer supports
it: a passing build only shows that the code the build reached compiled, so it is
evidence and not a guarantee. `mach dep verify` prints a note for every edge an
override replaced, without failing (`note: dependency 'b': the root declares ref =
"tag/v2.0.0", overriding root -> a -> b which requires version = "^1.2"; nothing
checks that 'b' supports the root's selection`). Treat each note as a claim to
confirm, by testing the requirer at that selection or by checking its own range.

## A release selects only releases

The rule follows how a manifest was reached, not where it sits:

- A dependency reached through a **release** (a `version` range or an exact
  `tag/`) may itself select dependencies only by `version` or `tag/`. A release
  is then reproducible from its tag, all the way down.
- A dependency reached through a `branch/` or `commit/` selection is in
  development, and its manifest may use any selector.

A release that breaks the rule is refused wherever it is reached. Resolution
stops when it reaches one, and `mach dep verify` refuses it,
naming the chain and the offending line:

```
error[dep.release_selector]: root -> a is a release (ref = "tag/v1.1.0"), and its manifest selects
[dep.b] by ref = "branch/main"; a release may select its dependencies only by
`version` or an exact `tag/`, so it cannot be reproduced from its tag
```

`mach dep verify <path> --release` holds the project itself to the same rule,
so a library's release workflow catches the mistake before it tags the
release, not when its first consumer resolves it.

## Pins are gitlinks; there is no lock file

The record of which commit a dependency is at is the **gitlink** committed in
the root repository, generated into `.gitmodules` by `mach dep`. Nothing else
records a pin: there is no `mach.lock`, and a file of that name in the project
root is an unrelated file no command reads.

A project does not need its own Git repository. In a repository root, Git
dependencies use the staged gitlinks as their pins. A subproject, a project in a
subdirectory of a repository, uses the gitlink the enclosing repository commits
under its prefix (`test/consumer/dep/std` for a subproject at `test/consumer`)
the same way: `pull` realizes that gitlink's commit and `update` moves it and
stages it. Without such a gitlink, and in a filesystem project, Git dependencies
are plain clones whose own checkout commits are verified. Local path dependencies
are verified from their filesystem realizations, independently of any Git index.

A version range is resolved for the whole closure, not for the root's own
declarations alone: a range a dependency declares, whether that dependency was
reached by a range, a `ref` or a `path`, is pinned under the root's `dep/` by
`mach dep add` and `mach dep update`. When the root has no checkout of the
identity yet, resolution starts from the declaring dependency's own committed
gitlink for it, so a dependency brings the pin it was tested with; `update
--all` moves every range to the highest release all of them admit; and a root
declaration of the same identity by `ref` or `path` overrides the range, noted
as above. `pull` refuses a range with
neither a gitlink nor a checkout under the root and names `mach dep update <root>
<id>`, which pins it wherever in the closure it is declared.

## The root owns the flat closure

The root's `dep/` holds every identity in its **transitive** closure, one
directory each, one level deep. The root manifest declares only what the root
uses directly (plus any override, below); a dependency's own dependencies reach
the root's `dep/` through closure computation and never need a declaration in
the consumer. A consumed dependency's own `dep/` is never initialized. A
package cloned on its own is a root and realizes its own flat `dep/`.

So with a root that declares `a`, and `a` that declares `b`, the layout is
`dep/a/` and `dep/b/`, and `a`'s `use b.*` resolves against the root's
`dep/b/`. Git materializes `a`'s own gitlink as an empty `dep/a/dep/b/`
directory; that entry is neither realized, verified, nor descended into.

## One identity, one commit

Identity is the project id, not the key and not the URL. One identity resolves
to exactly one commit per build, with no exception for majors: two majors of one
identity in one closure is a **clash**, not a case the build accommodates. The
diagnostic prints both requiring chains and the exact root declaration that
would resolve it:

```
error[dep.conflict]: dependency conflict: project id 'b' is reached with two different selections:
    root -> a -> b requires git <url> @ tag/v1.0.0
    root -> c -> b requires git <url> @ tag/v2.0.0
  the root decides by declaring the identity itself, for example:
    [dep.b]
    git = "<url>"
    ref = "tag/v2.0.0"
```

(`<url>` stands for the repository URL as declared.)

The root resolves by declaring the identity with a `ref`, which may point at
upstream or at a fork carrying the same id. A fork slots in without any
consumer source or manifest change, because identity is not the URL. Two
unrelated packages claiming one id is a collision and is rejected. URL
disagreement is a mirror, not a conflict: the root's declared URL wins, else the
first declaring path's; verification compares commits, never URLs. A realized
checkout whose remote points somewhere else (a `.git` suffix, another host, a
local mirror) verifies by its commit alone.

## What a build reads

A build locates each dependency as the directory its key names under `dep/`
and reads its manifest. That is the whole of its dealing with dependencies: it
never fetches, never writes under `dep/`, runs no Git process, and checks no
pin, selector, identity or checkout state. It builds what exists, so a stale
or drifted dependency surfaces as a build error, and `mach dep pull` brings it
back to its pin. A dependency with nothing checked out, whether `dep/<id>` is
absent or is the empty directory Git leaves for an uninitialized gitlink on a
fresh clone, is refused naming that command (`dependency 'std' is missing:
nothing is checked out at 'dep/std'; run `mach dep pull <root>``). A build
also refuses a cycle in the manifests it reads, and a compiler outside any
manifest's `[project].mach` (see [Compiler range](manifest.md#compiler-range)).

## What `mach dep verify` checks

`mach dep verify` locates the closure as a build does and checks, offline,
that:

1. every Git dependency is a clean checkout at its applicable pin
   (`dependency 'std': checkout is dirty:  M mach.toml`), and every path
   dependency is a contained filesystem tree without repository metadata;
2. its project id equals the directory name;
3. the closure computed from the realized manifests equals the set of
   directories under `dep/`: nothing missing, nothing extra, and no
   dependency's own `dep/` realized. A missing dependency is refused as a build
   refuses it;
4. there are no cycles (reported as the chain);
5. every realized manifest's `[project].mach` accepts the running compiler (see
   [Compiler range](manifest.md#compiler-range));
6. for every identity selected by `version`, the pinned commit carries a
   release tag, read from the checkout's own refs, and that release is inside
   every requirer's range, the root's included; and every release in the
   closure selects only releases (see [A release selects only
   releases](#a-release-selects-only-releases)).

A pin outside a range names the requirer chain, the range, the pinned release
and a runnable remedy (`dependency 'vb': root -> vb requires version '^1.2' but
the pinned release is 1.1.0; run `mach dep update <path> vb` for project
'<root>' to re-pin it, or declare the identity at the root to override`). A
pin no tag in the checkout names reads `... but no release tag in its checkout
names the pinned commit '<commit>'`, and names `mach dep pull` beside `mach dep
update`. A checkout that holds no tags at all, as a shallow clone (`git
submodule update --depth 1`) does, cannot say which release its pin is, and
the refusal says so rather than claim the pin is outside the range (`... but
the release of the pinned commit '<commit>' cannot be read: its checkout at
<dir> holds no tags (a shallow clone fetches none); run `mach dep pull <path>`
for project '<root>' to fetch them`).

The recorded gitlink is the pin, and two kinds of drift from it are refused,
each naming the identity and the command that fixes it:

- a `dep/<id>` checkout at another commit than its gitlink (`dependency 'b':
  the checkout is at '<commit>' but the recorded gitlink is '<other>'; run
  `mach dep pull <path>` for project '<root>' to restore the recorded pin, or
  `mach dep update <path> b` to re-pin it to the manifest's selection`);
- a gitlink outside the manifest's selection. The root's own `tag/` or
  `commit/` must be satisfied by the pin (`dependency 'b': exact ref
  'tag/v1.0.0' required by root -> b resolves to '<commit>' but the realized
  commit is '<other>'; run `mach dep update <path> b` for project '<root>' to
  re-pin it to the root's selection`; a root `commit/` that does not match
  reads `exact commit ref 'commit/<id>' is not satisfied by the realized commit
  '<other>'`), and so must every range the root declares (item 6). `pull`
  realizes the gitlink as it is, so it never cures this; `update` moves the
  gitlink to the selection.

A root `ref` or `path` is the override for its identity, so the requirers'
selectors are not checked against its pin (the override notes above name them).
For an identity the root does **not** declare, every requirer's exact selector
(`tag/`, resolved through the checkout's own refs, or `commit/`) must be
satisfied by the realized commit; a mismatch names both commits and the two
remedies (`dependency 'b': exact ref 'tag/v1.0.0' required by root -> a -> b
resolves to '<commit>' but the realized commit is '<other>'; run `mach dep
update <path> b` for project '<root>' to re-pin it, or declare the identity at
the root to override`). A `branch/` selector is an input to `update`, never a
verify fact. The verifier reads the git **index**, so a freshly realized
dependency is verifiable before it is committed.

`mach dep pull` reads what a Git dependency's `dep/<id>` holds together with
its record (the staged gitlink, its `.gitmodules` entry, and any module
directory Git retained) and takes the one step that brings it to what `mach
dep verify` checks:

- a staged gitlink with nothing checked out, as on a fresh clone or after the
  directory was deleted, is initialized in place (`realized std @ …
  (initialized the committed gitlink)`), first restoring its `.gitmodules`
  entry from the manifest if that entry is gone;
- a checkout at another commit than its gitlink is checked out at the gitlink;
- a clean checkout of its own with no gitlink is registered, moved to the
  declared selector from the declared source, and staged (`(registered the
  existing checkout)`);
- with neither, the submodule is added at the selector, reusing a module
  directory Git retained from an earlier removal.

Then, when the checkout lacks the tag its selection is read by (the tag a
`tag/` names, or a release tag naming a `version` selection's pin), pull
fetches its tags (`fetched the tags of std`). `update` and `outdated` fetch
them the same way to read a pin's release, unless `--offline`.

A symlink, a file, a directory that is not a checkout of its own, and a dirty
checkout that would be registered are refused and left as they are. `mach dep
add` takes the same step for its Git source, so re-adding a dependency whose
checkout `remove` retained registers that checkout, and it refuses a dirty one.
No gitlink command ever runs against a path that is not a checkout of its own.

A path dependency has no pin, so `mach dep pull` syncs its `dep/<id>` with the
declared `path` every time, and `mach dep update` does the same, once per
command. Unless `--quiet`, each says whether the copy was refreshed or reused (`realized hedge
from ../.. (copy refreshed from its source)`, or `(copy reused: it already
matched its source)`), so a copy left from another checkout cannot pass
unnoticed. A build reads the copy as it is and never syncs it. A changed
`path` realizes the new source. A file the source no longer has is removed and
named, and a file whose content differs from the source is overwritten and named
(`replaced 'src/lib.mach' with its source's content`), so local edits to the
copy do not survive a pull. A `dep/<id>` that is a symlink is refused and left
as it is.

A project root is identified by its own `mach.toml`, not by an enclosing git
repository; `dep/<id>` is resolved relative to the project root. A project
nested inside an unrelated repository or without any repository builds. Git
dependencies in these projects are verified from their own plain checkouts.

## Selection on `update`

`mach dep update` is the only command that moves a pin. It advances every
`branch/` selector to its current remote tip and re-stages the gitlink, and it
moves an exact selector to the commit it names, so an identity realized at a
dependency's selection lands on the root's declaration once the root declares
one (`b: 0564… -> e508… (pinned to the exact selector)`, or `(exact selector,
already pinned)` when nothing moves). `<name>` is looked up in the whole
dependency closure, so `mach dep update <path> b` for an identity the root does
not declare moves its checkout to the selector its requirers declare, and
`--all` moves every selector in the closure. Resolution reads a dependency
selected by `ref` at the commit its selector names, never from its checkout, so
a requirer that has just moved its selector is resolved against the manifest it
now selects. A
name outside the closure is refused (`dependency 'x' is not in the dependency
closure`). For an identity reached by
more than one path, one rule decides: the root's selector wins if the root
declares the identity; otherwise agreement among the requirers is taken;
otherwise the command stops, prints both chains, and names the root
declaration that would decide (the diagnostic above). A consumed
dependency's own gitlink records a tested commit, readable without initializing
that dependency's `dep/`. It is not a compatibility floor. Compatibility is
stated by ranges, and `update` resolves every version-selected identity as
described in [Releases and resolution](#releases-and-resolution).

## Removed forms

Two shapes of a realized closure are refused by `pull`, `verify` and every
build:

- a **key that is not the project id**, a `[dep.<key>]` whose realized project
  declares a different id:
  `[dep.foo] realizes project 'std': the manifest key, the directory under
  dep/, and the project id are one name, so rename the table to [dep.std] and
  the directory to dep/std`;
- a **nested realization**, a `dep/<id>/dep/<x>/mach.toml`: `dependency 'a':
  dep/a/dep/b is a nested realization: the root's dep/ owns the flat closure
  and a dependency's own dep/ is never realized, so delete dep/a/dep`. The
  empty directory git materializes for a consumed dependency's own gitlink is
  not a realization and passes.

Command-line usage (`pull`, `verify`, `add`, `update`, `remove`, `list`) is
documented by `mach help dep`.

A dependency's export surface — all a consumer sees — is its source module tree
(addressed by the dep's id), the module a bare `use <id>;` binds, its
`export = true` link entries and library artifact, and the steps and artifacts
those demand. Nothing else in a dependency's manifest applies to consumers.


## See also

- [manifest.md](manifest.md#depid) — the `[dep.<id>]` keys
- [build.md](build.md) — what a build selects and reads
