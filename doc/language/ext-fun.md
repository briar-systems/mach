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

Loose `.o` relocatable objects and static `.a` archives are linked **statically**
(a `.a` contributes every one of its member objects — all members are pulled, not
just those satisfying an undefined symbol). A shared `.so`, `.dylib`, framework,
or `.dll` is a **dynamic** dependency: its format-specific canonical loader name
is recorded and undefined `ext` functions become run-time imports. A static
definition always wins over a same-named dynamic import.

## See also

- [fun.md](fun.md) — regular function declarations
- [val-var.md](val-var.md) — `ext val` / `ext var`, the data analogue (foreign data imports)
- [visibility.md](visibility.md) — `pub` and `ext` modifiers
- [decorators.md](decorators.md) — `symbol`, `library`, and other codegen decorators
