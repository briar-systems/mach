# `ext fun` — external function

`ext fun` declares a function with C ABI as a forward reference — body-less.
The linker resolves the symbol at link time. This is the only body-less
function form Mach allows.

## Grammar

```mach
ext fun NAME(args) RET;
```

- No body block. The declaration ends with a semicolon.
- Argument and return types must be representable in C.
- `pub ext fun` exposes the import to other modules; without `pub`, the
  import is file-private.

## Examples

```mach
pub ext fun libc_write(fd: i64, buf: *u8, n: i64) i64;

ext fun strlen(s: *u8) i64;             # private, file-local
```

## C-variadic imports

A C function declared `int open(const char *, int, ...)` is **C-variadic**: its
parameter list ends in `...` and a call may pass further arguments the declaration
does not type. Spell it the same way:

```mach
ext fun open(path: *u8, flags: i32, ...) i32;
ext fun fcntl(fd: i32, cmd: i32, ...) i32;
```

- The `...` is a **bare** trailing marker written after the last fixed parameter.
  It is not a named parameter and has no type.
- **`ext` only.** A mach-bodied `fun` has no C-variadic form — its variadic is the
  comptime pack `va: ...`, an ordinary *named* parameter with an entirely different
  meaning (see [variadics.md](variadics.md)). A bare `...` in a non-`ext` parameter
  list is an error that names the pack.
- **At least one fixed parameter** must precede the `...`, as in C. Every psABI
  defines its variadic rule relative to the named arguments.
- Only the **call** side exists. Mach cannot *define* a C-variadic function: there
  is no `va_list` and no `va_arg`, so such a declaration cannot carry a body.

The declared parameters are the *fixed* arity. A call must supply all of them and
may supply any number of further arguments, including none.

### Naming the signature as a type

The same shape is spellable as a function **type**, so a C-variadic function can be
stored in a variable, passed as a callback, or held in a table:

```mach
ext fun printf(fmt: *u8, ...) i32;

def Printf: fun(*u8, ...) i32;

fun main() i32 {
    val p: Printf = printf;
    ret p("%d\n"::*u8, 42);
}
```

The `...` is part of the type's **identity**, not a modifier on it. `fun(*u8, ...) i32`
and `fun(*u8) i32` are different types and do not unify: a call through the first
places its tail by the target's variadic rule and a call through the second does not,
so mixing them would reach a C callee's `va_arg` with arguments laid out the ordinary
way — a wrong *value* at run time, not a link error. The same two rules as the
declaration apply: the `...` is bare and trailing, and at least one fixed parameter
must precede it.

An indirect call through such a type is checked and placed exactly as a direct one is;
the tail rule is read off the type, since no declaration is in view at the call.

### Arguments in the variadic tail

A tail argument has no declared parameter type to check against, so it is checked
against the C variadic contract directly. C applies **default argument promotions**
there: an integer narrower than `int` arrives as an `int`, and a `float` arrives as
a `double`. Mach inserts no implicit conversions anywhere and makes no exception
here — a narrow argument is **rejected**, naming the cast:

```mach
var c: u8  = 65;
var f: f32 = 1.5;

log(fmt, c);                 # error: cast it to `i32` / `u32`
log(fmt, f);                 # error: cast it to `f64`
log(fmt, c::i32, f::f64);    # correct
```

An argument of 32 bits or wider passes as written: `i32`/`u32`, `i64`/`u64`, `f64`,
any pointer, and a record or vector passed by value. A **secret** (`^T`) may not
enter a variadic tail: the callee is foreign code that prints or logs the value, so
it is an observation boundary — declassify with `:>T` first.

### Per-target argument passing

The fixed parameters always follow the target's ordinary calling convention. The
*tail* is where targets differ:

| Target | Variadic tail |
|--------|---------------|
| **Apple arm64** (`darwin` + `aarch64`) | **Every** tail argument is passed on the **stack**, naturally aligned with an 8-byte minimum. There is no register phase at all — not for integers, not for floats. |
| **Other AAPCS64** (`linux` + `aarch64`) | Ordinary rule: register-then-stack, exactly as for a named argument. |
| **System V x86-64** (`linux` / `darwin` + `x86_64`) | Ordinary rule, plus `AL` set to the number of vector registers the call uses. It is set for **every** call to a C-variadic callee, including one whose tail is empty. |
| **Microsoft x64** (`windows`) | Ordinary rule. A tail float rides **both** its XMM register and the integer register of the same positional slot, since a `va_arg` reader walks only the integer slots. The `ext`-boundary vector-by-reference rule below is unchanged. |

