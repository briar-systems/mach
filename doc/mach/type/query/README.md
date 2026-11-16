[mach](../../README.md) / [type](../README.md) / **query**

# mach.type.query

questions about the types in the store whose answers derive from the field tables:
substitution, the deep properties, extent, secrecy agreement, containment and
expansiveness, each a module under this one. the facts worth keeping are cached in one
record that only this module reads or writes, stamped with the projection generation
they were derived under

The directory's root module is [`mach.type.query`](../query.md).

| Module | |
| :--- | :--- |
| [`areas`](areas.md) | how a tag's cases share bytes on the build's target: a byte of a tag is never public under one case and secret under another, so two cases share an area only where they agree on the class of every byte both cover, the comparison a union's variants must pass |
| [`deep`](deep.md) | whether a type reaches a property anywhere, through pointers included, answered over the declarations so a generic that grows through a pointer still has a finite answer |
| [`expansive`](expansive.md) | whether a generic has unboundedly many instances, decided on the first question about it once its fields exist and recorded for the projection it was decided under |
| [`extent`](extent.md) | the size, alignment and member offsets of a type, as mirl's data layout of the build's target lays it out: each source type is interned in a type table the laying holds, no module's, and mirl answers every question of it, so mach computes no layout of its own |
| [`logical`](logical.md) | the types a logical-addressing target can declare |
| [`mention`](mention.md) | which type parameters a type mentions, and the declaration that owns them |
| [`placement`](placement.md) | where each parameter of a generic sits in the storage its instances lay out, answered over the declarations so a generic that grows through a pointer still has a finite answer |
| [`reach`](reach.md) | what a type holds or reaches, each property a leaf predicate over the one type walk |
| [`secrecy`](secrecy.md) | whether two types laid over one storage keep every byte's secrecy class: a cast or retype from one to the other, or two union variants |
| [`subst`](subst.md) | a generic's body under type arguments: substitution, and the field table an instance reads |

