# Decorators

A decorator is a codegen directive attached to a declaration. It expresses
metadata that influences how the compiler emits the symbol: its linker name,
alignment, section placement, inlining, dynamic import attribution, constant-time
obligations, or exclusion from auto-vectorization.

Decorators are **codegen-only**. Visibility (`pub` / `ext`) is separate and
unaffected by decorators.

## Surface

A decorator is written as an attribute:

```
#[name]            # bare flag (e.g. inline)
#[name(args)]      # directive with comptime-expr arguments
```

> A backtick form (`` `name(args)` ``) existed through v2.3.0 and was removed in
> v2.4.0; a backtick at decorator position is now a migration error. `#[...]` is
> the only decorator surface.

> One caveat the attribute form introduces: a line comment that begins `#[`
> (with no space) opens an attribute. Write such a comment with a separating
> space — `# [...]`.

## Grammar

```
#[symbol("name")]    # linker name override
#[library("dep")]    # dynamic import attribution (ext only)
#[inline]            # force inlining (no arguments)
#[noinline]          # forbid inlining (no arguments)
#[align(expr)]       # alignment; expr is a comptime integer
#[section(".name")]  # place in a named object section
#[oblivious]         # constant-time boundary (no arguments)
#[scalar]            # opt out of auto-vectorization (no arguments)
#[naked]             # no prologue or epilogue; body as written (no arguments)
#[embed("path")]     # compile-time file embedding (val only)
#[stage("name")]     # GPU pipeline stage; makes the function an entry point
#[workgroup(x,y,z)]  # compute workgroup dimensions (with #[stage("compute")])
#[input(n)]          # shader interface input at location n (global only)
#[output(n)]         # shader interface output at location n (global only)
#[builtin("name")]   # pipeline built-in variable (global only)
#[uniform(set, bnd)] # descriptor-bound uniform block, read-only (global only)
#[storage(set, bnd)] # descriptor-bound storage buffer, read-write (global only)
#[spirv_op(set,name)] # the SPIR-V instruction this function is (fun only)
```

Decorators appear **before** the declaration they target, one per line or
space-separated on the same line. They attach to the immediately following
declaration only and do not bleed across declarations.

```mach
#[inline]
#[symbol("big")]
fun big(a: i64, b: i64) i64 { ... }

#[align(64)] #[symbol("g_lit64")]
pub var g_lit64: u8 = 7;
```

Each directive is wrapped in its own clause: `#[name]` for a bare flag or
`#[name(args)]` for a directive that takes arguments. Arguments are comptime
expressions.

## Directives

### `symbol(str)` — linker name

Overrides the emitted or imported symbol name. Applies to functions and
globals.

```mach
#[symbol("main")]
fun entry(argc: i64, argv: **u8) i64 { ... }

#[symbol("write")]
ext fun libc_write(fd: i64, buf: *u8, n: i64) i64;
```

Without `symbol`, the compiler mangles the Mach name — except on an `ext`
declaration, which names a C declaration and takes the target's C symbol name for
it (Darwin's underscore prefix, nothing elsewhere; see
[ext-fun.md](ext-fun.md)). `symbol` gives the exact name the linker sees, with no
platform prefix applied to it.

### `library(str)` — dynamic import attribution

Pins an `ext` import to a specific dependency in the link set. Applies to
`ext` functions only.

```mach
#[library("ws2_32.dll")] #[symbol("WSAStartup")]
ext fun wsa_startup(ver: u16, data: *u8) i32;
```

- The value normally names a `[link.X]` requirement's stable logical identity:
  its `library` value, or `X` when that key is omitted. A bare command-line
  `-l name` also exposes `name`. Exact canonical loader names remain accepted.
  Pinning to an absent dependency is a link error, never a silent fallback. A
  logical identity may not equal a different dependency's loader name.
- PE and Mach-O use two-level namespaces, so every dynamic import on those
  targets needs a `library` attribution.
- On ELF (Linux) the loader resolves imports by global search, so `library`
  has no effect on the emitted binary; the value is still validated against
  the link's dependency set.
- `library` composes with `symbol`: the import is emitted under the renamed
  symbol within the named dependency.

### `inline` — force inlining

Marks a function for inlining at every call site, overriding the compiler's
size- and use-count heuristics. Applies to functions only; takes no arguments.

```mach
#[inline]
fun fast_path(x: i64) i64 { ret x * 2; }
```

