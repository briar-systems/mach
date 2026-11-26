# Migrating a project from mach 6 to mach 7

This page walks a version 6 project to version 7 in the order the work comes
up. `mach migrate` does most of it. The rest is a short list of changes made by
hand, each shown below as the version 6 form, the diagnostic version 7 gives
for it, and the version 7 form.

The reference for what `mach migrate` rewrites and reports is
[doc/language/migrate.md](doc/language/migrate.md). Where that page or another
language page states a rule, this page links to it and does not restate it.
std's own renames are in std's migration notes,
[dep/std/MIGRATION.md](dep/std/MIGRATION.md).

## 1. Before you run anything

**Commit first.** `mach migrate` edits files in place, so a clean tree makes its
change one reviewable diff.

**A project on mach 5** goes through mach 6 first. `mach migrate` carries only
the rows from the major before it (see
[migrate.md](doc/language/migrate.md)).

**A project id with a `-`** stops the migration partway, since a version 7 id
must be spellable in a `use` path ([manifest.md](doc/language/manifest.md#project)).
Rename it before migrating, and rename each `[dep.<id>]` table and `dep/<id>`
directory that names it in other projects.

Before (version 6 accepted it):

```toml
[project]
id = "my-lib"
```

```
error[manifest.invalid_name] mach.toml: [project].id "my-lib" is not allowed. a project id must start with a letter or '_' and hold only letters, digits and '_', so that a `use` path can name it
  @ ./mach.toml:2:6
2 | id = "my-lib"
  |      ^^^^^^^^
```

After:

```toml
[project]
id = "my_lib"
```

## 2. Run `mach migrate`

Run it once with `--check` to see what it will do, then for real:

```
mach migrate . --check
mach migrate .
```

It rewrites the root project's `mach.toml` and every source file under its
`[project].src`, names each file it changes, and reports what it leaves for you.
A repository holding several projects (demos, test projects, benchmarks, each
with its own `mach.toml`) needs one run per project. On a small version 6
binary project, `--check` printed:

```
--- ./mach.toml
+++ ./mach.toml
@@ -4,1 +4,1 @@ [project]
-mach    = "^6.5"
+mach    = "^7"
@@ -6,1 +6,1 @@ [project]
-out     = "out/{target.name}/{profile.name}"
+work    = "out/{target.name}/{profile.name}"
@@ -9,4 +9,3 @@ [target.linux-x86_64]
-isa = "x86_64"
-os  = "linux"
-abi = "sysv64"
-default = true
+arch = "x86_64"
+sys  = "linux"
+abi  = "sysv64"
@@ -16,1 +15,1 @@ [profile.debug]
-opt = 0
+optimize = false
@@ -19,2 +17,0 @@ [profile.debug]
-vectorize = true
-float_reassoc = false
@@ -23,1 +20,1 @@ [profile.release]
-opt = 2
+optimize = true
@@ -31,1 +28,1 @@ [artifact.vapp]
-out     = "bin/vapp"
+out     = "{project.work}/bin/vapp"
--- ./src/bin/main.mach
+++ ./src/bin/main.mach
@@ -11,1 +11,1 @@
-#[inline]
+#[inline("always")]
@@ -16,1 +16,1 @@
-    $if ($mach.build.os == $mach.os.linux) {
+    $if ($mach.build.sys == $mach.sys.linux) {
dependency 'std' was not read, as dep/ holds no manifest for it
dependency 'vlib' was not read, as dep/ holds no manifest for it
```

The full list of rewrites, the reports, the way scripts are read and the exit
codes are in [migrate.md](doc/language/migrate.md#the-changes-from-6-to-7).
Three points matter for the order of the work:

- Dependencies are never edited. `mach migrate` reads each one's manifest under
  `dep/` to rewrite `use <id>;`, so realize them (section 3) and run it again.
  A second run is safe, as a migrated project changes nothing.
- Scripts, CI workflows and step `argv` lines that pass a removed option or read
  a removed step variable are reported with their file and line, never
  rewritten (section 6).
- A report about the manifest, such as a linux target that needs an `env`, is
  for you to settle by hand (section 4).

Then run `mach fmt .` to realign source lines whose names changed length.

## 3. Dependencies

Each dependency must be on a release whose own manifest accepts mach 7. A
dependency still on version 6 cannot be read at all, and `mach migrate` names it:

```
dependency 'std' requires mach ^6.5, the previous major. it migrates as its own project
```

```
error[manifest.removed_key] dependency 'std' cannot be read (mach.toml: [project].out is not allowed. write the work directory as `work`, which a template names `{project.work}`)
  @ ./dep/std/mach.toml:6:1
6 | out     = "out/{target.name}/{profile.name}"
  | ^^^
```

Move each `[dep.<id>]` to a version 7 release (`mach dep update . <id>`), run
`mach dep pull .`, then run `mach migrate .` again. How a closure is checked
against the running compiler is in
[manifest.md](doc/language/manifest.md#compiler-range).

Several std modules moved into packages of their own in this release. Add a
`[dep.<id>]` for each one you import, as
[std's migration notes](dep/std/MIGRATION.md) list them, and import its modules
by source path as section 5 describes.

## 4. The manifest by hand

### A linux target that links a system library needs `env`

Version 6 found a system library in the gnu environment's directories when the
target named no environment. `mach migrate` reports each such target and exits 1:

```
mach.toml:8: [target.linux-x86_64] states no env, and [link.pthread] links a system library on it. 6 found it as the gnu environment does, and 7 finds a system library only through the target's environment. write `env = "gnu"`, under which a bin links pie unless its `linkage` says otherwise
```

Left as it is, the build is refused:

```
error[link.input_invalid] the target states no environment that provides system libraries, so `pthread` has nowhere to be found (linux)
```

After:

```toml
[target.linux-x86_64]
arch = "x86_64"
sys = "linux"
abi = "sysv64"
env = "gnu"
```

### Name the C library when you call it

Version 7 binds a reference only to a library the link names, so a program that
calls into the C library through `pthread`, `m`, `dl` or `rt` names `c` too (the
row in [migrate.md](doc/language/migrate.md#the-changes-from-6-to-7)).

Before:

```toml
[artifact.thr]
link = ["pthread"]
```

```
error[link.input_invalid] the link refers to symbols nothing defines (src/bin/main.mach)
  note nothing defines `pthread_self`
  note src/bin/main.mach refers to `pthread_self`
  note /lib64/libpthread.so.0 depends on `libc.so.6`, which the link does not name
```

After, with a `[link.c]` entry beside `[link.pthread]`:

```toml
[artifact.thr]
link = ["c", "pthread"]

[link.c]
source = "system"
name = "c"
sys = ["linux"]
arch = ["*"]
abi = ["*"]
```

### Artifact file names

`mach migrate` keeps each artifact's file name exactly as version 6 wrote it, so
`out = "lib/libvlib"` becomes `"{project.work}/lib/libvlib"` and still builds a
file named `libvlib` with no `.a`, and a windows binary still has no `.exe`. To
get the target's usual suffix, add `{artifact.suffix}` yourself
([manifest.md](doc/language/manifest.md#artifact-filenames-and-identity)):

```toml
[artifact.lib]
out = "{project.work}/lib/libvlib{artifact.suffix}"
```

On linux this writes `libvlib.a`. Scripts that run a built file by path may need
the new name.

### A windows link entry nothing imports from states `include = "referenced"`

Version 6 linked the DLL of a `[link.<key>]` entry whether or not the program
imported anything from it. A version 7 PE image loads no DLL it imports nothing
from, so an entry left at the default `include = "always"` whose DLL the program
never calls is refused at the entry:

```
error[link.input_invalid] link entry 'ws2_32' has the image load ws2_32.dll whether or not anything imports from it, and nothing does. a PE image loads no DLL it imports nothing from, so an entry the program may not reference states `include = "referenced"`
```

After:

```toml
[link.ws2_32]
source = "system"
name = "ws2_32.dll"
include = "referenced"
sys = ["windows"]
arch = ["*"]
abi = ["*"]
```

`mach migrate` leaves `include` as it is. Whether a program references an entry
is a property of the program, and on ELF and Mach-O `"always"` still names a
library nothing imports from.

### Other manifest reports

A spirv target listed beside other targets in one `bin` artifact, several
default libraries in one manifest, and a darwin `source = "system"` entry named by
a bare library name are reported by `mach migrate`. The last is also refused by
the link until it names the install name.
The answer for each is in
[migrate.md](doc/language/migrate.md#the-changes-from-6-to-7).

## 5. Source by hand

### Import a dependency by source path

A `use` path is the dependency's project id followed by a source path, so the
one-segment `use <id>;` names no module
([modules.md](doc/language/modules.md#a-use-path-is-a-project-id-and-a-source-path)).
`mach migrate` rewrites `use <id>;` and `fwd <id>;` from the dependency's
manifest. Any it leaves is refused:

```mach fragment
use vlib;
```

```
error[use.unresolved] use path 'vlib' does not name a module
```

After, naming the module that holds what you use (here the library's entry,
`src/lib/vlib.mach`):

```mach fragment
use vlib.lib.vlib;
```

A library's entry file is not the library's API. Where a package splits its
surface over several modules, import the modules that declare what you call.

### Truth values are `u1`

Comparisons, `&&`, `||`, `!` and `sel` produce `u1`, a condition must be a `u1`,
and std's `bool` is `def bool: u1;` ([types.md](doc/language/types.md)). Code
that treated a truth value as a `u8` is refused at each site. The fix is a
comparison, a cast at the boundary, or a `u1` destination, as the second table of
[migrate.md](doc/language/migrate.md#the-changes-from-6-to-7) says.

A `u8` condition, before:

```mach error a condition must have type u1
use std.runtime;

#[symbol("main")]
fun main(argc: i64, argv: **u8) i64 {
    if (argc & 1) {
        ret 0;
    }
    ret 1;
}
```

`error[condition.type] a condition must have type u1`. When the condition is a
`u8`, as `flags & 1` is for a `u8` named `flags`, the message is `a condition of
type u8 is a previous major's: a condition has type u1`. After:

```mach fragment
    if ((argc & 1) != 0) {
        ret 0;
    }
```

A comparison stored in a `u8`, before:

```mach error expected u8, found u1
use std.runtime;

rec Flags {
    closed: u8;
}

#[symbol("main")]
fun main(argc: i64, argv: **u8) i64 {
    var f: Flags;
    f.closed = argc == 1;
    ret 1 - f.closed::i64;
}
```

`error[type.mismatch] expected u8, found u1`. After, either cast where the value
enters the `u8`:

```mach fragment
    f.closed = (argc == 1)::u8;
```

or give the destination the truth type, which is the better fix for a private
field. A public field or parameter that callers write through a `*u8` keeps its
`u8` and takes the cast:

```mach fragment
use std.types.bool.bool;

rec Flags {
    closed: bool;
}
```

The same holds for secrets. A secret comparison is a `^u1`, so storing it in a
`^u8` is refused with `error[type.mismatch] expected ^u8, found ^u1`. Before:

```mach error expected ^u8, found ^u1
fun is_newline(c: ^u8) ^u8 {
    ret c == '\n';
}
```

After:

```mach fragment
fun is_newline(c: ^u8) ^u8 {
    ret (c == '\n')::^u8;
}
```

A u1 is one byte holding 0 or 1, so changing a field from `u8` to `bool` keeps
the record's layout.

### `x::bool` now asks whether `x` is non-zero

This one compiles in both majors and gives a different answer. Version 6's
`bool` was a `u8`, so `x::bool` kept the low byte, and `256::bool` was 0.
Version 7's `x::u1` asks whether the integer is non-zero
([types.md](doc/language/types.md)), so the same cast is 1:

```mach
use std.runtime;
use std.types.bool.bool;

#[symbol("main")]
fun main(argc: i64, argv: **u8) i64 {
    val flags: u32 = 256;
    val b: bool = flags::bool;
    if (b) {
        ret 0;
    }
    ret 1;
}
```

Built with mach 6 this exits 1, and with mach 7 it exits 0.

Search for `::bool` and check that a non-zero test is what each site meant. If
the low byte was the intent, write `(x & 0xff) != 0`.

### Constant-time code passes secrets only to functions it names

`mach migrate` rewrites `#[oblivious]` to `#[constant_time("soft")]`. A
constant-time function may pass a secret only to a constant-time callee it names
directly, or through a function value whose type is an alias marked
`#[constant_time]` ([secrecy.md](doc/language/secrecy.md#calls)). Version 6
accepted a secret passed through any function value from an `#[oblivious]`
function. Before:

```mach error through a function value whose type states no constant-time mode
use std.runtime;

#[constant_time("soft")]
fun wipe(p: *^u8) {
    @p = 0;
}

#[constant_time("soft")]
fun apply(f: fun(*^u8), p: *^u8) {
    f(p);
}

#[symbol("main")]
fun main(argc: i64, argv: **u8) i64 {
    var k: ^u8 = 7;
    apply(wipe, ?k);
    ret k:>u8::i64;
}
```

```
error[secret.call_untrusted] a constant-time function passes `*^u8`, which holds a secret, through a function value whose type states no constant-time mode. give the function type an alias marked `#[constant_time]`, or call a function declared constant time by name
   @ apply.mach:10:5
10 |     f(p);
   |     ^^^^
```

After, calling the callee by name:

```mach fragment
#[constant_time("soft")]
fun apply(p: *^u8) {
    wipe(p);
}
```

Where the callee really varies, give the function value a marked alias as its
type, and every function stored in it must be constant time itself:

```mach fragment
#[constant_time("soft")]
def Wipe: fun(*^u8);

#[constant_time("soft")]
fun apply(f: Wipe, p: *^u8) {
    f(p);
}
```

### Tags that hold secrets can grow

A tag whose payloads disagree on which bytes are secret may change size and
payload offsets with no source change. Nothing is rewritten or reported. Check
code that persists such a tag's bytes, shares it with C, or relies on its
`$size_of`, as [migrate.md](doc/language/migrate.md#the-changes-from-6-to-7)
explains.

## 6. Scripts, CI and the command line

`mach migrate` reports each removed option it finds in a script, a CI workflow
or a step's `argv`, with the spelling to use, and the command line refuses it by
name. The full list is the "Left for you to change" table of
[migrate.md](doc/language/migrate.md#the-changes-from-6-to-7), and which files
count as scripts is under
[How a script is read](doc/language/migrate.md#how-a-script-is-read). Some of
the refusals:

```
mach build . -O2         error[cli.flag_removed] option '-O2' is not allowed. use -O or --optimize
mach build . -g          error[cli.flag_removed] option '-g' is not allowed. use --debug
mach build . --jobs 2    error[cli.flag_removed] option '--jobs' is not allowed. use --threads
mach build . --emit-ir   error[cli.flag_removed] option '--emit-ir' is not allowed. use --emit ir
mach build . --pie       error[cli.flag_removed] option '--pie' is not allowed. use --linkage pie, or the linkage key of a bin artifact
```

After:

```
mach build . -p release --threads 2
mach build . --emit ir --emit exe
```

A flag that changes the selected profile, as `-O` or `--debug` does, writes a
differently configured build, so it names its output with `-o` or `-w`:
`mach build . -O --debug -o out/app-fast`.

Two more that scripts carry:

- `mach dep add ... --version <range>` is now `--range <range>`. The old flag is
  refused as `cli.flag_removed`, naming `--range`.
- `mach check` takes no `--threads`. Only `build`, `test` and `run` do, and a
  `--threads` on `mach check` is refused as `cli.flag_unknown`.

### Build steps read `MACH_TARGET_ARCH` and `MACH_TARGET_SYS`

A step's environment carries `MACH_TARGET_ARCH`, `MACH_TARGET_SYS` and
`MACH_TARGET_ABI` ([manifest.md](doc/language/manifest.md#stepname--build-steps)).
A step still reading version 6's `MACH_TARGET_ISA` or `MACH_TARGET_OS` is not
refused. It reads an unset variable, so a script with a default quietly builds
for its default on every target. `mach migrate` reports each one. Before:

```toml
[step.probe]
argv = ["sh", "-c", "echo \"${MACH_TARGET_ISA:-unset} ${MACH_TARGET_OS:-unset}\" > {project.work}/probe.txt"]
in = []
out = ["{project.work}/probe.txt"]
```

writes `unset unset`. After:

```toml
[step.probe]
argv = ["sh", "-c", "echo \"${MACH_TARGET_ARCH:-unset} ${MACH_TARGET_SYS:-unset}\" > {project.work}/probe.txt"]
in = []
out = ["{project.work}/probe.txt"]
```

writes `x86_64 linux` on a linux-x86_64 build.

## 7. Spellings `mach migrate` rewrites

These are listed so that code written by hand, or copied from version 6
sources after the migration, is recognized. Each old spelling is refused with a
message that names its replacement.

```mach fragment
#[inline]                      # decorator.arity, now #[inline("always")]
#[noinline]                    # decorator.unknown, now #[inline("never")]
#[scalar]                      # decorator.unknown, now #[vectorize("never")]
#[oblivious]                   # decorator.unknown, now #[constant_time("soft")]
asm x86_64 { pause }           # asm.head, now asm x86-64 { pause }
$if ($mach.build.os == $mach.os.linux) { }   # comptime.path_unknown, now $mach.build.sys and $mach.sys
```

The complete list, manifest keys included, is the first table of
[migrate.md](doc/language/migrate.md#the-changes-from-6-to-7). The decorators
are described in [decorators.md](doc/language/decorators.md) and the `asm`
heads in [asm.md](doc/language/asm.md).

## 8. std

std's renames, removals and moves to packages are in
[dep/std/MIGRATION.md](dep/std/MIGRATION.md), which belongs to std. `mach
migrate` does not rewrite them. A removed or renamed std name is refused as
`visibility.not_exported` or `use.unresolved` at each site, so the first `mach
check` after the migration lists every site to change.

## 9. What stays the same

- Entry files, by convention `src/bin/main.mach` for a program and
  `src/lib/<id>.mach` for a library, and `use std.runtime;` in each executable
  entry ([files.md](doc/language/files.md)).
- Test declarations: `test <identifier> { ... }`, listed as
  `<module path>#<name>` ([test.md](doc/language/test.md)).
- The layout under the work directory: `obj/`, and `ir/` and `asm/` for
  `--emit ir` and `--emit asm`
  ([manifest.md](doc/language/manifest.md#the-work-directory)).
- A dependency is the files under `dep/<id>`, and `mach dep pull <path>`
  realizes them ([dependencies.md](doc/language/dependencies.md)).

## 10. Check the result

```
mach migrate . --check      # prints no diff and exits 0
mach fmt --check .
mach check . --all
mach test .
```
