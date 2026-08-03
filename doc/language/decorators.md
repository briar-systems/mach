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

Without `symbol`, the compiler mangles the Mach name. `symbol` gives the
exact name the linker sees.

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
be a comptime integer.

Which spellings are accepted depends on the target, because the two are decided
at different points in the compile:

| target | literal | `$size_of(T)` / `$align_of(T)` |
|---|---|---|
| global variable | yes | yes |
| record / union type | yes | **no** |

A type's alignment is settled while the layouts it would measure are themselves
still being resolved, so a layout intrinsic has no value yet at that point; a
global's alignment is decided later, once layout is known. Binding the intrinsic
to a `val` first does not help — the same ordering applies. Tracked as #2442.

```mach
#[align(64)]
pub var cache_line: u8 = 0;

#[align($size_of(Pair))]   # ok: layout is known by the time a global is aligned
pub var g_cmp: u8 = 0;

#[align(32)]               # ok: a literal needs no layout
rec Over { a: u8; }
```

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
branch, select a variable-latency instruction on a secret operand, or eliminate
a zeroizing write to secret storage; a translation validator re-derives the
secret taint over the lowered MIR and rejects any such leak. Inline `asm` is
rejected inside an `#[oblivious]` function, since a type system cannot check it.

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

The set is closed. New directives require a compiler change.

## See also

- [ext-fun.md](ext-fun.md) — `ext` imports, `library` and `symbol` use cases
- [visibility.md](visibility.md) — `pub` / `ext` visibility (not decorator-controlled)
- [comptime-intrinsics.md](comptime-intrinsics.md) — `$size_of` / `$align_of` as `align` arguments
- [secrecy.md](secrecy.md) — `^` secret types and the `oblivious` constant-time contract
- [asm.md](asm.md) — inline `asm`, the only body a `naked` function may have
- [../manifest.md](../manifest.md) — the `vectorize` profile key `scalar` opts out of
