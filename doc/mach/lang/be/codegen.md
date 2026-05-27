# mach.lang.be.codegen

Backend code generator. Lowers an optimised
[`ir.Module`](../me/ir.md#module) into a target-specific
[`MIR`](codegen/mir.md) representation, allocates registers, lays
out stack frames, encodes the result into machine bytes, and hands
the assembled section data to [`obj`](obj.md) for object-file
construction. Source is `new/lang/be/codegen.mach` (currently empty).

Codegen is the compute body of
[`Q_CODEGEN`](../query.md#integration); each module's
[`ir.Module`](../me/ir.md#module) produces one
[`of.ObjectImage`](../target/of.md#objectimage).

## Types

### `CodegenContext`

```mach
pub rec CodegenContext {
    s:       *sess.Session;
    tgt:     *target.Target;
    ir:      *ir.Module;
    mir:     mir.MirModule;
    image:   of.ObjectImage;
}
```

| Field | Type                                                  | Description                                            |
|-------|-------------------------------------------------------|--------------------------------------------------------|
| s     | [`*sess.Session`](../session.md#session)              | Shared session.                                        |
| tgt   | [`*target.Target`](../target.md#target)               | Selected target — drives isel, regalloc, frame layout, encoder. |
| ir    | [`*ir.Module`](../me/ir.md#module)                    | Input IR.                                              |
| mir   | [`mir.MirModule`](codegen/mir.md#mirmodule)           | Intermediate MIR (lowered, allocated, frame-laid-out). |
| image | [`of.ObjectImage`](../target/of.md#objectimage)       | Output object image (sections, symbols, relocations), built via [`be.obj`](obj.md). |

## Functions

### `codegen_module`

```mach
pub fun codegen_module(
    s:   *sess.Session,
    tgt: *target.Target,
    ir:  *ir.Module,
) Result[of.ObjectImage, str]
```

Pipeline:

1. **Lower** — [`mir.lower_ir`](codegen/mir.md#lower_ir) translates
   each IR function into MIR with virtual registers and abstract
   ABI markers.
2. **Isel** — [`isel.run`](codegen/isel.md) selects concrete
   target instructions for each MIR opcode using the active
   [`target.arch`](../target/isa.md#isavtable) dispatch table.
3. **Regalloc** — [`regalloc.run`](codegen/regalloc.md) resolves
   virtual registers to physical ones using the
   [`target.arch.reg_classes`](../target/isa.md#isavtable).
4. **Frame** — [`frame.run`](codegen/frame.md) lays out the stack
   frame and emits prologue / epilogue sequences per the active
   [`target.abi`](../target/abi.md#abivtable).
5. **Encode** — [`encode.run`](codegen/encode.md) walks the MIR and
   produces section bytes + a pending-relocation list.
6. **Pack** — populate an
   [`of.ObjectImage`](../target/of.md#objectimage) via the
   [`be.obj`](obj.md) builder with the encoded sections, symbols,
   and relocations.

Returns the populated object image.

## Dependencies

`std.types.bool`, `std.types.size`, `std.types.result`,
[`mach.lang.session`](../session.md),
[`mach.lang.target`](../target.md),
[`mach.lang.me.ir`](../me/ir.md),
[`mach.lang.be.codegen.mir`](codegen/mir.md),
[`mach.lang.be.codegen.isel`](codegen/isel.md),
[`mach.lang.be.codegen.regalloc`](codegen/regalloc.md),
[`mach.lang.be.codegen.frame`](codegen/frame.md),
[`mach.lang.be.codegen.encode`](codegen/encode.md),
[`mach.lang.be.obj`](obj.md),
[`mach.lang.target.of`](../target/of.md).
