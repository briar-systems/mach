[mach](../../README.md) / [base](../README.md) / **source**

# mach.base.source

the source map: the files of a compilation and the spans, locations and lines in them

a file is written once into a slot that never moves, so a `*SourceFile` stays valid until the
map is released, and reloading or releasing its text bumps its revision in place. a location is
a span of one file, or nowhere when its file is FILE_NIL.

The directory's root module is [`mach.base.source`](../source.md).

| Module | |
| :--- | :--- |
| [`provider`](provider.md) | the contract a module load reads source through: which names are modules and their text |

| Directory | |
| :--- | :--- |
| [`provider/`](provider/README.md) | the contract a module load reads source through: which names are modules and their text |

