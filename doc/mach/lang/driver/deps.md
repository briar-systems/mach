# mach.lang.driver.deps

## fun set_git_config_env

```mach
pub fun set_git_config_env(env: **u8);
```

## rec RootOverride

```mach
pub rec RootOverride;
```

a requirer's selector that the root's own declaration replaced. a root `ref` or `path`
wins over every requirer with no check that the dependency supports it; a root
`version` intersects with the requirers' ranges instead, so it is never one

name: the dependency key both declarations use
chain: the requiring chain, ending at the dependency
requested: the requirer's selector, as `ref = "..."`, `version = "..."` or `path = "..."`
declared: the root's selector, in the same form

## fun root_overrides

```mach
pub fun root_overrides(s: *session.Session, root_m: *manifest.Manifest, deps: *project.DepEntry, dep_count: u32,
out: *Vector[RootOverride]) err[outcome.Fail];
```

every edge of a realized closure whose selector a root override replaced, in closure order

s: the session; the returned ids are interned in it
root_m: the root manifest
deps: the realized closure
dep_count: its length
out: receives one RootOverride per replaced edge

## fun root_override_text

```mach
pub fun root_override_text(s: *session.Session, o: *RootOverride) res[str, outcome.Fail];
```

the note `mach dep verify` prints for one override

## fun override_note

```mach
pub fun override_note(a: *A.Allocator, name: str, declared: str, chain: str, requested: str) res[str, outcome.Fail];
```

the note naming one requirement a root declaration overrode, as `mach dep pull`, `update`,
`add` and `verify` print it

a: allocates the note
name: the dependency identity
declared: the root's winning selector, as its manifest line spells it
chain: the requiring chain, ending at the dependency
requested: the selector the chain asked for, in the same form

## rec GitCandidates

```mach
pub rec GitCandidates;
```

## fun git_candidates_init

```mach
pub fun git_candidates_init(s: *session.Session, offline: bool, local: str) res[GitCandidates, outcome.Fail];
```

## fun git_candidates_dnit

```mach
pub fun git_candidates_dnit(c: *GitCandidates);
```

## fun git_candidates_add_local

```mach
pub fun git_candidates_add_local(c: *GitCandidates, url: str, dir: str) err[outcome.Fail];
```

the local repository offline resolution reads `url`'s releases from; the first existing one
named wins, so a url with no checkout yet is refused by offline_dir and not by git

## fun git_candidate_source

```mach
pub fun git_candidate_source(c: *GitCandidates) cand.CandidateSource;
```

## fun release_for_commit

```mach
pub fun release_for_commit(c: *GitCandidates, url: str, commit: str) res[str, outcome.Fail];
```