A tail argument keeps its *form* on every target: a record too large to pass by
value is still passed by reference, and only the hidden pointer's location moves.

The *fixed* parameters diverge on Apple arm64 too, on a separate axis: a fixed
argument that lands on the stack there takes its **natural size and alignment**,
where AAPCS64 rounds every stack argument up to an 8-byte slot. Past the eight GP
registers, `(…, int i, short j, long long k)` occupies `[sp+0]`, `[sp+4]`, `[sp+8]`
on `darwin-aarch64` and `[sp+0]`, `[sp+8]`, `[sp+16]` on `linux-arm64`. Mach applies
each target's own rule, so nothing in a declaration has to say which one is in force.
Note only that Apple's two rules genuinely differ from each other: the variadic tail
in the table above keeps its 8-byte minimum on the very target where a fixed argument
does not.

Apple arm64 is the reason this form exists. The obvious workaround — declaring the
call at its fixed arity, `ext fun open(path: *u8, flags: i32, mode: u32) i32` —
works on Linux and on x86-64 and is **silently wrong** there: the fixed-arity
lowering puts `mode` in `x2` while libSystem reads it off the stack, so the callee
sees an uninitialized value. That is a wrong value, not a link error and not a
crash. Mach cannot detect the mistake, because nothing in a fixed-arity declaration
says the callee is variadic — so **declare every C-variadic callee with `...`**, on
every target.

## `va_list` parameters

A C function may take a `va_list` as an ordinary **fixed** parameter. The common
shape is a logging callback:

```c
typedef void (*TraceLogCallback)(int logLevel, const char *text, va_list args);
void SetTraceLogCallback(TraceLogCallback callback);
```

That is a fixed 3-arity function. It has no `...` in it, and binding it needs
nothing from the C-variadic form above — only a parameter type that is ABI-correct
in the third slot. Declare that type with `#[abi_type("va_list")]` on a bodyless
`def`:

```mach
#[abi_type("va_list")]
def VaList;

pub def TraceLogCallback: fun(i32, *u8, VaList) i64;

#[library("raylib")]
ext fun SetTraceLogCallback(cb: TraceLogCallback);

ext fun vsnprintf(buf: *u8, n: usize, fmt: *u8, ap: VaList) i32;

fun trace(level: i32, text: *u8, args: VaList) i64 {
    var buf: [512]u8;
    ret vsnprintf(?buf[0], 512, text, args)::i64;
}
```

The declaration carries no size of its own. The selected target declares what a
`va_list` is on that platform, and the same source text binds correctly on every
one of them. A target that declares none refuses the declaration by name rather
than guessing.

### Mach can forward one and never read one

The whole scope is **forward-only**: receive an opaque token from C and pass it to
C. There is no `va_start`, no `va_arg`, no `va_end`, and no way to construct one.
Each of these is refused where it is written:

```mach
val saved: VaList = args;   # refused: a local binding
rec Held { ap: VaList; }    # refused: a record or union field
var kept: VaList;           # refused: a global
fun make() VaList { … }     # refused: a return position
fun peek(ap: *VaList) { … } # refused: behind a pointer
val raw: ptr = args :~ ptr; # refused: a cast in either direction
```

Reading one needs `va_arg`, and mach cannot have `va_arg`: it has no runtime
variadics, so it has no callee that walks its own argument tail. That is not a gap
waiting to be filled. Constructing a `va_list` is only meaningful for such a
callee, and mach never needs to originate one — it would call `printf` rather than
`vprintf`.

One further rule comes from C rather than from mach: **a forwarded `va_list` is
spent.** Handing one to a function that reads it leaves its value indeterminate,
exactly as in C, and what actually happens differs per platform — under AAPCS64 the
type is a composite the caller copies, so each callee walks its own; under System V
x86-64 it is a pointer into shared state that the callee advances. Forward it once.

### Why the type is declared per target

`va_list` is the one C type mach binds whose shape cannot be written down portably:

