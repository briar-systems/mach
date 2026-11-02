# Decorators

A decorator attaches metadata to a declaration. It can provide source-use
notices or influence how the compiler emits the symbol: its linker name,
alignment, section placement, inlining, dynamic import attribution, constant-time
obligations, or exclusion from auto-vectorization.

Visibility (`pub` / `ext`) is separate. `#[export]` builds on `pub` and never
replaces it.

## Surface

A decorator is written as `#[...]`:

```
#[<name>]          # bare flag (e.g. naked)
#[<name>(args)]    # decorator with constant arguments
#[gpu.<name>(..)]  # a decorator of the GPU set
```

Every name is lowercase, with `snake_case` where it is several words. Only the
GPU set is dotted: it is a large domain that applies only to shader stages and
uses generic words such as `input` and `shared`.

> `#[...]` is the only decorator surface. A backtick is not a token: one
> anywhere in source is a lexer error.

> One caveat the decorator form introduces: a line comment that begins `#[`
> (with no space) opens a decorator. Write such a comment with a separating
> space — `# [...]`.

## Grammar

```
#[deprecated]        # external uses warn
#[deprecated("msg")] # external uses warn with this message
#[testing]           # exists only for tests; omitted from ordinary builds
#[expect("key", ..)] # acknowledge the named warnings inside this declaration
#[symbol("name")]    # linker name override
#[library("dep")]    # dynamic import attribution (ext only)
#[export]            # export from a shared library (pub only)
#[inline("always")]  # force inlining
#[inline("never")]   # forbid inlining
#[align(expr)]       # alignment; expr is a comptime integer
#[packed]            # lay a rec / uni / tag out with no padding (no arguments)
#[volatile]          # every access to a rec / uni / tag is a volatile access (no arguments)
#[section(".name")]  # place in a named object section
#[at(offset)]        # place a module global at an offset in the image
#[thread]            # one copy of a module var per thread (no arguments)
#[constant_time("soft")] # constant-time boundary; the mode is "soft" or "hard"
#[vectorize("never")] # opt out of auto-vectorization
#[naked]             # no prologue or epilogue; body as written (no arguments)
#[extension("a", "b")] # the function may use these instruction-set extensions
#[embed("path")]     # compile-time file embedding (val only)
#[intrinsic(tgt,set,name)] # the target instruction this function is (bodyless fun only)
#[handle(tgt,ctor,..)] # the target type this declares (bodyless def only)
#[abi_type("name")]   # a C type whose layout the target declares (bodyless def only)
#[gpu.stage("name")]     # GPU pipeline stage; makes the function an entry point
#[gpu.workgroup(x,y,z)]  # compute workgroup dimensions, constants or #[gpu.spec] vars (with #[gpu.stage("compute")])
#[gpu.input(n)]          # shader interface input at location n (global only)
#[gpu.output(n)]         # shader interface output at location n (global only)
#[gpu.builtin("name")]   # pipeline built-in variable (global only)
#[gpu.uniform(set, bnd)] # descriptor-bound uniform block, read-only (global only)
#[gpu.storage(set, bnd)] # descriptor-bound storage buffer, read-write (global only)
#[gpu.storage(set, bnd, "readonly")] # the same buffer, with writes refused
#[gpu.storage(set, bnd, "writeonly")] # the same buffer, with reads refused
#[gpu.storage(set, bnd, "coherent")] # the same buffer, its writes visible across workgroups
#[gpu.sampler(set, bnd)] # descriptor-bound image / sampler handle (global only)
#[gpu.push]              # push-constant block, read-only (global only)
#[gpu.spec(id)]          # specialization constant the host supplies (module var only)
#[gpu.shared]            # compute workgroup memory, zero when a stage starts (global only)
```

Decorators appear **before** the declaration they target, one per line or
space-separated on the same line. They attach to the immediately following
declaration only and do not bleed across declarations.

```mach fragment
#[inline("always")]
#[symbol("big")]
fun big(a: i64, b: i64) i64 { ... }

#[align(64)] #[symbol("g_lit64")]
pub var g_lit64: u8 = 7;
```

Each decorator is wrapped in its own clause: `#[<name>]` for a bare flag or
`#[<name>(args)]` for a decorator that takes arguments. Every argument is a
constant expression: a number is an integer, and every name, mode or path is a
string. No decorator takes a bare word. A mode is an argument of one
decorator, never a sibling decorator: `inline("never")`, not `noinline`.

### Constant arguments

Where a decorator takes a string or an integer, the argument is a constant
expression of that type, evaluated at compile time in the declaring module. A
literal is one. So is a `val`, one an `$if` arm selects, or one imported from
another module. A decorator's argument never depends on the build that
evaluates it, only on the constants in scope. `#[symbol]` stays exactly the
name you give it: the compiler adds no platform decoration of its own, and a
`val` gated by comptime is how one declaration names its symbol per target.

```mach
$if ($mach.build.sys == $mach.sys.darwin) {
    val SPIN: *u8 = "_spin";
}
$or {
    val SPIN: *u8 = "spin";
}

#[symbol(SPIN)]
pub fun spin() i64 {
    ret 0;
}
```

