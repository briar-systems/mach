# mach.lang.be.obj

Object-image builder. The backend's convenience layer for producing
an [`of.ObjectImage`](../target/of.md#objectimage) — the
format-agnostic object model (sections, symbols, relocations) — and
handing it off to a format writer. The object model itself is defined
in [`target.of`](../target/of.md), where it sits as the contract
between the backend and the per-format writers; this module only
provides the builder API and the [`emit`](#emit) dispatch.

Source is `new/lang/be/obj.mach` (currently empty).

## Functions

### `init`

```mach
pub fun init(alloc: *Allocator, name: intern.StrId) Result[of.ObjectImage, str]
```

Allocates an empty [`of.ObjectImage`](../target/of.md#objectimage)
with the given module name. Section / symbol / relocation arrays
start `nil` and grow on first append.

| Param | Type                                  | Description                              |
|-------|---------------------------------------|------------------------------------------|
| alloc | `*Allocator`                          | Allocator backing every array on the image. |
| name  | [`intern.StrId`](../intern.md#strid)  | Module FQN, used for the output file's basename. |

### `dnit`

```mach
pub fun dnit(o: *of.ObjectImage)
```

Releases the section, symbol, and relocation arrays plus each
section's `bytes` buffer. `nil` is a no-op.

### `add_section`, `add_symbol`, `add_relocation`

```mach
pub fun add_section(o: *of.ObjectImage, s: of.Section) Result[u32, str]
pub fun add_symbol(o: *of.ObjectImage, sym: of.Symbol) Result[u32, str]
pub fun add_relocation(o: *of.ObjectImage, r: of.Relocation) Result[bool, str]
```

Append helpers used by [`codegen`](codegen.md). `add_section` and
`add_symbol` return the new entry's index (for back-references —
e.g. a [`Symbol.section`](../target/of.md#symbol) index, or a
[`Relocation.section`](../target/of.md#relocation) index). Each grows
its backing array on demand; errors on allocation failure.

### `emit`

```mach
pub fun emit(
    o:        *of.ObjectImage,
    tgt:      *target.Target,
    out_path: zstr,
) Result[bool, str]
```

Writes the object image to disk. Looks up the active format's
[`OfVTable`](../target/of.md#ofvtable) from
[`tgt.of`](../target.md#target) and invokes its
[`emit_object`](../target/of.md#writerfn) writer, passing
[`tgt.arch`](../target.md#target) as the architecture. This module
never knows the format-specific layout — that lives entirely in the
[`target/of/`](../target/of.md) writers.

| Param    | Type                                              | Description                              |
|----------|---------------------------------------------------|------------------------------------------|
| o        | [`*of.ObjectImage`](../target/of.md#objectimage)  | Image to serialise.                      |
| tgt      | [`*target.Target`](../target.md#target)           | Active target — supplies the format vtable and arch. |
| out_path | `zstr`                                            | Output file path.                        |

Returns `ok(true)` on success, or the writer's error.

## Dependencies

`std.types.bool`, `std.types.size`, `std.types.zstr`,
`std.types.result`, `std.allocator`,
[`mach.lang.intern`](../intern.md),
[`mach.lang.target`](../target.md),
[`mach.lang.target.of`](../target/of.md).
