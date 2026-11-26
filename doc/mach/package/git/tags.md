[mach](../../README.md) / [package](../README.md) / [git](README.md) / **tags**

# mach.package.git.tags

tags: the git half of the source contract that lists releases. a release is a
`v`-prefixed semver tag at a url, listed with one `ls-remote` each. the first manifest read
at a url brings every listed release's manifest into a scratch bare repository at once:
one shallow fetch of the listed tags without file contents, then one fetch of each
release's mach.toml by its object id. offline, both come from a local repository, a
realized checkout of the url, and nothing reaches the network

Source: [`src/package/git/tags.mach`](../../../../src/package/git/tags.mach)

Uses types from [`mach.base.fail`](../../base/fail.md), [`mach.package.git`](../git.md), [`mach.package.report`](../report.md), [`mach.package.source`](../source.md), [`mach.session`](../../session.md).

| Type | | |
| :--- | :--- | :--- |
| [`Remotes`](#remotes) | `rec` | `fetched` holds each url whose listed releases' manifests the scratch holds |

| On `Remotes` | |
| :--- | :--- |
| [`init`](#init) | open a releases table over git |
| [`dnit`](#dnit) | release the table; a candidate scratch that stays is the failure, once everything else is released |
| [`remember`](#remember) | the local repository offline resolution reads `url`'s releases from; the first existing one named wins, so a url with no checkout yet is refused by offline_dir and not by git |
| [`release_of`](#release_of) | the release of `url` whose commit is `commit`, as its version text owned by the caller, or an owned "" when no release tag names that commit: a dependency's committed gitlink is a commit, and resolution deals in releases |

| Functions | |
| :--- | :--- |
| [`releases`](#releases) | every `v`-prefixed semver tag at a url as a release, listed once per url and cached |
| [`shipped`](#shipped) | the manifest a release ships, read at its commit from the scratch, or offline from the local repository |

---

## Remotes

```mach
pub rec Remotes {
    s:             *mach.session.Session;
    gi:            mach.package.git.Inspector;
    listings:      Vector[CachedListing];
    locals:        Vector[LocalRepo];
    fetched:       Vector[str];
    offline:       bool;
    local:         str;
    scratch:       str;
    scratch_ready: bool;
    rep:           *mach.package.report.Report;
}
```

`fetched` holds each url whose listed releases' manifests the scratch holds. `rep` hears a
line before each url's releases are read from the network, nil for none

### init

```mach
pub fun init(s: *mach.session.Session, offline: bool, local: str, rep: *mach.package.report.Report) res[Remotes, mach.base.fail.Fail]
```

open a releases table over git

| Parameter | |
| :--- | :--- |
| `s` | the session, whose allocator holds the table's state |
| `offline` | read releases only from local repositories, reaching no network |
| `local` | the repository a url with no remembered checkout is read from offline, or "" |
| `rep` | hears a line before a url's releases are read from the network, nil for none |
| **returns** | the table, released with dnit, err when git is not on PATH |

### dnit

```mach
pub fun dnit(c: *Remotes) err[mach.base.fail.Fail]
```

release the table; a candidate scratch that stays is the failure, once
everything else is released

### remember

```mach
pub fun remember(c: *Remotes, url: str, dir: str) err[mach.base.fail.Fail]
```

the local repository offline resolution reads `url`'s releases from; the first existing one
named wins, so a url with no checkout yet is refused by offline_dir and not by git

### release_of

```mach
pub fun release_of(c: *Remotes, url: str, commit: str) res[str, mach.base.fail.Fail]
```

the release of `url` whose commit is `commit`, as its version text owned by the
caller, or an owned "" when no release tag names that commit: a dependency's committed
gitlink is a commit, and resolution deals in releases

## Functions

### releases

```mach
pub fun releases(ctx: ptr, url: str, out: *Vector[mach.package.source.Release]) err[mach.base.fail.Fail]
```

every `v`-prefixed semver tag at a url as a release, listed once per url and cached

| Parameter | |
| :--- | :--- |
| `ctx` | the table, a *Remotes |
| `url` | the source url |
| `out` | receives the releases, their strings owned by the session's allocator |
| **returns** | err when the tags cannot be listed or a release cannot be kept |

### shipped

```mach
pub fun shipped(ctx: ptr, url: str, rel: *mach.package.source.Release) res[mach.package.source.Shipped, mach.base.fail.Fail]
```

the manifest a release ships, read at its commit from the scratch, or offline from the local
repository

| Parameter | |
| :--- | :--- |
| `ctx` | the table, a *Remotes |
| `url` | the source url |
| `rel` | the release |
| **returns** | the manifest, owned by the caller, or why the release is no candidate when its manifest is not valid TOML, states a compiler range the running compiler is outside of, does not load or names another version, err when the manifest cannot be fetched or read at all |

