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
#[packed]            # lay a rec / uni out with no padding (no arguments)
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
#[storage(set, bnd, "readonly")] # the same buffer, with writes refused
#[sampler(set, bnd)] # descriptor-bound image / sampler handle (global only)
#[op(tgt,set,name)]   # the target instruction this function is (bodyless fun only)
#[handle(tgt,ctor,..)] # the target type this declares (bodyless def only)
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

A mangled name is the source FQN, dotted, with generic arguments after a `$`:
`std.types.string.str_len`, `std.types.option.unwrap$ptr`. Each argument is
introduced by a run of `$` whose length is its nesting depth, so a nested
argument closes without a bracket — `f[Map[Vec[i64], str], u8]` is
`m.f$m.Map$$m.Vec$$$i64$$str$u8`. `p$u8` is `*u8`, `sec$u32` is `^u32`,
`arr4$u8` is `[4]u8`, `fn$$i64$$u8` is `fun(u8) i64`, a record is its own dotted
origin FQN, a comptime value is its literal, and a variadic-pack instance carries
a `pack` marker before its element list. A `test "label"` symbol keeps the quoted
label as its name. There is no prefix: a mangled name always contains a `.`, and
a C identifier never can. Both `.` and `$` are legal in an inline-asm symbol, so
any emitted symbol can be named from `asm` — but the spelling is not a stability
promise, and binding to one from C is not a supported use.

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

The alignment holds for **every** object of the type, including a local on the
stack. An ABI only promises the stack 16 bytes at a call boundary, so a function
holding a local that asks for more gets a prologue that masks the stack pointer
down to the largest alignment its frame contains, and addresses its locals and
spills from there. The cost falls on those functions alone: one masking
instruction and up to `N - 16` bytes of frame. A function with nothing
over-aligned emits exactly the prologue it always did.

The frame pointer stays where the ABI put it, so incoming stack arguments, the
frame record and a stack walk through it are unaffected.

### `packed` — no padding

Lays a `rec` or `uni` out with no padding: every field sits immediately after the
previous one, there is no padding at the tail, and the type takes no alignment from
its fields. Takes no arguments.

`align` only ever raises alignment. `packed` is the inverse, and it exists for the
case where the layout is not mach's to choose — a C struct, a file header, a wire
frame, a vertex whose stride a buffer fixes. Without it such a shape cannot be
described as a record at all.

```mach
#[packed]
rec Header {
    magic:    u8;    # offset 0
    version:  u16;   # offset 1
    length:   u32;   # offset 3
    checksum: u64;   # offset 7
}                    # $size_of == 15, $align_of == 1
```

Naturally the same shape is 24 bytes. `$size_of`, `$align_of` and `$offset_of` all
report the packed layout, and so does the code that reads and writes the fields —
there is one layout, not a declared one and an emitted one.

#### Composition with `align`

The two compose rather than conflict, and each owns one question:

- `packed` decides **padding** — none between fields, none at the tail.
- `align(N)` decides the **record's own alignment**, and rounds its size up to a
  multiple of `N`.

```mach
#[packed] #[align(8)]
rec Frame { a: u8; b: u32; }   # fields at 0 and 1; $align_of == 8, $size_of == 8
```

#### Packing is not transitive

A `packed` record packs **its own** fields. A record it contains keeps its own
internal padding and is merely *placed* without padding. This matches C, and it is
the rule that composes: an inner type's layout does not change depending on who
holds it.

```mach
rec Point { x: u8; y: u32; }   # natural: y at 4, size 8

#[packed]
rec Msg { tag: u8; p: Point; } # p at offset 1, still 8 bytes; $size_of(Msg) == 9
```

A transitive rule would make `Msg` 6 bytes and silently change `Point`'s meaning
inside it. If the inner record must be packed too, write `#[packed]` on it as well.

#### What is refused

**The address of a packed field.** `?r.b` on a packed record would yield a `*u32`,
and a `*u32` states alignment 4 to everything downstream of it while the storage it
names has none. The access through such a pointer is correct on the targets mach
supports today; the **pointer type** is what is untrue, and it travels — an atomic,
which every ISA requires naturally aligned, is exactly what a caller does with a
pointer it was handed. The refusal covers the whole access chain, so `?r.arr[0]`,
`?r.inner.x` and `?p.b` through a `*Packed` are refused for the same reason.

`?r` on the **whole** record stays legal: a `*Packed` describes an align-1 pointee
correctly, and nothing is lost by handing it out. To work with a field's value, copy
it into a local.

