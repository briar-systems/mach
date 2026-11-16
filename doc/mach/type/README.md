[mach](../README.md) / **type**

# mach.type

the type table: every type interned once, with its facts and its field tables

a `TypeInterner` interns each type once and names it by a `TypeId`. each type carries the facts
its parts spell, read when it is interned. the field tables, tag discriminators and layout
decorations belong to the current projection (`mach.type.field`), and a fact read from them is
kept until the projection resets or a table is published over one that held other fields.

<sub>[`mach.type`](../type.md) · 18 modules · imports [`base`](../base/README.md), [`report`](../report/README.md), [`session`](../session/README.md) · imported by [`build`](../build/README.md), [`compile`](../compile/README.md), [`front`](../front/README.md), [`lower`](../lower/README.md), [`session`](../session/README.md), [`target`](../target/README.md)</sub>

| Module | |
| :--- | :--- |
| [`deprecation`](deprecation.md) | the deprecation notice a declaration or a tag case carries |
| [`field`](field.md) | the field tables, tag discriminators and layout decorations of one type projection |
| [`kind`](kind.md) | type ids and the kinds of type the type table holds |
| [`prim`](prim.md) | the primitive types: their catalog, widths, names and integer ranges, and vector spellings |
| [`query`](query.md) | questions about the types in the store whose answers derive from the field tables: substitution, the deep properties, extent, secrecy agreement, containment and expansiveness, each a module under this one |
| [`scan`](scan.md) | the visited sets of nested scans over the type table |
| [`shape`](shape.md) | the mirl shape of a source type: the one place mach decides how a type is held in mirl's type table, which lowering interns into its module's table and sema into the table it lays types out over, so the size sema answers and the type codegen builds cannot disagree |
| [`visit`](visit.md) | one walk over the type graph: a property is a leaf predicate, and the walk follows the edges the property names from a type to the types it is built from |

| Directory | Modules | |
| :--- | ---: | :--- |
| [`query/`](query/README.md) | 10 | questions about the types in the store whose answers derive from the field tables: substitution, the deep properties, extent, secrecy agreement, containment and expansiveness, each a module under this one |

