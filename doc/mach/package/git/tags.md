[mach](../../README.md) / [package](../README.md) / [git](README.md) / **tags**

# mach.package.git.tags

tags: the git half of the source contract that lists releases. a release is a
`v`-prefixed semver tag at a url, listed with one `ls-remote` each; the manifest of a
release is read from a scratch bare repository after a shallow fetch of its tag. offline,
both come from a local repository, a realized checkout of the url, and nothing reaches
the network

<sub>source: [`src/package/git/tags.mach`](../../../../src/package/git/tags.mach) · imported by 1 module</sub>

<sub>`session` is [`mach.session`](../../session.md) · `git` is [`mach.package.git`](../git.md) · `CachedListing` is private · `LocalRepo` is private · `fail` is [`mach.base.fail`](../../base/fail.md) · `package_source` is [`mach.package.source`](../source.md)</sub>

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
| [`releases`](#releases) · [`shipped`](#shipped) | |

---

## Remotes

```mach
pub rec Remotes {
    s:             *session.Session;
    gi:            git.Inspector;
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
pub fun init(s: *session.Session, offline: bool, local: str) res[Remotes, fail.Fail]
```

### dnit

```mach
pub fun dnit(c: *Remotes) err[fail.Fail]
```

release the table; a candidate scratch that stays is the failure, once
everything else is released

### remember

```mach
pub fun remember(c: *Remotes, url: str, dir: str) err[fail.Fail]
```

the local repository offline resolution reads `url`'s releases from; the first existing one
named wins, so a url with no checkout yet is refused by offline_dir and not by git

### release_of

```mach
pub fun release_of(c: *Remotes, url: str, commit: str) res[str, fail.Fail]
```

the release of `url` whose commit is `commit`, as its version text owned by the
caller, or an owned "" when no release tag names that commit: a dependency's committed
gitlink is a commit, and resolution deals in releases

## Functions

### releases

```mach
pub fun releases(ctx: ptr, url: str, out: *Vector[package_source.Release]) err[fail.Fail]
```

### shipped

```mach
pub fun shipped(ctx: ptr, url: str, rel: *package_source.Release) res[package_source.Shipped, fail.Fail]
```

