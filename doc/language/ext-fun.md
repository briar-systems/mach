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

## Symbol rename

By default the linker symbol matches the Mach name. Override it with the
`.symbol` attribute:

```mach
$libc_write.symbol = "write";
pub ext fun libc_write(fd: i64, buf: *u8, n: i64) i64;
```

Common reasons to rename:

- The C name (e.g. `write`) would shadow other things in your namespace.
- The target's calling convention or ABI decorates symbol names and you
  need to match the decorated form.

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

# search dir + library name: resolves lib<name>.o / <name>.o / lib<name>.a / <name>.a, then lib<name>.so
mach build . -L build/libs -l foo

# link against a system shared library (libc) dynamically
mach build . -l c
```

- A bare argument that contains a `/`, ends in `.o` (object) or `.a` (archive),
  or names a `.so` (shared library) is treated as an explicit input path. A
  relative path is tried first against the working directory, then against the
  project root.
- `-l <name>` resolves to an object, archive, or shared library: each `-L <dir>`
  is searched for `lib<name>.o`, `<name>.o`, `lib<name>.a`, then `<name>.a`;
  finally the working directory is searched for the same four names (loose
  objects preferred over archives). Only if no static candidate exists does it
  fall back to a shared `lib<name>.so` (in the `-L` dirs and the system library
  directories). `-L` and `-l` may each be repeated.

### Manifest

`[targets.*].libs` lists project-level link inputs, each an explicit object /
archive / shared-library path (project-relative or absolute) or a bare
`-l`-style name:

```toml
[targets.linux]
# ...
libs = ["build/libs/libfoo.a", "bar", "c"]
```

`link` is accepted as an alias for `libs`. Manifest inputs and command-line
inputs are both included; a name that resolves to no existing file is a hard
error.

### Scope

Loose `.o` relocatable objects and static `.a` archives are linked **statically**
(a `.a` contributes every one of its member objects — all members are pulled, not
just those satisfying an undefined symbol). A shared `.so` library is a **dynamic**
dependency: its `DT_SONAME` is recorded and undefined `ext` symbols are bound
against it at load time through a `PT_INTERP`/PLT in the produced binary. A
static definition always wins over a same-named dynamic import. Dynamic linking
is currently implemented for the ELF (Linux) target; the PE (Windows) and Mach-O
(Darwin) import paths are in progress.

## See also

- [fun.md](fun.md) — regular function declarations
- [visibility.md](visibility.md) — `pub` and `ext` modifiers
- [comptime-attrs.md](comptime-attrs.md) — `.symbol` and other attributes
