[mach](../../README.md) / [front](../README.md) / **comptime**

# mach.front.comptime

the comptime evaluator: an expression's value under the capabilities of the phase that
asks, over the scope it runs in

The directory's root module is [`mach.front.comptime`](../comptime.md).

| Module | |
| :--- | :--- |
| [`capability`](capability.md) | what each phase lets a comptime evaluation ask of it: members, identifiers, types, fields and layout, published as one record per phase |
| [`deep`](deep.md) | the deep value of a constant: an array, record or case literal with every member evaluated, an address as the symbol it points into, and a leaf that has no compile-time value kept with its refusal |
| [`failure`](failure.md) | why a comptime evaluation has no value: refused for good, waiting on a later phase or an instance, or failed inside the compiler |
| [`fold`](fold.md) | comptime arithmetic through mirl's constant folder: every operation a constant expression computes is one row of mirl's opcode table folded on constants, so a constant means at compile time what the same operation means at run time and no fold rule is stated here |
| [`gate`](gate.md) | comptime gates: which later phase a gate's condition depends on, and the verdict that selects or discards its arm |
| [`intrinsic`](intrinsic.md) | the comptime intrinsic registry: one row per `$name(...)` the compiler answers, which every phase reads by id |
| [`path`](path.md) | the comptime path registry: one row per `$root.segment...` path the compiler answers, matched once against a written path's segments |
| [`scope`](scope.md) | the scope a comptime evaluation runs in: the description of the build, the stages' bindings over a module, its frames and the gates it decided |
| [`value`](value.md) | the comptime value model: its kinds, constructors, integer and float arithmetic and casts between scalars |

