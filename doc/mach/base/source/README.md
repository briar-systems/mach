[mach](../../README.md) / [base](../README.md) / **source**

# mach.base.source

the source map: the files of a compilation and the spans, locations and lines in them

a file is written once into a slot that never moves, so a `*SourceFile` stays valid until the
map is released, and reloading or releasing its text bumps its revision in place. a location is
a span of one file, or nowhere when its file is FILE_NIL.

<sub>[`mach.base.source`](../source.md) · 3 modules · imported by [`build`](../../build/README.md), [`cli`](../../cli/README.md), [`compile`](../../compile/README.md), [`front`](../../front/README.md), [`lower`](../../lower/README.md), [`manifest`](../../manifest/README.md), [`package`](../../package/README.md), [`report`](../../report/README.md), [`session`](../../session/README.md), [`tool`](../../tool/README.md), [`type`](../../type/README.md)</sub>

| Module | |
| :--- | :--- |
| [`provider`](provider.md) | the contract a module load reads source through: which names are modules and their text |

| Directory | Modules | |
| :--- | ---: | :--- |
| [`provider/`](provider/README.md) | 2 | the contract a module load reads source through: which names are modules and their text |

