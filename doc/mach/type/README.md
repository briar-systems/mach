[mach](../README.md) / **type**

# mach.type

the type table: every type interned once, with its facts and its field tables

a `TypeInterner` interns each type once and names it by a `TypeId`. each type carries the facts
its parts spell, read when it is interned. the field tables, tag discriminators and layout
decorations belong to the current projection (`mach.type.field`), and a fact read from them is
kept until the projection resets or a table is published over one that held other fields.

a table may be layered over a base, which is how work off the owner thread makes types. its
contract:
- a layer reads its base without a lock, so the base takes nothing new while the layer lives. it
  borrows the base's storage and copies a part only when it first adds to it, so it reads every
  type, parameter run, handle and field table of its base as the base reads its own
- a layer numbers what it adds past its base. its numbers are its own: two layers over one base
  give one number to different types, so no id a layer made is read anywhere but through the
  layer until it is merged, and no store shared between tables keeps a fact of one (memo_keyed)
- what a table keeps beside its types (`Side`) is moved, never shared: a layer is handed the side
  of the table or thread working on it, when it is made or for a span of its work (`side_lend`),
  and hands it back, with every memo of a type past its base dropped at both ends, so a memo is
  only ever read under the numbering it was made in
- `merge` interns each type a layer made into another table over the same base, in the order the
  layer made them, and says where each landed. types are equal by their parts, so when the owner
  merges layers at the points one thread would have made their types, and in that order, every
  merged id is the id the one thread would have given
- `fold` appends a layer to its base, which took nothing since, so every id the layer gave stands

The directory's root module is [`mach.type`](../type.md).

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

| Directory | |
| :--- | :--- |
| [`query/`](query/README.md) | questions about the types in the store whose answers derive from the field tables: substitution, the deep properties, extent, secrecy agreement, containment and expansiveness, each a module under this one |

