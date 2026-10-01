# Decorators

A decorator attaches metadata to a declaration. It can provide source-use
notices or influence how the compiler emits the symbol: its linker name,
alignment, section placement, inlining, dynamic import attribution, constant-time
obligations, or exclusion from auto-vectorization.

Visibility (`pub` / `ext`) is separate and unaffected by decorators.

## Surface

A decorator is written as an attribute:

```
#[name]            # bare flag (e.g. inline)
#[name(args)]      # directive with comptime-expr arguments
```

> `#[...]` is the only decorator surface. A backtick is not a token: one
> anywhere in source is a lexer error.

> One caveat the attribute form introduces: a line comment that begins `#[`
> (with no space) opens an attribute. Write such a comment with a separating
> space — `# [...]`.

## Grammar

```
#[deprecated]        # external uses warn
#[deprecated("msg")] # external uses warn with this message
#[testing]           # exists only for tests; omitted from ordinary builds
#[expect("key", ..)] # acknowledge the named warnings inside this declaration
#[symbol("name")]    # linker name override
#[library("dep")]    # dynamic import attribution (ext only)
#[inline]            # force inlining (no arguments)
#[noinline]          # forbid inlining (no arguments)
#[align(expr)]       # alignment; expr is a comptime integer
#[packed]            # lay a rec / uni out with no padding (no arguments)
#[volatile]          # every access to a rec / uni / tag is a volatile access (no arguments)
#[section(".name")]  # place in a named object section
#[oblivious]         # constant-time boundary (no arguments)
#[scalar]            # opt out of auto-vectorization (no arguments)
#[naked]             # no prologue or epilogue; body as written (no arguments)
#[extensions(a, b)]  # the function may use these instruction-set extensions
#[embed("path")]     # compile-time file embedding (val only)
#[stage("name")]     # GPU pipeline stage; makes the function an entry point
#[workgroup(x,y,z)]  # compute workgroup dimensions, constants or #[spec] vars (with #[stage("compute")])
#[input(n)]          # shader interface input at location n (global only)
#[output(n)]         # shader interface output at location n (global only)
#[builtin("name")]   # pipeline built-in variable (global only)
#[uniform(set, bnd)] # descriptor-bound uniform block, read-only (global only)
#[storage(set, bnd)] # descriptor-bound storage buffer, read-write (global only)
#[storage(set, bnd, "readonly")] # the same buffer, with writes refused
#[storage(set, bnd, "writeonly")] # the same buffer, with reads refused
#[storage(set, bnd, "coherent")] # the same buffer, its writes visible across workgroups
#[sampler(set, bnd)] # descriptor-bound image / sampler handle (global only)
#[push]              # push-constant block, read-only (global only)
#[spec(id)]          # specialization constant the host supplies (module var only)
#[shared]            # compute workgroup memory, zero when a stage starts (global only)
#[op(tgt,set,name)]   # the target instruction this function is (bodyless fun only)
#[handle(tgt,ctor,..)] # the target type this declares (bodyless def only)
#[abi_type("name")]   # a C type whose layout the target declares (bodyless def only)
```

Decorators appear **before** the declaration they target, one per line or
space-separated on the same line. They attach to the immediately following
declaration only and do not bleed across declarations.

```mach fragment
#[inline]
#[symbol("big")]
fun big(a: i64, b: i64) i64 { ... }

#[align(64)] #[symbol("g_lit64")]
pub var g_lit64: u8 = 7;
```

Each directive is wrapped in its own clause: `#[name]` for a bare flag or
`#[name(args)]` for a directive that takes arguments. Arguments are comptime
expressions.

### Constant arguments

Where a directive takes a string or an integer, the argument is a constant
expression of that type, evaluated at compile time in the declaring module. A
literal is one. So is a `val`, one an `$if` arm selects, or one imported from
another module. A directive's argument never depends on the build that
evaluates it, only on the constants in scope. `#[symbol]` stays exactly the
name you give it: the compiler adds no platform decoration of its own, and a
`val` gated by comptime is how one declaration names its symbol per target.

