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
link time by an external precompiled object. Provide those objects to
`mach build` either on the command line or through the project manifest. An
undefined `ext` symbol with no object supplying it is a link error.

### Command line

C-toolchain-style flags, consumed by `mach build`:

```sh
# explicit object-file path
mach build . path/to/libfoo.o

# search dir + library name: resolves `lib<name>.o` then `<name>.o` in each -L dir
mach build . -L build/libs -l foo
```

- A bare argument that contains a `/` or ends in `.o` is treated as an explicit
  object path. A relative path is tried first against the working directory,
  then against the project root.
- `-l <name>` resolves to a loose object: each `-L <dir>` is searched for
  `lib<name>.o`, then `<name>.o`; finally the working directory is searched for
  the same two names. `-L` and `-l` may each be repeated.

### Manifest

`[targets.*].libs` lists project-level link inputs, each an explicit object
path (project-relative or absolute) or a bare `-l`-style name:

```toml
[targets.linux]
# ...
libs = ["build/libs/libfoo.o", "bar"]
```

`link` is accepted as an alias for `libs`. Manifest inputs and command-line
inputs are both included; a name that resolves to no existing file is a hard
error.

### Scope

Only loose `.o` relocatable objects are linked. Static archives (`.a`) and
shared libraries (`.so`) are not yet supported — extract or recompile the
member objects you need, or wait on the dynamic-linker work.

## See also

- [fun.md](fun.md) — regular function declarations
- [visibility.md](visibility.md) — `pub` and `ext` modifiers
- [comptime-attrs.md](comptime-attrs.md) — `.symbol` and other attributes
