# GPU shaders

A shader is ordinary Mach built for the `vulkan-spirv` target into a SPIR-V module. Decorators in the `gpu.` family mark the pipeline stages and the module-level variables the pipeline binds.

## The target

```toml
[target.vulkan-spirv]
arch = "spirv"
sys  = "vulkan"
abi  = "shader"
env  = "vulkan1.3"

[artifact.rings]
kind = "module"
entry = "rings.mach"
out = "{project.work}/rings{artifact.suffix}"
targets = ["vulkan-spirv"]
```

A shader artifact's `kind` is `module`. Its entry file and everything that file reaches compile into one SPIR-V module at `out`, where `{artifact.suffix}` is `.spv`.

A `bin` artifact in the same project must list its machine targets by name. `targets = ["*"]` would take in the shader target, which runs no executable.

`env` names the Vulkan version that consumes the module. It fixes the SPIR-V version and bounds the capabilities the module may declare.

| `env` | SPIR-V | The bound admits |
|---|---|---|
| `vulkan1.0` | 1.0 | The Vulkan 1.0 set |
| `vulkan1.1` | 1.3 | Also subgroup operations, 16-bit storage and variable pointers |
| `vulkan1.2` | 1.5 | Also 8-bit storage, descriptor indexing, the Vulkan memory model and physical pointers |
| `vulkan1.3` | 1.6 | Also demotion to a helper invocation and integer dot products |
| absent | 1.6 | No bound, with the optional device features on |

A capability that Vulkan leaves to an optional device feature must also be named in the target's `ext`, as `ext = ["Float64"]`. Without it an `f64` is refused:

```text
error[spirv.capability] 'app.rings' needs Float64, which the target's selection does not hold
```

## A fragment shader

Both shaders on this page import the mach-shader library, described under [The shader library](#the-shader-library).

```mach fragment
use sh: shader.math;

rec Frame {
    resolution: f32x2;
    time:       f32;
}

#[gpu.uniform(0, 0)]
var frame:      Frame;
#[gpu.builtin("frag_coord")]
var frag_coord: f32x4;
#[gpu.output(0)]
var out_colour: f32x4;

#[gpu.stage("fragment")]
fun shade() {
    val res:  f32x2 = frame.resolution;
    val p:    f32x2 = f32x2{frag_coord[0] - res[0] * 0.5, frag_coord[1] - res[1] * 0.5};
    val dist: f32   = sh.length_2(p) / res[1];
    val ring: f32   = 0.5 + 0.5 * sh.sin(dist * 40.0 - frame.time * 3.0);
    out_colour = f32x4{ring, ring * 0.4, 1.0 - ring, 1.0};
}
```

`#[gpu.stage]` makes a function a stage's entry point. Its argument is `"vertex"`, `"fragment"` or `"compute"`.

- A stage takes no parameters and returns nothing. It reads and writes module-level variables.
- The entry point's name in the module is the function's own name, here `shade`.
- A module may hold several stages. One that holds none is refused.

This file is `src/rings.mach`. Add the library, build, and validate:

```sh
mach dep add . shader --git https://github.com/briar-systems/mach-shader
mach build . -t vulkan-spirv
spirv-val --target-env vulkan1.3 out/vulkan-spirv/debug/rings.spv
```

The [playground](https://machlang.org/playground/) runs fragment shaders like this one in the browser.

## The interface

Each decorator goes on a module-level variable, and a variable carries exactly one.

| Decorator | The variable is |
|---|---|
| `gpu.input(n)` | A value read from the previous stage at location `n` |
| `gpu.output(n)` | A value written for the next stage at location `n` |
| `gpu.builtin("name")` | A value the pipeline supplies or consumes, such as `"position"`, `"frag_coord"` or `"global_invocation"` |
| `gpu.uniform(set, binding)` | A read-only block the host binds |
| `gpu.storage(set, binding)` | A read-write buffer the host binds |
| `gpu.sampler(set, binding)` | A texture or sampler handle from `shader.texture` |
| `gpu.push` | The push-constant block, read-only |
| `gpu.spec(id)` | A specialization constant the host may set when it creates the pipeline |

- A `gpu.uniform`, `gpu.storage` or `gpu.push` variable is a `rec`, even for a single value.
- `gpu.storage` takes qualifiers after the pair: `"readonly"`, `"writeonly"` and `"coherent"`.
- A `gpu.spec` variable is a scalar and needs an initializer, which is its default. Variables the host or the pipeline fills take none.
- Each built-in has a fixed type and stage. `"frag_coord"` is an `f32x4` in a fragment stage.

## The shader library

`sqrt`, `sin` and `dot` are not operators. The [mach-shader](https://github.com/briar-systems/mach-shader) library declares each as an `#[intrinsic]`: a function with no body that names one SPIR-V instruction, which every call to it becomes.

| Module | Holds |
|---|---|
| `shader.math` | Float and integer maths, derivatives, `dot_3`, `normalize_3`, `mix` |
| `shader.texture` | Image and sampler handles, sampling, fetches, storage images |
| `shader.compare`, `shader.logic`, `shader.vector` | Comparisons, selects and lane shuffles no operator writes |
| `shader.sync`, `shader.atomic`, `shader.subgroup` | Barriers, atomics and subgroup operations for compute |

A scalar function has the plain name and a vector one carries its lane count: `sqrt`, `sqrt_3`, `length_2`. Use the library instead of declaring an `#[intrinsic]` by hand.

## A compute shader

A compute stage has no varyings. It reads and writes `gpu.storage` buffers, and `#[gpu.workgroup(x, y, z)]` sizes its workgroup.

```mach fragment
use sh: shader.math;

rec Field {
    values: [1024]f32;
}

#[gpu.storage(0, 0, "readonly")]
var input:  Field;
#[gpu.storage(0, 1)]
var output: Field;
#[gpu.builtin("global_invocation")]
var id:     u32x3;

#[gpu.stage("compute")]
#[gpu.workgroup(64, 1, 1)]
fun roots() {
    val i: u32 = id[0];
    output.values[i] = sh.sqrt(input.values[i]);
}
```

Saved as `src/roots.mach`, it is a second artifact beside the first:

```toml
[artifact.roots]
kind = "module"
entry = "roots.mach"
out = "{project.work}/roots{artifact.suffix}"
targets = ["vulkan-spirv"]
```

`mach build . -t vulkan-spirv` then writes both modules. Without `gpu.workgroup` the workgroup is `(1, 1, 1)`.

Layout rules, textures, workgroup memory and the instructions an `#[intrinsic]` may name are in the reference for [GPU decorators](../language/gpu.md) and the [shader target](../language/manifest.md).