```mach
$if ($mach.build.os == $mach.os.darwin) {
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
refused with `decorator.not_constant`, naming the directive. A constant of the
wrong type is refused with `decorator.argument`.

## Directives

### `deprecated` / `deprecated(str)` — source-use notice

Marks a declaration deprecated. The optional argument is one constant string
carrying a message. Repeating the attribute, giving it more than one argument,
or giving it an argument that is not a constant string is an error. The attribute changes nothing about visibility, type identity, ABI or
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
attribute because they declare no externally usable name.

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
#[library("ws2_32.dll")]
#[symbol("WSAStartup")]
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

Marks a function for inlining at every direct call site, overriding the
compiler's size and use-count heuristics and exempt from the caller's expansion
budget. Applies to functions only and takes no arguments. The optimization
pipeline must enable inlining. Indirect calls and recursive call cycles are not
expanded by this attribute. Taking a function's address retains its callable
identity even when direct calls are inlined.

Release optimization makes small ordinary helper bodies available across source
modules without emitting extra definitions. A helper is small when its body has
fewer than 25 live instructions after promotion, debug annotations excluded, so
`-g` never moves the decision. Extraction, import and per-caller heuristic expansion each
have a limit of 1024 copied IR instructions and 256 KiB of owned payload. An
`inline` function is expanded at every direct call site in the same module
without charging that budget, so the outcome never depends on what else the
caller expanded first; across modules it is imported within the extraction and
import limits like any other body. An
`oblivious` function is expanded only into another `oblivious` function, so
its instructions never leave a constant-time validated body; a `naked` or
`noinline` function, a recursive cycle and an indirect call are never expanded.
A remaining call or taken address still names the original defining function.
Generic, comptime and pack specializations keep their existing shared weak
linkage. When several modules materialize that same specialization, body import
uses an already available definition or the first acquired provider of that
linkage, and tracks that provider as a query dependency.
Helpers referencing compiler-local literal pools retain their calls because those
objects have module-local identity. Named globals keep their original symbols,
and copied instructions preserve effects, assembly bindings and debug locations.

```mach
#[inline]
fun fast_path(x: i64) i64 {
    ret x * 2;
}
```

### `noinline` — forbid inlining

The inverse of `inline`: forbids inlining a function into any caller, overriding
the compiler's size- and use-count heuristics that would otherwise fold it in.
Applies to functions only; takes no arguments.

```mach
#[noinline]
fun cold_path(code: i64) i64 {
    ret code * 100;
}
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
  identically when a callee body is available from another module. Recursive
  peeling also respects `noinline` and `scalar`.

### `align(expr)` — alignment override

Sets the alignment of a global variable, a record/union type, or a function's
entry. `expr` must
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
its fields. Takes no arguments.

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

**Vector fields.** A vector in a packed record is refused for now, including one
reached through an array or a nested record. The reason is evidence rather than
arithmetic: an unaligned **scalar** access is measured on real hardware, and that
measurement is what `#[packed]` rests on.

The vector measurement now exists too. The codegen corpus's `vec/vec_mem` case
writes `f32x3` and `f32x5` — the two shapes whose size and alignment disagree — into
packed buffers and records where every write has a live neighbour, folding the
neighbour after the write so a store too wide by a lane changes the checksum. It
runs at both pipelines on every target with an execution engine, against a C
reference the host's own compiler built, so a dropped lane or a disturbed
neighbouring byte is a differing number rather than a passing run. The row that
matters is `aarch64-linux` on `ubuntu-24.04-arm`, because aarch64 has 128-bit forms
with alignment requirements; `x86_64-linux` and `x86_64-windows` carry it too.
`riscv64-linux` also passes and is not evidence: it runs under qemu-user, and
riscv64 declares no 128-bit vector support, so the access there is a scalar
expansion rather than a vector access.