An argument that is not a constant expression, such as a `var` or a call, is
refused with `decorator.not_constant`, naming the decorator. A constant of the
wrong type is refused with `decorator.argument`.

## The decorator set

### `deprecated` / `deprecated(str)` — source-use notice

Marks a declaration deprecated. The optional argument is one constant string
carrying a message. Repeating the decorator, giving it more than one argument,
or giving it an argument that is not a constant string is an error. The decorator changes nothing about visibility, type identity, ABI or
codegen.

A use of the deprecated declaration from another source module warns at the
identifier, carrying the message. Value references, calls and type references
are covered, including through imports, re-exports and generic instantiation,
and each source site warns once even when a generic body is instantiated more
than once. The declaring module does not warn on its own uses, and an unused
import alone produces no warning.

```mach
# file: src/legacy.mach
#[deprecated("use replacement")]
pub fun old() i32 {
    ret replacement();
}

pub fun replacement() i32 {
    ret 1;
}

# file: src/main.mach
use example.legacy;

fun caller() i32 {
    ret legacy.old(); # warning: `old` is deprecated: use replacement
}
```

It applies to `fun`, `rec`, `uni`, `tag`, `def`, `val`, `var`, `use` and `fwd`
declarations, and to a tag case, where it is the only decorator a case accepts:

```mach
# file: src/reply.mach
#[deprecated("the whole tag")]
pub tag Old: u8 { empty; }

pub tag Reply: u8 {
    empty;
    #[deprecated("use fresh")]
    value: i64;
    fresh: i64;
}

# file: src/main.mach
use example.reply;

fun read(r: reply.Reply) i64 {
    if (sel r.value) { ret r.value; } # both sites warn: tag case `value` is deprecated: use fresh
    ret 0;
}

fun make() reply.Reply {
    ret reply.Reply.value{1}; # construction warns too
}

fun stale() reply.Old {
    ret reply.Old.empty{}; # warning: `Old` is deprecated: the whole tag
}
```

A deprecated case warns at every external use that names it: `Reply.value{...}`
construction, the `sel place.value` test and the `place.value` payload place.
Descriptor forms that name no case in source (`Reply.[c]{...}`, `sel v.[c]`,
`v.[c]`) warn nowhere.

Notices follow imported symbols and re-exports. A `#[deprecated]` on a `use`
alias or a `fwd` re-export belongs to the forwarding module and replaces any
inherited notice for that exported name; a clean alias of the same canonical
definition keeps no notice. `test` blocks and comptime directives reject the
decorator because they declare no externally usable name.

### `testing` — test-only declaration

Marks a declaration as existing only for tests, giving a fixture the semantics
a [`test`](test.md) block already has. It takes no arguments and may appear
once.

A `#[testing]` declaration is resolved and type-checked in every build, so it
cannot rot, and a type error in one fails `mach build` as well as `mach test`.
It is omitted from IR and object files in ordinary builds and emitted under
`mach test`.

A reference to a `#[testing]` declaration is legal only from a `test` body or
from another `#[testing]` declaration, including its decorators. Any other
reference is an error at the use site. Every reference that names the
declaration is checked: value references, calls, address-of, type references
(a field, parameter or return type of a production declaration), generic
instantiation, comptime evaluation and decorator arguments. The check has no
same-module exemption.

```mach fragment
# file: src/queue.mach
#[testing]
pub fun filled(n: u32) Queue { ... }

test queue__drains_in_order {
    var q: Queue = filled(3);          # a test body may use the fixture
    ...
}

#[testing]
fun drained() Queue { ret filled(0); } # so may another testing declaration

pub fun reset() Queue { ret filled(0); }
# error[testing.use]: `filled` is a `#[testing]` declaration: only a test body or another
#        `#[testing]` declaration may reference it
```

The mark follows imports. A plain `use` of a testing declaration is itself
testing, so every reference through it is checked. A `#[testing] use` confines
its alias even when the target is an ordinary declaration. A `fwd` of a testing
declaration must itself be marked `#[testing]`, because an unmarked `fwd` puts
the name on a production surface. `pub #[testing]` is valid, and a dependency's
testing helper is usable from a dependent's tests.

It applies to `fun`, `rec`, `uni`, `tag`, `def`, `val`, `var`, `use` and `fwd`
declarations at module scope. `test` blocks reject it because they are already
test-only, and comptime directives reject it too. It cannot combine with
`ext` or with `symbol`, `section`, `stage`, `input`, `output`, `builtin`,
`uniform`, `storage`, `sampler` or `push`, because each names a consumer outside Mach
source that the check cannot see. Inline `asm` `{name}` operands bind only
locals, so they never reference a declaration.

The mark is not a cycle escape: `mach test` builds with testing declarations
present, so a `use` cycle that only tests need is still an error. `mach doc`
omits testing declarations.

### `expect(key)` — acknowledge a warning