This is fail-closed on purpose. Refusing can be relaxed later, once alignment can
ride in a pointer type; permitting cannot be tightened later without breaking
programs that came to depend on it. Rust refuses; C permits, and it is a standing
source of faults.

**Atomics on a packed field** are refused by that same rule, not by one of their own.
`std.sync.atomic` is ordinary functions over `*i64`, so a pointer is the only route
an atomic has to a field, and there is no pointer to hand it.

**Vector fields.** A vector in a packed record is refused for now, including one
reached through an array or a nested record. The reason is evidence rather than
arithmetic: an unaligned **scalar** access is measured on real hardware, and that
measurement is what `#[packed]` rests on.

The vector measurement now exists too. `int/surface/unaligned-vector` stores and
loads `i32x4`, `f32x4`, `i32x3` and `f32x3` at deliberately misaligned offsets,
checks every lane by hand against a byte image rather than through the access being
measured, and asserts a sentinel in the bytes on both sides of each extent. It is
**correct in both profiles on every native leg**: `linux-arm64` on `ubuntu-24.04-arm`,
which is the row that matters because aarch64 has 128-bit forms with alignment
requirements, plus `linux` and `windows` on x86-64. Nothing faults, no lane is
dropped, and no neighbouring byte is disturbed. `linux-riscv64` also passes but is
not evidence: it runs under qemu-user, and riscv64 declares no 128-bit vector
support, so the access there is a scalar expansion rather than a vector access.