### `noinline` — forbid inlining

The inverse of `inline`: forbids inlining a function into any caller, overriding
the compiler's size- and use-count heuristics that would otherwise fold it in.
Applies to functions only; takes no arguments.

```mach
#[noinline]
fun cold_path(code: i64) i64 { panic("unreachable state"); }
```

Use it to keep a function's frame and symbol real — for a profiler or stack
sampler to attribute its cost correctly, to keep a cold path from bloating a hot
caller's instruction cache, or to hold code size down on a constrained target.

- `inline` and `noinline` on the same function is a direct contradiction and is
  rejected in sema; neither wins silently.
- `scalar` already declines inlining as a side effect (#2141), so pairing it
  with `noinline` is legal but redundant.
- Purely a hint to the inliner; it does not otherwise change codegen. It binds
  at every optimization level — the debug pipeline runs no inlining pass at
  all, so `noinline` is inert (and unnecessary) there — and it will bind
  identically to any future cross-module or LTO inlining path, which is not a
  separate mechanism exempt from it.

### `align(expr)` — alignment override

Sets the alignment of a global variable or a record/union type. `expr` must
be a comptime integer — either a literal or a comptime expression such as
`$size_of(T)` or `$align_of(T)`, in both positions.

A type's alignment is settled during type resolution, before layouts are otherwise
known; the measured type's layout is established on demand when the intrinsic asks
for it, so the answer does not depend on whether `T` is declared above or below.

```mach
#[align(64)]
pub var cache_line: u8 = 0;

#[align($size_of(Pair))]
pub var g_cmp: u8 = 0;

#[align($align_of(Pair))]
rec Over { a: u8; }
```

A type aligned to a measurement of itself — `#[align($size_of(Self))]`, or two
types each aligned to the other's size — is a layout cycle and is reported as one,
naming the type that closes it.

- On a `var` / `val`, sets the global's section alignment and address
  alignment.
- On a `rec` or `uni`, sets the type's own alignment, which is then inherited
  by any global of that type.
- `align` does not apply to `def` aliases (transparent, no layout of their
  own).

### `section(str)` — object section placement

Places a function or global variable in a named section instead of the
default `.text` / `.data`.

```mach
#[section(".hottext")] #[symbol("f_hot")]
fun f_hot(x: i64) i64 { ret x + 1; }

#[section(".machsec")] #[symbol("g_sec")]
pub var g_sec: u64 = 100;
```

The named section is created if absent. Cross-section calls and accesses use
ordinary relocations.

### `oblivious` — constant-time boundary

Marks a function as a constant-time boundary. Applies to functions only; takes
no arguments. Inside it the backend must not introduce a secret-dependent
branch or select a variable-latency instruction on a secret operand; a
translation validator re-derives the secret taint over the lowered MIR and
rejects any such leak.

Inline `asm` inside such a function is **validated rather than rejected**: the
block is parsed and walked for the same leaks, and refused only where a leak is
found or where the construct cannot be modelled. See
[secrecy.md](secrecy.md#oblivious--the-codegen-contract) for what is checked and
what is refused, including the x86-64 conditional-branch limitation.

The **zeroizing-write** guarantee is *not* one of the decorator's obligations,
and describing it as one understates it. A write into secret storage carries a
taint applied at lowering, keyed on the storage's secrecy rather than on any
decorator, so a zeroizing wipe is protected in a function carrying no
`#[oblivious]` at all. See [secrecy.md](secrecy.md#the-zeroizing-write-guarantee)
for what that covers and what it does not.

```mach
#[oblivious]
fun ct_eq(a: ^[8]u8, b: ^[8]u8) u8 { ... }
```

The decorator is purely subtractive — on a secret-free function it is a no-op.
A function instance that *computes* on a `^` secret (arithmetic, bitwise, shift,
comparison, negation) is **required** to carry it; an instance that only moves,
stores, or declassifies secrets stays annotation-free.

It is rejected outright for a target whose back half emits a module for a
downstream compiler rather than the executed instructions (the experimental
SPIR-V backend): the contract cannot be validated or upheld there.

> **Experimental preview.** The constant-time guarantee is not complete and has
> not been audited — see [secrecy.md](secrecy.md#assurance) for the known open
> holes. Do not build production cryptography on it at this version.

### `scalar` — opt out of auto-vectorization

Excludes a function from loop auto-vectorization, so its loops compile to scalar
code even in the release pipeline on a vector-capable target. Applies to
functions only; takes no arguments.

```mach
#[scalar]
fun reference_sum(a: *i64, n: usize) i64 { ... }
```

A `#[scalar]` function is also declined by the inliner, so the opt-out survives
inlining — it cannot be lost by the body moving into an unflagged caller. Use it
for a scalar reference twin in a differential test, or where vectorized codegen
is undesirable for a specific function. The project-wide equivalent is the
`vectorize` profile key (see [manifest.md](../manifest.md#profilename)).

### `naked` — no prologue, no epilogue, body as written

Emits the function's body exactly as written and nothing else: no frame-pointer
record, no stack allocation, no callee-save stores, no argument moves, and no
return. Applies to functions only; takes no arguments.

```mach
#[naked] #[symbol("_start")]
fun start() {
    $if ($mach.build.arch == $mach.arch.x86_64) {
        asm x86_64 {
            mov rdi, [rsp]        # argc, straight off the kernel-supplied stack
            lea rsi, [rsp+8]      # argv
            call main
        }
    }
    $or { asm aarch64 { ... } }
}
```

The programmer owns the frame, the stack alignment, the link register, and the
return. That is the whole point: a reset vector, an interrupt handler that must
return with `iret`/`rti` rather than `ret`, a syscall or context-switch stub, or
a thread entry point whose register state at entry *is* the interface.

- **The body may contain only inline `asm`** — plus the `$if $mach.build.arch`
  chain that is how mach spells per-ISA assembly. Any other statement is
  rejected. A local, an expression, or a `ret` lowers to code that assumes a
  frame the function does not have, and the result would run and return a wrong
  answer rather than fail.
- **No return is generated.** If the asm falls off the end, control runs into
  whatever the linker placed next. Write the return the ABI (or the interrupt
  controller) actually calls for.
- **Parameters and the return type are still checked** at every call site, so a
  naked function is called like any other. No moves are emitted for them: the
  arguments arrive in the ABI's registers and the body reads them there.
- **Mutually exclusive with `inline`** — there is no coherent winner between a
  body spliced into a caller and one that owns its own frame — and with
  `oblivious`, which already forbids inline asm because a type system cannot
  check it. Both combinations are rejected in sema. `noinline` is redundant: the
  inliner declines a naked function unconditionally.
- **Debug info carries no `DW_AT_frame_base`** for a naked subprogram. Every
  other function has a frame base the compiler established and can name; this
  one does not, so it declares none rather than pointing at a register the asm
  may have moved.

Frame *elision* is a separate, automatic thing: the compiler already omits the
prologue for a leaf that provably never touches its frame. `naked` is the
declared form, and it is unconditional — it suppresses the frame whether or not
the compiler could prove it safe, because the proof obligation is the author's.
Merely containing an `asm` block does **not** suppress a frame: a function that
also makes a call gets one, since an unaligned call boundary (x86-64) or a
clobbered link register (aarch64, riscv64) is not something the author asked
for by writing assembly.

### `embed(str)` — compile-time file embedding

Sources a `val`'s bytes from a file at compile time: the file's content **is**
the initializer. Applies to `val` only — not `var` (the storage is read-only
data) and not an `ext` data import (which has no storage here). Takes one
string-literal argument.

```mach
#[embed("assets/logo.qoi")]
val LOGO: [_]u8;          # length taken from the file's byte count

#[embed("boot/sector.bin")]
val SECTOR: [512]u8;      # length pinned; a size change fails the build
```

- The declaration carries no initializer of its own; writing one alongside
  `embed` is rejected. This is a second exemption to `val`'s
  requires-an-initializer rule, alongside `ext` (see
  [val-var.md](val-var.md#ext--foreign-data-imports)).
- Exactly one argument, a string literal. Escapes are **not** decoded, matching
  `symbol` and `section` — the path is taken as written.
- The path resolves relative to the **declaring source file's** directory. An
  absolute path is taken as written.
- The annotation must be `[_]u8` or `[N]u8`; the element type must be `u8`.
  `[_]` is an inferred array length, legal **only** on an `#[embed]`
  declaration — written anywhere else it is rejected (see
  [grammar.md](grammar.md#types)). A `[_]u8` embed can be asked for its own
  length: `$length_of(LOGO)` is its element count and `$size_of(LOGO)` its byte
  count, both folded at compile time (see
  [comptime-intrinsics.md](comptime-intrinsics.md)). The explicit `[N]u8` form
  is for pinning a size by contract, not for recovering one.
- An explicit `[N]u8` whose `N` disagrees with the file is rejected, naming
  both counts. This is how a declaration pins a fixed-size asset — a boot
  sector, a ROM image — so the build fails the moment it stops being that
  size.
- Bytes are placed in read-only data exactly like any other constant byte
  array: no runtime I/O, no copy. Works for every artifact kind and target,
  freestanding included.
- Two `#[embed]` globals whose files hold byte-identical content and whose final
  section name, kind, and alignment match share **one** read-only data placement
  within a module, so their addresses compare equal. This is specific to
  embedded data — an ordinary global is never merged this way, and a named
  object's address is otherwise its own.
- A missing file, a directory where a file is required, an unreadable file, and
  a file larger than the 4,294,967,295-byte array/section limit each report once,
  naming the declaration and the resolved path.
- The embedded file is a build input: its content digest feeds the embedding
  module's incremental cutoff, so editing the asset invalidates that module
  and an untouched asset stays a cache hit — see
  [manifest.md](../manifest.md#stepname--build-steps) for the equivalent
  guarantee on `[step]` `in` entries.

### `stage(str)` — GPU pipeline stage

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
fun vertex_main() { }

#[stage("fragment")]
fun fragment_main() { }
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

### `workgroup(x, y, z)` — compute workgroup dimensions

Sizes the workgroup of a `#[stage("compute")]` function. The three arguments are
comptime integers giving the x, y and z dimensions.

```mach
#[stage("compute")] #[workgroup(64, 1, 1)]
fun compute_main() { }
```

It requires a stage on the same function — without one it would silently mean
nothing — and it applies only to the compute stage. When it is omitted, a compute
stage takes the single-invocation default `(1, 1, 1)`; the dimensions are always
declared in the emitted module, since a compute stage that does not state its
workgroup size is not one a consumer can dispatch.

### `input(n)` / `output(n)` / `builtin(str)` / `uniform(set, binding)` / `storage(set, binding)` — shader interface

A pipeline stage does not receive its inputs or return its results through a call.
It reads and writes **module-scope variables** that the pipeline binds, and these
four directives say which kind each variable is. They apply only to module-level
`val` / `var` bindings, and a variable carries **exactly one** of them — the four
are mutually exclusive.

```mach
#[input(0)]            var in_position: f32x4;
#[output(0)]           var out_colour:  f32x4;
#[builtin("position")] var position:    f32x4;

rec Camera { view: f32x4; proj: f32x4; }
#[uniform(0, 0)] var camera: Camera;

rec Particles { pos: [64]f32x4; }
#[storage(0, 1)] var particles: Particles;
```

`input` and `output` number a **varying** with a location, which is how one
stage's outputs line up with the next stage's inputs: the producer's
`#[output(0)]` feeds the consumer's `#[input(0)]`.

`builtin` names a value the pipeline supplies or consumes instead of one a
location carries. The accepted set is closed:

| Value                 | Meaning                        | Direction |
|-----------------------|--------------------------------|-----------|
| `"position"`          | clip-space vertex position     | written   |
| `"point_size"`        | rasterized point size          | written   |
| `"vertex_index"`      | index of the current vertex    | read      |
| `"instance_index"`    | index of the current instance  | read      |
| `"frag_coord"`        | fragment window coordinate     | read      |
| `"global_invocation"` | compute global invocation id   | read      |
| `"local_invocation"`  | compute local invocation id    | read      |
| `"workgroup_id"`      | compute workgroup id           | read      |

The direction is a property of the built-in, not something you restate — a stage
writes its position and reads what the pipeline hands it — so there is no
input/output marker to pair with `builtin`, and none that could disagree with it.

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

As with `#[stage(...)]`, these are accepted on every target and acted on only by a
target that forms pipeline stages. On `spirv` each becomes an `OpVariable` in the
matching storage class, carrying the matching decoration, and the Input and Output
variables are named in every entry point's interface list.

### `spirv_op(set, name)` — a function that *is* a SPIR-V instruction

A shader needs `sqrt`, `normalize`, `dot` and `mix`. None of them is an operator,
and none of them is a call SPIR-V can make: each is one instruction. This directive
says which one a function is, so that on a `spirv` target a call to it becomes that
instruction, inline, rather than a call.

```mach
#[spirv_op("GLSL.std.450", "Sqrt")]
pub fun sqrt(x: f32) f32;

#[spirv_op("GLSL.std.450", "Normalize")]
pub fun normalize(v: f32x4) f32x4;

#[spirv_op("core", "OpDot")]
pub fun dot(a: f32x4, b: f32x4) f32;
```

The first argument names the instruction set and the second the instruction within
it. Both value sets are closed and checked at compile time, on **every** target —
the directive is legal everywhere, so a typo caught only where it is acted on would
go unreported on a CPU build.

| Set              | Meaning                                                     |
|------------------|-------------------------------------------------------------|
| `"core"`         | the core opcode space; needs no import                       |
| `"GLSL.std.450"` | the standard extended set; imported once per module, on use  |

The substitution is uniform: the emitted instruction's **result type is the
function's declared return type** and its **operands are the function's parameters
in declaration order**. That is what lets `dot` and `length` return a scalar from
vectors, and `refract` mix a scalar operand with vector ones, without any of them
being a special case.

`OpExtInstImport "GLSL.std.450"` is emitted **once per module and only when that
module uses the set**. A module that calls none of these carries no import.

Note that `dot` is **core `OpDot`**, not a GLSL.std.450 instruction, even though
GLSL spells it beside `normalize` and `length`. Check each function against the
specification rather than against GLSL's surface.

On every target other than `spirv` the directive is inert, and a decorated function
is an ordinary function. A **bodiless** one — which is what the shader-side maths
library uses — is then an undefined symbol, so a CPU build that calls it fails at
link time naming the symbol. That is a deliberate design choice on the library's
part, not a property of the directive: a decorated function may have a body, and if
it does, that body is what every non-`spirv` target runs while `spirv` substitutes
the instruction. A `spirv` build never emits the body at all.

The set of accepted instruction names is a table in `mach.lang.spirvop`, which also
records why each omission from GLSL.std.450 is one. Adding an instruction is a row
in it.

## Applicability

| Directive   | `fun` | `ext fun` | `val` / `var` | `rec` / `uni` |
|-------------|:-----:|:---------:|:-------------:|:-------------:|
| `symbol`    |  yes  |    yes    |      yes      |      no       |
| `library`   |  no   |    yes    |      no       |      no       |
| `inline`    |  yes  |    no     |      no       |      no       |
| `noinline`  |  yes  |    no     |      no       |      no       |
| `align`     |  no   |    no     |      yes      |      yes      |
| `section`   |  yes  |    yes    |      yes      |      no       |
| `oblivious` |  yes  |    no     |      no       |      no       |
| `scalar`    |  yes  |    no     |      no       |      no       |
| `naked`     |  yes  |    no     |      no       |      no       |
| `embed`     |  no   |    no     |      yes      |      no       |
| `stage`     |  yes  |    no     |      no       |      no       |
| `workgroup` |  yes  |    no     |      no       |      no       |
| `input`     |  no   |    no     |      yes      |      no       |
| `output`    |  no   |    no     |      yes      |      no       |
| `builtin`   |  no   |    no     |      yes      |      no       |
| `uniform`   |  no   |    no     |      yes      |      no       |
| `storage`   |  no   |    no     |      yes      |      no       |
| `spirv_op`  |  yes  |    no     |      no       |      no       |

The `val` / `var` column is shared, but `embed` accepts only `val` — a `var`
is refused (see [`embed`](#embedstr--compile-time-file-embedding) above).

The set is closed. New directives require a compiler change.

## See also

- [ext-fun.md](ext-fun.md) — `ext` imports, `library` and `symbol` use cases
- [visibility.md](visibility.md) — `pub` / `ext` visibility (not decorator-controlled)
- [comptime-intrinsics.md](comptime-intrinsics.md) — `$size_of` / `$align_of` as `align` arguments
- [secrecy.md](secrecy.md) — `^` secret types and the `oblivious` constant-time contract
- [asm.md](asm.md) — inline `asm`, the only body a `naked` function may have
- [val-var.md](val-var.md) — `val` / `var` bindings, and the `embed` exemption to `val`'s initializer requirement
- [grammar.md](grammar.md#types) — the `[_]` inferred array length `embed` introduces
- [types.md](types.md) — the SIMD vector types a shader stage computes over and `spirv_op` operates on
- [../manifest.md](../manifest.md) — the `vectorize` profile key `scalar` opts out of, and content-fingerprinted build inputs (`embed`, `[step]` `in`)
