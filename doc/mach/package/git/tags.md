[mach](../../README.md) / [package](../README.md) / [git](README.md) / **tags**

# mach.package.git.tags

tags: the git half of the source contract that lists releases. a release is a
`v`-prefixed semver tag at a url, listed with one `ls-remote` each; the manifest of a
release is read from a scratch bare repository after a shallow fetch of its tag. offline,
both come from a local repository, a realized checkout of the url, and nothing reaches
the network

Source: [`src/package/git/tags.mach`](../../../../src/package/git/tags.mach)

Uses types from [`mach.base.fail`](../../base/fail.md), [`mach.package.git`](../git.md), [`mach.package.source`](../source.md), [`mach.session`](../../session.md).

| Type | | |
| :--- | :--- | :--- |
| [`Remotes`](#remotes) | `rec` |  |

| On `Remotes` | |
| :--- | :--- |
| [`init`](#init) | |
| [`dnit`](#dnit) | release the table; a candidate scratch that stays is the failure, once everything else is released |
| [`remember`](#remember) | the local repository offline resolution reads `url`'s releases from; the first existing one named wins, so a url with no checkout yet is refused by offline_dir and not by git |
| [`release_of`](#release_of) | the release of `url` whose commit is `commit`, as its version text owned by the caller, or an owned "" when no release tag names that commit: a dependency's committed gitlink is a commit, and resolution deals in releases |

| Functions | |
| :--- | :--- |
| [`releases`](#releases), [`shipped`](#shipped) | |

---

## Remotes

```mach
pub rec Remotes {
    s:             *mach.session.Session;
    gi:            mach.package.git.Inspector;
    listings:      Vector[CachedListing];
    locals:        Vector[LocalRepo];
    offline:       bool;
    local:         str;
    scratch:       str;
    scratch_ready: bool;
}
```

### init

```mach
pub fun init(s: *mach.session.Session, offline: bool, local: str) res[Remotes, mach.base.fail.Fail]
```

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

### shipped

```mach
pub fun shipped(ctx: ptr, url: str, rel: *mach.package.source.Release) res[mach.package.source.Shipped, mach.base.fail.Fail]
```

