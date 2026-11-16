[mach](../../README.md) / [package](../README.md) / **git**

# mach.package.git

git: the one outside tool mach runs. a dependency is acquired and pinned through git
(package/git holds the source and pin members built on it); nothing else in the
toolchain spawns it, and a build never does. every git process starts here, with one
environment that keeps the user's configuration and drops whatever would redirect git to
another repository, and every failure names the command, how it ended and what git printed

The directory's root module is [`mach.package.git`](../git.md).

| Module | |
| :--- | :--- |
| [`checkout`](checkout.md) | checkout: the git half of the source contract that realizes dep/\<id> |
| [`gitlink`](gitlink.md) | gitlink: the git member of the pin contract |
| [`member`](member.md) | member: git as a member of the source and pin contracts, its tables assembled from the parts beside it |
| [`tags`](tags.md) | tags: the git half of the source contract that lists releases |
| [`verify`](verify.md) | verify: what `mach dep verify` holds a git dependency to |

