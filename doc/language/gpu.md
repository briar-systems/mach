# GPU decorators

The decorators a GPU target reads: the pipeline stage a function enters, its
compute workgroup, the module bindings of the shader interface, specialization
constants, the handle types a target mints, and functions that are target
instructions. Every decorator, and where each one applies, is listed in
[decorators.md](decorators.md#applicability). The `spirv` target and its
environments are described in [manifest.md](manifest.md#finished-module-targets).

## `stage(str)` — GPU pipeline stage

Marks a function as the entry point of a graphics or compute pipeline stage. The
argument names the stage and the set is closed:

| Value        | Stage                     |
|--------------|---------------------------|
| `"vertex"`   | vertex shader             |
| `"fragment"` | fragment (pixel) shader   |
| `"compute"`  | compute shader            |

An unrecognized value is a compile error, not a module that quietly forms no
stage.

```mach
#[stage("vertex")]
fun vertex_main() {}

#[stage("fragment")]
fun fragment_main() {}
```

A staged function **takes no parameters and returns nothing**. A pipeline stage
does not have a caller: its inputs arrive through input interface variables and
its results leave through output ones, so there is no argument list or return
value to carry them. A staged function with either is rejected.

The decorator is accepted on every target, because which target a module is built
for is not a property of its source. Only a target that has pipeline stages acts
on it: on `spirv` a staged function becomes an `OpEntryPoint` with the matching
execution model, and on a machine target the stage is ignored and the function is
compiled normally.

A module that declares any stage is a **shader module**, and that changes the whole
artifact rather than just the one function. A shader module carries entry points
and no external linkage at all; a module with no stage is a **library module**,
which publishes each function as a linkage export so a consumer can find it. The
two are exclusive — a Vulkan consumer refuses a module carrying linkage — so
adding the first `#[stage(...)]` to a module stops it exporting its functions.

The entry point's name, as a pipeline-creation call looks it up, is the function's
**bare source name** (`vertex_main` above), not a mangled linker symbol. A shader
module has no linker symbols to mangle.

## `workgroup(x, y, z)` — compute workgroup dimensions

Sizes the workgroup of a `#[stage("compute")]` function. The three arguments are
comptime integers giving the x, y and z dimensions.

```mach
#[stage("compute")]
#[workgroup(64, 1, 1)]
fun compute_main() {}
```

It requires a stage on the same function — without one it would silently mean
nothing — and it applies only to the compute stage. When it is omitted, a compute
stage takes the single-invocation default `(1, 1, 1)`; the dimensions are always
declared in the emitted module, since a compute stage that does not state its
workgroup size is not one a consumer can dispatch.

Any dimension may instead name a `#[spec]` var (see
[`spec`](#specid--specialization-constants)), which the host sets when it creates
the pipeline. The var must be a `u32` or `i32`, and any other variable is refused.

```mach fragment
#[spec(0)] var tile: u32 = 64;

#[stage("compute")]
#[workgroup(tile, 1, 1)]
fun compute_main() {}
```

On `spirv` how a specialized workgroup is emitted depends on the environment.
Where it allows it, the stage carries `LocalSizeId` over the spec constant. That
needs SPIR-V 1.2 and, on Vulkan, the `maintenance4` feature, which only Vulkan
1.3 requires of every device, so the form is used with no `env` and with
`vulkan1.3`. Every other environment gets a `WorkgroupSize` built-in over an
`OpSpecConstantComposite`. That built-in sizes **every** compute stage in the
module, so a module in such an environment whose compute stages size their
workgroups differently, and at least one of them through a `#[spec]` var, is
refused. Give the stages one workgroup or put them in separate modules.

## `input(n)` / `output(n)` / `builtin(str)` / `uniform(set, binding)` / `storage(set, binding)` / `sampler(set, binding)` / `push` / `spec(id)` / `shared` — shader interface

A pipeline stage does not receive its inputs or return its results through a call.
It reads and writes **module-scope variables** that the pipeline binds, and these
decorators say which kind each variable is. They apply only to module-level
`val` / `var` bindings, and a variable carries **exactly one** of them — they
are mutually exclusive. `spec`, described in its own section below, is one of
them too.

```mach fragment
#[input(0)]            var in_position: f32x4;
#[output(0)]           var out_colour:  f32x4;
#[builtin("position")] var position:    f32x4;

rec Camera { view: f32x4; proj: f32x4; }
#[uniform(0, 0)] var camera: Camera;

rec Particles { pos: [64]f32x4; }
#[storage(0, 1)] var particles: Particles;

#[sampler(1, 0)] var albedo: Sampler2D;

rec Params { tint: f32x4; count: u32; }
#[push] var params: Params;
```

`input` and `output` number a **varying** with a location, which is how one
stage's outputs line up with the next stage's inputs: the producer's
`#[output(0)]` feeds the consumer's `#[input(0)]`.

`builtin` names a value the pipeline supplies or consumes instead of one a
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

The direction is a property of the built-in, not something you restate — a stage
writes its position and reads what the pipeline hands it — so there is no
input/output marker to pair with `builtin`, and none that could disagree with it.

The **type** is a property of the built-in too, and it is a requirement rather
than a suggestion: the pipeline binds the variable itself, so a wider or narrower
one is an invalid module rather than a wasteful one. Declaring a built-in at any
other type is a compile error naming both the declared type and the required one.
The scalar integer rows accept `i32` as well as `u32`, because the compiler carries
an integer's width and not its sign and the emitted type is sign-less either way.

The **stage** is a property of the built-in as well. Each row exists in the stages
SPIR-V defines it in, in its direction: the pipeline supplies an input built-in to
those execution models alone and consumes an output one from them alone, so
`position` and `point_size` are a vertex stage's outputs, `frag_coord` is a
fragment stage's input, the seven compute rows are the `GLCompute` execution
model's, and `subgroup_size` and `subgroup_invocation` are every stage's, decorated
`Flat` as a fragment stage's integer inputs. A stage that uses a built-in outside
its row, directly or through a function it calls, is a compile error naming the
built-in and both stages. The four subgroup built-ins need SPIR-V 1.3, and outside
a compute stage the `subgroup_graphics_stages` feature, as the subgroup operations do.

`uniform` binds a read-only block by descriptor set and binding. Its type **must
be a `rec`**: a uniform is a block with a host-visible layout, and a bare scalar
or vector has no block layout for a pipeline to bind. The record is emitted with
its `Block` decoration and an explicit byte offset on every member, taken from the
same layout the rest of the compiler uses, so what the shader reads is what the
host wrote. Wrap a single value in a one-field record.

`storage` binds a **read-write** buffer by descriptor set and binding, where
`uniform` binds a read-only one. Both must be a `rec` for the same reason, and both
are emitted as a `Block`-decorated struct with an explicit offset on every member.

They differ in one place: their **layout rules**. A uniform block follows
std140-shaped rules, under which an array's stride is rounded up to 16 — which
mach's own layout does not do, so an array of anything narrower than 16 bytes is
refused rather than silently repacked. A storage buffer follows std430-shaped
rules, which use the element's natural stride, and that *is* mach's layout, so
`[8]f32` is fine in a `storage` block and rejected in a `uniform` one.

A compute stage's data path is `storage`: Vulkan forbids the `Output` storage class
in a compute execution model, so a compute shader reads and writes buffers rather
than varyings.

`storage` takes **memory qualifiers** after the descriptor pair, any number of them
in any order: `"readonly"`, `"writeonly"` and `"coherent"`. `"readonly"` says that
nothing writes the binding:

```mach
rec Palette { columns: [512]f32x4; }
#[storage(0, 3, "readonly")]
var palette: Palette;
```

A store through a `"readonly"` binding is a compile error on every target, naming
the line that wrote it. That is what the qualifier buys over what the compiler
works out on its own: a buffer no body in the module stores through is emitted with
the SPIR-V `NonWritable` decoration whether or not it is marked, and Vulkan reads
that decoration to decide whether a stage needs `vertexPipelineStoresAndAtomics`.
So an accidental write does not produce a wrong module, it produces a **correct one
that quietly costs a hardware feature**. Marking the binding turns that into a
diagnostic instead.

The inference is one-sided on purpose. Anything the compiler cannot follow, such as
the binding's address handed to a function, counts as a write, so a missing
decoration is possible and a wrong one is not.

`"writeonly"` is the mirror: it says that nothing reads the binding, and the buffer
is emitted `NonReadable`.

```mach
rec Frame { texels: [4096]f32x4; }
#[storage(0, 4, "writeonly")]
var frame: Frame;
```

A read of a `"writeonly"` binding is a compile error on every target, naming the
expression that read it. Storing into a field or an element reads nothing, and
neither does taking the binding's address, so `frame.texels[i] = c` and
`?frame.texels[i]` are accepted. Writing **one lane** of a vector inside it is
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
  write in such a function is coherent in a module that binds a coherent image.

```mach
rec Counters { done: u32; }
#[storage(0, 5, "coherent")]
var counters: Counters;
```

`sampler` binds a **handle** by descriptor set and binding, at the same descriptor
addressing `uniform` and `storage` use, so a host binds one the way it binds the
others. Its type must be a **handle type**, a bodyless `def` carrying `#[handle]`
(see [types.md](types.md)), and a handle type must carry this decorator: a handle
names a descriptor rather than an object with storage, so one with no descriptor
address is reachable from no stage. A handle cannot sit behind a pointer, inside an
array, or in a local binding, and each of those is a compile error naming why.

A **storage image** is the exception: an `image` handle whose `Sampled` operand is
`2` is read and written directly rather than sampled, which is Vulkan's
`STORAGE_IMAGE` descriptor, so it binds through `storage` and not `sampler`. The
`"readonly"`, `"writeonly"` and `"coherent"` qualifiers apply to it exactly as they
do to a buffer, checked against the instructions that read and write its texels. A
storage texel buffer, an image of `Dim` `Buffer` with `Sampled` `2`, binds the same
way, and a uniform texel buffer (`Sampled` `1`) keeps `sampler`. Binding a storage
image through `sampler`, or any other handle through `storage`, is a compile error.
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
`image_gather_extended` extension, and `Offset` on a fetch or a sample `maintenance8` as well,
since Vulkan admits it outside a gather only under that feature. A sample takes
`MinLod` (`0x80`), the least level of detail it reads, which an explicit-lod sample
takes only with `Grad` and which needs the `resource_min_lod` extension.
`OpImageGather` reads one component, a constant id, of the four texels a sample of a
`2D` or `Cube` image would filter. A constant operand is a constant by emission: a
literal, a vector of literals directly or through a binding, a module-scope `val`,
which a shader reads as the constant it is, or an aggregate passed whole as an array
literal of constants or a copy of a `val`, written as one composite constant. An
aggregate any run-time value writes, or one written only in part, is a value, and is
refused with `op.operand_not_constant`. `OpImageQuerySamples` reads the sample
count of a multisampled image only, and `OpImageQuerySizeLod` does not take one.
`OpImageQuerySize` reads an image with no level of detail to choose, a multisampled
image, a storage image or a texel buffer, so a single-sampled sampled image is
queried with `OpImageQuerySizeLod` instead.
Each is refused at the call with `op.operand_value`.

```mach fragment
#[handle("spirv", "image", TEXEL_F32, DIM_2D, NO_DEPTH, NONARRAYED, MULTISAMPLED, SAMPLED, FORMAT_UNKNOWN)]
pub def TextureMS;

#[op("spirv", "core", "OpImageFetch")]
fun fetch_sample(img: TextureMS, at: i32x2, mask: u32, sample: i32) f32x4;

#[op("spirv", "core", "OpImageQuerySamples")]
fun sample_count(img: TextureMS) i32;
```

```mach fragment
#[handle("spirv", "image", TEXEL_F32, DIM_2D, NO_DEPTH, NONARRAYED, SINGLE_SAMPLED, STORAGE, FORMAT_RGBA8)]
pub def Target2D;

#[op("spirv", "core", "OpImageWrite")]
fun image_write(img: Target2D, at: i32x2, texel: f32x4);

#[storage(0, 6, "writeonly")] var target: Target2D;
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
#[op("spirv", "core", "OpImageSampleDrefImplicitLod")]
fun shadow(s: ShadowSampler, uv: f32x2, depth: f32) f32;

#[op("spirv", "core", "OpImageSampleDrefExplicitLod")]
fun shadow_lod(s: ShadowSampler, uv: f32x2, depth: f32, mask: u32, lod: f32) f32;

#[op("spirv", "core", "OpImageDrefGather")]
fun shadow_gather(s: ShadowSampler, uv: f32x2, depth: f32, mask: u32, offset: i32x2) f32x4;
```

The projective forms, `OpImageSampleProj*` and `OpImageSampleProjDref*`, have no
row. Each divides the coordinate, and a comparison's reference, by the
coordinate's last component before sampling, which a shader writes as that
division and passes to the plain form: they add no sampling a row here does not
already give, HLSL, MSL and WGSL have none, and each brings its own refusals (no
`Cube` image, no arrayed one). A declaration naming one is refused as an
instruction the target does not define.

`push` binds a **push-constant block**, a small `rec` the host supplies with the
command that records a dispatch or draw rather than through a descriptor, so it
takes no arguments: there is no set or binding to name. Like `uniform` and
`storage` it must be a `rec`, and it is emitted as a `Block`-decorated struct with
an explicit offset on every member. Its layout rules are std430-shaped, the same
ones a `storage` buffer follows and checked the same way, so `[8]f32` is fine in a
push block.

A push block is **read-only** in the shader. A store to it is a compile error on
every target, and on `spirv` so is a store the compiler follows through its address
handed to a function. Vulkan admits **one push-constant block per entry point**:
two push blocks used by the same stage are refused, naming both, while two stages
that each use a different one are accepted.

```mach
rec Params { scale: f32; slot: u32; }
#[push]
var params: Params;
```

Sampling a handle is an `#[op(...)]` declaration rather than a language form,
because a sample IS one SPIR-V instruction like `sqrt` and `dot` are:

```mach fragment
#[op("spirv", "core", "OpImageSampleImplicitLod")]
fun sample(s: Sampler2D, uv: f32x2) f32x4;

#[stage("fragment")]
fun frag_main() {
    out_colour = sample(albedo, in_uv);
}
```

The separately-bound form works the same way, with the instruction that combines
an image and a sampler declared alongside it:

```mach fragment
#[op("spirv", "core", "OpSampledImage")]
fun combine(t: Texture2D, s: Sampler) Sampler2D;

#[sampler(1, 0)] var base_tex: Texture2D;
#[sampler(1, 1)] var base_smp: Sampler;

#[stage("fragment")]
fun frag_sep() { out_colour = sample(combine(base_tex, base_smp), in_uv); }
```

The combined value is handed straight to the sample rather than named: SPIR-V
requires an `OpSampledImage` result be consumed by an image instruction in the block
that produced it, which is the same rule that makes a handle-typed local a compile
error. Returning one, or passing one to a function, is refused with
`op.result_flow`, and so is a sample whose later operand branches, as `&&` and `||`
do, since its sampled image would then be consumed in another block. Compute such an
operand into a binding before the call.

`shared` declares **workgroup memory**: one instance per workgroup of a compute
stage, which every invocation of that workgroup reads and writes. It applies to a
`var` only, since a `val` of workgroup memory could only ever read zero. It takes no
arguments, and the variable has no descriptor and no location, because the pipeline
never binds it.

```mach fragment
#[builtin("local_invocation")] var local_id: u32x3;
#[shared] var tile: [256]f32;

#[stage("compute")]
#[workgroup(64, 1, 1)]
fun blur() { tile[local_id[0]] = 1.0; }
```

Workgroup memory exists only in a compute stage, so a `#[shared]` variable used from
a vertex or fragment stage, directly or through a function the stage calls, is a
compile error naming the stage.

A `#[shared]` variable is **zero** when a compute stage starts, as every mach
variable is, on every environment. How depends on the environment:

- Where workgroup memory is zero-initialized by the consumer, the variable carries an
  `OpConstantNull` initializer. `vulkan1.3` guarantees that
  (`shaderZeroInitializeWorkgroupMemory` is core there), and a target that selects the
  `zero_init_workgroup` extension declares it for an earlier version (see
  [manifest.md](manifest.md#instruction-set-extensions)). The consumer then has to
  enable the feature (`VK_KHR_zero_initialize_workgroup_memory`).
- Otherwise the compiler zeroes it itself, at the start of each compute stage that uses
  it. Each invocation stores zero to its own slice, the elements of an array its local
  invocation index reaches in steps of the workgroup size and the whole of any other
  type for invocation 0, and then the stage executes one workgroup `OpControlBarrier`.
  The barrier precedes all of the stage's own code, so every invocation reaches it.

Because the value on entry is always zero, a `#[shared]` variable cannot have an
initializer. Assign it inside the stage.

Workgroup memory is **coherent** among the invocations of a workgroup under either
memory model. Under GLSL450 it is so by definition. Under the Vulkan model an access
is private unless it says otherwise, and a barrier orders no private access between
invocations, so each load and store of a `#[shared]` variable carries
`MakePointerVisible` or `MakePointerAvailable` with `NonPrivatePointer` at the
`Workgroup` scope, the compiler's own zeroing stores included. A workgroup barrier
with acquire-release semantics then orders them as it does under GLSL450.

Whether a variable may carry an **initializer** is settled by its role, since the
role says who puts the first value in it:

| Role                                             | Initializer | Why                                                   |
|--------------------------------------------------|-------------|-------------------------------------------------------|
| `input`, a read built-in                         | refused     | the previous stage or the pipeline supplies the value |
| `uniform`, `storage`, `sampler`, `push`          | refused     | the host binds or supplies the memory                 |
| `shared`                                         | refused     | workgroup memory is zero when a stage starts          |
| `spec`                                           | required    | it is the default the pipeline keeps                  |
| `output`, a written built-in                     | allowed     | it is the value the variable starts at                |

A refused initializer is a compile error, because the value it writes would never be
the one the shader sees. An `output` or a written built-in starts at its
initializer, and at zero without one, as every mach `var` does:

```mach fragment
#[output(0)] var out_colour: f32x4 = f32x4{0.0, 0.0, 0.0, 1.0};
#[output(1)] var out_mask:   u32;
```

On `spirv` the Output `OpVariable` carries that value as its initializer: the
constant the initializer spells, or `OpConstantNull` where it is zero or absent.
SPIR-V and Vulkan both admit an initializer on an Output variable.

As with `#[stage(...)]`, these are accepted on every target and acted on only by a
target that forms pipeline stages. On `spirv` each becomes an `OpVariable` in the
matching storage class, carrying the matching decoration, and the Input and Output
variables are named in every entry point's interface list. A `sampler` binding
becomes an `OpVariable` in the `UniformConstant` class — the one class Vulkan
permits an image, sampler or sampled-image variable in — carrying `DescriptorSet`
and `Binding` exactly as a `uniform` does. A `push` block becomes an `OpVariable`
in the `PushConstant` class, with no `DescriptorSet` or `Binding`. A `shared` variable
becomes an `OpVariable` in the `Workgroup` class, named in the interface of each entry
point that uses it from SPIR-V 1.4.

## `spec(id)` — specialization constants

A specialization constant is a value the host supplies when it creates the
pipeline, after the shader has been compiled. It is declared as a module-level
`var` carrying the constant's id, and its initializer is the default the pipeline
keeps when the host supplies nothing for that id. The initializer is required:

```mach
#[spec(0)]
var tile_size: u32 = 64;
#[spec(1)]
var gain:      f32 = 0.5;
```

It is a `var` like every other value the host supplies, and that settles how the
compiler treats it:

- It is **never a compile-time value**. It cannot be an array length or a comptime
  operand, since what it holds is decided after the build. A `#[spec]` on a `val`
  is refused for the same reason.
- The optimizer **never folds it to its initializer**, even when nothing in the
  module writes it. A mutable global is never replaced by its initial value, and
  that is exactly what keeps the host's value live.
- A **store to it is refused**, naming the line that wrote it. On the GPU it is a
  constant once the pipeline exists, so there is nothing to write. Copy it into a
  local to change the value. Handing its address to a function counts as a store.

The type must be a scalar integer or float. There is no boolean specialization
constant, because mach has no boolean type the compiler knows: `bool` is an alias
of `u8`. Write a flag as an integer spec var, which the host sets with the same 4
bytes as a `VkBool32`:

```mach
#[spec(3)]
var use_fog: u32 = 1;
```

A narrower integer such as `u8` works too, but it needs the capability for its
width like any other `u8` in a shader. On `spirv` each one becomes an
`OpSpecConstant` whose literal is the initializer, decorated with `SpecId`, and a
read uses that constant directly with no load. Two `#[spec]` vars with one id in
the same module are refused, since the host names the constant by its id. On a
machine target the decorator has no effect and the var is an ordinary global.

A `#[spec]` var may also size a compute workgroup (see
[`workgroup`](#workgroupx-y-z--compute-workgroup-dimensions)).

## `handle(target, constructor, operands...)` — a type the target mints

A bodyless `def` carrying this decorator declares a type whose representation is
**not the program's**: the owning target mints it and the pipeline binds it.

```mach fragment
#[handle("spirv", "image", TEXEL_F32, DIM_2D, NO_DEPTH, NONARRAYED, SINGLE_SAMPLED, SAMPLED, FORMAT_UNKNOWN)]
pub def Texture2D;

#[handle("spirv", "sampled_image", Texture2D)]
pub def Sampler2D;
```

The first argument names the target and the second the type constructor within it.
Both are constant strings matched against the target's own definition table.
Everything after them is **operands to that
constructor**, never rule knobs: the rules a handle carries are fixed and closed
(see [types.md](types.md)) and never vary per declaration.

An operand is an ordinary comptime constant, with one exception. A constructor that
composes over another handle takes a **type name**, and that is the only place a
decorator argument is read as a type rather than as a value. It exists so a
composing declaration names what it wraps instead of restating it, which is what
keeps the two from disagreeing. The named type must be a handle the same target
mints, with the constructor that position requires.

A declaration addressed to a target this build did not select is **inert**: it
still denotes a type and still sizes at the target's pointer width, so a library of
handles compiles on a machine target. A constructor name the selected target does
not define, an operand count that disagrees with the constructor's, or an operand
combination the target cannot emit is a compile error at the declaration.

## `op(target, set, name)` — a function that *is* a target instruction

A shader needs `sqrt`, `normalize`, `dot` and `mix`. None of them is an operator,
and none of them is a call SPIR-V can make: each is one instruction. This decorator
says which one a function is, so that on a `spirv` target a call to it becomes that
instruction, inline, rather than a call.

```mach
#[op("spirv", "GLSL.std.450", "Sqrt")]
pub fun sqrt(x: f32) f32;

#[op("spirv", "GLSL.std.450", "Normalize")]
pub fun normalize(v: f32x4) f32x4;

#[op("spirv", "core", "OpDot")]
pub fun dot(a: f32x4, b: f32x4) f32;
```

The first argument names the **target**, the second the instruction set, and the
third the instruction within it. The target is the ISA name the manifest selects
with, so nothing about this decorator is specific to one back end. The arguments
must be strings on every target, but the instruction set and name are checked only
when the named target is the one selected: a declaration for any other target is
inert, and that target's table is not consulted. When it is checked, the parameter
count is held to the instruction's own operand count, which is not uniform across a
family that looks it.

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
a function-local object or an image, since an `op` has no body and so no boundary at
which its parameter's pointer could be given one. Any access chain is accepted, a
member or element as well as a whole object. The kinds are also what the
`"readonly"` and `"writeonly"` qualifiers of a `storage` binding are checked
against: an atomic load through a `readonly` binding is accepted and an atomic add
on it is refused, and an atomic store into a `writeonly` binding is accepted and an
atomic load from it is refused. A handle passed as an ordinary value names its
descriptor and touches none of its memory, and a handle read or write is checked the
same way, so `OpImageWrite` into a `"readonly"` storage image and `OpImageRead` from a
`"writeonly"` one are refused.

A non-constant argument to a constant id or a literal is refused at the call,
naming the operand. A row **without a result** is declared with no return type,
and a row with one must return it. A row may also **return a pointer** into a
storage class the row itself declares, as `OpImageTexelPointer` returns an `Image`
pointer, and that result is accepted as a later instruction's pointer operand. A
row whose result is a **truth value**, such as `OpGroupNonUniformElect`, is declared
returning an integer, which receives 1 or 0. A `bool` return is an 8-bit integer,
which a target without `int8` carries at 32 bits like any other 8-bit local
([manifest.md](manifest.md#finished-module-targets)), so the module needs no Int8 for it.

```mach
#[op("spirv", "core", "OpControlBarrier")]
pub fun barrier(execution: u32, memory: u32, semantics: u32);

#[op("spirv", "core", "OpMemoryBarrier")]
pub fun memory_barrier(memory: u32, semantics: u32);

#[op("spirv", "core", "OpAtomicIAdd")]
pub fun atomic_add(p: *u32, scope: u32, semantics: u32, v: u32) u32;
```

On the target that owns the instruction, a decorated function **is the
instruction and never its body**, so a call to it is never inlined away or
deleted, and the optimizer treats it as reading and writing all memory. No
load or store is moved across a barrier or an atomic, at any optimization
level. `OpControlBarrier` takes an execution scope, a memory scope and memory
semantics, and `OpMemoryBarrier` a memory scope and semantics, each an integer
constant. A memory scope and memory semantics are held to the module's memory model:
under the Vulkan model the `Device` scope needs the `vulkan_memory_model_device_scope`
extension, and under GLSL450 the `QueueFamily` scope and the `MakeAvailable`,
`MakeVisible` and `Volatile` semantics need `vulkan_memory_model` (see
[manifest.md](manifest.md#finished-module-targets)). An atomic's scope and semantics
are held the same way. The execution scope is not, since a Vulkan barrier executes
at `Workgroup` or `Subgroup` only. `SequentiallyConsistent` semantics are refused under
either model, since Vulkan defines no sequentially consistent order
(VUID-StandaloneSpirv-MemorySemantics-10866): use `Acquire`, `Release` or
`AcquireRelease`.

A control barrier must be reached in **uniform control flow**: every
invocation of its execution scope executes it, or none does. That is the
program's obligation, as it is in GLSL and WGSL, because whether a branch is
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
#[op("spirv", "core", "OpGroupNonUniformIAdd")]
pub fun subgroup_add(scope: u32, operation: u32, v: u32) u32;

#[op("spirv", "core", "OpGroupNonUniformIAdd")]
pub fun subgroup_cluster_add(scope: u32, operation: u32, v: u32, cluster_size: u32) u32;
```

Its operation is a `GroupOperation`. `Reduce` (0), `InclusiveScan` (1) and
`ExclusiveScan` (2) need the `subgroup_arithmetic` extension and declare
`GroupNonUniformArithmetic`, and `ClusteredReduce` (3) needs `subgroup_clustered`,
declares `GroupNonUniformClustered` and is followed by the ClusterSize operand, so
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
| a requirement's extension the target does not select   | `spirv.capability`, naming the extension |
| a capability whose SPIR-V version the environment is below | `spirv.capability`, naming the first `env` that reaches it |

A device feature is an extension the target names in its `extensions` once the
consumer enables it, since no environment guarantees it: `subgroup_arithmetic` is
Vulkan's `VK_SUBGROUP_FEATURE_ARITHMETIC_BIT`, and a target naming no `env` holds
every extension. A module declares a capability only when something it emits needs
it, with `OpExtension` for a capability a SPIR-V extension defines.

A row may also be **typed**: its requirement depends on the type it operates on, read
from one operand (a pointer's pointee), and on the storage class that operand's
memory lives in. The atomics are typed. A declaration whose type the row admits in no
storage class is refused with `op.signature`, and each call is checked where its
storage class is known. A load reads its pointer and every other atomic writes it, so
a `"readonly"` binding admits only an atomic load.

```mach
#[op("spirv", "core", "OpAtomicIAdd")]
pub fun atomic_add64(p: *u64, scope: u32, semantics: u32, v: u64) u64;

#[op("spirv", "core", "OpAtomicFAddEXT")]
pub fun atomic_fadd(p: *f32, scope: u32, semantics: u32, v: f32) f32;
```

A 32-bit integer atomic is core in every storage class. Every other type needs the
Vulkan device feature of its storage class, named for its `shaderBuffer*`,
`shaderShared*` or `shaderImage*` member: `buffer_*` on buffer memory
(`StorageBuffer`, `Uniform` before SPIR-V 1.3, or `PhysicalStorageBuffer` through a
[physical pointer](types.md#pointers-on-spir-v)), `shared_*` on [`#[shared]`](#inputn--outputn--builtinstr--uniformset-binding--storageset-binding--samplerset-binding--push--specid--shared--shader-interface)
workgroup memory, and `image_*` on a storage image texel (`Image`, through
`OpImageTexelPointer`), where Vulkan defines only a 64-bit integer and an `f32`. Any
other storage class is refused. An image of 64-bit texels (`R64ui` or `R64i`) declares
`Int64ImageEXT` (`SPV_EXT_shader_image_int64`), which `image_int64_atomics` enables, so
the image itself needs that feature whatever reaches it. An `f16` atomic also needs
`float16`, under which `f16` memory is the `OpTypeFloat 16` the atomic operates on, and
the storage buffer holding one needs `storage_buffer_16bit_access`
([manifest.md](manifest.md#finished-module-targets)).

| Rows | Type | Extensions | Vulkan feature | Capability (SPIR-V extension) |
|------|------|-----------|----------------|-------------------------------|
| all 15 integer atomics | `u32`, `i32` | none | core | none |
| all 15 integer atomics | `u64`, `i64` | `buffer_int64_atomics`, `shared_int64_atomics`, `image_int64_atomics` | `shaderBufferInt64Atomics`, `shaderSharedInt64Atomics`, `shaderImageInt64Atomics` | `Int64Atomics` |
| `OpAtomicLoad`, `OpAtomicStore`, `OpAtomicExchange` | `f16`, `f32`, `f64` | `buffer_float{16,32,64}_atomics`, `shared_float{16,32,64}_atomics`, `image_float32_atomics` | `shader{Buffer,Shared}Float{16,32,64}Atomics`, `shaderImageFloat32Atomics` | none |
| `OpAtomicFAddEXT` | `f32`, `f64` | `buffer_float{32,64}_atomic_add`, `shared_float{32,64}_atomic_add`, `image_float32_atomic_add` | `shader{Buffer,Shared}Float{32,64}AtomicAdd`, `shaderImageFloat32AtomicAdd` | `AtomicFloat{32,64}AddEXT` (`SPV_EXT_shader_atomic_float_add`) |
| `OpAtomicFAddEXT` | `f16` | `buffer_float16_atomic_add`, `shared_float16_atomic_add` | `shader{Buffer,Shared}Float16AtomicAdd` | `AtomicFloat16AddEXT` (`SPV_EXT_shader_atomic_float16_add`) |
| `OpAtomicFMinEXT`, `OpAtomicFMaxEXT` | `f16`, `f32`, `f64` | `buffer_float{16,32,64}_atomic_min_max`, `shared_float{16,32,64}_atomic_min_max`, `image_float32_atomic_min_max` | `shader{Buffer,Shared}Float{16,32,64}AtomicMinMax`, `shaderImageFloat32AtomicMinMax` | `AtomicFloat{16,32,64}MinMaxEXT` (`SPV_EXT_shader_atomic_float_min_max`) |

The features come from `VkPhysicalDeviceShaderAtomicInt64Features`,
`VkPhysicalDeviceShaderImageAtomicInt64FeaturesEXT`,
`VkPhysicalDeviceShaderAtomicFloatFeaturesEXT` and
`VkPhysicalDeviceShaderAtomicFloat2FeaturesEXT`. No environment guarantees any of
them, so a target names each one its consumer enables, and a target naming no `env`
holds them all. The integer atomics are `OpAtomicLoad`, `OpAtomicStore`,
`OpAtomicExchange`, `OpAtomicCompareExchange`, `OpAtomicIIncrement`,
`OpAtomicIDecrement`, `OpAtomicIAdd`, `OpAtomicISub`, `OpAtomicSMin`, `OpAtomicUMin`,
`OpAtomicSMax`, `OpAtomicUMax`, `OpAtomicAnd`, `OpAtomicOr` and `OpAtomicXor`.

| At the call                                            | Is refused with                    |
|--------------------------------------------------------|------------------------------------|
| a type the row admits only in other storage classes    | `spirv.capability`, naming the class |
| a type whose feature the target does not select        | `spirv.capability`, naming the feature |

A row also states how its operands' types and its result's **relate** to the type it
operates on, the type of the one operand its typing reads (a pointer's pointee). A
declaration that breaks a relation is refused with `op.signature`, naming both the
parameter (or the return type) and the operand it relates to, so a mismatch is caught
at the declaration rather than as an invalid module.

| Rows | Relation |
|------|----------|
| every atomic | the result and each value operand are the pointer's pointee |
| `OpImageRead`, `OpImageFetch`, `OpImageSample*Lod`, `OpImageGather` | the result is a 4-vector of the image's texel scalar, a sampled image's being its image's |
| `OpImageSampleDref*Lod` | the result is a scalar of the image's texel scalar, and the reference is an `f32` scalar |
| `OpImageDrefGather` | the result is a 4-vector of the image's texel scalar, and the reference is an `f32` scalar |
| `OpImageWrite` | the texel is a scalar or vector of the image's texel scalar, with at least as many components as the image's format stores (any for `Unknown`) |
| `OpImageTexelPointer` | the result points to the image's texel scalar, into a storage image of `R32ui`, `R32i`, `R32f`, `R64ui` or `R64i` format |
| `OpSampledImage` | the result is a sampled image composed over the image operand's own type, and the second operand is a `sampler` |
| `OpGroupNonUniformBroadcast*`, `Shuffle*`, `Quad*` and the arithmetic rows | the result is the value operand's type |
| the GLSL.std.450 math rows, float and integer | the result and every operand are the first operand's type, except `Refract`'s `eta`, and `Length` and `Distance`, whose result is a scalar |
| `Modf` | the result is the value's type, and the out-pointer points to the value's type, where the whole part is stored |
| `Frexp` | the result is the value's type, and the out-pointer points to an `i32` scalar or vector with as many components as the value, where the exponent is stored |
| `OpDot` | the second vector is the first's type |

A relation on a pointer operand, such as the out-pointer `Modf` and `Frexp` store
their second part through, holds the type it points to. The pointer may address a
local, a `#[shared]` variable or a storage buffer, or any part of one, and the store
counts as a write to that binding, so a `readonly` one is refused.

```mach
#[op("spirv", "GLSL.std.450", "Frexp")]
pub fun frexp(x: f32x4, exp: *i32x4) f32x4;
```

A declaration that returns a handle is refused with `op.signature` unless its row
names the operand its result derives from, since a handle holds a binding's
descriptor and only that operand says whose. Each math row and subgroup row states the
class of number it operates on. The GLSL.std.450 float rows and `OpDot` operate on a
scalar or vector of floats, and the subgroup rows on a scalar or vector of integers or
floats. The GLSL.std.450 integer rows operate on the signedness their name states:
`SAbs`, `SSign`, `SMin`, `SMax`, `SClamp` and `FindSMsb` on signed integers, `UMin`,
`UMax`, `UClamp` and `FindUMsb` on unsigned ones, and `FindILsb` on either. A data
operand of any other type is refused with `op.signature`: an integer for `FAbs`, an
`i32` for `UMin`, a handle, a pointer or an aggregate. GLSL.std.450 removed `IMix`, so
it has no row.

Each image row names the rule its texel's component count comes from, and the
refusal quotes it: the 4-vector read result is Vulkan's `VUID-StandaloneSpirv-Result-04780`,
the 4-vector fetch, sample and gather results and the scalar depth-comparison result are SPIR-V's own, and the write's count against the
format is Vulkan's `VUID-RuntimeSpirv-OpImageWrite-07112`, which spirv-val cannot check
because it sees no `VkFormat`.


The **subgroup operations** are the `OpGroupNonUniform*` rows, each taking the
Subgroup scope (3) as its first operand. Every one needs SPIR-V 1.3, so `vulkan1.1`
or later, and each family needs its capability and the feature Vulkan reports it by:

| Family            | Rows                                                         | Feature                     |
|-------------------|--------------------------------------------------------------|-----------------------------|
| basic             | `Elect`                                                      | none: every vulkan1.1 device |
| vote              | `All`, `Any`, `AllEqual`                                     | `subgroup_vote`             |
| arithmetic        | `IAdd`, `FAdd`, `IMul`, `FMul`, `SMin`, `UMin`, `FMin`, `SMax`, `UMax`, `FMax`, `BitwiseAnd`, `BitwiseOr`, `BitwiseXor`, `LogicalAnd`, `LogicalOr`, `LogicalXor` with `Reduce` or a scan | `subgroup_arithmetic` |
| clustered         | the same rows with `ClusteredReduce` and a ClusterSize       | `subgroup_clustered`        |
| ballot            | `Ballot`, `InverseBallot`, `BallotBitExtract`, `BallotBitCount`, `BallotFindLSB`, `BallotFindMSB`, `Broadcast`, `BroadcastFirst` | `subgroup_ballot` |
| shuffle           | `Shuffle`, `ShuffleXor`                                      | `subgroup_shuffle`          |
| relative shuffle  | `ShuffleUp`, `ShuffleDown`                                   | `subgroup_shuffle_relative` |
| quad              | `QuadBroadcast`, `QuadSwap`                                  | `subgroup_quad`             |

`BallotBitCount` takes a `GroupOperation` too, which its ballot capability covers
and which has no clustered form. `Broadcast`'s lane, `QuadBroadcast`'s index and
`QuadSwap`'s direction are constant ids, which every SPIR-V version accepts. Vulkan
guarantees subgroup operations only in compute stages, so a use reached from a
vertex or fragment stage, an operation or a subgroup built-in alike, also needs
`subgroup_graphics_stages`, the device's `subgroupSupportedStages`.

```mach
use std.types.bool.bool;

#[op("spirv", "core", "OpGroupNonUniformBallot")]
pub fun subgroup_ballot(scope: u32, predicate: bool) u32x4;

#[op("spirv", "core", "OpGroupNonUniformShuffleXor")]
pub fun subgroup_shuffle_xor(scope: u32, v: u32, mask: u32) u32;
```

`OpExtInstImport "GLSL.std.450"` is emitted **once per module and only when that
module uses the set**. A module that calls none of these carries no import.

Note that `dot` is **core `OpDot`**, not a GLSL.std.450 instruction, even though
GLSL spells it beside `normalize` and `length`. Check each function against the
specification rather than against GLSL's surface.

On every target other than `spirv` the decorator is inert, and a decorated function
is an ordinary function. A **bodiless** one — which is what the shader-side maths
library uses — is then an undefined symbol, so a CPU build that calls it fails at
link time naming the symbol. That is a deliberate design choice on the library's
part, not a property of the decorator: a decorated function may have a body, and if
it does, that body is what every non-`spirv` target runs while `spirv` substitutes
the instruction. A `spirv` build never emits the body at all.

The set of accepted instructions is the table in
`src/lang/target/isa/spirv/defs.mach`, where each row carries its operand kinds, its
result, its requirements, its typing and the enumerations of its literals. The capabilities, with
the SPIR-V version and extension each needs, are the table in
`src/lang/target/isa/spirv.mach`. Adding an instruction is a row in it.


## See also

- [decorators.md](decorators.md) — the decorator set and where each decorator applies
- [manifest.md](manifest.md#finished-module-targets) — the `spirv` target, its environments and its device features
- [types.md](types.md) — the vector types a shader stage computes over and the handle types `handle` declares
