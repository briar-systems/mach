# GPU decorators

The decorators a GPU target reads: the pipeline stage a function enters, its
compute workgroup, the module bindings of the shader interface, specialization
constants, the handle types a target mints, and functions that are target
instructions. Every decorator, and where each one applies, is listed in
[decorators.md](decorators.md#applicability). The `spirv` target and its
environments are described in [manifest.md](manifest.md#finished-module-targets).

The GPU set is dotted, `#[gpu.stage]`, `#[gpu.input]` and the rest, since its
generic words apply only to shader stages. v6 spelled each without the prefix,
and `intrinsic` as `op`. Each v6 spelling is refused, naming its replacement.

## `gpu.stage(str)` — GPU pipeline stage

Marks a function as the entry point of a graphics or compute pipeline stage. The
argument names the stage and the set is closed:

| Value        | Stage                     |
|--------------|---------------------------|
| `"vertex"`   | vertex shader             |
| `"fragment"` | fragment (pixel) shader   |
| `"compute"`  | compute shader            |

An unrecognized value is a compile error (`decorator.argument`), not a module that
quietly forms no stage.

```mach
#[gpu.stage("vertex")]
fun vertex_main() {}

#[gpu.stage("fragment")]
fun fragment_main() {}
```

A staged function **takes no parameters and returns nothing**. A pipeline stage
does not have a caller: its inputs arrive through input interface variables and
its results leave through output ones, so there is no argument list or return
value to carry them. A `spirv` build rejects a staged function with either, with
`spirv.stage_signature`.

The decorator is accepted on every target, because which target a module is built
for is not a property of its source. Only a target that has pipeline stages acts
on it: on `spirv` a staged function becomes an `OpEntryPoint` with the matching
execution model, and on a machine target the stage is ignored and the function is
compiled normally.

Whether a build makes a **shader module** or a **library module** is the artifact's
[`kind`](manifest.md#artifactname), not the source's, so one source set can build both.
Every module an artifact holds, its dependencies' included, compiles as one unit into
one SPIR-V module, since the rules of a finished module span it whole: what each stage
reaches through every function it calls, wherever that function is written. A
`module` artifact is a shader module: that module carries the entry point of every
stage and no external linkage at all, since a Vulkan consumer refuses a module
carrying linkage, and a `module` that holds no stage is refused. An `object` artifact
is a library module: its one object keeps the `Linkage` capability and publishes each
function that is not an entry point as a linkage export, so a later link that joins it
to other prebuilt modules
can find it, and no Vulkan consumer takes it as it is. Adding a `#[gpu.stage(...)]` to
a source does not change which of the two a build makes.

The entry point's name, as a pipeline-creation call looks it up, is the function's
**bare source name** (`vertex_main` above), not a mangled linker symbol. A shader
module has no linker symbols to mangle.

A SPIR-V module has no call stack, so the static call graph of an entry point holds no
cycle (SPIR-V 2.16.1): a function a stage reaches that reaches itself again, directly or
through the functions it calls, is a compile error (`spirv.recursion`) at the call that
closes the cycle. Write the recursion as a loop.

## `gpu.workgroup(x, y, z)` — compute workgroup dimensions

Sizes the workgroup of a `#[gpu.stage("compute")]` function. The three arguments are
comptime integers giving the x, y and z dimensions.

```mach
#[gpu.stage("compute")]
#[gpu.workgroup(64, 1, 1)]
fun compute_main() {}
```

It requires a stage on the same function, since without one it would silently mean
nothing, and it applies only to the compute stage. Each is refused with
`decorator.requires`. When it is omitted, a compute stage takes the
single-invocation default `(1, 1, 1)`. The dimensions are always declared in the
emitted module, since a compute stage that does not state its
workgroup size is not one a consumer can dispatch.

Any dimension may instead name a `#[gpu.spec]` var (see
[`gpu.spec`](#gpuspecid--specialization-constants)), which the host sets when it creates
the pipeline. The var must be a `u32` or `i32`, and any other variable is refused.

```mach fragment
#[gpu.spec(0)] var tile: u32 = 64;

#[gpu.stage("compute")]
#[gpu.workgroup(tile, 1, 1)]
fun compute_main() {}
```

On `spirv` how a specialized workgroup is emitted depends on the environment.
Where it allows it, the stage carries `LocalSizeId` over the spec constant. That
needs SPIR-V 1.2 and, on Vulkan, the `maintenance4` feature, which only Vulkan
1.3 requires of every device, so the form is used with no `env` and with
`vulkan1.3`. Every other environment gets a `WorkgroupSize` built-in over an
`OpSpecConstantComposite`, and each stage's `LocalSize` states the defaults, which
the built-in overrides. That built-in sizes **every** compute stage in the
module, so a module in such an environment whose compute stages size their
workgroups differently, and at least one of them through a `#[gpu.spec]` var, is
refused. Two stages size a dimension the same way only where both name one
`#[gpu.spec]` var or both write one number, since the host may specialize the var
to any size. Give the stages one workgroup or put them in separate modules.

## `gpu.input(n)` / `gpu.output(n)` / `gpu.builtin(str)` / `gpu.uniform(set, binding)` / `gpu.storage(set, binding)` / `gpu.sampler(set, binding)` / `gpu.push` / `gpu.spec(id)` / `gpu.shared` — shader interface

A pipeline stage does not receive its inputs or return its results through a call.
It reads and writes **module-scope variables** that the pipeline binds, and these
decorators say which kind each variable is. They apply only to module-level
`val` / `var` bindings, and a variable carries **exactly one** of them, since they
are mutually exclusive. A second one is refused with `decorator.conflict`.
`gpu.spec`, described in its own section below, is one of
them too.

```mach fragment
#[gpu.input(0)]            var in_position: f32x4;
#[gpu.output(0)]           var out_colour:  f32x4;
#[gpu.builtin("position")] var position:    f32x4;

rec Camera { view: f32x4; proj: f32x4; }
#[gpu.uniform(0, 0)] var camera: Camera;

rec Particles { pos: [64]f32x4; }
#[gpu.storage(0, 1)] var particles: Particles;

#[gpu.sampler(1, 0)] var albedo: Sampler2D;

rec Params { tint: f32x4; count: u32; }
#[gpu.push] var params: Params;
```

`gpu.input` and `gpu.output` number a **varying** with a location, which is how one
stage's outputs line up with the next stage's inputs: the producer's
`#[gpu.output(0)]` feeds the consumer's `#[gpu.input(0)]`.

`gpu.builtin` names a value the pipeline supplies or consumes instead of one a
location carries. The accepted set is closed:

| Value                      | Meaning                                | Type    | Direction | Stage    |
|----------------------------|----------------------------------------|---------|-----------|----------|
| `"position"`               | clip-space vertex position             | `f32x4` | written   | vertex   |
| `"point_size"`             | rasterized point size                  | `f32`   | written   | vertex   |
| `"vertex_index"`           | index of the current vertex            | `u32`   | read      | vertex   |
| `"instance_index"`         | index of the current instance          | `u32`   | read      | vertex   |
| `"frag_coord"`             | fragment window coordinate             | `f32x4` | read      | fragment |
| `"global_invocation"`      | compute global invocation id           | `u32x3` | read      | compute  |
| `"local_invocation"`       | compute local invocation id            | `u32x3` | read      | compute  |
| `"workgroup_id"`           | compute workgroup id                   | `u32x3` | read      | compute  |
| `"num_workgroups"`         | compute workgroup count of a dispatch  | `u32x3` | read      | compute  |
| `"local_invocation_index"` | compute local invocation id, flattened | `u32`   | read      | compute  |
| `"subgroup_size"`          | invocations in a subgroup              | `u32`   | read      | every    |
| `"subgroup_invocation"`    | the invocation's index in its subgroup | `u32`   | read      | every    |
| `"subgroup_id"`            | the subgroup's index in its workgroup  | `u32`   | read      | compute  |
| `"num_subgroups"`          | subgroups in the workgroup             | `u32`   | read      | compute  |

The direction is a property of the built-in, not something you restate. A stage
writes its position and reads what the pipeline hands it, so there is no
input/output marker to pair with `gpu.builtin`, and none that could disagree with it.

The **type** is a property of the built-in too, and it is a requirement rather
than a suggestion: the pipeline binds the variable itself, so a wider or narrower
one is an invalid module rather than a wasteful one. Declaring a built-in at any
other type is a compile error naming both the declared type and the required one,
on every target, since the type is the built-in's and not the target's.
The integer rows accept `i32` as well as `u32`, and `i32x3` as well as `u32x3`,
because the compiler carries an integer's width and not its sign, the emitted type
is sign-less either way, and the Vulkan environment asks of each only that its
components be 32-bit integers.

The **stage** is a property of the built-in as well. Each row exists in the stages
SPIR-V defines it in, in its direction: the pipeline supplies an input built-in to
those execution models alone and consumes an output one from them alone, so
`position` and `point_size` are a vertex stage's outputs, `frag_coord` is a
fragment stage's input, the seven compute rows are the `GLCompute` execution
model's, and `subgroup_size` and `subgroup_invocation` are every stage's, decorated
`Flat` as a fragment stage's integer inputs. A stage that uses a built-in outside
its row, directly or through a function it calls, is a compile error naming the
built-in and both stages. The four subgroup built-ins need SPIR-V 1.3, and outside
a compute stage the device's `subgroupSupportedStages`, as the subgroup operations do.

`gpu.uniform` binds a read-only block by descriptor set and binding. Its type **must
be a `rec`**: a uniform is a block with a host-visible layout, and a bare scalar
or vector has no block layout for a pipeline to bind. The record is emitted with
its `Block` decoration and an explicit byte offset on every member, taken from the
same layout the rest of the compiler uses, so what the shader reads is what the
host wrote. Wrap a single value in a one-field record.

`gpu.storage` binds a **read-write** buffer by descriptor set and binding, where
`gpu.uniform` binds a read-only one. Both must be a `rec` for the same reason, and both
are emitted as a `Block`-decorated struct with an explicit offset on every member.

They differ in one place: their **layout rules**. A block keeps mach's own layout,
and the layout is held to the rules Vulkan holds its class to, refused where it
breaks them rather than silently repacked. Under `vulkan1.0` and `vulkan1.1` a uniform
block follows std140-shaped rules, under which an array's stride is rounded up to 16,
which mach's own layout does not do, so an array of anything narrower than 16 bytes is
refused. A storage buffer follows std430-shaped rules, which use the element's natural
stride, as mach's layout does, so `[8]f32` is fine in a `gpu.storage` block and
rejected in a `gpu.uniform` one there. Vulkan 1.2 requires `uniformBufferStandardLayout`
of every device, under which a uniform block takes the storage buffer's rules, so under
`vulkan1.2`, `vulkan1.3` and with no `env` a uniform block follows the std430-shaped
rules too. Both place a vector at a multiple of its own alignment, twice its
lane's for two lanes and four times for three or four. Under `vulkan1.1` and later,
and with no `env`, whose relaxed block layout Vulkan 1.1 took into its core, a vector
member may sit at any multiple of its lane's size that keeps it within one 16-byte
line. `vulkan1.0` holds no relaxed layout. The refusal names the
first byte the layout places where the rules would not.

A compute stage's data path is `gpu.storage`: Vulkan forbids the `Output` storage class
in a compute execution model, so a compute shader reads and writes buffers rather
than varyings.

`gpu.storage` takes **memory qualifiers** after the descriptor pair, any number of them
in any order: `"readonly"`, `"writeonly"` and `"coherent"`. `"readonly"` says that
nothing writes the binding:

```mach
rec Palette { columns: [512]f32x4; }
#[gpu.storage(0, 3, "readonly")]
var palette: Palette;
```

A store through a `"readonly"` binding is a compile error on every target, naming
the line that wrote it. That is what the qualifier buys over what the compiler
works out on its own: a buffer or storage image no body in the module stores
through is emitted with the SPIR-V `NonWritable` decoration whether or not it is
marked, an atomic through a texel pointer counting as a store to its image, and Vulkan reads
that decoration to decide whether a stage needs `vertexPipelineStoresAndAtomics`.
So an accidental write does not produce a wrong module, it produces a **correct one
that quietly costs a hardware feature**. Marking the binding turns that into a
diagnostic instead.

A store is judged by the storage it writes. A physical pointer read out of the
binding reaches another buffer, so a store through it writes that buffer and not
the binding, and it is accepted whether it is written inline or through a local:

```mach
rec Shard { v: [64]u32; }
rec Table { shards: [4]*Shard; }
#[gpu.storage(0, 0, "readonly")]
var table: Table;

fun fill(i: u32, x: u32) {
    table.shards[0].v[i] = x; # writes the shard, not `table`
    val p: *Shard = table.shards[1];
    p.v[i] = x; # the same store through a local
}
```

The inference is one-sided on purpose. It follows the binding's address into every
function it is handed to, since every module of an artifact is one unit, and anything
it cannot follow counts as a write, so a missing decoration is possible and a wrong
one is not.

`"writeonly"` is the mirror: it says that nothing reads the binding, and the buffer
is emitted `NonReadable`.

```mach
rec Frame { texels: [4096]f32x4; }
#[gpu.storage(0, 4, "writeonly")]
var frame: Frame;
```

A read of a `"writeonly"` binding is a compile error on every target, naming the
expression that read it. Storing into a field or an element reads nothing, and
neither does taking the binding's address, so `frame.texels[i] = c` and
`?frame.texels[i]` are accepted. A store through a pointer read out of the binding
reads the binding for the pointer, though it writes only what the pointer reaches.
Writing **one lane** of a vector inside it is
refused, because a lane write loads the whole vector and stores it back: store the
whole vector instead. On SPIR-V, a load the compiler follows through the binding's
address is refused as well, with the same caution as `"readonly"`: anything the
compiler cannot follow counts as a read. `"readonly"` and `"writeonly"` together
are refused, since that binding would be neither read nor written.

`"coherent"` makes a write one invocation makes visible to invocations in other
workgroups, which is what atomics and flags shared across workgroups rely on. It
combines with either of the other two. How depends on the memory model the target
selects (see [manifest.md](manifest.md#finished-module-targets)):

- Under the GLSL450 model the buffer is decorated `Coherent`.
- Under the Vulkan model, which has no such decoration, each access through the buffer
  carries its own availability or visibility: a store `MakePointerAvailable`, a load
  `MakePointerVisible`, each with `NonPrivatePointer`, at the `QueueFamily` scope,
  every invocation of the dispatch and of later work on that queue family. That is
  glslang's reading of `coherent`, and it needs no device-scope feature. A storage
  image's `OpImageRead` and `OpImageWrite` carry `MakeTexelVisible` or
  `MakeTexelAvailable` with `NonPrivateTexel` the same way. A storage image handed to
  a function as a parameter carries no qualifier into it, so every image read and
  write in such a function is coherent in a module that binds a coherent image, and
  likewise every load and store through a pointer parameter into buffer memory is
  coherent in a module that binds a coherent buffer.

```mach
rec Counters { done: u32; }
#[gpu.storage(0, 5, "coherent")]
var counters: Counters;
```

`gpu.sampler` binds a **handle** by descriptor set and binding, at the same descriptor
addressing `gpu.uniform` and `gpu.storage` use, so a host binds one the way it binds the
others. Its type must be a **handle type**, a bodyless `def` carrying `#[handle]`
(see [types.md](types.md)), and a handle type must carry this decorator: a handle
names a descriptor rather than an object with storage, so one with no descriptor
address is reachable from no stage. A handle's one indirection is a pointer to its
binding, passed to a function. A handle cannot sit behind a pointer to a pointer,
inside an array, or in a local binding, and each of those is a compile error naming
why.

A **storage image** is the exception: an `image` handle whose `Sampled` operand is
`2` is read and written directly rather than sampled, which is Vulkan's
`STORAGE_IMAGE` descriptor, so it binds through `gpu.storage` and not `gpu.sampler`. The
`"readonly"`, `"writeonly"` and `"coherent"` qualifiers apply to it exactly as they
do to a buffer, checked against the instructions that read and write its texels. A
storage texel buffer, an image of `Dim` `Buffer` with `Sampled` `2`, binds the same
way, and a uniform texel buffer (`Sampled` `1`) keeps `gpu.sampler`. Binding a storage
image through `gpu.sampler`, or any other handle through `gpu.storage`, is refused at the
binding with `handle.role`: the target's handle catalog states which kinds, and which
operand values, make a storage resource, and the binding alone decides it.
A storage image is read and written by `OpImageRead` and `OpImageWrite`, a uniform
texel buffer or a sampled image is fetched texel by texel by `OpImageFetch`, and
`OpImageQuerySize`, `OpImageQuerySizeLod`, `OpImageQueryLevels` and
`OpImageQuerySamples` read an image's descriptor under the `ImageQuery` capability,
which every Vulkan version accepts.

`OpImageRead`, `OpImageWrite`, `OpImageFetch`, `OpImageSampleImplicitLod` and
`OpImageGather` take an optional `Image Operands` mask leading their tail, so a declaration either stops at
the instruction's required operands or passes the mask and the operands its bits
bring, each typed by its bit. `Sample` (`0x40`) names one sample of a multisampled
image, and a fetch's `Lod` (`0x2`) the level it reads. A multisampled image is read,
written and fetched only with `Sample`, and only a multisampled image takes it.
`OpImageSampleExplicitLod` always takes the mask, which must set `Lod` or `Grad`
(`0x4`), whose two operands are the coordinate's derivatives along x and y, and never
both. An implicit-lod sample takes `Bias` (`0x1`), a float added to the level it
derives, and is reached only from a fragment stage, the one stage with the coordinate
derivatives it derives that level from. A fetch, a sample and a gather take
`ConstOffset` (`0x8`), an integer constant added to the coordinate, or `Offset`
(`0x10`), the same computed at run time, one component per dimension of the
coordinate, and never on a `Cube` image. A gather may take `ConstOffsets` (`0x20`)
instead, a constant `[4]i32x2` of the offsets of the four texels it reads. An
instruction takes one offset bit at most. `Offset` and `ConstOffsets` need the
`ImageGatherExtended` extension, and Vulkan admits `Offset` outside a gather only under
its `maintenance8` feature, which enables no capability and so has no `ext` name and which
no Vulkan version requires, so a run-time `Offset` on any other image instruction is refused
at the call with `op.operand_value`, and a constant offset is `ConstOffset`. A sample takes
`MinLod` (`0x80`), the least level of detail it reads, which an explicit-lod sample
takes only with `Grad` and which needs the `MinLod` extension.
`OpImageGather` and `OpImageSparseGather` read one component, a constant id, of the four
texels a sample of a `2D` or `Cube` image would filter (Vulkan's
`VUID-StandaloneSpirv-OpImageGather-04664`), and a run-time component is refused at the
call with `op.operand_not_constant`. A constant operand is a constant by emission: an
argument the compiler evaluates at compile time, as it does a literal, `-1` or
`A | B` over module-scope `val`s, a vector or array literal whose every element is one,
as `i32x2{1, -1}`, passed directly or through a binding, a module-scope `val`, which a
shader reads as the constant it is, or an aggregate passed whole as a copy of a `val`,
written as one composite constant. An aggregate any run-time value writes, or one
written only in part, is a value, and is refused with `op.operand_not_constant`. An `OpImageRead` of a `SubpassData` image reads
the texel at the fragment's own place, so its coordinate is the constant vector (0, 0)
(Vulkan's `VUID-StandaloneSpirv-SubpassData-04660`): a run-time coordinate is refused at
the call with `op.operand_not_constant` and a constant with a component that is not zero
with `op.operand_value`. A `SubpassData` image is declared with `Arrayed` 0, `Sampled` 2
and the `Unknown` format (see [types.md](types.md)), is bound with its input attachment
index and is read from a fragment stage alone (see
[`gpu.attachment`](#gpuattachmentindex-and-subpass-inputs)). `OpImageQuerySamples` reads the sample
count of a multisampled image only, and `OpImageQuerySizeLod` does not take one.
`OpImageQuerySize` reads an image with no level of detail to choose, a multisampled
image, a storage image or a texel buffer, so a single-sampled sampled image is
queried with `OpImageQuerySizeLod` instead.
Each is refused at the call with `op.operand_value`.

```mach fragment
#[handle("spirv", "image", f32, "2D", 0, 0, 1, 1, "Unknown")]
pub def TextureMS;

#[intrinsic("spirv", "core", "OpImageFetch")]
fun fetch_sample(img: TextureMS, at: i32x2, mask: u32, sample: i32) f32x4;

#[intrinsic("spirv", "core", "OpImageQuerySamples")]
fun sample_count(img: TextureMS) i32;
```

```mach fragment
#[handle("spirv", "image", f32, "2D", 0, 0, 0, 2, "Rgba8")]
pub def Target2D;

#[intrinsic("spirv", "core", "OpImageWrite")]
fun image_write(img: Target2D, at: i32x2, texel: f32x4);

#[gpu.storage(0, 6, "writeonly")] var target: Target2D;
```

A **depth comparison** samples a depth image, one whose `Depth` operand is `1`
(see [types.md](types.md)), and compares each texel it reads against a reference
value, an `f32` scalar that follows the coordinate. `OpImageSampleDrefImplicitLod`
and `OpImageSampleDrefExplicitLod` filter the comparisons into one scalar of the
image's texel scalar, and `OpImageDrefGather` returns the comparisons of the four
texels a gather reads. Each takes the `Image Operands` its plain form takes, after
the reference, under the same rules: the implicit-lod comparison takes `Bias`,
an offset and `MinLod` and is reached only from a fragment stage, the explicit-lod
one always takes the mask with `Lod` or `Grad`, and the gather takes an offset or
`ConstOffsets` and reads only a `2D` or `Cube` image. A comparison of an image
whose `Depth` is `0` is refused with `op.operand_value`, and so is one of a `3D`
image, which Vulkan never compares (`VUID-StandaloneSpirv-OpImage-04777`).

```mach fragment
#[intrinsic("spirv", "core", "OpImageSampleDrefImplicitLod")]
fun shadow(s: ShadowSampler, uv: f32x2, depth: f32) f32;

#[intrinsic("spirv", "core", "OpImageSampleDrefExplicitLod")]
fun shadow_lod(s: ShadowSampler, uv: f32x2, depth: f32, mask: u32, lod: f32) f32;

#[intrinsic("spirv", "core", "OpImageDrefGather")]
fun shadow_gather(s: ShadowSampler, uv: f32x2, depth: f32, mask: u32, offset: i32x2) f32x4;
```

The projective forms, `OpImageSampleProj*` and `OpImageSampleProjDref*`, are rows
like any other. Each divides the coordinate, and a comparison's reference, by the
coordinate's last component before sampling, so its coordinate has one component
more than the plain form's, and SPIR-V admits no `Cube` or arrayed image for one.

`gpu.push` binds a **push-constant block**, a small `rec` the host supplies with the
command that records a dispatch or draw rather than through a descriptor, so it
takes no arguments: there is no set or binding to name. Like `gpu.uniform` and
`gpu.storage` it must be a `rec`, and it is emitted as a `Block`-decorated struct with
an explicit offset on every member. Its layout rules are std430-shaped, the same
ones a `gpu.storage` buffer follows and checked the same way, so `[8]f32` is fine in a
push block.

A push block is **read-only** in the shader. A store to it is a compile error on
every target, and on `spirv` so is a store the compiler follows through its address
handed to a function. Vulkan admits **one push-constant block per entry point**:
two push blocks used by the same stage are refused, naming both, while two stages
that each use a different one are accepted.

```mach
rec Params { scale: f32; slot: u32; }
#[gpu.push]
var params: Params;
```

Sampling a handle is an `#[intrinsic(...)]` declaration rather than a language form,
because a sample IS one SPIR-V instruction like `sqrt` and `dot` are:

```mach fragment
#[intrinsic("spirv", "core", "OpImageSampleImplicitLod")]
fun sample(s: Sampler2D, uv: f32x2) f32x4;

#[gpu.stage("fragment")]
fun frag_main() {
    out_colour = sample(albedo, in_uv);
}
```

The separately-bound form works the same way, with the instruction that combines
an image and a sampler declared alongside it:

```mach fragment
#[intrinsic("spirv", "core", "OpSampledImage")]
fun combine(t: Texture2D, s: Sampler) Sampler2D;

#[gpu.sampler(1, 0)] var base_tex: Texture2D;
#[gpu.sampler(1, 1)] var base_smp: Sampler;

#[gpu.stage("fragment")]
fun frag_sep() { out_colour = sample(combine(base_tex, base_smp), in_uv); }
```

The combined value is handed straight to the sample rather than named: SPIR-V
requires an `OpSampledImage` result be consumed by an image instruction in the block
that produced it, which is the same rule that makes a handle-typed local a compile
error. Returning one, or passing one to a function, is refused with
`op.result_flow`, and so is a sample whose later operand branches, as `&&` and `||`
do, since its sampled image would then be consumed in another block. Compute such an
operand into a binding before the call.

`gpu.shared` declares **workgroup memory**: one instance per workgroup of a compute
stage, which every invocation of that workgroup reads and writes. It applies to a
`var` only, since a `val` of workgroup memory could only ever read zero. It takes no
arguments, and the variable has no descriptor and no location, because the pipeline
never binds it.

```mach fragment
#[gpu.builtin("local_invocation")] var local_id: u32x3;
#[gpu.shared] var tile: [256]f32;

#[gpu.stage("compute")]
#[gpu.workgroup(64, 1, 1)]
fun blur() { tile[local_id[0]] = 1.0; }
```

Workgroup memory exists only in a compute stage, so a `#[gpu.shared]` variable used from
a vertex or fragment stage, directly or through a function the stage calls, is a
compile error naming the stage.

A `#[gpu.shared]` variable is **zero** when a compute stage starts, as every mach
variable is, on every environment. How depends on the environment:

- Where workgroup memory is zero-initialized by the consumer, the variable carries an
  `OpConstantNull` initializer. `vulkan1.3` guarantees that
  (`shaderZeroInitializeWorkgroupMemory` is core there). The feature enables no
  capability, so no `ext` name selects it for an earlier version (see
  [manifest.md](manifest.md#instruction-set-extensions)).
- Otherwise the compiler zeroes it itself, at the start of each compute stage that uses
  it. Each invocation stores zero to its own slice, the elements of an array its local
  invocation index reaches in steps of the workgroup size and the whole of any other
  type for invocation 0, and then the stage executes one workgroup `OpControlBarrier`.
  The barrier precedes all of the stage's own code, so every invocation reaches it.

Because the value on entry is always zero, a `#[gpu.shared]` variable cannot have an
initializer. Assign it inside the stage.

Workgroup memory is **coherent** among the invocations of a workgroup under either
memory model. Under GLSL450 it is so by definition. Under the Vulkan model an access
is private unless it says otherwise, and a barrier orders no private access between
invocations, so each load and store of a `#[gpu.shared]` variable carries
`MakePointerVisible` or `MakePointerAvailable` with `NonPrivatePointer` at the
`Workgroup` scope, the compiler's own zeroing stores included. A workgroup barrier
with acquire-release semantics then orders them as it does under GLSL450.

Whether a variable may carry an **initializer** is settled by its role, since the
role says who puts the first value in it:

| Role                                             | Initializer | Why                                                   |
|--------------------------------------------------|-------------|-------------------------------------------------------|
| `gpu.input`, a read built-in                         | refused     | the previous stage or the pipeline supplies the value |
| `gpu.uniform`, `gpu.storage`, `gpu.sampler`, `gpu.push`          | refused     | the host binds or supplies the memory                 |
| `gpu.shared`                                         | refused     | workgroup memory is zero when a stage starts          |
| `gpu.spec`                                           | required    | it is the default the pipeline keeps                  |
| `gpu.output`, a written built-in                     | allowed     | it is the value the variable starts at                |

A refused initializer is a compile error, because the value it writes would never be
the one the shader sees. An `gpu.output` or a written built-in starts at its
initializer, and at zero without one, as every mach `var` does:

```mach fragment
#[gpu.output(0)] var out_colour: f32x4 = f32x4{0.0, 0.0, 0.0, 1.0};
#[gpu.output(1)] var out_mask:   u32;
```

It starts there in each stage that reaches it, which is each stage that names it,
directly or through the functions it calls. A stage that does not reach an output
does not have it, so that stage writes nothing to the output's location, whatever
the initializer says. A fragment stage that writes only `out_mask` above has no
location 0 output.

On `spirv` the Output `OpVariable` carries that value as its initializer: the
constant the initializer spells, or `OpConstantNull` where it is zero or absent.
SPIR-V and Vulkan both admit an initializer on an Output variable. An output that no
entry point of the module reaches is in no interface list and the module holds no
`OpVariable` of it.

As with `#[gpu.stage(...)]`, these are accepted on every target and acted on only by a
target that forms pipeline stages. On `spirv` each becomes an `OpVariable` in the
matching storage class, carrying the matching decoration, and each entry point's
interface list names the Input and Output variables that stage reaches, directly or
through the functions it calls. A binding's class is
decided by its role and the type it holds. A `gpu.sampler` binding, which holds a
sampler or a sampled image, and a `gpu.storage` binding that holds a storage image
become `OpVariable`s in the `UniformConstant` class, the one class Vulkan permits a
handle variable in. A binding that holds a block becomes one in `Uniform` for
`gpu.uniform` and in `StorageBuffer` for `gpu.storage`. A handle is never a
`gpu.uniform` binding, and never sits in an array, both refused before code is made.
Every binding carries `DescriptorSet` and `Binding`. Two bindings may name one set and
binding: nothing refuses them, each variable carries that pair, and the module passes
`spirv-val`.
A `gpu.push` block becomes an `OpVariable`
in the `PushConstant` class, with no `DescriptorSet` or `Binding`. A `gpu.shared` variable
becomes an `OpVariable` in the `Workgroup` class, named in the interface of each entry
point that uses it from SPIR-V 1.4.

## `gpu.attachment(index)` and subpass inputs

A **subpass input** reads the attachment a previous subpass of the render pass wrote,
at the fragment's own place. It is a handle the target declares one, an `image` of
`Dim` `SubpassData` (see [types.md](types.md)), which is a storage image of the
`Unknown` format, so it binds through `gpu.storage` at its descriptor set and binding.
Beside that it states which input attachment it reads, the index into the subpass's
`pInputAttachments`, by `#[gpu.attachment(index)]`:

```mach
#[handle("spirv", "image", f32, "SubpassData", 0, 0, 0, 2, "Unknown")]
pub def SubpassInput;

#[intrinsic("spirv", "core", "OpImageRead")]
fun subpass_load(img: SubpassInput, at: i32x2) f32x4;

#[gpu.storage(0, 0)] #[gpu.attachment(0)] var albedo: SubpassInput;
#[gpu.storage(0, 1)] #[gpu.attachment(1)] var normal: SubpassInput;
#[gpu.output(0)] var colour: f32x4;

#[gpu.stage("fragment")]
fun main() {
    colour = subpass_load(albedo, i32x2{0, 0}) * subpass_load(normal, i32x2{0, 0});
}
```

The index is an integer constant, an unsigned 32-bit one like a descriptor set, a
binding, a location or a specialization id, and a negative or wider value is refused
with `decorator.argument`. On `spirv` it becomes the variable's `InputAttachmentIndex`
decoration. The decorator is **required** on a subpass input: Vulkan matches a subpass
input to its attachment by that decoration alone, and requires it of every subpass
input outside dynamic rendering with `dynamicRenderingLocalRead`
(`VUID-RuntimeSpirv-None-09558`). The index is written even there, where it names the
attachment the host maps to it. A subpass input without one is refused at its
declaration with `handle.unbound`, which `spirv-val` does not check. The decorator is
refused with `decorator.misplaced` anywhere else: on a declaration that is no
module-level `val` or `var`, on a binding of an ordinary type, and on a handle the
target does not declare a subpass input, as the target's handle catalog states.
Vulkan places `InputAttachmentIndex` only on a variable in `UniformConstant`
(`VUID-StandaloneSpirv-InputAttachmentIndex-06678`), where every subpass input lives.
A subpass input never sits in an array, as no handle does, so the consecutive indices
Vulkan gives the elements of an array of subpass inputs, from the one stated, do not
arise. The index must be below the device's `maxPerStageDescriptorInputAttachments`,
which the compiler cannot know.

A subpass input exists only in a **fragment stage**: SPIR-V's `OpTypeImage` admits
`SubpassData` in the `Fragment` execution model alone, the one stage with a place in
the attachment to read. A vertex or compute stage that reaches a subpass input,
directly or through a function it calls, is refused with `spirv.interface`, naming
the stage, its entry point and the binding, as a built-in reached outside its stages
is. A module may still declare one beside a compute stage that does not reach it.

## `gpu.spec(id)` — specialization constants

A specialization constant is a value the host supplies when it creates the
pipeline, after the shader has been compiled. It is declared as a module-level
`var` carrying the constant's id, and its initializer is the default the pipeline
keeps when the host supplies nothing for that id. The initializer is required:

```mach
#[gpu.spec(0)]
var tile_size: u32 = 64;
#[gpu.spec(1)]
var gain:      f32 = 0.5;
```

It is a `var` like every other value the host supplies, and that settles how the
compiler treats it:

- It is **never a compile-time value**. It cannot be an array length or a comptime
  operand, since what it holds is decided after the build. A `#[gpu.spec]` on a `val`
  is refused for the same reason.
- The optimizer **never folds it to its initializer**, even when nothing in the
  module writes it. A mutable global is never replaced by its initial value, and
  that is exactly what keeps the host's value live.
- A **store to it is refused**, naming the line that wrote it. On the GPU it is a
  constant once the pipeline exists, so there is nothing to write. Copy it into a
  local to change the value. **Taking its address is refused** too, on every target:
  where a target states it each read is the constant itself, so there is no storage
  for an address to name, and an address handed on would count as a store. Pass the
  value, or copy it into a local and take that address.

The type must be a scalar integer or float. There is no boolean specialization
constant, because the compiler knows no boolean type: `bool` is an alias
of `u1`. Write a flag as an integer spec var, which the host sets with the same 4
bytes as a `VkBool32`:

```mach
#[gpu.spec(3)]
var use_fog: u32 = 1;
```

A narrower integer such as `u8` works too, and is carried like any other `u8` in a
shader: under the capability for its width it is a constant of its own width, and
without it a 32-bit one, so the host supplies as many bytes as the constant the
module states. On `spirv` each one becomes an
`OpSpecConstant` whose literal is the initializer, decorated with `SpecId`, and a
read uses that constant directly with no load. Two `#[gpu.spec]` vars with one id that
one SPIR-V module reads are refused, whichever source modules declare them, since the
host names the constant by its id. On a
machine target the decorator has no effect and the var is an ordinary global.

A `#[gpu.spec]` var may also size a compute workgroup (see
[`gpu.workgroup`](#gpuworkgroupx-y-z--compute-workgroup-dimensions)).

## `handle(target, constructor, operands...)` — a type the target mints

A bodyless `def` carrying this decorator declares a type whose representation is
**not the program's**: the owning target mints it and the pipeline binds it.

```mach fragment
#[handle("spirv", "image", f32, "2D", 0, 0, 0, 1, "Unknown")]
pub def Texture2D;

#[handle("spirv", "sampled_image", Texture2D)]
pub def Sampler2D;
```

The first argument names the target and the second the type constructor within it.
Both are constant strings, which name the kind the target mints as `target.constructor`, such as `spirv.image`, in mirl's handle catalog of the selected target.
Everything after them is **operands to that
constructor**, never rule knobs: the rules a handle carries are fixed and closed
(see [types.md](types.md)) and never vary per declaration.

An operand is an ordinary comptime constant, an integer where the target's row
takes a number and a string naming one of its grammar's enumerants where the row
takes an enumerant (`"2D"`, `"Rgba8"`), except where the kind's part
takes a **type**: there the operand is a type name, the only place a decorator
argument is read as a type rather than as a value. An image's texel and a
cooperative matrix's component are a scalar written as itself (`f32`) or through an
alias of one, and a
constructor that composes over another handle names that handle, so a composing
declaration names what it wraps instead of restating it, which keeps the two from
disagreeing. The named type must be a handle the same target mints, of the kind that
position requires.

A declaration addressed to a target this build did not select is **inert**: it
still denotes a type and still sizes at the target's pointer width, so a library of
handles compiles on a machine target. A constructor name the selected target does
not define, an operand count that disagrees with the constructor's, or an operand
combination the target cannot emit is a compile error at the declaration.

## `intrinsic(target, set, name)` — a function that *is* a target instruction

A shader needs `sqrt`, `normalize`, `dot` and `mix`. None of them is an operator,
and none of them is a call SPIR-V can make: each is one instruction. This decorator
says which one a function is, so that on a `spirv` target a call to it becomes that
instruction, inline, rather than a call.

```mach
#[intrinsic("spirv", "GLSL.std.450", "Sqrt")]
pub fun sqrt(x: f32) f32;

#[intrinsic("spirv", "GLSL.std.450", "Normalize")]
pub fun normalize(v: f32x4) f32x4;

#[intrinsic("spirv", "core", "OpDot")]
pub fun dot(a: f32x4, b: f32x4) f32;
```

The first argument names the **target**, the second the instruction set, and the
third the instruction within it. The target is the ISA name the manifest selects
with, so nothing about this decorator is specific to one back end. The reserved
first argument `"mirl"` instead names one of mirl's own operations on every
target, which [decorators.md](decorators.md#intrinsicowner-set-name--a-function-that-is-an-instruction-or-an-operation)
describes. The rest of this section is the target form. The arguments
must be strings on every target, but the instruction set and name are checked only
when the named target is the one selected: a declaration for any other target is
inert, and that target's table is not consulted. When it is checked, the parameter
count is held to the instruction's own operand count, which is not uniform across a
family that looks it, and a count outside it is refused with `op.operand_count` at
the declaration, where it is read against the target's table whether or not anything
calls it.

| Set              | Meaning                                                     |
|------------------|-------------------------------------------------------------|
| `"core"`         | the core opcode space; needs no import                       |
| `"GLSL.std.450"` | the standard extended set; imported once per module, on use  |

The substitution is uniform: the emitted instruction's **result type is the
function's declared return type** and its **operands are the function's parameters
in declaration order**. That is what lets `dot` and `length` return a scalar from
vectors, and `refract` mix a scalar operand with vector ones, without any of them
being a special case.

Each instruction's row in the target's table also says **how each operand is
written** and **whether the instruction has a result**, and when the target is
selected the declaration's types are checked against both:

| Kind            | The operand is                                              | Parameter type |
|-----------------|-------------------------------------------------------------|----------------|
| value           | an ordinary id, the argument's value                        | not a pointer  |
| constant id     | an id that must be an integer constant by emission, such as a `Scope` or `MemorySemantics` | an integer |
| literal         | a constant written inline as a literal word, such as an image-operands mask | an integer |
| pointer read    | the argument's address, only read through                   | a pointer      |
| pointer write   | the argument's address, only stored through                 | a pointer      |
| pointer update  | the argument's address, read and written (read-modify-write) | a pointer     |
| handle read     | a handle whose memory the instruction reads, such as a storage image's texels | a handle |
| handle write    | a handle whose memory the instruction writes                | a handle       |
| truth value     | a predicate the instruction takes as SPIR-V's boolean, true where the argument is nonzero | an integer |

A **pointer operand takes its storage class from the call site**: the argument's
own access chain decides whether it points into a storage buffer, workgroup memory,
a function-local object or an image, since on its target an `intrinsic` is the instruction
and never a body, so there is no boundary at which its parameter's pointer could be given one. Any access chain is accepted, a
member or element as well as a whole object. The kinds are also what the
`"readonly"` and `"writeonly"` qualifiers of a `gpu.storage` binding are checked
against: an atomic load through a `readonly` binding is accepted and an atomic add
on it is refused, and an atomic store into a `writeonly` binding is accepted and an
atomic load from it is refused. A handle passed as an ordinary value names its
descriptor and touches none of its memory, and a handle read or write is checked the
same way, so `OpImageWrite` into a `"readonly"` storage image and `OpImageRead` from a
`"writeonly"` one are refused. An atomic through a texel pointer `OpImageTexelPointer` makes is an
access of the storage image the pointer points into, so an atomic add through one
into a `"readonly"` image is refused, and so is any atomic but a store through one
into a `"writeonly"` image.

A non-constant argument to a constant id or a literal is refused at the call,
naming the operand. A row **without a result** is declared with no return type,
and a row with one must return it. A row may also **return a pointer** into a
storage class the row itself declares, as `OpImageTexelPointer` returns an `Image`
pointer, and that result is accepted as a later instruction's pointer operand. No call
passes a pointer into the `Image` class (SPIR-V's logical addressing takes none as a
function's argument, with or without variable pointers), so passing a texel pointer to
a function is refused at the call with `spirv.unsupported`. A row whose result is a
**structure**, as each `OpImageSparse` instruction gives its residency code and its
texels, is declared returning a `rec` of those members, which receives the structure.
Where the typing data states the members, the `rec` must have exactly those members, each
of the type its rule asks, as `FrexpStruct`'s significand and exponent. A
row whose result is a **truth value**, such as `OpGroupNonUniformElect`, is declared
returning an integer, which receives 1 or 0, and one that gives a truth value per lane,
as `OpIsNan` over a vector, returning an integer vector of those lanes. A `bool` return is a
`u1`, which the module states as SPIR-V's boolean type, so it needs no `Int8`.

```mach
#[intrinsic("spirv", "core", "OpControlBarrier")]
pub fun barrier(execution: u32, memory: u32, semantics: u32);

#[intrinsic("spirv", "core", "OpMemoryBarrier")]
pub fun memory_barrier(memory: u32, semantics: u32);

#[intrinsic("spirv", "core", "OpAtomicIAdd")]
pub fun atomic_add(p: *u32, scope: u32, semantics: u32, v: u32) u32;
```

On the target that owns the instruction, a decorated function **is the
instruction and never its body**, so a call to it is never inlined away or
deleted, and the optimizer treats it as reading and writing all memory. No
load or store is moved across a barrier or an atomic, at any optimization
level. `OpControlBarrier` takes an execution scope, a memory scope and memory
semantics, and `OpMemoryBarrier` a memory scope and semantics, each an integer
constant. A memory scope and memory semantics are held to the module's memory model:
under the Vulkan model the `Device` scope needs the `VulkanMemoryModelDeviceScope`
extension, declared where the target selects it and refused with `spirv.capability`
naming it where it does not, and under GLSL450 the `QueueFamily` scope and the
`MakeAvailable`, `MakeVisible` and `Volatile` semantics need `VulkanMemoryModel`,
which a target selecting it would have written under the Vulkan model, so each is
refused with `spirv.capability` naming it (see
[manifest.md](manifest.md#finished-module-targets)). An atomic's scope and semantics
are held the same way, the compiler's own atomics included, which take the `Device`
scope. The execution scope is not, since a Vulkan barrier executes
at `Workgroup` or `Subgroup` only. `SequentiallyConsistent` semantics are refused under
either model, since Vulkan defines no sequentially consistent order
(VUID-StandaloneSpirv-MemorySemantics-10866): use `Acquire`, `Release` or
`AcquireRelease`.

A control barrier must be reached in **uniform control flow**: every
invocation of its execution scope executes it, or none does. That is the
program's obligation, because whether a branch is
uniform is not statically decidable in general. The compiler does not check
it, and a barrier inside a branch or loop that some invocations of the scope
skip is undefined behavior on the device.

A row may also carry **requirements**: a capability and the extensions of the
target's vocabulary that every use of it needs. A **literal operand can be
enumerated**, so that its value is one of a closed set the row names (or, for a
mask, a union of that set's bits), and each value brings a requirement of its own
and, where the instruction grows with it, operands at the end of the instruction.
Such a row has an **optional tail**: a declaration may take its required operands
alone or the tail too, and each call must pass exactly the operands its literal's
value brings. `OpGroupNonUniformIAdd` is one:

```mach
#[intrinsic("spirv", "core", "OpGroupNonUniformIAdd")]
pub fun subgroup_add(scope: u32, operation: u32, v: u32) u32;

#[intrinsic("spirv", "core", "OpGroupNonUniformIAdd")]
pub fun subgroup_cluster_add(scope: u32, operation: u32, v: u32, cluster_size: u32) u32;
```

Its operation is a `GroupOperation`. `Reduce` (0), `InclusiveScan` (1) and
`ExclusiveScan` (2) need the `GroupNonUniformArithmetic` extension and declare it,
and `ClusteredReduce` (3) needs and declares `GroupNonUniformClustered` and is followed by the ClusterSize operand, so
it is passed only to the four-parameter declaration.

Where the specification makes the literal itself optional, as it does an
instruction's `Image Operands` or `Memory Operands`, the literal **leads the tail**:
a declaration leaves it out with every operand it would bring, or takes it followed
by those operands. A mask's set bits bring theirs in ascending bit order, the order
the specification writes them in, so the parameters after the mask are declared in
that order. Each value types the operands it brings, so `Grad` brings two values and
`ConstOffset` one constant wherever they land after the mask, and a parameter
receiving one is held to its kind at the call rather than at the declaration. Each is
checked at the call, where the literal's value is known:

| At the call                                            | Is refused with                    |
|--------------------------------------------------------|------------------------------------|
| a value outside the operand's enumeration              | `op.operand_value`, naming the values |
| a value whose operands the declaration does not pass, or passes without it | `op.operand_value`, naming the count |
| a parameter the value's operand kind does not admit     | `op.signature`, naming the operand |
| an instruction whose own row requires what the calling function's selection does not hold, as a subgroup operation below SPIR-V 1.3 | `spirv.capability`, naming what it needs |
| a requirement's extension the target does not select   | `spirv.capability`, naming the extension |
| a capability whose SPIR-V version the environment is below | `spirv.capability`, naming the SPIR-V version it needs, as `spv1.3` |

A device feature is an extension the target names in its `ext` once the
consumer enables it, since no environment guarantees it: `GroupNonUniformArithmetic` is
enabled by Vulkan's `VK_SUBGROUP_FEATURE_ARITHMETIC_BIT`, and a target naming no `env` holds
every device feature these pages name ([manifest.md](manifest.md#finished-module-targets)). A module declares a capability only when something it emits needs
it, with `OpExtension` for a capability a SPIR-V extension defines.

A call that passes a pointer into a `#[gpu.storage]` binding, or into any part of
one, needs the device feature `VariablePointersStorageBuffer`, Vulkan's
`variablePointersStorageBuffer`, since logical addressing lets a call pass a
storage buffer pointer only under that capability (SPIR-V 2.16.1). A target that
names it in its `ext` declares the capability, with `SPV_KHR_variable_pointers`
below SPIR-V 1.3, and one that does not is refused with `spirv.capability`, naming
it. The callee is never inlined to avoid it.

Logical addressing limits what else a call may be passed (SPIR-V 2.16.1). A
pointer argument may be:

- a whole local, a whole module-scope `var` or `#[gpu.shared]` variable, or a
  pointer parameter passed on, which needs nothing;
- a pointer into a `#[gpu.storage]` binding, whole or any part of it, under
  `VariablePointersStorageBuffer` as above;
- a part of a `#[gpu.shared]` variable, a member or an element, under
  `VariablePointers`, Vulkan's `variablePointers`, declared the same way and
  refused the same way without it.

Nothing else may be passed: neither a part of a local or of a module-scope `var`,
nor a pointer into a `#[gpu.uniform]`, `#[gpu.push]`, `#[gpu.input]` or
`#[gpu.output]` variable, whole or part. Such a call is refused with
`spirv.unsupported`, naming the storage class, and the program passes the value
instead or reaches the binding in the callee.

A pointer parameter takes its SPIR-V type from what its calls pass. A whole
`#[gpu.storage]` or `#[gpu.uniform]` binding is a `Block`-decorated structure, and a
member or an element of one, of the same record type, is the plain structure, so the
two are different pointer types. A parameter every call passes a whole binding is a
pointer to the block, and one every call passes a member or an element is a pointer
to the plain structure, whether the argument is reached directly or through another
function's parameter. A function no call reaches takes the block for a record in a
class that holds blocks. A function one call passes a whole binding and another a
member or an element of one for the same parameter is refused with
`spirv.unsupported` at the second call. The program splits it into one function for
each kind of argument.

A row may also be **typed**: its requirement depends on the type it operates on, read
from one operand (a pointer's pointee), and for a pointer on the storage class that
operand's memory lives in. The atomics and the integer dot products are typed, and two
rows whose result may take one of two shapes are typed by their result alone:
`OpArrayLength` and `OpReadClockKHR`. A
declaration whose pointee the row admits in no storage class, or whose operand or
result is a type the row admits none of, is refused with `op.signature` at the
declaration, and each call is checked where its storage class is known. A load reads its pointer and every other atomic writes it, so
a `"readonly"` binding admits only an atomic load.

```mach
#[intrinsic("spirv", "core", "OpAtomicIAdd")]
pub fun atomic_add64(p: *u64, scope: u32, semantics: u32, v: u64) u64;

#[intrinsic("spirv", "core", "OpAtomicFAddEXT")]
pub fun atomic_fadd(p: *f32, scope: u32, semantics: u32, v: f32) f32;
```

An atomic operates on buffer memory, workgroup memory or a storage image texel, the
storage classes Vulkan admits one in (`VUID-StandaloneSpirv-None-04686`), and one on
any other memory, a local or a module-scope `var` among it, is refused with
`spirv.capability`, naming the class. A 32-bit integer atomic is core in each of
them. Every other type needs the
capability of its row, named in the target's `ext`, and the consumer enables the
Vulkan device feature of its storage class that provides it, named for its
`shaderBuffer*`, `shaderShared*` or `shaderImage*` member: buffer memory
(`StorageBuffer`, `Uniform` before SPIR-V 1.3, or `PhysicalStorageBuffer` through a
[physical pointer](types.md#pointers-on-spir-v)), [`#[gpu.shared]`](#gpuinputn--gpuoutputn--gpubuiltinstr--gpuuniformset-binding--gpustorageset-binding--gpusamplerset-binding--gpupush--gpuspecid--gpushared--shader-interface)
workgroup memory, or a storage image texel (`Image`, through
`OpImageTexelPointer`), where Vulkan defines only a 64-bit integer and an `f32`. Any
other storage class is refused. An image of 64-bit texels (`R64ui` or `R64i`) declares
`Int64ImageEXT` (`SPV_EXT_shader_image_int64`), which `shaderImageInt64Atomics`
enables, so the image itself needs that capability whatever reaches it. An `f16` atomic also needs
`Float16`, under which `f16` memory is the `OpTypeFloat 16` the atomic operates on, and
the storage buffer holding one needs `StorageBuffer16BitAccess`
([manifest.md](manifest.md#finished-module-targets)).

| Rows | Type | Vulkan feature | `ext` (capability, SPIR-V extension) |
|------|------|----------------|--------------------------------------|
| all 15 integer atomics | `u32`, `i32` | core | none |
| all 15 integer atomics | `u64`, `i64` | `shaderBufferInt64Atomics`, `shaderSharedInt64Atomics`, `shaderImageInt64Atomics` | `Int64Atomics` |
| `OpAtomicLoad`, `OpAtomicStore`, `OpAtomicExchange` | `f16`, `f32`, `f64` | `shader{Buffer,Shared}Float{16,32,64}Atomics`, `shaderImageFloat32Atomics` | none |
| `OpAtomicFAddEXT` | `f32`, `f64` | `shader{Buffer,Shared}Float{32,64}AtomicAdd`, `shaderImageFloat32AtomicAdd` | `AtomicFloat{32,64}AddEXT`, `SPV_EXT_shader_atomic_float_add` |
| `OpAtomicFAddEXT` | `f16` | `shader{Buffer,Shared}Float16AtomicAdd` | `AtomicFloat16AddEXT`, `SPV_EXT_shader_atomic_float16_add` |
| `OpAtomicFMinEXT`, `OpAtomicFMaxEXT` | `f16`, `f32`, `f64` | `shader{Buffer,Shared}Float{16,32,64}AtomicMinMax`, `shaderImageFloat32AtomicMinMax` | `AtomicFloat{16,32,64}MinMaxEXT`, `SPV_EXT_shader_atomic_float_min_max` |

The features come from `VkPhysicalDeviceShaderAtomicInt64Features`,
`VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT`,
`VkPhysicalDeviceShaderAtomicFloatFeaturesEXT` and
`VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT`. No environment guarantees any of
them, so a target names the capability of each one its consumer enables, and a target
naming no `env` holds them all. The integer atomics are `OpAtomicLoad`, `OpAtomicStore`,
`OpAtomicExchange`, `OpAtomicCompareExchange`, `OpAtomicIIncrement`,
`OpAtomicIDecrement`, `OpAtomicIAdd`, `OpAtomicISub`, `OpAtomicSMin`, `OpAtomicUMin`,
`OpAtomicSMax`, `OpAtomicUMax`, `OpAtomicAnd`, `OpAtomicOr` and `OpAtomicXor`.

| At the call                                            | Is refused with                    |
|--------------------------------------------------------|------------------------------------|
| a type the row admits only in other storage classes    | `spirv.capability`, naming the class |
| a type whose capability the target does not select     | `spirv.capability`, naming the capability |

The six integer dot products (`OpSDot`, `OpUDot`, `OpSUDot` and their `AccSat` forms)
are typed by the shape of Vector 1, which Vector 2 shares: a 32-bit scalar is the
packed form, four 8-bit components in one integer, read under its Packed Vector Format
operand. Each shape needs an input capability beside `DotProduct` (SPIR-V 1.6, OpSDot:
"32-bit integers (enabled by the DotProductInput4x8BitPacked capability) or vectors of
integer type (enabled by the DotProductInput4x8Bit or DotProductInputAll
capability)"). A call declares the narrowest capability the target's `ext` holds that
covers its shape, and is refused with `spirv.capability`, naming the narrowest one,
where the target holds none of them. Vulkan's `shaderIntegerDotProduct` feature
(`VkPhysicalDeviceVulkan13Features`, or `VkPhysicalDeviceShaderIntegerDotProductFeaturesKHR`
under `VK_KHR_shader_integer_dot_product`) enables all four capabilities.

| Vector 1 | `ext`, the narrowest first |
|----------|----------------------------|
| `u32` or `i32`, packed | `DotProductInput4x8BitPacked` |
| `i8x4` or `u8x4` | `DotProductInput4x8Bit`, which implies `Int8`, or `DotProductInputAll` |
| any other integer vector | `DotProductInputAll` |

An 8-bit vector on a target that holds neither `Int8` nor `DotProductInput4x8Bit` is
carried in 32-bit components ([manifest.md](manifest.md#finished-module-targets)), so it
needs `DotProductInputAll`.

A row also states how its operands' types and its result's **relate** to the type it
operates on, the type of the one operand its typing reads (a pointer's pointee). A
declaration that breaks a relation is refused with `op.signature`, naming both the
parameter (or the return type) and the operand it relates to, so a mismatch is caught
at the declaration rather than as an invalid module, whether or not anything calls it.

A row holds a declaration's signedness only where the instruction reads its operands or
result as signed or as unsigned, so that the other signedness would compute another
value than the declaration says: the signed and unsigned minimum, maximum, clamp,
absolute value, sign and most significant bit, the signed and unsigned comparisons and
bit-field extracts, the signed and unsigned atomic and subgroup minimum and maximum, the
carry and borrow, and the result of the signed, unsigned and mixed integer dot products. Everywhere else a row asks what the specification asks, the
class, width and component count and the sameness of a type, and accepts either
signedness: the module writes an integer of any declared signedness unsigned, but for
the sampled type and texels of an image of a signed integer format, which it converts
at the instruction, so a count, index, id, scope, ballot or address, and an operand
that must be the result's type, meets the specification's rule whatever signedness it
is declared with.

| Rows | Relation |
|------|----------|
| every atomic | the result and each value operand are the pointer's pointee, of either signedness but for the signed and unsigned minimum and maximum |
| `OpImageRead`, `OpImageFetch`, `OpImageSample*Lod`, `OpImageGather` | the result is a 4-vector of the image's texel scalar, a sampled image's being its image's |
| `OpImageSampleDref*Lod` | the result is a scalar of the image's texel scalar, and the reference is an `f32` scalar |
| `OpImageDrefGather` | the result is a 4-vector of the image's texel scalar, and the reference is an `f32` scalar |
| `OpImageWrite` | the texel is a scalar or vector of the image's texel scalar, with at least as many components as the image's format stores (any for `Unknown`) |
| `OpImageTexelPointer` | the result points to the image's texel scalar, into a storage image of `R32ui`, `R32i`, `R32f`, `R64ui` or `R64i` format |
| `OpSampledImage` | the result is a sampled image composed over the image operand's own type, and the second operand is a `gpu.sampler` |
| `OpGroupNonUniformBroadcast*`, `Shuffle*`, `Quad*` and the arithmetic rows | the result is the value operand's type |
| the GLSL.std.450 math rows, float and integer | the result and every operand are the first operand's type, except `Refract`'s `eta`, and `Length` and `Distance`, whose result is a scalar |
| `Modf` | the result is the value's type, and the out-pointer points to the value's type, where the whole part is stored |
| `Frexp` | the result is the value's type, and the out-pointer points to an `i32` scalar or vector with as many components as the value, where the exponent is stored |
| `OpDot` | the second vector is the first's type |
| `OpSDot`, `OpUDot`, `OpSUDot` and their `AccSat` forms | the result is a signed integer scalar (unsigned for `OpUDot` and `OpUDotAccSat`) at least as wide as the components of the first vector where it is a vector, the second vector is the first's type (`OpSUDot`: its component count and width, of either signedness), and the accumulator is the result's type |
| `OpImage` | the sampled image's image is the result's type |
| `OpIsNan`, `OpIsInf` | the result is a truth value per lane of the operand, a scalar or vector of floats |
| `OpSMod`, `OpExpectKHR` | the result and every operand have the first operand's component count and width, integers of either signedness |
| `OpFMod`, `NMin`, `NMax`, `NClamp` | the result and every operand are the first operand's type, a scalar or vector of floats |
| `OpQuantizeToF16` | the result is the operand's type, a scalar or vector of 32-bit floats |
| `ModfStruct` | the result is a `rec` of two members, each the operand's type, a scalar or vector of floats |
| `FrexpStruct` | the result is a `rec` of two members, the operand's type, a scalar or vector of floats, and 32-bit integers of its component count |
| `OpIAddCarry`, `OpISubBorrow` | the result is a `rec` of two members, each the operands' type, unsigned integers |
| `OpArrayLength` | the result is a `u32` or `u64` scalar, and the structure a pointer to a `rec` whose last member is a run-time array, `[0]T`. The member operand, the index of that last member, is not held |
| `OpReadClockKHR` | the result is a `u64` scalar or a `u32x2` vector |
| `OpGroupNonUniformRotateKHR` | the result has the value's component count, class and width, the delta is an integer scalar, and the ClusterSize an integer constant power of two |
| `OpSubgroupFirstInvocationKHR` | the result has the value's class and width, a 32-bit integer or float scalar |
| `OpSubgroupBallotKHR` | the result is a 4-vector of 32-bit integers, and the predicate a truth value |
| `OpSubgroupAllKHR`, `OpSubgroupAnyKHR`, `OpSubgroupAllEqualKHR`, `OpGroupNonUniformQuadAllKHR`, `OpGroupNonUniformQuadAnyKHR` | the result and the predicate are truth values |
| `OpIsHelperInvocationEXT` | the result is a truth value, in a fragment stage |
| `OpAssumeTrueKHR` | the condition is a truth value |
| `OpImageSampleProj*Lod` | as the sample rows, with a coordinate of 32-bit float components carrying the projective divisor |
| `OpImageQueryLod` | the result is a 2-vector of floats, and the coordinate is 32-bit floats |
| the `OpImageSparse*` rows | the coordinate is 32-bit floats (integers for `OpImageSparseFetch`, either for `OpImageSparseRead`), and the reference an `f32` scalar; the result structure is not held |
| `OpControlBarrier`, `OpMemoryBarrier`, every atomic and every `OpGroupNonUniform*` row | each scope and memory semantics operand is a 32-bit integer scalar constant (SPIR-V's `Scope <id>` and `Memory Semantics <id>`, and 2.16.2) |

A relation on a pointer operand, such as the out-pointer `Modf` and `Frexp` store
their second part through, holds the type it points to. The pointer may address a
local, a `#[gpu.shared]` variable or a storage buffer, or any part of one, and the store
counts as a write to that binding, so a `readonly` one is refused.

```mach
#[intrinsic("spirv", "GLSL.std.450", "Frexp")]
pub fun frexp(x: f32x4, exp: *i32x4) f32x4;
```

A declaration that returns a handle is refused with `op.signature` unless its row
names the operand its result derives from, since a handle holds a binding's
descriptor and only that operand says whose. Each math row and subgroup row states the
class of number it operates on. The GLSL.std.450 float rows and `OpDot` operate on a
scalar or vector of floats, the trigonometric and exponential rows, `Pow`, `Radians` and
`Degrees` on floats of 16 or 32 bits only, the pack and unpack rows on the 32-bit and 64-bit
types their names state, `FindILsb`, `FindSMsb`, `FindUMsb` and the `InterpolateAt` rows on
32-bit components, and the `InterpolateAt` rows execute in a fragment stage alone. The
core bit rows `OpBitCount`, `OpBitReverse`, `OpBitFieldInsert`, `OpBitFieldSExtract` and
`OpBitFieldUExtract` take a `Base` of 32-bit integer components, which Vulkan's
`VUID-RuntimeSpirv-None-10824` requires unless the device enables `maintenance9`, a feature
no Vulkan version guarantees, and their offset and count are integer scalars. The
subgroup rows on a scalar or vector of integers or
floats. The GLSL.std.450 integer rows operate on the signedness their name states:
`SAbs`, `SSign`, `SMin`, `SMax`, `SClamp` and `FindSMsb` on signed integers, `UMin`,
`UMax`, `UClamp` and `FindUMsb` on unsigned ones, and `FindILsb` on either. A subgroup
row's lane, mask, delta, index and direction operands, its ballot and the counts and bit
positions it returns are 32-bit integers of either signedness, which the module writes
with the Signedness of 0 SPIR-V states. A data
operand of any other type is refused with `op.signature`: an integer for `FAbs`, an
`i32` for `UMin`, a handle, a pointer or an aggregate. GLSL.std.450 removed `IMix`, which
its grammar still lists: masc's typing data states that no environment admits it, so a call
of it is refused with `op.unknown`, quoting the removal. A declaration nothing calls builds.

Each image row names the rule its texel's component count comes from, and the
refusal quotes it: the 4-vector read result is Vulkan's `VUID-StandaloneSpirv-Result-04780`,
the 4-vector fetch, sample and gather results and the scalar depth-comparison result are SPIR-V's own, and the write's count against the
format is Vulkan's `VUID-RuntimeSpirv-OpImageWrite-07112`, which spirv-val cannot check
because it sees no `VkFormat`.


The **subgroup operations** are the `OpGroupNonUniform*` rows, each taking the
Subgroup scope (3) as its first operand. Every one needs SPIR-V 1.3, so `vulkan1.1`
or later, and each family needs its capability, named in the target's `ext`, which
the consumer enables through the feature Vulkan reports it by:

| Family            | Rows                                                         | `ext`                       |
|-------------------|--------------------------------------------------------------|-----------------------------|
| basic             | `Elect`                                                      | none: every vulkan1.1 device |
| vote              | `All`, `Any`, `AllEqual`                                     | `GroupNonUniformVote`             |
| arithmetic        | `IAdd`, `FAdd`, `IMul`, `FMul`, `SMin`, `UMin`, `FMin`, `SMax`, `UMax`, `FMax`, `BitwiseAnd`, `BitwiseOr`, `BitwiseXor`, `LogicalAnd`, `LogicalOr`, `LogicalXor` with `Reduce` or a scan | `GroupNonUniformArithmetic` |
| clustered         | the same rows with `ClusteredReduce` and a ClusterSize       | `GroupNonUniformClustered`        |
| ballot            | `Ballot`, `InverseBallot`, `BallotBitExtract`, `BallotBitCount`, `BallotFindLSB`, `BallotFindMSB`, `Broadcast`, `BroadcastFirst` | `GroupNonUniformBallot` |
| shuffle           | `Shuffle`, `ShuffleXor`                                      | `GroupNonUniformShuffle`          |
| relative shuffle  | `ShuffleUp`, `ShuffleDown`                                   | `GroupNonUniformShuffleRelative` |
| quad              | `QuadBroadcast`, `QuadSwap`                                  | `GroupNonUniformQuad`             |

`BallotBitCount` takes a `GroupOperation` too, which its ballot capability covers
and which has no clustered form. A clustered row's ClusterSize is an integer scalar of
either signedness, a constant, and a power of two, so at least 1 (SPIR-V 1.6, the
arithmetic group operations): a float or vector is refused with `op.signature`, a run-time value with
`op.operand_not_constant` and another constant with `op.operand_value`. The rule is
masc's typing data on the optional operand, read by mirl. A ClusterSize larger than the
device's subgroup is undefined behaviour SPIR-V leaves to the program, since the size is
the device's. `QuadSwap`'s direction is a constant id in every SPIR-V
version. `Broadcast`'s lane and `QuadBroadcast`'s index are constant ids before SPIR-V 1.5,
so under `vulkan1.1`, and from 1.5 on may be run-time values, which must then be
dynamically uniform (Vulkan admits them under `subgroupBroadcastDynamicId`, which every
Vulkan 1.2 device has). A run-time value where a constant id is asked is refused at the
call with `op.operand_not_constant`. The rule is masc's typing data, read by mirl. Vulkan
guarantees subgroup operations only in compute stages, so a use reached from a
vertex or fragment stage, an operation or a subgroup built-in alike, also needs the
device's `subgroupSupportedStages` to include that stage, which enables no capability
and so has no `ext` name.

```mach
use std.types.bool.bool;

#[intrinsic("spirv", "core", "OpGroupNonUniformBallot")]
pub fun subgroup_ballot(scope: u32, predicate: bool) u32x4;

#[intrinsic("spirv", "core", "OpGroupNonUniformShuffleXor")]
pub fun subgroup_shuffle_xor(scope: u32, v: u32, mask: u32) u32;
```

`OpExtInstImport "GLSL.std.450"` is emitted **once per module and only when that
module uses the set**. A module that calls none of these carries no import.

Note that `dot` is **core `OpDot`**, not a GLSL.std.450 instruction, while `normalize`
and `length` are the extended set's `Normalize` and `Length`. Check each function's
set against the specification.

On every target other than `spirv` the decorator is inert, and a decorated function
is an ordinary function. A **bodiless** one, which is what the shader-side maths
library uses, is then an undefined symbol, so a CPU build that calls it fails at
link time naming the symbol. That is a deliberate design choice on the library's
part, not a property of the decorator: a decorated function may have a body, and if
it does, that body is what every non-`spirv` target runs while `spirv` substitutes
the instruction. A `spirv` build never emits the body at all.

### Which instructions an `intrinsic` may name

An `intrinsic` may name any instruction of its set except three kinds, each refused
with `op.unknown` at a call, saying what a program writes instead or why the
instruction is not available:

- **An instruction a construct of the language writes.** Each operation has one
  spelling, so the instruction an operator, a cast, an access, a literal, a call, a
  declaration or a statement is written as stays that construct's, and the refusal
  names the construct: `` `OpIAdd` is not an `intrinsic`. write it with the
  `+` operator on integers ``. These are the instructions of the arithmetic,
  bitwise and shift operators (`OpIAdd`, `OpISub`, `OpIMul`, `OpUDiv`, `OpSDiv`,
  `OpUMod`, `OpSRem`, `OpSNegate`, `OpNot`, `OpBitwiseAnd`, `OpBitwiseOr`,
  `OpBitwiseXor`, `OpShiftLeftLogical`, `OpShiftRightLogical`,
  `OpShiftRightArithmetic`, `OpFAdd`, `OpFSub`, `OpFMul`, `OpFDiv`, `OpFNegate`), the
  comparisons they are written as (`OpIEqual`, `OpINotEqual`, `OpULessThan`,
  `OpULessThanEqual`, `OpSLessThan`, `OpSLessThanEqual`, `OpFOrdEqual`,
  `OpFOrdLessThan`, `OpFOrdLessThanEqual`, `OpFUnordNotEqual`), the casts
  (`OpBitcast`, `OpConvertFToS`, `OpConvertFToU`, `OpConvertSToF`, `OpConvertUToF`,
  `OpFConvert`, `OpSConvert`, `OpUConvert`), the
  [widening multiply](operators.md#arithmetic) (`OpUMulExtended`, `OpSMulExtended`),
  literals, lanes and members (`OpCompositeConstruct`, `OpCompositeExtract`,
  `OpCompositeInsert`), member access and indexing (`OpAccessChain`,
  `OpPtrAccessChain`), reading and assigning (`OpLoad`, `OpStore`), `var`
  (`OpVariable`), calls (`OpFunctionCall`).
- **What is no instruction a program writes**: a type, a constant or a
  specialization constant, a decoration, debug information, a capability, an
  extension and the memory model (the target's `env` and `ext`), an entry point and
  its execution modes (`#[gpu.stage]` and its decorators), the parts of a function
  (`fun`), and control flow, which the compiler derives from `if`, `for`, `brk`,
  `cnt` and `ret` (`OpBranch`, `OpBranchConditional`, `OpReturn`, `OpReturnValue`,
  `OpLabel`, `OpPhi`, `OpLoopMerge`, `OpSelectionMerge`, `OpSwitch`,
  `OpUnreachable`). The refusal says what it is: `` `OpTypeInt` is no
  `intrinsic`: a type is declared, not called ``.
- **An instruction that is not available**, since its rule cannot be held for any
  declaration of the language, refused saying why: `OpUndef`, since no value of the
  language is undefined, an operation on an untyped pointer (every `OpUntyped*`
  instruction, as `OpUntypedVariableKHR` and `OpUntypedAccessChainKHR`), which no
  declaration can state, `OpCopyLogical`, a copy between two distinct types alike
  member for member, where records of the same members are one type, the
  instructions over matrices (`OpTranspose`, `OpOuterProduct`,
  `OpMatrixTimesScalar`, `OpMatrixTimesVector`, `OpVectorTimesMatrix`,
  `OpMatrixTimesMatrix`, and `Determinant` and `MatrixInverse` of `GLSL.std.450`),
  which no declaration can state either, and `OpInBoundsPtrAccessChain`, whose
  element is the pointee's stride, which a physical pointer, an address, does not
  carry: `` `OpTranspose` is not available as an `intrinsic`. it is over matrices,
  which no declaration can state. write the operation over vectors ``.

A named instruction the compiler has typing rules for is held to them, at the
declaration with `op.signature`, whether or not anything calls it, and at the call where
a rule depends on the arguments: an operand that must be a constant and its value, the
operands an enumerant value brings, the image operands a mask sets, the member an index
chain's constant selects, the storage class a pointer argument points into and the
capability a typed row's type needs. One it has no typing rules for is held to its operand kinds alone: a
constant or literal operand taken as an integer, a handle operand taken as a handle,
a pointer it reads or writes through taken as a pointer, and a result returned only
where the instruction has one. Its types are the author's to get right, and a wrong
declaration of one can build a module a validator rejects. The rules are masc's
typing data (`dep/masc/gen/data/spirv/typings.txt`), one or more lines per typed
instruction citing the specification text each holds, and the relations each
instruction is held to are in the table of relations above.

An instruction nothing falls through after, as `OpKill` and `OpTerminateInvocation`,
ends its block: the call is the last instruction of the block it stands in, and what
follows it in the source is unreachable and is not compiled. A fragment that discards
itself under a condition calls one in an `if`. `OpTerminateInvocation` ends the
invocation, and `OpDemoteToHelperInvocation` makes it a helper that goes on running
without writing, which a fragment that still computes derivatives for its neighbours
needs.

```mach fragment
#[intrinsic("spirv", "core", "OpTerminateInvocation")]
fun discard();

#[gpu.stage("fragment")]
fun main() {
    if (alpha < 0.5) {
        discard();
    }
    color = alpha;
}
```

An instruction is refused as a construct's only when a construct emits it. One the
compiler writes only inside another construct's code, or never, is an intrinsic's,
held to its row and typed by masc's overlay like any other. Where a construct
writes the instruction's operation by emitting another instruction, as `>` by a
less-than with its operands exchanged, the instruction is an intrinsic's too, and
the compiler's table of constructs (`mach.lower.written`) records the construct
with `emits: false`:

| Instructions | Why no construct writes them |
|--------------|------------------------------|
| `OpSGreaterThan`, `OpSGreaterThanEqual`, `OpUGreaterThan`, `OpUGreaterThanEqual`, `OpFOrdGreaterThan`, `OpFOrdGreaterThanEqual` | `>` and `>=` are the less-than instructions with their operands exchanged |
| `OpFOrdNotEqual`, `OpFUnordEqual`, `OpFUnordLessThan`, `OpFUnordLessThanEqual`, `OpFUnordGreaterThan`, `OpFUnordGreaterThanEqual`, `OpIsNan` | a float comparison is ordered but for `!=`, which is unordered, and `x != x` is that comparison |
| `OpLogicalAnd`, `OpLogicalOr`, `OpLogicalNot`, `OpLogicalEqual`, `OpLogicalNotEqual` | `&&` and `\|\|` branch, and a truth value is an integer to the other operators |
| `OpSelect` | the language has no conditional expression |
| `OpAll`, `OpAny` | the language has no reduction of a vector's lanes |
| `OpBitCount` | the language has no count of one bits |
| `OpIAddCarry`, `OpISubBorrow` | the language has no carry out of `+` or borrow out of `-` |
| `OpFRem` | a float `%` is computed to its definition by code the compiler provides |
| `OpVectorShuffle` | a range `v[start, count]` takes consecutive lanes of one vector, and a shuffle of two vectors or of lanes in any order has no construct |
| `OpVectorExtractDynamic`, `OpVectorInsertDynamic` | a lane index is a constant |
| `OpVectorTimesScalar` | a vector multiplies a vector |
| `OpCopyObject`, `OpCopyMemory` | a value is used as itself, and an assignment of an aggregate copies its members |
| `OpPtrEqual`, `OpPtrNotEqual` | a logical pointer is never compared, and a physical one is compared as its address |
| `OpInBoundsAccessChain` | an access, `r.f` or `a[i]`, makes no claim that it stays in bounds |
| `OpConvertPtrToU`, `OpConvertUToPtr` | a cast between a physical pointer and `u64` reinterprets the address's bits |
| `OpNop` | nothing writes it |

Some of these the target cannot run. `OpLessOrGreater`, `OpOrdered` and `OpUnordered`
need `Kernel`, `OpCopyMemorySized` needs `Addresses`, and `OpFmaKHR` needs `FMAKHR`,
which no Vulkan environment holds, so a call of one is refused with
`spirv.capability`, naming it.

What the overlay states of each is held at the declaration with `op.signature`, as for
every row, and what depends on the arguments is held at the call. A comparison, `OpIsNan`
and a logical instruction return a truth value per lane of their operands, so each is
declared over scalars or over vectors, a vector result an integer vector of the operands'
lane count that holds 1 or 0 in each lane. The objects of `OpSelect` and `OpCopyObject`
are scalars or vectors, since an aggregate argument reaches a row as its address. An
`OpIAddCarry` or `OpISubBorrow` result is a `rec` of two members of the operands' type.
An `OpVectorShuffle` result has the component type of its two vector operands and as
many components as the call passes component literals, each literal below the two
operands' lane count together or `0xFFFFFFFF`. The two pointers of an `OpCopyMemory`
point into memory of one layout. The derivatives (`OpDPdx`, `OpDPdy`, `OpFwidth` and their
`Fine` and `Coarse` forms) take and return one type, a 32-bit float or a vector of them,
and `Ldexp` takes its exponent as 32-bit integers of the value's lane count.

Pointers reach five instructions as values:

- `OpInBoundsAccessChain` takes a pointer and indexes and returns a pointer into the same
  storage class. Each index is an integer, a constant where it selects a member of a
  record and below the record's member count or a constant array's length where it is
  constant, and the result points to the type the indexes reach.
- `OpConvertUToPtr` takes a 64-bit integer and returns a physical pointer, and
  `OpConvertPtrToU` takes a physical pointer and returns a 64-bit integer, of either
  signedness: under the target's
  `PhysicalStorageBuffer64` addressing an address is 64 bits wide.
- `OpPtrEqual` and `OpPtrNotEqual` compare two pointers of one type into one storage
  class: `StorageBuffer` under `VariablePointersStorageBuffer` or `Workgroup` under
  `VariablePointers`, which the selection must hold, refused with `spirv.capability`
  naming it where it does not. No pointer into `PhysicalStorageBuffer` is compared, as
  SPIR-V forbids it (2.16.1), so such a call is refused with `op.signature`.

`OpDemoteToHelperInvocation` takes nothing and executes in a fragment stage alone, as
the typing data states.

The set of accepted instructions is masc's SPIR-V table (`src/isa/spirv/gen_row.mach`
in masc), which masc's generator writes from the SPIR-V grammar and the overlay under
`gen/data/spirv/`. The capabilities, with the SPIR-V version and extension each needs,
are masc's extension catalog (`src/isa/spirv/extension.mach`).


## See also

- [decorators.md](decorators.md): the decorator set and where each decorator applies
- [manifest.md](manifest.md#finished-module-targets): the `spirv` target, its environments and its device features
- [types.md](types.md): the vector types a shader stage computes over and the handle types `handle` declares
