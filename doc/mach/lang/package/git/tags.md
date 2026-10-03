# mach.lang.package.git.tags

tags: the git half of the source contract that lists releases. a release is a
`v`-prefixed semver tag at a url, listed with one `ls-remote` each; the manifest of a
release is read from a scratch bare repository after a shallow fetch of its tag. offline,
both come from a local repository, a realized checkout of the url, and nothing reaches
the network

## rec Remotes

```mach
pub rec Remotes;
```

## fun open

```mach
pub fun open(s: *session.Session, offline: bool, local: str) res[Remotes, fail.Fail];
```

## fun close

```mach
pub fun close(c: *Remotes);
```

## fun remember

```mach
pub fun remember(c: *Remotes, url: str, dir: str) err[fail.Fail];
```

the local repository offline resolution reads `url`'s releases from; the first existing one
named wins, so a url with no checkout yet is refused by offline_dir and not by git

## fun releases

```mach
pub fun releases(ctx: ptr, url: str, out: *Vector[package_source.Release]) err[fail.Fail];
```

## fun release_of

```mach
pub fun release_of(c: *Remotes, url: str, commit: str) res[str, fail.Fail];
```

the release of `url` whose commit is `commit`, as its version text owned by the
caller, or an owned "" when no release tag names that commit: a dependency's committed
gitlink is a commit, and resolution deals in releases

## fun shipped

```mach
pub fun shipped(ctx: ptr, url: str, rel: *package_source.Release) res[package_source.Shipped, fail.Fail];
```