| Target | `va_list` in a parameter slot |
|--------|-------------------------------|
| **System V x86-64** | `__va_list_tag[1]`, an array type, so a parameter decays to one pointer |
| **Apple arm64** | `char *` |
| **Microsoft x64** and **Windows arm64** | `char *` |
| **RISC-V lp64d** | `void *` |
| **AAPCS64 linux** | a 32-byte, 8-aligned composite |

On the first four, spelling the parameter `ptr` is already ABI-correct. On
AAPCS64-linux it is not, and the way it fails is the reason this type exists rather
than a documented caveat: AAPCS64 passes a composite over 16 bytes indirectly, with
the **caller** owning the copy, so a `ptr` forwards the received pointer instead of
a fresh copy and the next callee advances the caller's list. That usually appears to
work, which is worse than failing.

## Aggregate arguments and the caller's copy

An aggregate too large for registers is passed as an **address of a copy the caller
allocates** on AAPCS64, under the RISC-V psABI, and under the Microsoft convention.
The callee treats that memory as its own parameter and may overwrite it, so mach
materializes a fresh temporary for every such argument at an `ext` boundary and
passes the temporary's address. A C callee that writes to its by-value parameter
therefore cannot reach the caller's object:

```mach
rec Wide { a: i64; b: i64; c: i64; d: i64; }
ext fun consume(w: Wide) i64;
```

A Mach→Mach call keeps mach's own convention, where the callee gives every
parameter storage of its own and copies into it at entry. The two reach the same
by-value semantics from opposite ends, and only the foreign one is visible here.

## Symbol name

The declaration names a C declaration, so its linker symbol is whatever the
target's C toolchain emits for that identifier. On Linux and Windows that is the
name as written; on Darwin the C ABI prefixes an underscore, so `ext fun
mad_open` binds the `_mad_open` a C object there defines. You write the C name
either way — the prefix is never spelled in source.

Override the symbol outright with the `symbol` decorator:

```mach
#[symbol("write")]
pub ext fun libc_write(fd: i64, buf: *u8, n: i64) i64;
```

An overridden name is taken **literally**: it is the object symbol, so no
platform prefix is applied to it and you spell the exact name the object carries
(`_write` on Darwin, `write` elsewhere). That is what lets inline asm and the
runtime entry points name a symbol exactly.

Common reasons to rename:

- The C name (e.g. `write`) would shadow other things in your namespace.
- The target's calling convention or ABI decorates symbol names and you
  need to match the decorated form.

## Library attribution

On a two-level dynamic format — PE (Windows) or Mach-O (Darwin) — the `library`
decorator pins an `ext` import to the dependency that exports it:

```mach
#[library("ws2_32.dll")]
ext fun WSAStartup(ver: u16, data: *u8) i32;
```

- The value names a link dependency's stable logical identity. A `[link.X]`
  requirement supplies it through `library = "..."`, defaulting to `X`; a bare
  command-line `-l name` uses `name`. The dependency's exact canonical loader
  name (`libfoo.so.3`, an `LC_ID_DYLIB` install name, or `foo.dll`) is also
  accepted for compatibility. The named library **must** be among the link's
  dependencies; pinning to one that is not is a hard link error
  (`import '<sym>' pinned to library '<lib>' not among the link's dependencies`),
  never a silent fallback. A logical identity that equals a different
  dependency's loader name is rejected as ambiguous.
- An `ext` import with no `library` is unattributed. PE and Mach-O require every
  dynamic import to identify its provider, so an unattributed import is a hard
  link error on those targets.
