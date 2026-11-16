[mach](../../README.md) / [build](../README.md) / **step**

# mach.build.step

the build steps a manifest declares: each step's inputs are collected, its
invocation expanded, its outputs staged and published, and its stamp kept, so
a step runs again only when what it reads changed. the build engine runs a
project's own steps and its dependencies' steps as phases of a unit

<sub>[`mach.build.step`](../step.md) · 2 modules · imports [`base`](../../base/README.md), [`compile`](../../compile/README.md), [`host`](../../host/README.md), [`manifest`](../../manifest/README.md), [`report`](../../report/README.md)</sub>

| Module | |
| :--- | :--- |
| [`glob`](glob.md) | the input patterns a build step names: a wildcard over one directory, or one recursive `**` segment, expanded beneath the project without following a link |
| [`walk`](walk.md) | a contained path walk: each component of a path beneath a root is read without following a symlink, refused when it is not what the caller expects, and made when the caller's policy creates it |