Acknowledges warnings a declaration raises on purpose. Each argument is a
constant string naming a warning key, or a family of keys, from the
[diagnostic key table](manifest.md#silencing-warnings). A warning of a named
kind raised inside the declaration, its doc comment and body included, is
neither printed nor counted. Warnings elsewhere, and warnings of other kinds,
still report.

```mach fragment
#[expect("vector.scalarize")]
#[testing]
fun divide_i32x4(a: i32x4, b: i32x4) i32x4 {
    ret a / b; # scalarizes on x86_64 and aarch64, by design
}

#[expect("float")]
test float__rounding {
    val x: f32 = 3.14159265358979; # float.inexact, acknowledged by its family
    ...
}
```

A warning is matched to the declaration whose source contains its site. That
holds for a warning raised late in the build, such as `vector.scalarize` from
lowering, and for a generic instance, whose warnings belong to the generic's
declaration. A warning replayed from the build cache is matched exactly as a
fresh one, and a warning `#[expect]` covers counts as acknowledged even when a
profile's `allow` silences its key too.

An acknowledgement cannot rot silently. When a key the source alone decides
(`import.unused`, `decl.deprecated`, `doc.lint`, `float.inexact`) names no
warning raised inside the declaration, that is itself a warning,
`expect.unfulfilled`. A key whose warning depends on the target or on what the
build compiles, such as `vector.scalarize`, may be quiet in a given build, so
its expectation is not reported. A build with errors judges no expectation.

It applies to `fun`, `rec`, `uni`, `tag`, `val`, `var` and `test` declarations
and may appear once, naming every key it acknowledges. There is no
module-wide form: a profile's [`allow`](manifest.md#silencing-warnings)
silences a key across the build. An unknown key, a key that covers only
errors, and a key named twice are refused, since an error is never
acknowledged:

```
error[expect.error_key]: `secret.not_oblivious` names an error, and an error is never acknowledged
```

### `symbol(str)` — linker name

Overrides the emitted or imported symbol name. Applies to functions and
globals.

```mach fragment
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
`std.types.string.str_len`, `std.collections.vector.push$ptr`. Each argument is
introduced by a run of `$` whose length is its nesting depth, so a nested
argument closes without a bracket — `f[Map[Vec[i64], str], u8]` is
`m.f$m.Map$$m.Vec$$$i64$$str$u8`. `p$u8` is `*u8`, `sec$u32` is `^u32`,
`arr4$u8` is `[4]u8`, `fn$$i64$$u8` is `fun(u8) i64`, a record is its own dotted
origin FQN, a comptime value is its literal, and a variadic-pack instance carries
a `pack` marker before its element list. A test's symbol is its qualified name,
`<module path>#<identifier>` (see [test.md](test.md#grammar)). There is no prefix:
a mangled name always contains a `.` or a `#`, and a C identifier never can. Both `.` and `$` are legal in an inline-asm symbol, so
any emitted symbol can be named from `asm` — but the spelling is not a stability
promise, and binding to one from C is not a supported use.

### `library(str)` — dynamic import attribution

Pins an `ext` import to a specific dependency in the link set. Applies to
`ext` functions only.

```mach
#[library("ws2_32")]
#[symbol("WSAStartup")]
ext fun wsa_startup(ver: u16, data: *u8) i32;
```

- The value names a `[link.X]` requirement by its table key, `X`. A bare
  command-line `-l name` exposes `name`. A loader name such as `ws2_32.dll`
  does not bind, and an import attributed to one is refused with a message
  naming the entry whose key to write. Pinning to an absent dependency is a
  link error, never a silent fallback. A key may not equal a different
  dependency's loader name.
- PE and Mach-O use two-level namespaces, so every dynamic import on those
  targets needs a `library` attribution.
- On ELF (Linux) the loader resolves imports by global search, so `library`
  has no effect on the emitted binary; the value is still validated against
  the link's dependency set.
- `library` composes with `symbol`: the import is emitted under the renamed
  symbol within the named dependency.

### `inline(mode)` — force or forbid inlining

Takes one mode, `"always"` or `"never"`, and applies to functions only. It may
stand once on a function, so the two modes never meet.

`inline("always")` marks a function for inlining at every direct call site,
overriding the compiler's size and use-count heuristics and exempt from the
caller's expansion budget. The optimization
pipeline must enable inlining. Indirect calls and recursive call cycles are not
expanded by this decorator. Taking a function's address retains its callable
identity even when direct calls are inlined.

Release optimization makes small ordinary helper bodies available across source
modules without emitting extra definitions. A helper is small when its body has
fewer than 25 live instructions after promotion, debug annotations excluded, so
`debug = true` never moves the decision. Extraction, import and per-caller heuristic expansion each
have a limit of 1024 copied IR instructions and 256 KiB of owned payload. An
`inline` function is expanded at every direct call site in the same module
without charging that budget, so the outcome never depends on what else the
caller expanded first; across modules it is imported within the extraction and
import limits like any other body. A
`constant_time` function is expanded only into another `constant_time` function, so
its instructions never leave a constant-time validated body; a `naked` or
`inline("never")` function, a recursive cycle and an indirect call are never expanded.
A remaining call or taken address still names the original defining function.
Generic, comptime and pack specializations keep their existing shared weak
linkage. When several modules materialize that same specialization, body import
uses an already available definition or the first acquired provider of that
linkage, and tracks that provider as a query dependency.
Helpers referencing compiler-local literal pools retain their calls because those
objects have module-local identity. Named globals keep their original symbols,
and copied instructions preserve effects, assembly bindings and debug locations.

```mach
#[inline("always")]
fun fast_path(x: i64) i64 {
    ret x * 2;
}
```

`inline("never")` forbids inlining a function into any caller, overriding the
compiler's size- and use-count heuristics that would otherwise fold it in.

```mach
#[inline("never")]
fun cold_path(code: i64) i64 {
    ret code * 100;
}
```

Use it to keep a function's frame and symbol real — for a profiler or stack
sampler to attribute its cost correctly, to keep a cold path from bloating a hot
caller's instruction cache, or to hold code size down on a constrained target.

- A second `inline` on the same function is refused with `decorator.duplicate`,
  so `"always"` and `"never"` never contradict each other.
- `vectorize("never")` already declines inlining as a side effect, so pairing it
  with `inline("never")` is legal but redundant.
- Purely a hint to the inliner; it does not otherwise change codegen. It binds
  at every optimization level — the debug pipeline runs no inlining pass at
  all, so `inline("never")` is inert (and unnecessary) there — and it will bind
  identically when a callee body is available from another module. Recursive
  peeling also respects `inline("never")` and `vectorize("never")`.
- v6's bare `#[inline]` is `#[inline("always")]`, and its `#[noinline]` is
  `#[inline("never")]`. Each v6 spelling warns, naming its replacement (see
  [v6 spellings](#v6-spellings)).

### `align(expr)` — alignment override

Sets the alignment of a global variable, a record/union type, or a function's
entry, which the object places at that boundary. `expr` must
be a comptime integer — either a literal or a comptime expression such as
`$size_of(T)` or `$align_of(T)`, in both positions.

A type's alignment is settled during type resolution, before layouts are otherwise
known; the measured type's layout is established on demand when the intrinsic asks
for it, so the answer does not depend on whether `T` is declared above or below.

```mach
rec Pair { a: u64; b: u64; }

#[align(64)]
pub var cache_line: u8 = 0;

#[align($size_of(Pair))]
pub var g_cmp: u8 = 0;

#[align($align_of(Pair))]
rec Over { a: u8; }

#[align(64)]
fun hot(n: i64) i64 {
    ret n + 1;
}
```

A type aligned to a measurement of itself — `#[align($size_of(Self))]`, or two
types each aligned to the other's size — is a layout cycle and is reported as one,
naming the type that closes it.

- On a `var` / `val`, sets the global's section alignment and address
  alignment.
- On a `rec` or `uni`, sets the type's own alignment, which is then inherited
  by any global of that type.
- On a `fun`, sets the alignment of the function's entry address. Without it
  a function still starts at the target's own entry alignment: 16 bytes on
  x86-64 and aarch64, as gcc and llvm align them, and no padding on riscv. The
  bytes before an entry are an instruction that traps. `align` only raises the
  target's alignment, so a value below it changes nothing.
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
its fields. On a `tag` it places the payload immediately after the discriminator
(see [tag.md](tag.md)). Takes no arguments.

`align` only ever raises alignment. `packed` is the inverse, and it exists for the
case where the layout is not mach's to choose — a C struct, a file header, a wire
frame, a vertex whose stride a buffer fixes. Without it such a shape cannot be
described as a record at all.

```mach
use std.print;
use std.runtime;

#[packed]
rec Header {
    magic:    u8; # offset 0
    version:  u16; # offset 1
    length:   u32; # offset 3
    checksum: u64; # offset 7
} # $size_of == 15, $align_of == 1

#[symbol("main")]
fun main(argc: i64, argv: **u8) i64 {
    print.printlnf("{} {} {}", $size_of(Header), $align_of(Header), $offset_of(Header, checksum));
    ret 0;
}
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
use std.print;
use std.runtime;

#[packed]
#[align(8)]
rec Frame { a: u8; b: u32; } # fields at 0 and 1; $align_of == 8, $size_of == 8

#[symbol("main")]
fun main(argc: i64, argv: **u8) i64 {
    print.printlnf("{} {} {}", $size_of(Frame), $align_of(Frame), $offset_of(Frame, b));
    ret 0;
}
```

#### Packing is not transitive

A `packed` record packs **its own** fields. A record it contains keeps its own
internal padding and is merely *placed* without padding. This matches C, and it is
the rule that composes: an inner type's layout does not change depending on who
holds it.

```mach
use std.print;
use std.runtime;

rec Point { x: u8; y: u32; } # natural: y at 4, size 8

#[packed]
rec Msg { tag: u8; p: Point; } # p at offset 1, still 8 bytes; $size_of(Msg) == 9

#[symbol("main")]
fun main(argc: i64, argv: **u8) i64 {
    print.printlnf("{} {} {} {}", $offset_of(Point, y), $size_of(Point), $offset_of(Msg, p), $size_of(Msg));
    ret 0;
}
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

**Vector fields.** A vector in a packed record is refused, including one reached
through an array or a nested record. Packing places the vector at an offset nothing
guarantees is a multiple of its width, and a vector's size and alignment can
disagree (`f32x3`, `f32x5`), so its footprint depends on its lane count through
aggregate layout and ABI classification. Store the lanes as scalar fields, or drop
`#[packed]` from the record.

**Interface blocks.** `packed` cannot apply to a `#[gpu.uniform]`, `#[gpu.storage]` or `#[gpu.push]` block:
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

### `volatile` — every access to the type is a volatile access

`#[volatile]` on a `rec`, `uni` or `tag` declaration makes every load and store
of that type's storage volatile: the optimizer keeps each one, in program order,
at the width written. A volatile access is never elided as dead or redundant,
never hoisted out of a loop, never merged with its neighbour into a wider or
unaligned access, and never promoted to a register (`mem2reg` leaves a function
with one alone; `licm` refuses to hoist one; the bulk-memory combiner skips
one). It takes no arguments and applies to nothing else: `#[volatile]` on a
function or a variable is refused with `` `volatile` decorator applies only to
records, unions, and tags``.

Volatility is a property of a **declared type**, so every volatile access in a
program traces back to a declaration. There is no variable-level decorator and
no pointer qualifier (see [types.md](types.md#pointer)); a memory-mapped device is
expressed by declaring its register block as a volatile record and casting its
address to a pointer to it:

```mach
#[volatile]
rec Fb {
    status: u32;
    px:     [1024]u32;
}

val fb: *Fb = 0xB8000::*Fb;

fun fill(c: u32) {
    var i: u64 = 0;
    for (i < 1024) {
        fb.px[i] = c; # one volatile store per iteration
        i        = i + 1;
    }
    fb.status = 1; # a volatile store, ordered after the loop
}
```

The access decides by the storage it reaches, not by its form. A member
(`fb.status`), a projection, an index (`fb.px[i]`), a dereference (`@fb`) and a
whole-record copy are volatile alike, and so is a field of a plain record stored
inside a volatile one (`blk.ctl.bits` when `Blk` is volatile and `Ctl` is not),
because the storage is the volatile block. An indirection ends the walk: a
pointer field of a volatile record is itself read volatile, but what it points
at is ordinary storage unless its own type says otherwise. A raw scalar pointer
(`@p` with `p: *u32`) is never volatile, since it traces to no declaration.

A volatile record copied by value is a volatile copy in both directions, and a
`#[volatile]` type is compatible with `#[packed]` and `#[align(N)]`, which only
shape the layout. `rec.md`, `uni.md` and `tag.md` link here.

### `section(str)` — object section placement

Places a function or global variable in a named section instead of the
default `.text` / `.data`.

```mach
#[section(".hottext")]
#[symbol("f_hot")]
fun f_hot(x: i64) i64 {
    ret x + 1;
}

#[section(".machsec")]
#[symbol("g_sec")]
pub var g_sec: u64 = 100;
```

The named section is created if absent, and the globals of one object that
name it share that one section, each at its own aligned offset. A function's
section holds code, a `var`'s writable data, a `val`'s read-only data and a
`#[thread]` var's thread-local template. Cross-section calls and accesses use
ordinary relocations.

### `at(offset)` — placement in the image

Places a module global at an integer offset in the image, counted from the start
of the region it runs in, or from the image base where it runs in none (see
[manifest.md](manifest.md#memory-regions)). The global becomes a section of its
own, which the link places at the offset, so a boot signature, a header a loader
looks for at a fixed byte or a page table at a known offset is stated in source.

```mach fragment
#[at(510)]
val SIGNATURE: [2]u8 = [2]u8{0x55, 0xaa};
```

- It applies to a module `val` or `var` only, and takes one integer constant.
- It cannot combine with `section`, since a placed global is a section of its
  own, or with `thread`, since each thread's copy lies in its own block. An
  `ext` declaration places nothing.
- Two globals at one offset are refused at the link. A format whose loader maps
  the image itself is refused by mink, which places the section.

### `export` — a shared library's export

Exports a function or global from a shared library under its symbol name. It
requires `pub` and is refused with `decorator.requires` on a declaration that is
not `pub`, and with `decorator.misplaced` on an `ext` declaration, which another
image defines. `pub` alone states a module's interface to other Mach modules and
never exports. Takes no arguments, and `#[testing]` cannot combine with it. An
exported declaration is seen from outside the image wherever its project sits in
the build, so a dependency's `#[export]` is exported as the root project's is.

```mach fragment
#[export]
pub fun version() u32 { ... }
```

### `thread` — a thread-local module var

Gives each thread its own copy of a module-level `var`. Applies to a `var`
only. Its initializer is a constant, as every module binding's is, and it is
the template each thread's copy starts from. Nothing runs when a thread exits.

```mach fragment
#[thread]
var depth: u32 = 0;
```

- An executable reaches its thread-locals at offsets its link fixes, the
  local-exec model. A shared library reaches them at offsets its loader fixes
  when the process starts, the initial-exec model, and a `pub` one is exported
  with the library. General-dynamic access is never lowered, so a library that
  uses thread-locals and is opened with `dlopen` after start draws on the
  loader's small static thread-local reserve, and a loader that has none left
  refuses to open it.
- A thread-local an `ext` declaration names may live in another image, so it is
  reached as a library's is.
- The address of a thread-local is no constant, since each thread holds its
  copy at its own address. A module binding whose initializer takes one is
  refused with `address.thread_local`.
- A shader interface role and `thread` are refused together.

### `constant_time(mode)` — constant-time boundary

Marks a function as a constant-time boundary. Applies to functions only, and
takes one mode, `"soft"` or `"hard"`, which is required. Inside it the backend must not introduce a secret-dependent
branch or select a variable-latency instruction on a secret operand; a
translation validator re-derives the secret taint over the lowered MIR and
rejects any such leak.

Inline `asm` inside such a function is **validated rather than rejected**: the
block is parsed and walked for the same leaks, and refused only where a leak is
found or where the construct cannot be modelled. See
[secrecy.md](secrecy.md#constant_time--the-codegen-contract) for what is checked and
what is refused, including the x86-64 conditional-branch limitation.

The **zeroizing-write** guarantee is *not* one of the decorator's obligations,
and describing it as one understates it. A write into secret storage carries a
taint applied at lowering, keyed on the storage's secrecy rather than on any
decorator, so a zeroizing wipe is protected in a function carrying no
`#[constant_time]` at all. See [secrecy.md](secrecy.md#the-zeroizing-write-guarantee)
for what that covers and what it does not.

```mach fragment
#[constant_time("hard")]
fun ct_eq(a: ^[8]u8, b: ^[8]u8) u8 { ... }
```

The decorator is purely subtractive — on a secret-free function it is a no-op.
A function instance that *computes* on a `^` secret (arithmetic, bitwise, shift,
comparison, negation) is **required** to carry it; an instance that only moves,
stores, or declassifies secrets stays annotation-free.

It is rejected outright for a target whose back half emits a module for a
downstream compiler rather than the executed instructions (the experimental
SPIR-V backend): the contract cannot be validated or upheld there.

v6 spelled it `#[oblivious]`, which warns naming `constant_time` and its two
modes (see [v6 spellings](#v6-spellings)).

> **Experimental preview.** The constant-time guarantee is not complete and has
> not been audited — see [secrecy.md](secrecy.md#assurance) for the known open
> holes. Do not build production cryptography on it at this version.

### `vectorize("never")` — opt out of auto-vectorization

Excludes a function from loop auto-vectorization, so its loops compile to scalar
code even in the release pipeline on a vector-capable target. Applies to
functions only, and takes one mode, `"never"`. v6 spelled it `#[scalar]`, which
warns naming its replacement.

```mach fragment
#[vectorize("never")]
fun reference_sum(a: *i64, n: usize) i64 { ... }
```

A `#[vectorize("never")]` function is also declined by the inliner, so the opt-out survives
inlining — it cannot be lost by the body moving into an unflagged caller. Use it
for a scalar reference twin in a differential test, or where vectorized codegen
is undesirable for a specific function. The project-wide equivalent is the
`vectorize` profile key (see [manifest.md](manifest.md#profilename)).

### `naked` — no prologue, no epilogue, body as written

Emits the function's body exactly as written and nothing else: no frame-pointer
record, no stack allocation, no callee-save stores, no argument moves, and no
return. Applies to functions only; takes no arguments.

```mach fragment
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
- **No return is generated.** Control that falls off the end of the asm reaches
  a trap the compiler places there. Write the return the ABI (or the interrupt
  controller) actually calls for.
- **An asm block of a naked body binds no `{name}`.** The function has no frame
  to hold a local, so a binding is refused where the block is written.
- **Parameters and the return type are still checked** at every call site, so a
  naked function is called like any other. No moves are emitted for them: the
  arguments arrive in the ABI's registers and the body reads them there.
- **Mutually exclusive with `inline("always")`** — there is no coherent winner
  between a body spliced into a caller and one that owns its own frame — and
  with `constant_time`, which already forbids inline asm because a type system
  cannot check it. Both combinations are rejected in sema. `inline("never")` is
  redundant: the inliner declines a naked function unconditionally.
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

### `extension(names)` — an outlier function

Lets one function use instruction-set extensions the target does not select.
Applies to a function with a body, once, and takes one or more extension names,
each a string. v6 wrote `#[ext(sha)]` and `#[extensions(sha)]` with bare names,
which warn naming `#[extension("sha")]`. The manifest's `ext` key and
`$mach.build.ext` keep their names.

```mach fragment
#[extension("sha", "ssse3")]
fun compress_sha_ni(state: *[8]u32, block: *[64]u8) {
    asm x86_64 {
        # sha256msg1, sha256rnds2, pshufb, ...
    }
}

fun compress(state: *[8]u32, block: *[64]u8) {
    if (cpu_has_sha_ni()) { compress_sha_ni(state, block); }
    or                    { compress_portable(state, block); }
}
```

This is the same contract as Rust's `target_feature(enable = "...")` attribute and
gcc's `__attribute__((target("...")))`. Inside the function, inline `asm` may
use every instruction the named extensions admit, in addition to what the
target selects. **The caller owns the run-time check.** Calling an outlier on a
processor that lacks one of its extensions is undefined behaviour, typically an
illegal-instruction trap. The compiler neither inserts the check nor verifies
that one precedes the call, because only the program knows how it detects the
processor's features and when that answer holds.

- **Names come from every instruction set.** A name no extension catalog names
  is refused, and the refusal names the catalogs it was read against (masc's x86,
  aarch64 and riscv catalogs, and mach's own spirv vocabulary) and lists
  the names the compiler emits against. A name a catalog holds that the compiler
  emits nothing against, or that another instruction set declares, admits
  nothing on this target and is not an error, so one
  declaration serves a multi-arch source: `#[extension("sse41", "sha2")]` admits
  `sse41` on x86_64 and `sha2` on aarch64, and an `asm aarch64 {}` block in an
  x86_64 build still refuses `sha256h` because the tag, not the decorator, picks
  the isa. Each name may appear once. The names are those the manifest's
  [`ext`](manifest.md#instruction-set-extensions) key takes, closed over
  what they imply (`sse41` admits `pshufb`), except the rows only a target
  selects (riscv `i`, `c`, `f`, `d`, `zkt`), which are refused with the reason.
- **One predicate, everywhere.** A function's admitted set is the target's
  selection plus what the decorator names. An instruction requiring an extension
  is emitted only into a function whose admitted set holds it (see
  [asm.md](asm.md#extension-instructions)); the encoder checks every row against
  it, and the inliner checks it before moving a body, so an outlier the target
  already selects may still inline into a baseline caller.
- **Only the instructions change.** The decorator admits instructions in the
  function's inline `asm`. It does not change how the compiler generates the
  rest of the body, which stays within the target's selection. The function is
  called through the ordinary ABI, and its address is an ordinary `fun(...)`
  value that dispatch through a pointer calls like any other.
- **It is never inlined into a caller that admits less.** The inliner declines
  to move an outlier's body into a function whose admitted set does not hold
  every extension the outlier's does, so the extension instructions stay behind
  the call the run-time check guards. `#[inline("always")]` does not override this. A
  caller carrying a superset of the outlier's extensions may still inline it,
  and any function may be inlined *into* an outlier. A generic function's
  instances carry the decorator's set.
- **Nothing else changes.** The decorator composes with `inline`, `symbol`,
  `section` and `align`. `#[constant_time]` already refuses the instructions a
  constant-time check cannot model, and an extension row it can model is
  checked like any other.

### `embed(str)` — compile-time file embedding

Sources a `val`'s bytes from a file at compile time: the file's content **is**
the initializer. Applies to `val` only — not `var` (the storage is read-only
data) and not an `ext` data import (which has no storage here). Takes one
constant string argument.

```mach fragment
#[embed("assets/logo.qoi")]
val LOGO: [_]u8;          # length taken from the file's byte count

#[embed("boot/sector.bin")]
val SECTOR: [512]u8;      # length pinned; a size change fails the build
```

- The declaration carries no initializer of its own; writing one alongside
  `embed` is rejected. This is a second exemption to `val`'s
  requires-an-initializer rule, alongside `ext` (see
  [val-var.md](val-var.md#ext--foreign-data-imports)).
- Exactly one argument, a constant string, decoded like any string: a literal's
  escapes are its value, as they are for `symbol` and `section`.
- The path resolves relative to the **declaring source file's** directory. An
  absolute path is taken as written. The resolved file must lie inside the
  project root: an embed that escapes it (`../../outside.txt` from `src/`) is
  refused at the decorator (`` `embed` path escapes the project root; an
  embedded file must live inside the project and the file is not read ``) and
  the file outside is never read. Keep assets under the project.
- A path holding `{artifact.<id>.out}` names the output of a required artifact
  and resolves against the **root project's** directory rather than the
  declaring file's directory; the required artifact is built first. The name is
  read in the manifest that owns the declaring module: the root's own module
  names an artifact the built artifact requires, and a dependency's module names
  an artifact that dependency's `export = true` library artifact requires,
  which the consumer's build produces for it. No other template variable may
  appear in an `embed` path. See
  [manifest.md](manifest.md#artifact-requirements).
- The annotation must be `[_]u8` or `[N]u8`; the element type must be `u8`.
  `[_]` is an inferred array length, which an `#[embed]` declaration takes from
  the file and an array literal gives its own annotation (see
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
  [manifest.md](manifest.md#stepname--build-steps) for the equivalent
  guarantee on `[step]` `in` entries.

### GPU decorators

The `gpu.` decorators shape a pipeline stage and its workgroup, the shader
interface and specialization constants. They, with the handle types a target
mints (`handle`) and the functions that are target instructions (`intrinsic`),
are described in [gpu.md](gpu.md). v6 spelled the GPU set without its prefix
(`#[stage]`, `#[input]` and the rest) and the instruction decorator `#[op]`. Each
warns naming its replacement, which takes the same arguments.

### `abi_type(name)` — a C type whose layout the target declares

A bodyless `def` carrying this decorator declares a C type whose size and alignment
come from the **selected target** and whose contents the program never reaches.

```mach
#[abi_type("va_list")]
pub def VaList;
```

`va_list` is the only name, and the set is closed in the front end for the reason
`intrinsic`'s instruction names are: which target a module is built for is not a property
of the source, so a typo checked only where it is acted on would go unreported on
every other build of the same library. A target that declares no layout for the
named type refuses the declaration rather than substituting a default.

The type is an opaque aggregate of the declared extent. A value of one may be
received as a parameter and passed on, and nothing else: a local binding, a record
or union field, a global, a return position, a pointer or array of one, and a cast
in either direction are each refused where they are written. See
[ext-fun.md](ext-fun.md) for the recipe and for why forwarding is the whole scope.

## Applicability

Where each decorator stands. The declarations are named by their keyword,
`ext` is an `ext` import of any kind, and `case` is a tag case. A decorator
written anywhere else is refused with `decorator.misplaced`. The last column is
a further rule the decorator's own section states.

| Decorator | Stands on | Only on |
|---|---|---|
| `deprecated` | `fun`, `rec`, `uni`, `tag`, `def`, `val`, `var`, `use`, `fwd`, `case` |  |
| `testing` | `fun`, `rec`, `uni`, `tag`, `def`, `val`, `var`, `use`, `fwd` | not an `ext` declaration |
| `expect` | `fun`, `rec`, `uni`, `tag`, `val`, `var`, `test` |  |
| `symbol` | `fun`, `val`, `var` |  |
| `section` | `fun`, `val`, `var` |  |
| `library` | `ext` |  |
| `export` | `fun`, `val`, `var` | a `pub` declaration that is not `ext` |
| `inline` | `fun` |  |
| `align` | `fun`, `rec`, `uni`, `tag`, `val`, `var` |  |
| `packed` | `rec`, `uni`, `tag` | not a `gpu.uniform`, `gpu.storage` or `gpu.push` block |
| `volatile` | `rec`, `uni`, `tag` |  |
| `at` | `val`, `var` | not `ext`, `section` or `thread` |
| `thread` | `var` | no shader interface role |
| `constant_time` | `fun` |  |
| `vectorize` | `fun` |  |
| `naked` | `fun` |  |
| `extension` | `fun` | a function with a body |
| `embed` | `val` | a `val` that is not `ext`, checked by `embed` itself |
| `intrinsic` | `fun` | a function with no body |
| `handle` | `def` | a `def` with no type, checked by `handle` itself |
| `abi_type` | `def` | a `def` with no type, checked by `abi_type` itself |
| `gpu.stage` | `fun` |  |
| `gpu.workgroup` | `fun` | a function that also carries `#[gpu.stage("compute")]` |
| `gpu.input` | `val`, `var` |  |
| `gpu.output` | `val`, `var` |  |
| `gpu.builtin` | `val`, `var` |  |
| `gpu.uniform` | `val`, `var` |  |
| `gpu.storage` | `val`, `var` |  |
| `gpu.sampler` | `val`, `var` |  |
| `gpu.push` | `val`, `var` |  |
| `gpu.spec` | `val`, `var` | a `var` |
| `gpu.shared` | `val`, `var` | a `var` |

The set is closed. New decorators require a compiler change. A name outside it
is refused with `decorator.unknown`, and the refusal lists every decorator.

### v6 spellings

A v6 spelling still reads as the decorator it names, with its v6 arguments, and
each use warns with `decorator.renamed`, naming the v7 spelling. It becomes a
refusal once std and the family's sources use the v7 spellings, and `mach
migrate` rewrites each one.

| v6 | v7 |
|---|---|
| `#[ext(a, b)]`, `#[extensions(a, b)]` | `#[extension("a", "b")]` |
| `#[inline]` | `#[inline("always")]` |
| `#[noinline]` | `#[inline("never")]` |
| `#[scalar]` | `#[vectorize("never")]` |
| `#[oblivious]` | `#[constant_time("soft")]` or `#[constant_time("hard")]` |
| `#[op(tgt, set, name)]` | `#[intrinsic(tgt, set, name)]` |
| `#[stage]` … `#[shared]` | `#[gpu.stage]` … `#[gpu.shared]` |

## See also

- [test.md](test.md) — `test` blocks, whose semantics `testing` gives a declaration
- [ext-fun.md](ext-fun.md) — `ext` imports, `library` and `symbol` use cases
- [visibility.md](visibility.md) — `pub` / `ext` visibility (not decorator-controlled)
- [comptime-intrinsics.md](comptime-intrinsics.md) — `$size_of` / `$align_of` as `align` arguments
- [secrecy.md](secrecy.md) — `^` secret types and the `constant_time` contract
- [asm.md](asm.md) — inline `asm`, the only body a `naked` function may have, and the extension instructions `extension` admits
- [val-var.md](val-var.md) — `val` / `var` bindings, and the `embed` exemption to `val`'s initializer requirement
- [grammar.md](grammar.md#types) — the `[_]` inferred array length `embed` introduces
- [types.md](types.md) — the SIMD vector types a shader stage computes over and `intrinsic` operates on, and the handle types `handle` declares
- [manifest.md](manifest.md) — the `vectorize` profile key `vectorize("never")` opts out of, and content-fingerprinted build inputs (`embed`, `[step]` `in`)