- A symbol with **no `ext` declaration at all** — one a linked static archive
  leaves undefined — has nothing to decorate. Its provider is named from the
  manifest instead, by the `symbols` key on the `[link.X]` entry that supplies
  it; see [manifest.md](../manifest.md#linkname--link-requirements). The two
  declarations write the same attribution, so a symbol may be claimed only once.
  A claim is spelled like the source name, not like the object symbol: both routes
  key on the link name, and Mach applies the target's C symbol prefix to the
  manifest name just as it does to a declaration's.
- "Only once" is checked across every declaration, and two `#[library]`
  decorators are no exception. Two `ext` declarations that resolve to the same
  link name but name different libraries are a hard error identifying both
  (`import '<sym>' is attributed to library '<a>' by declaration '<d>' in module
  '<m>' and to library '<b>' by declaration ...`), so which library the import
  table names never depends on module load order. Two declarations naming the
  *same* library are accepted: they name one provider, which is what a shared
  binding declared in two modules does.
- Declarations under **different arms of one `$if` chain** are exempt, because no
  target selects both arms, so they never both apply. A target-conditional
  attribution is written the obvious way and is not a conflict.

`library` composes with `symbol`: the rename sets the imported symbol's name,
`library` sets the dependency it is imported from. On one decl the import is emitted
under the renamed symbol within the named library.

```mach
# imported as `socket` from ws2_32.dll, called as `ws2_socket` in Mach
#[library("ws2_32.dll")] #[symbol("socket")]
ext fun ws2_socket(af: i32, kind: i32, proto: i32) i64;
```

A linked Windows COFF object compiled with C/C++ `dllimport` commonly refers to
`__imp_X`, the address of X's Import Address Table cell, instead of calling X
directly. The PE linker applies X's normal `#[library]` or manifest `symbols`
attribution, emits the undecorated loader import X, and resolves the object
reference to that IAT cell. If another object also refers to X directly, both
spellings share one import and one IAT entry; do not declare or map `__imp_X` as
a separate export.

`__imp_X` names that IAT cell, which the import table synthesizes — it is never
itself an export, so no library provides a symbol under that name. An import
whose loader-facing name still carries the prefix is a hard link error naming the
export it denotes, not something the image is allowed to carry: such an image
links clean and fails only when Windows loads it, with a missing-entry-point
error. Attribute and import the undecorated `X`; every `__imp_X` reference
resolves to its address cell from there.

On a format whose loader resolves imports by global search (ELF), an import is
not bound to a single dependency, so `library` has no effect on the emitted
binary — the value is still validated against the link's dependencies, but every
dependency is searched at load time regardless.

## Vector arguments and the C ABI

A 128-bit vector (`f32x4`, `i32x4`, …) crossing an `ext` boundary follows the
target's **C** vector convention, not Mach's internal one, so the call is
bit-compatible with a C `__m128` parameter:

- **x86_64-windows (Microsoft x64):** the caller passes each vector argument **by
  reference** — it stores the vector to a 16-byte-aligned temporary and passes that
  temporary's address in the parameter's integer register (RCX/RDX/R8/R9) or, once
  those are exhausted, on the stack. A variadic vector argument is passed the same
  way. Vector **returns** ride XMM0, which both conventions already agree on.
- **Every other target** (System V, AAPCS64, RISC-V lp64d): the C convention already
  matches Mach's internal one, so nothing special happens at the boundary.

A vector **wider** than the target's vector register (`f32x8` on any target today)
has no C convention to follow: it is an AVX/SVE type the baseline does not have, so
no psABI classifies it. Mach gives it the memory class on every convention — a
hidden pointer to its storage for an argument, the indirect-result pointer for a
return — which is the placement it already has internally.

Only a *direct* call to a declared `ext fun` marshals — an ordinary Mach→Mach call
always keeps the internal convention (a vector in a vector register). A call through
a function *pointer* does not yet carry the `ext` fact, so it is not covered.

## Linking external objects

An `ext fun` is only a forward reference — its definition must be supplied at
link time, either **statically** by an external precompiled object/archive or
**dynamically** by a shared library bound at load time. Provide those inputs to
`mach build` either on the command line or through the project manifest. An
undefined `ext` symbol that no static object defines and no shared library can
bind is a link error.

### Command line

C-toolchain-style flags, consumed by `mach build`:

```sh
# explicit object / archive / shared-library path
mach build . path/to/libfoo.o
mach build . path/to/libfoo.a
mach build . path/to/libfoo.so
mach build . path/to/libfoo.dylib
mach build . foo.dll

# search dir + library name: static candidates first, then the target's .so/.dylib spelling
mach build . -L build/libs -l foo

# link against a system shared library (libc) dynamically
mach build . -l c
```

- A bare argument that contains a `/`, ends in `.o` (object) or `.a` (archive),
  or names a `.so`, `.dylib`, or `.dll` is treated as an explicit input. A
  relative path is tried first against the working directory, then against the
  project root.
- `-l <name>` resolves to an object, archive, or shared library: each `-L <dir>`
  is searched for `lib<name>.o`, `<name>.o`, `lib<name>.a`, then `<name>.a`;
  finally the working directory is searched for the same four names (loose
  objects preferred over archives). Only if no static candidate exists does it
  fall back to the selected target's shared spelling: `lib<name>.so[.N]` for
  ELF or `lib<name>[.<N>].dylib` for Mach-O. A resolved `@rpath/` dylib carries
  its selected directory into the executable as `LC_RPATH`. `-L` and `-l` may
  each be repeated.

### Manifest

Artifacts name typed `[link.X]` requirements. Use `system` for a named library,
`framework` for a Darwin framework, or `local` for an object/archive/shared
library path. Filters select the applicable target:

```toml
[link.foo-unix]
source  = "system"
name    = "foo"
library = "foo"
os      = ["linux", "darwin"]
isa     = "*"
abi     = "*"
export  = false

[link.foo-win]
source  = "system"
name    = "foo.dll"
library = "foo"
os      = "windows"
isa     = "*"
abi     = "*"
export  = false
```

Both entries expose the logical name `foo`, so `#[library("foo")]` is portable
across the mutually exclusive target filters. Manifest and command-line inputs
are both included; a name that cannot be resolved is a hard error.

### Scope

Loose `.o` relocatable objects and static `.a` archives are linked **statically**.
An archive contributes only the members that define a symbol left undefined by
the objects and members before it, selected to a fixed point within that
archive, so a vendored archive costs the binary exactly the members it uses (see
[cli.md](../cli.md#static-vs-dynamic-resolution)). A shared `.so`, `.dylib`, framework,
or `.dll` is a **dynamic** dependency: its format-specific canonical loader name
is recorded and undefined `ext` functions become run-time imports. A shared
input is validated before it is recorded: a `.so` that is not a loadable ELF
shared object for the selected architecture (a linker script, a
foreign-architecture file) is refused (`'<file>' is not a loadable ELF shared
object for the selected architecture`). A static definition always wins over a
same-named dynamic import.

## See also

- [fun.md](fun.md) — regular function declarations
- [val-var.md](val-var.md) — `ext val` / `ext var`, the data analogue (foreign data imports)
- [visibility.md](visibility.md) — `pub` and `ext` modifiers
- [decorators.md](decorators.md) — `symbol`, `library`, and other codegen decorators
- [variadics.md](variadics.md) — the comptime pack `...`, which is a Mach calling convention and not this one

## Windows vector carriers

The `win64` ABI maps a vector's lane bytes to the following C carrier. This
mapping applies to arguments, returns, and typed function pointers, including
calls between Mach functions. It is independent of the selected C compiler.

| Vector extent | C carrier | Arguments | Return |
|---|---|---|---|
| 2 bytes | `unsigned short` | integer register or stack slot | low 16 bits of `RAX` |
| 4 bytes | `unsigned int` | integer register or stack slot | low 32 bits of `RAX` |
| 8 bytes | `__m64` | integer register or stack slot | `RAX` |
| 16 bytes | `__m128`, `__m128i`, or `__m128d` | pointer to a caller-owned, 16-byte-aligned copy | `XMM0` |
| Any other extent `N` | `struct { unsigned char bytes[N]; }` | pointer to a caller-owned, 16-byte-aligned copy | caller-provided result storage |

Include `<mmintrin.h>` for `__m64` and `<emmintrin.h>` for the 128-bit intrinsic
carriers. The byte-array carrier has exactly `N` bytes and no trailing padding.
The result-storage pointer for an aggregate return occupies the first integer
argument position, shifting the other arguments by one position. The callee
returns that pointer in `RAX`.

Lane zero occupies the first bytes. Each lane keeps its little-endian integer
or IEEE floating-point bit representation. Carrier conversion copies bits,
without numeric conversion, lane widening, or padding between lanes. Signed
and floating-point lanes therefore use the same transport as unsigned lanes
of the same width. A C function can inspect or construct lane bytes with a
union or `memcpy`. Only the vector's `N` bytes belong to the value, even when
its argument temporary has additional alignment padding.

For example, `i16x4` uses `__m64`, `f32x4` uses a 128-bit intrinsic carrier,
and `f32x3` uses `struct { unsigned char bytes[12]; }`. The memory layout of
an enclosing Mach record is a separate contract from this function-boundary
carrier mapping.

A generic C extension such as `short __attribute__((vector_size(8)))` is not
a substitute for `__m64`: GCC and Clang assign that extension different Windows
argument and return conventions. Declare the carrier above at the boundary.