the release of `url` whose commit is `commit`, as its version text owned by the
caller, or an owned "" when no release tag names that commit: a dependency's committed
gitlink is a commit, and resolution deals in releases (#3689)

## fun release_needs

```mach
pub fun release_needs(ctx: ptr, id: str, url: str, rel: *cand.Release, out: *resolver.ReleaseNeeds) err[outcome.Fail];
```

a release's requirements as the resolver reads them: its compiler range and its dependencies
selected by version range, or by an exact release tag. a release selecting anything else
(a branch, a commit, a path) is not reproducible from its tag and is refused (#3496 §7),
unless the root declares that identity and so selects it itself (#3553)

## val MACH_KEY_SINCE

```mach
pub val MACH_KEY_SINCE: str = "5.3.0"
```

the first release that reads `[project].mach`; an earlier compiler refuses the key as unknown

## fun mach_range_floor

```mach
pub fun mach_range_floor(a: *A.Allocator, running: str) res[str, format.FormatError];
```

the compiler range a project is told to declare when `running` is the compiler: the oldest
release of its major that reads the key, since a caret cannot span majors. it depends only on
the running major, so two authors on one project write the same range (#3572). owned by `a`

## fun running_mach_range_floor

```mach
pub fun running_mach_range_floor(a: *A.Allocator) res[str, format.FormatError];
```

mach_range_floor for this compiler

## val DEP_ROOT_REFUSED

```mach
pub val DEP_ROOT_REFUSED: str = "dependency root 'dep' must be a physical directory, not a symlink or another kind of entry"
```

`dep` is the project's reserved dependency root: absent, or a physical directory. a
symlink there would resolve the closure from another tree, so every command that reads
or realizes the closure refuses it through this one check (#3478)

## fun check_dep_root

```mach
pub fun check_dep_root(project_root: str) res[bool, outcome.Fail];
```

whether the dependency root exists; err when it is anything but a physical directory

## fun resolve_deps

```mach
pub fun resolve_deps(p: *project.Project, m: *manifest.Manifest, project_root: str) err[outcome.Fail];
```

## fun scope_closure_to_dependency

```mach
pub fun scope_closure_to_dependency(p: *project.Project, owner: str) err[outcome.Fail];
```

make a resolved closure the closure of one of its dependencies: the dependency's
entry moves to `p.config.owner` and `p.config.deps` keeps only what it reaches
through its own edges, in closure order

p: a project whose `p.config.deps` holds the root's resolved closure
owner: the dependency's project id
ret: ok; err when the closure holds no such dependency

## fun resolve_requirement_scopes

```mach
pub fun resolve_requirement_scopes(p: *project.Project, root: *manifest.Manifest, profile: str) err[outcome.Fail];
```

the requirement scope of each project a configured cell compiles: the declaring
project's, holding the cell artifact's requirements, then one per closure
dependency, holding what its default library artifacts require

p: a configured project whose `art_reqs` and `deps` are resolved
root: the root manifest, whose out and profile the dependencies' outputs expand in
profile: the resolved profile name
ret: ok with `p.config.req_scopes` set; err from output expansion

## fun resolve_cascade_libs

```mach
pub fun resolve_cascade_libs(p: *project.Project, isa: str, os: str, abi: str,
own_libs: *manifest.LinkRequirement, own_count: u32) err[outcome.Fail];
```

## fun release_selects_releases

```mach
pub fun release_selects_releases(s: *session.Session, chain: str, what: str, m: *manifest.Manifest,
root: *manifest.Manifest) err[outcome.Fail];
```

a manifest reached through a release (a version range or a `tag/`) may itself select only
releases, so the release is reproducible from its tag all the way down (#3496 §7)

s: the session
chain: the requirer chain that reached `m`, or the root's name
what: how `m` was selected, as its manifest line spells it
m: the manifest to check
ret: err naming the first dependency selected by a branch, a commit or a path

## fun parse_toml_file

```mach
pub fun parse_toml_file(alloc: *A.Allocator, path: str) res[toml.Table, outcome.Fail];
```

## fun cell_tmpl_vars

```mach
pub fun cell_tmpl_vars(p: *project.Project) manifest.TmplVars;
```

## fun owner_tmpl_vars

```mach
pub fun owner_tmpl_vars(p: *project.Project, owner: str) manifest.TmplVars;
```

`cell_tmpl_vars` whose `{artifact.<id>.out}` resolves in the requirement scope
of the project `owner` names, empty when the cell binds no scope for it

## fun inside_work_tree

```mach
pub fun inside_work_tree(s: *session.Session, dir: str) res[bool, outcome.Fail];
```

whether `dir` lies inside a Git work tree; false for a plain directory

## fun initialize_repository

```mach
pub fun initialize_repository(s: *session.Session, root: str) err[outcome.Fail];
```

## fun check_dep_identity

```mach
pub fun check_dep_identity(s: *session.Session, key: str, declared: str) err[outcome.Fail];
```

the manifest key, the directory under dep/ and the project id are one name

## fun refuse_nested_realization

```mach
pub fun refuse_nested_realization(s: *session.Session, alias: str, dep_dir: str) err[outcome.Fail];
```

a realized nested dependency (`dep/<id>/dep/<x>/mach.toml`) is refused: the
root owns the flat closure and a dependency's own dep/ is never realized.
an empty directory git materializes for a consumed dependency's gitlink is not
a realization and passes

## fun declared_dep_id

```mach
pub fun declared_dep_id(s: *session.Session, dep_full: str) res[str, outcome.Fail];
```

## fun verify_dep_manifest_id

```mach
pub fun verify_dep_manifest_id(s: *session.Session, dep_full: str, id: str) err[outcome.Fail];
```

## val REALIZE_REPO_ROOT

```mach
pub val REALIZE_REPO_ROOT: u8 = 0
```

## val REALIZE_NESTED

```mach
pub val REALIZE_NESTED:    u8 = 1
```

## fun realization_mode

```mach
pub fun realization_mode(s: *session.Session, root: str) res[u8, outcome.Fail];
```

## fun dep_rel_of

```mach
pub fun dep_rel_of(alloc: *A.Allocator, id: str) res[str, outcome.Fail];
```

## fun dep_full_of

```mach
pub fun dep_full_of(alloc: *A.Allocator, root: str, id: str) res[str, outcome.Fail];
```

## fun staged_gitlink

```mach
pub fun staged_gitlink(s: *session.Session, root: str, id: str) res[opt[str], outcome.Fail];
```

## fun staged_paths

```mach
pub fun staged_paths(s: *session.Session, root: str, rel: str) res[str, outcome.Fail];
```

## fun own_work_tree

```mach
pub fun own_work_tree(alloc: *A.Allocator, dep_full: str) bool;
```

## fun checkout_commit

```mach
pub fun checkout_commit(s: *session.Session, dep_full: str, commit: str) err[outcome.Fail];
```

## fun checkout_head

```mach
pub fun checkout_head(s: *session.Session, dep_full: str) res[str, outcome.Fail];
```

## fun release_at_head

```mach
pub fun release_at_head(s: *session.Session, dep_full: str) res[str, outcome.Fail];
```

the release version a checkout's HEAD is tagged with (an owned "" when no `v`-prefixed
semver tag points at it); the highest wins when several do, and the caller frees the result

## val SLOT_ABSENT

```mach
pub val SLOT_ABSENT:   u8 = 0
```

what occupies dep/<id>

## val SLOT_EMPTY

```mach
pub val SLOT_EMPTY:    u8 = 1
```

## val SLOT_CHECKOUT

```mach
pub val SLOT_CHECKOUT: u8 = 2
```

## val SLOT_OCCUPIED

```mach
pub val SLOT_OCCUPIED: u8 = 3
```

## val SLOT_SYMLINK

```mach
pub val SLOT_SYMLINK:  u8 = 4
```

## val SLOT_ENTRY

```mach
pub val SLOT_ENTRY:    u8 = 5
```

## rec GitSlot

```mach
pub rec GitSlot;
```

a git dependency's slot: what dep/<id> holds, and its record in the enclosing
repository. the record lives at `top`, the repository's top level where
.gitmodules is, under the path `record`: dep/<id> when the project root is the
repository root, and the project's prefix ahead of it when the project is a
subdirectory of a repository, so a committed gitlink under a subproject is
its pin exactly as the root's is. `top` is empty when the root is in no work
tree, and then there is no record to read

## val REALIZED_NOTHING

```mach
pub val REALIZED_NOTHING:     u8 = 0
```

what realizing a git dependency changed

## val REALIZED_INITIALIZED

```mach
pub val REALIZED_INITIALIZED: u8 = 1
```

## val REALIZED_PINNED

```mach
pub val REALIZED_PINNED:      u8 = 2
```

## val REALIZED_REGISTERED

```mach
pub val REALIZED_REGISTERED:  u8 = 3
```

## val REALIZED_ADOPTED

```mach
pub val REALIZED_ADOPTED:     u8 = 4
```

## val REALIZED_CLONED

```mach
pub val REALIZED_CLONED:      u8 = 5
```

## fun git_slot_dnit

```mach
pub fun git_slot_dnit(alloc: *A.Allocator, slot: *GitSlot);
```

## fun git_slot_recorded

```mach
pub fun git_slot_recorded(slot: *GitSlot) bool;
```

whether the slot has a record to read or write: the project root is a
repository root, or a subdirectory of one

## fun observe_git_slot

```mach
pub fun observe_git_slot(s: *session.Session, root: str, id: str, mode: u8) res[GitSlot, outcome.Fail];
```

## fun refuse_git_slot

```mach
pub fun refuse_git_slot(s: *session.Session, root: str, id: str, mode: u8, slot: *GitSlot, accept: bool) err[outcome.Fail];
```

refuse a slot whose realization would replace or record work mach did not make

## fun realize_git_slot

```mach
pub fun realize_git_slot(s: *session.Session, root: str, id: str, url: str, ref: str, mode: u8,
slot: *GitSlot) res[u8, outcome.Fail];
```

bring a slot `refuse_git_slot` accepted to the realization a build verifies

## fun realize_git_dependency

```mach
pub fun realize_git_dependency(s: *session.Session, root: str, id: str, url: str, ref: str, mode: u8,
accept: bool) res[u8, outcome.Fail];
```

observe, refuse and realize one git dependency

## fun remove_dependency_index

```mach
pub fun remove_dependency_index(s: *session.Session, root: str, id: str, mode: u8) err[outcome.Fail];
```

## fun realize_path_dependency

```mach
pub fun realize_path_dependency(s: *session.Session, root: str, id: str, src_dir: str,
mode: u8) res[bool, outcome.Fail];
```

sync dep/<id> with a path dependency's source

ret: true when the copy was refreshed, false when it already matched its source and was reused

## fun realized_ids

```mach
pub fun realized_ids(alloc: *A.Allocator, root: str) res[Vector[str], outcome.Fail];
```

a dependency slot is a directory under dep/ named by a valid project id

## fun fetch_and_checkout

```mach
pub fun fetch_and_checkout(s: *session.Session, dep_full: str, ref: str) err[outcome.Fail];
```

## fun gitlink_recorded

```mach
pub fun gitlink_recorded(s: *session.Session, root: str, id: str, mode: u8) res[bool, outcome.Fail];
```

whether a moved checkout of dep/<id> is to be staged: always in a repository
root, where the gitlink is the pin, and in a subproject exactly when the
enclosing repository already records a gitlink for it (#3686)

## fun stage_dependency

```mach
pub fun stage_dependency(s: *session.Session, root: str, id: str) err[outcome.Fail];
```

