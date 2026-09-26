# mach.lang.driver.resolver

## rec Need

```mach
pub rec Need;
```

`who` names the requirer a diagnostic attributes a root need to: the root itself
("" reads as the problem's root) or the chain of a dependency that declared the
range, since every requirer's range takes part in the root's problem (#3702)

## rec ReleaseNeeds

```mach
pub rec ReleaseNeeds;
```

what a release asks for: the compiler range its manifest states ("" for none) and its
dependencies selected by version range. `excluded` is "" for a candidate and otherwise says
why the release is not one (its manifest does not load, or names another version): the
release is set aside and the reason explains a failure it takes part in, while a failure to
read the release at all is the NeedsFn's err and stops resolution

## def NeedsFn

```mach
pub def NeedsFn: fun(ptr, str, str, *cand.Release, *ReleaseNeeds) err[outcome.Fail]
```

fills `out` (initialized by the caller over the solve's allocator) for one release

## rec Lock

```mach
pub rec Lock;
```

a preferred version, kept while it still satisfies every requirement (`update <name>`)

## rec Problem

```mach
pub rec Problem;
```

fixed ids are identities the root selects itself (an exact or branch ref, or a path): the root
overrides them, so no requirement on them takes part in resolution

## rec Choice

```mach
pub rec Choice;
```

## fun resolve

```mach
pub fun resolve(a: *A.Allocator, src: *cand.CandidateSource, needs: NeedsFn, needs_ctx: ptr,
problem: *Problem) res[Vector[Choice], outcome.Fail];
```

the selected release of every identity the root reaches, allocated in `a`

