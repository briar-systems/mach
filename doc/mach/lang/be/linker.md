# mach.lang.be.linker

Object-image linker. Combines every per-module
[`of.ObjectImage`](../target/of.md#objectimage) produced by codegen
plus the runtime startup objects into a single output executable or
relocatable. Linking is the compute body of
[`Q_LINK`](../query.md#integration).

Source is `new/lang/be/linker.mach` (currently empty).

## Functions

### `link`

```mach
pub fun link(
    s:        *sess.Session,
    tgt:      *target.Target,
    modules:  *of.ObjectImage,
    module_count: u32,
    out_path: zstr,
    mode:     LinkMode,
) Result[bool, str]
```

| Param        | Type                                                  | Description                                          |
|--------------|-------------------------------------------------------|------------------------------------------------------|
| s            | [`*sess.Session`](../session.md#session)              | Shared session.                                      |
| tgt          | [`*target.Target`](../target.md#target)               | Active target — picks the object-format writer.      |
| modules      | [`*of.ObjectImage`](../target/of.md#objectimage)      | All compiled modules' object output.                 |
| module_count | `u32`                                                 | Length of `modules`.                                 |
| out_path     | `zstr`                                                | Final output path.                                   |
| mode         | [`LinkMode`](#linkmode)                               | Executable vs relocatable vs shared.                 |

Returns `ok(true)` after the final binary has been written.

## Types

### `LinkMode`

```mach
pub def LinkMode: u8;
```

| Constant            | Value | Meaning                                              |
|---------------------|-------|------------------------------------------------------|
| `LINK_EXE`          | 0     | Executable (default for `mach build`).               |
| `LINK_RELOCATABLE`  | 1     | Single object output (no symbol resolution).         |
| `LINK_SHARED`       | 2     | Shared object / dynamic library.                     |

## Pipeline

1. **Collect symbols.** Walk every input
   [`of.ObjectImage`](../target/of.md#objectimage); merge their
   symbol tables into a global table keyed by `intern.StrId`. Defined
   symbols resolve to a `(module, symbol_index)` pair; duplicate
   definitions (other than weak) are an error.
2. **Resolve relocations.** For each
   [`of.Relocation`](../target/of.md#relocation), look up the target symbol;
   compute the final virtual address (in `LINK_EXE` / `LINK_SHARED`)
   or leave the relocation in place (in `LINK_RELOCATABLE`).
3. **Lay out.** Concatenate same-kind sections from every input,
   honouring per-section alignment. The `std.runtime` startup object
   is placed at the canonical entry offset for the active OS.
4. **Patch.** Apply each resolved relocation to the laid-out
   section bytes.
5. **Emit.** Hand the final image to
   [`target.of`](../target/of.md) via
   [`obj.emit`](obj.md#emit).

`LINK_RELOCATABLE` mode skips steps 2 (resolution) and 4 (patching),
preserving relocations in the output object.

## Diagnostics

| Condition                              | Diagnostic                                                              |
|----------------------------------------|-------------------------------------------------------------------------|
| Undefined symbol referenced by a reloc | `"undefined symbol: <name>"`                                            |
| Duplicate strong definition            | `"multiple definition of '<name>' in <a> and <b>"`                      |
| Relocation overflow                    | `"relocation '<kind>' overflows at <module>:<offset>"`                  |
| Unsupported relocation                 | `"relocation '<kind>' not supported by <format>"`                        |

## Dependencies

`std.types.bool`, `std.types.size`, `std.types.zstr`,
`std.types.result`, `std.allocator`,
[`mach.lang.session`](../session.md),
[`mach.lang.target`](../target.md),
[`mach.lang.target.of`](../target/of.md),
[`mach.lang.be.obj`](obj.md).