What the refusal still waits on is the other half,
[#2687](https://github.com/briar-systems/mach/issues/2687) — a lane-dependent vector
footprint through aggregate layout and ABI classification. This is a sequencing
decision and is expected to be lifted, not a permanent rule.

**Interface blocks.** `packed` cannot apply to a `#[uniform]`, `#[storage]` or `#[push]` block:
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

```mach fragment
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

```mach fragment
#[scalar]
fun reference_sum(a: *i64, n: usize) i64 { ... }
```

A `#[scalar]` function is also declined by the inliner, so the opt-out survives
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

### `extensions(names)` — an outlier function

Lets one function use instruction-set extensions the target does not select.
Applies to a function with a body; takes one or more bare extension names.

```mach fragment
#[extensions(sha, ssse3)]
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

This is the same contract as Rust's `#[target_feature(enable = "...")]` and
gcc's `__attribute__((target("...")))`. Inside the function, inline `asm` may
use every instruction the named extensions admit, in addition to what the
target selects. **The caller owns the run-time check.** Calling an outlier on a
processor that lacks one of its extensions is undefined behaviour, typically an
illegal-instruction trap. The compiler neither inserts the check nor verifies
that one precedes the call, because only the program knows how it detects the
processor's features and when that answer holds.

- **Names come from every instruction set.** A name no instruction set declares
  is refused, and the refusal lists the known names. A name another instruction
  set declares admits nothing on this target and is not an error, so one
  declaration serves a multi-arch source: `#[extensions(sse41, sha2)]` admits
  `sse41` on x86_64 and `sha2` on aarch64, and an `asm aarch64 {}` block in an
  x86_64 build still refuses `sha256h` because the tag, not the decorator, picks
  the isa. Each name may appear once. The names are those the manifest's
  [`extensions`](manifest.md#instruction-set-extensions) key takes, closed over
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
  the call the run-time check guards. `#[inline]` does not override this. A
  caller carrying a superset of the outlier's extensions may still inline it,
  and any function may be inlined *into* an outlier. A generic function's
  instances carry the decorator's set.
- **Nothing else changes.** The decorator composes with `inline`, `noinline`,
  `symbol` and `section`. `#[oblivious]` already refuses the instructions a
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
  an artifact that dependency's `default = true` library artifact requires,
  which the consumer's build produces for it. No other template variable may
  appear in an `embed` path. See
  [manifest.md](manifest.md#artifact-requirements).
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
  [manifest.md](manifest.md#stepname--build-steps) for the equivalent
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

### `workgroup(x, y, z)` — compute workgroup dimensions

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

### `input(n)` / `output(n)` / `builtin(str)` / `uniform(set, binding)` / `storage(set, binding)` / `sampler(set, binding)` / `push` / `spec(id)` / `shared` — shader interface

A pipeline stage does not receive its inputs or return its results through a call.
It reads and writes **module-scope variables** that the pipeline binds, and these
directives say which kind each variable is. They apply only to module-level
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

`"coherent"` decorates the buffer `Coherent`, so a write one invocation makes is
visible to invocations in other workgroups under the GLSL450 memory model, which
is what atomics and flags shared across workgroups rely on. It combines with
either of the other two.

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

`OpImageRead`, `OpImageWrite` and `OpImageFetch` take an optional `Image Operands`
mask leading their tail, so a declaration either stops at the instruction's
required operands or passes the mask and the operands its bits bring. `Sample`
(`0x40`) names one sample of a multisampled image, and a fetch's `Lod` (`0x2`)
the level it reads. A multisampled image is read, written and fetched only with
`Sample`, and only a multisampled image takes it. `OpImageSampleExplicitLod`
always takes the mask, which must set `Lod`. `OpImageQuerySamples` reads the sample
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
requires an `OpSampledImage` result be consumed in the block that produced it,
which is the same rule that makes a handle-typed local a compile error.

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

### `spec(id)` — specialization constants

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

### `handle(target, constructor, operands...)` — a type the target mints

A bodyless `def` carrying this directive declares a type whose representation is
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

### `abi_type(name)` — a C type whose layout the target declares

A bodyless `def` carrying this directive declares a C type whose size and alignment
come from the **selected target** and whose contents the program never reaches.

```mach
#[abi_type("va_list")]
pub def VaList;
```

`va_list` is the only name, and the set is closed in the front end for the reason
`op`'s instruction names are: which target a module is built for is not a property
of the source, so a typo checked only where it is acted on would go unreported on
every other build of the same library. A target that declares no layout for the
named type refuses the declaration rather than substituting a default.

The type is an opaque aggregate of the declared extent. A value of one may be
received as a parameter and passed on, and nothing else: a local binding, a record
or union field, a global, a return position, a pointer or array of one, and a cast
in either direction are each refused where they are written. See
[ext-fun.md](ext-fun.md) for the recipe and for why forwarding is the whole scope.

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
with, so nothing about this directive is specific to one back end. The arguments
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
which a module may hold only where the environment has Int8 (from `vulkan1.2`), so
below that a truth result is declared `u32` and compared, as in `elect(3) == 1`.

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
constant.

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
that order. Each is checked at the call, where the literal's value is known:

| At the call                                            | Is refused with                    |
|--------------------------------------------------------|------------------------------------|
| a value outside the operand's enumeration              | `op.operand_value`, naming the values |
| a value whose operands the declaration does not pass, or passes without it | `op.operand_value`, naming the count |
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
Vulkan device feature of its storage class, named for its `shaderBuffer*` or
`shaderShared*` member: `buffer_*` on a storage buffer (`StorageBuffer`, or `Uniform`
before SPIR-V 1.3), `shared_*` on [`#[shared]`](#inputn--outputn--builtinstr--uniformset-binding--storageset-binding--samplerset-binding--push--specid--shared--shader-interface)
workgroup memory. Any other storage class is refused. An `f16` atomic is refused: its
pointee must be an `OpTypeFloat 16`, and an `f16` in memory is carried as its 16-bit
integer.

| Rows | Type | Extensions | Vulkan feature | Capability (SPIR-V extension) |
|------|------|-----------|----------------|-------------------------------|
| all 15 integer atomics | `u32`, `i32` | none | core | none |
| all 15 integer atomics | `u64`, `i64` | `buffer_int64_atomics`, `shared_int64_atomics` | `shaderBufferInt64Atomics`, `shaderSharedInt64Atomics` | `Int64Atomics` |
| `OpAtomicLoad`, `OpAtomicStore`, `OpAtomicExchange` | `f32`, `f64` | `buffer_float{32,64}_atomics`, `shared_float{32,64}_atomics` | `shader{Buffer,Shared}Float{32,64}Atomics` | none |
| `OpAtomicFAddEXT` | `f32`, `f64` | `buffer_float{32,64}_atomic_add`, `shared_float{32,64}_atomic_add` | `shader{Buffer,Shared}Float{32,64}AtomicAdd` | `AtomicFloat{32,64}AddEXT` (`SPV_EXT_shader_atomic_float_add`) |
| `OpAtomicFMinEXT`, `OpAtomicFMaxEXT` | `f32`, `f64` | `buffer_float{32,64}_atomic_min_max`, `shared_float{32,64}_atomic_min_max` | `shader{Buffer,Shared}Float{32,64}AtomicMinMax` | `AtomicFloat{32,64}MinMaxEXT` (`SPV_EXT_shader_atomic_float_min_max`) |

The features come from `VkPhysicalDeviceShaderAtomicInt64Features`,
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
| `OpImageRead`, `OpImageFetch`, `OpImageWrite` | the texel, the result or the last operand, is a vector of the image's texel scalar |
| `OpGroupNonUniformBroadcast*`, `Shuffle*`, `Quad*` and the arithmetic rows | the result is the value operand's type |
| the GLSL.std.450 math rows | the result and every operand are the first operand's type, except `Refract`'s `eta`, and `Length` and `Distance`, whose result is a scalar |
| `OpDot` | the second vector is the first's type |


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

On every target other than `spirv` the directive is inert, and a decorated function
is an ordinary function. A **bodiless** one — which is what the shader-side maths
library uses — is then an undefined symbol, so a CPU build that calls it fails at
link time naming the symbol. That is a deliberate design choice on the library's
part, not a property of the directive: a decorated function may have a body, and if
it does, that body is what every non-`spirv` target runs while `spirv` substitutes
the instruction. A `spirv` build never emits the body at all.

The set of accepted instructions is the table in
`src/lang/target/isa/spirv/defs.mach`, where each row carries its operand kinds, its
result, its requirements, its typing and the enumerations of its literals. The capabilities, with
the SPIR-V version and extension each needs, are the table in
`src/lang/target/isa/spirv.mach`. Adding an instruction is a row in it.

## Applicability

| Directive   | `fun` | `ext fun` | `val` / `var` | `rec` / `uni` |
|-------------|:-----:|:---------:|:-------------:|:-------------:|
| `deprecated`|  yes  |    yes    |      yes      |      yes      |
| `testing`   |  yes  |    no     |      yes      |      yes      |
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
| `extensions`|  yes  |    no     |      no       |      no       |
| `embed`     |  no   |    no     |      yes      |      no       |
| `stage`     |  yes  |    no     |      no       |      no       |
| `workgroup` |  yes  |    no     |      no       |      no       |
| `input`     |  no   |    no     |      yes      |      no       |
| `output`    |  no   |    no     |      yes      |      no       |
| `builtin`   |  no   |    no     |      yes      |      no       |
| `uniform`   |  no   |    no     |      yes      |      no       |
| `storage`   |  no   |    no     |      yes      |      no       |
| `push`      |  no   |    no     |      yes      |      no       |
| `spec`      |  no   |    no     |      yes      |      no       |
| `op`        |  yes  |    no     |      no       |      no       |
| `handle`    |  no   |    no     |      no       |      no       |
| `abi_type`  |  no   |    no     |      no       |      no       |

The `val` / `var` column is shared, but `embed` accepts only `val` — a `var`
is refused (see [`embed`](#embedstr--compile-time-file-embedding) above).
`spec` is the reverse and accepts only `var` (see
[`spec`](#specid--specialization-constants)).
`deprecated` also applies to `tag`, `def`, `use` and `fwd` declarations and to a
tag case, and `testing` to `tag`, `def`, `use` and `fwd` declarations, none of
which the table columns cover.

The set is closed. New directives require a compiler change.

## See also

- [test.md](test.md) — `test` blocks, whose semantics `testing` gives a declaration
- [ext-fun.md](ext-fun.md) — `ext` imports, `library` and `symbol` use cases
- [visibility.md](visibility.md) — `pub` / `ext` visibility (not decorator-controlled)
- [comptime-intrinsics.md](comptime-intrinsics.md) — `$size_of` / `$align_of` as `align` arguments
- [secrecy.md](secrecy.md) — `^` secret types and the `oblivious` constant-time contract
- [asm.md](asm.md) — inline `asm`, the only body a `naked` function may have, and the extension instructions `extensions` admits
- [val-var.md](val-var.md) — `val` / `var` bindings, and the `embed` exemption to `val`'s initializer requirement
- [grammar.md](grammar.md#types) — the `[_]` inferred array length `embed` introduces
- [types.md](types.md) — the SIMD vector types a shader stage computes over and `op` operates on, and the handle types `handle` declares
- [manifest.md](manifest.md) — the `vectorize` profile key `scalar` opts out of, and content-fingerprinted build inputs (`embed`, `[step]` `in`)