What the refusal still waits on is the other half,
[#2687](https://github.com/briar-systems/mach/issues/2687) — a lane-dependent vector
footprint through aggregate layout and ABI classification. This is a sequencing
decision and is expected to be lifted, not a permanent rule.

**Interface blocks.** `packed` cannot apply to a `#[uniform]` or `#[storage]` block:
its member offsets are fixed by the std140 / std430 layout rules and emitted as
explicit SPIR-V `Offset` decorations, which packing would contradict.

#### Target note: riscv64

Unaligned access is permitted-but-may-trap on RV64. Where the hardware does not do it,
Linux emulates the access in the kernel, so a packed field access there is expected to
be **correct and pathologically slow** — a trap-and-emulate round trip per access
rather than a load. Correctness tests on that target will pass and prove nothing about
usability, so treat a green riscv64 leg as evidence about correctness only.

This has not been measured on riscv64 hardware. It cannot be: qemu-user emulates a
misaligned guest load directly and never takes the kernel path, so a qemu measurement
shows no cost whether or not real silicon would. If the cost turns out to matter, the
answer is byte-wise lowering of packed field access on faulting targets, which is
codegen work and not part of `#[packed]` as it stands.

x86-64 and aarch64 do unaligned scalar access in hardware. The aarch64 answer is
measured on real hardware rather than assumed — `int`'s `linux-arm64` leg runs
natively.

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

### `input(n)` / `output(n)` / `builtin(str)` / `uniform(set, binding)` / `storage(set, binding)` / `sampler(set, binding)` — shader interface

A pipeline stage does not receive its inputs or return its results through a call.
It reads and writes **module-scope variables** that the pipeline binds, and these
directives say which kind each variable is. They apply only to module-level
`val` / `var` bindings, and a variable carries **exactly one** of them — they
are mutually exclusive.

```mach
#[input(0)]            var in_position: f32x4;
#[output(0)]           var out_colour:  f32x4;
#[builtin("position")] var position:    f32x4;

rec Camera { view: f32x4; proj: f32x4; }
#[uniform(0, 0)] var camera: Camera;

rec Particles { pos: [64]f32x4; }
#[storage(0, 1)] var particles: Particles;

#[sampler(1, 0)] var albedo: Sampler2D;
```

`input` and `output` number a **varying** with a location, which is how one
stage's outputs line up with the next stage's inputs: the producer's
`#[output(0)]` feeds the consumer's `#[input(0)]`.

`builtin` names a value the pipeline supplies or consumes instead of one a
location carries. The accepted set is closed:

| Value                 | Meaning                        | Type    | Direction |
|-----------------------|--------------------------------|---------|-----------|
| `"position"`          | clip-space vertex position     | `f32x4` | written   |
| `"point_size"`        | rasterized point size          | `f32`   | written   |
| `"vertex_index"`      | index of the current vertex    | `u32`   | read      |
| `"instance_index"`    | index of the current instance  | `u32`   | read      |
| `"frag_coord"`        | fragment window coordinate     | `f32x4` | read      |
| `"global_invocation"` | compute global invocation id   | `u32x3` | read      |
| `"local_invocation"`  | compute local invocation id    | `u32x3` | read      |
| `"workgroup_id"`      | compute workgroup id           | `u32x3` | read      |

The direction is a property of the built-in, not something you restate — a stage
writes its position and reads what the pipeline hands it — so there is no
input/output marker to pair with `builtin`, and none that could disagree with it.

The **type** is a property of the built-in too, and it is a requirement rather
than a suggestion: the pipeline binds the variable itself, so a wider or narrower
one is an invalid module rather than a wasteful one. Declaring a built-in at any
other type is a compile error naming both the declared type and the required one.
The two integer rows accept `i32` as well as `u32`, because the compiler carries
an integer's width and not its sign and the emitted type is sign-less either way.

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

`storage` takes **memory qualifiers** after the descriptor pair. There is one,
`"readonly"`, and it says that nothing writes the binding:

```mach
rec Palette { columns: [512]f32x4; }
#[storage(0, 3, "readonly")] var palette: Palette;
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

`sampler` binds a **handle** by descriptor set and binding, at the same descriptor
addressing `uniform` and `storage` use, so a host binds one the way it binds the
others. Its type must be a **handle type**, a bodyless `def` carrying `#[handle]`
(see [types.md](types.md)), and a handle type must carry this decorator: a handle
names a descriptor rather than an object with storage, so one with no descriptor
address is reachable from no stage. A handle cannot sit behind a pointer, inside an
array, or in a local binding, and each of those is a compile error naming why.

Sampling a handle is an `#[op(...)]` declaration rather than a language form,
because a sample IS one SPIR-V instruction like `sqrt` and `dot` are:

```mach
#[op("spirv", "core", "OpImageSampleImplicitLod")]
fun sample(s: Sampler2D, uv: f32x2) f32x4;

#[stage("fragment")]
fun frag_main() {
    out_colour = sample(albedo, in_uv);
}
```

The separately-bound form works the same way, with the instruction that combines
an image and a sampler declared alongside it:

```mach
#[op("spirv", "core", "OpSampledImage")]
fun combine(t: Texture2D, s: Sampler) Sampler2D;

#[sampler(1, 0)] var base_tex: Texture2D;
#[sampler(1, 1)] var base_smp: Sampler;

#[stage("fragment")]
fun frag_sep() { out_colour = sample(combine(base_tex, base_smp), in_uv); }
```

The combined value is handed straight to the sample rather than named: SPIR-V
requires an `OpSampledImage` result be consumed in the block that produced it,
which is the same rule that makes a handle-typed local a compile error.

As with `#[stage(...)]`, these are accepted on every target and acted on only by a
target that forms pipeline stages. On `spirv` each becomes an `OpVariable` in the
matching storage class, carrying the matching decoration, and the Input and Output
variables are named in every entry point's interface list. A `sampler` binding
becomes an `OpVariable` in the `UniformConstant` class — the one class Vulkan
permits an image, sampler or sampled-image variable in — carrying `DescriptorSet`
and `Binding` exactly as a `uniform` does.

### `handle(target, constructor, operands...)` — a type the target mints

A bodyless `def` carrying this directive declares a type whose representation is
**not the program's**: the owning target mints it and the pipeline binds it.

```mach
#[handle("spirv", "image", TEXEL_F32, DIM_2D, NO_DEPTH, NONARRAYED, SINGLE_SAMPLED, SAMPLED)]
pub def Texture2D;

#[handle("spirv", "sampled_image", Texture2D)]
pub def Sampler2D;
```

The first argument names the target and the second the type constructor within it.
Both are matched against the target's own definition table rather than evaluated,
so both must be string literals. Everything after them is **operands to that
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

### `op(target, set, name)` — a function that *is* a target instruction

A shader needs `sqrt`, `normalize`, `dot` and `mix`. None of them is an operator,
and none of them is a call SPIR-V can make: each is one instruction. This directive
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
with, so nothing about this directive is specific to one back end. All three value
sets are closed and checked at compile time, on **every** target: the directive is
legal everywhere, so a typo caught only where it is acted on would go unreported on
a CPU build. The parameter count is checked against the instruction's own operand
count, which is not uniform across a family that looks it.

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
| `packed`    |  no   |    no     |      no       |      yes      |
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
| `op`        |  yes  |    no     |      no       |      no       |
| `handle`    |  no   |    no     |      no       |      no       |

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
- [types.md](types.md) — the SIMD vector types a shader stage computes over and `op` operates on, and the handle types `handle` declares
- [../manifest.md](../manifest.md) — the `vectorize` profile key `scalar` opts out of, and content-fingerprinted build inputs (`embed`, `[step]` `in`)
