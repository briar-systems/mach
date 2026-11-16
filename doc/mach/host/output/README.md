[mach](../../README.md) / [host](../README.md) / **output**

# mach.host.output

output: how a compiler-controlled file reaches its final name. a build
product under `out/` is regenerable by the next build or `mach clean`, so
it is written in place with no staging, lock or journal, except an `obj/`
object: the object cache reads it back, so it goes through
`through_temporary`. a file in the source tree goes through `replace`. both
write a sibling temporary and rename it over the destination, so an
interrupted write leaves the original intact. the build owns its output
layout: `directory` and `reserve` replace a stale entry of the wrong kind,
and every failure names the path and the operation through `fail.refusal`

<sub>[`mach.host.output`](../output.md) · 2 modules · imports [`base`](../../base/README.md) · imported by [`build`](../../build/README.md), [`cli`](../../cli/README.md), [`compile`](../../compile/README.md), [`manifest`](../../manifest/README.md), [`package`](../../package/README.md), [`tool`](../../tool/README.md)</sub>

| Module | |
| :--- | :--- |
| [`layout`](layout.md) | the layout a build owns beneath a cell's expanded `[project].work`: where its objects, IR, assembly, tests, compiler state, step staging and dependency artifacts go |
| [`remove`](remove.md) | descriptor-relative removal beneath a project root: every component is opened without following symlinks so no part of the path is resolved as a string and a link out of the tree is refused rather than followed |

