# Shipping an application

`mach build` produces a binary. This page covers everything after that: how to
decide what links statically, what each operating system expects a shipped
application to look like, and how much of a three-platform release one machine
can actually produce (more than you would expect, but not all of it — see
[the darwin limit](#the-darwin-limit-vendored-c-builds-on-macos)).

Mach owns the binary and its link. It does not own code signing, installers, or
store submission — those are other people's tools, and where one is required
this page names it and says so plainly. Nothing here is a `mach` subcommand
waiting to be written; the division is deliberate.

The worked example throughout is a project with one `bin` artifact, a vendored C
library (`libqz.a`) built by a `[step]`, and a loose asset directory. See
[manifest.md](manifest.md) for the manifest keys it uses.

## The shape of a release

One artifact per target, staged into a directory, archived. The archive is the
deliverable.

| Target  | Archive  | Contains |
|---------|----------|----------|
| linux   | `tar.gz` | one top-level directory: the binary, assets, `LICENSE` |
| windows | `zip`    | one top-level directory: the `.exe`, any DLLs, assets, `LICENSE` |
| darwin  | `zip`    | a `.app` bundle at the archive root |

A top-level directory on linux and windows keeps an extraction from scattering
files into whatever directory the user was standing in. The darwin archive is
the exception: a `.app` is *already* a directory, and macOS expects to find it at
the root of the archive it arrives in.

> Mach's own release archives are flat — a single `mach` binary and `LICENSE`,
> no top-level directory (see `.github/workflows/cd.yml`). That is a reasonable
> shape for a single-file CLI tool and a poor one for an application with assets.

## linux

### The static route

Link everything you can. A linux executable whose every input is a loose `.o` or
a static `.a` has no dynamic dependencies at all:

```sh
mach build . --profile release
```

```
$ file out/linux-x86_64/release/bin/demo
ELF 64-bit LSB executable, x86-64, version 1 (SYSV), statically linked, stripped
$ ldd out/linux-x86_64/release/bin/demo
	not a dynamic executable
```

This is the default outcome, not a mode you opt into. Mach links its own objects
without a system linker and does not link libc, so a pure-Mach project is
statically linked whether or not you thought about it. A vendored C dependency
joins that binary the moment its `[link.X]` resolves to an `.a`:

```toml
[link.qz]
source = "local"
path   = "{project.out}/obj/qz/libqz.a"
os     = "*"
isa    = "*"
abi    = "*"
export = false
```

An `.a` joins with classic archive semantics: a member object is pulled in only
to satisfy an undefined symbol, transitively to a fixed point, so an archive you
vendor costs the binary exactly the members it uses and no more.

**No rpath is needed, and none is emitted.** `rpath` exists to tell a dynamic
loader where to look; a static executable never invokes one. There is no
interpreter, no `DT_NEEDED`, no search path to get wrong, and no `LD_LIBRARY_PATH`
wrapper script to write. The binary in the archive is the binary that runs.

### Where the dynamic boundary belongs

Some libraries should not be vendored. A system library that is part of the
running desktop — X11, OpenGL, Wayland, ALSA — must be the *host's* copy, because
it talks to a server and a driver stack that shipped with the host. Ship your own
`libX11.so` and you have built a program that cannot talk to the display it was
installed next to.

Declare those as `system` and let them stay dynamic:

```toml
[link.x11]
source  = "system"
name    = "X11"
library = "X11"
os      = "linux"
isa     = "*"
abi     = "*"
export  = false
```

```
$ file out/linux-x86_64/release/bin/demo-x11
ELF 64-bit LSB executable, x86-64, version 1 (SYSV), dynamically linked,
interpreter /lib64/ld-linux-x86-64.so.2, no section header
$ readelf -dW out/linux-x86_64/release/bin/demo-x11 | grep NEEDED
 0x0000000000000001 (NEEDED)             Shared library: [libX11.so.6]
```

The recorded name is the library's `DT_SONAME` — `libX11.so.6`, with its ABI
version, not the `libX11.so` development symlink the link resolved through. That
is what makes the dependency portable across distributions: every distribution
that ships an ABI-6 X11 satisfies it.

Note that still no rpath is emitted. `libX11.so.6` lives in the system library
path, which the loader searches by default. An rpath is only interesting for a
library you ship yourself in a place the loader would not otherwise look — which,
on this route, you are not doing.

The rule of thumb: **vendor what you compiled, borrow what the desktop owns.**
Everything you built from source goes in the binary; anything that brokers access
to hardware or a display server stays dynamic.

### tar.gz layout

```
demo-0.1.0-linux-x86_64/
demo-0.1.0-linux-x86_64/demo
demo-0.1.0-linux-x86_64/assets/
demo-0.1.0-linux-x86_64/assets/level1.dat
demo-0.1.0-linux-x86_64/LICENSE
```

```sh
mkdir -p stage/demo-0.1.0-linux-x86_64
cp out/linux-x86_64/release/bin/demo stage/demo-0.1.0-linux-x86_64/
cp -r assets LICENSE stage/demo-0.1.0-linux-x86_64/
tar -C stage -czf dist/demo-0.1.0-linux-x86_64.tar.gz demo-0.1.0-linux-x86_64
```

The binary finds `assets/` relative to its own location, not the working
directory — a user who launches from a desktop entry has a working directory you
did not choose.

## windows

### Console or GUI

A PE records in its optional header which environment it wants, and the Windows
loader honours it: a `console` image gets a console window attached to the
process, a `gui` image does not. For a graphical application this is the
difference between launching cleanly and launching with an empty black console
sitting behind your window.

The default is `console`. A graphical application says so:

```toml
[artifact.game]
kind    = "bin"
entry   = "main.mach"
out     = "bin/game.exe"
targets = ["windows-x86_64"]
link    = ["kernel32"]
need    = []
subsystem = "gui"
```

`--subsystem console|gui` overrides the key for one build. Either way, the header
is where you confirm what you got:

```
$ objdump -p out/windows-x86_64/release/bin/game.exe | grep "^Subsystem"
Subsystem		00000002	(Windows GUI)
```

Omitting the key leaves the output byte-identical to a build from before the key
existed, and the key is **inert on every other target** rather than an error — a
linux build with `subsystem = "gui"` set produces a byte-identical ELF, because
only the PE writer reads it. One manifest therefore describes every platform
without splitting into a per-OS file. [manifest.md](manifest.md#subsystem--the-windows-consolegui-selector)
and [cli.md](cli.md#mach-build) carry the full rules.

This is a packaging decision, not a code one — it changes nothing the program
does, only what Windows hands it at startup. Note the consequence for a `gui`
image: it starts with no console attached, so anything the program writes to
stdout goes nowhere by default. A graphical application that also wants
diagnostics should write them to a file rather than assume a stream is there.

### Icon and version resources

Without resources, a shipped `.exe` shows the generic application icon in
Explorer and an empty Details tab in its properties dialog. Two artifact keys
fix both:

```toml
[artifact.game]
kind     = "bin"
entry    = "main.mach"
out      = "bin/game.exe"
targets  = ["windows-x86_64"]
link     = ["kernel32"]
need     = []
icon     = "assets/game.ico"
manifest = "assets/game.manifest"
```

On a windows target either key adds a `.rsrc` section to the PE. `icon` must
name a valid ICO container; mach unpacks it, emits each contained image as
`RT_ICON`, and adds the `RT_GROUP_ICON` that indexes them — which is what
Explorer, the taskbar, and the window chrome pick their icon from. `manifest`
is embedded unchanged as `RT_MANIFEST`; that is where UAC elevation requests
and DPI-awareness declarations live, if your application needs them.

Either resource also gets a `VS_VERSIONINFO` (`RT_VERSION`) whose values are
derived rather than repeated: `FileVersion` and `ProductVersion` come from
`[project].version`, `InternalName` and `ProductName` from the artifact's table
key, and `OriginalFilename` from the basename of `out`. Version your project in
one place and the exe metadata follows it. There is deliberately no
`FileDescription` — the manifest schema has no description field, so there is
nothing to set.

Like `subsystem`, both keys are **inert off windows** — the paths are not even
read — so one artifact stanza still serves every target. If a `[step]`
generates the resource (an icon rendered at build time, a manifest stamped
with a CI version), name the step in the artifact's `need` so it runs before
the link. The linker fingerprints resource contents, so editing the asset
relinks a warm build.

Confirm what landed with `llvm-readobj --coff-resources` — one entry each of
`ICON`, `GROUP_ICON`, `MANIFEST`, and `VERSIONINFO` for the stanza above:

```
$ llvm-readobj --coff-resources out/windows-x86_64/release/bin/demo.exe | grep "Type:"
  Type: ICON (ID 3) [
  Type: GROUP_ICON (ID 14) [
  Type: VERSIONINFO (ID 16) [
  Type: MANIFEST (ID 24) [
```

(The version strings are UTF-16, so `strings -e l` reads them; `FileVersion`
and `ProductVersion` will read back the `[project].version` they were derived
from.)

### DLLs beside the exe

Anything you ship dynamically on windows goes in the same directory as the `.exe`.
This works because of what the link records: a PE import directory names the
dependency by **bare basename** and nothing else — no path, no version, no
SONAME equivalent.

```
$ mach build . --profile release --target windows-x86_64
$ objdump -p out/windows-x86_64/release/bin/demo.exe | grep "DLL Name"
	DLL Name: kernel32.dll
	DLL Name: render.dll
```

The Windows loader resolves that basename at load time, and the directory
containing the executable is the first directory it searches. So a DLL sitting
next to the `.exe` wins over any other copy on the system, which is what you want
for a library you shipped, and is the whole mechanism — there is nothing to
configure. (The protected system libraries, `kernel32.dll` among them, are
resolved from the system directory regardless; you cannot shadow those, and
should not want to.)

A consequence worth knowing: because only the name is recorded, **the DLL does
not need to exist when you link.** `render.dll` above exists nowhere on the build
machine. That is what makes cross-building to windows from linux practical, and
it is also why a typo'd DLL name is not caught until someone runs the program.

Declare each one as a `system` entry with its full filename:

```toml
[link.render]
source = "system"
name   = "render.dll"
os     = ["windows"]
isa    = ["*"]
abi    = ["*"]
export = false
```

and attribute the imports, which PE requires — an unattributed dynamic import is
a hard link error on a two-level-namespace format, never a silent fallback:

```mach
#[library("render.dll")]
ext fun render_init() i32;
```

See [language/ext-fun.md](language/ext-fun.md#library-attribution) for the
attribution rules.

### The `.exe` extension

An artifact's `out` is written literally, so the extension is yours to spell.
Per-target naming is a second artifact stanza rather than a per-cell exception:

```toml
[artifact.demo]
kind    = "bin"
entry   = "main.mach"
out     = "bin/demo"
targets = ["linux-x86_64"]
link    = ["qz"]
need    = []

[artifact.demo-win]
kind    = "bin"
entry   = "main.mach"
out     = "bin/demo.exe"
targets = ["windows-x86_64"]
link    = ["qz"]
need    = []
```

### zip layout

```
demo-0.1.0-windows-x86_64/
demo-0.1.0-windows-x86_64/demo.exe
demo-0.1.0-windows-x86_64/render.dll
demo-0.1.0-windows-x86_64/assets/
demo-0.1.0-windows-x86_64/LICENSE
```

```sh
bsdtar -C stage -a -cf dist/demo-0.1.0-windows-x86_64.zip demo-0.1.0-windows-x86_64
```

`bsdtar` (libarchive) writes both `tar.gz` and `zip`, so one tool covers every
archive on this page; it is what macOS ships as `tar`, and a package away
elsewhere. `zip -r` produces the same thing where you have it instead.

## darwin

### The `.app` bundle

macOS expects a graphical application to be a directory with a fixed shape:

```
Demo.app/
Demo.app/Contents/
Demo.app/Contents/Info.plist
Demo.app/Contents/MacOS/
Demo.app/Contents/MacOS/Demo
Demo.app/Contents/Resources/
Demo.app/Contents/Resources/Demo.icns
```

`Contents/MacOS/<name>` is the executable, `Contents/Resources/` holds icons and
data files, and `Contents/Info.plist` tells Launch Services how to treat the
whole thing. The name in `CFBundleExecutable` must match the file in
`Contents/MacOS/`.

You do not need a build step to assemble this. An artifact's `out` is just a
path under the project output, so point it into the bundle and the link writes
the executable where it belongs:

```toml
[artifact.demo-mac]
kind    = "bin"
entry   = "main.mach"
out     = "Demo.app/Contents/MacOS/Demo"
targets = ["darwin-x86_64"]
link    = ["qz"]
need    = []
```

The `Info.plist` and `Resources/` are static files you copy in beside it. A
minimum viable plist:

```xml
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN"
  "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleName</key>               <string>Demo</string>
    <key>CFBundleDisplayName</key>        <string>Demo</string>
    <key>CFBundleIdentifier</key>         <string>org.example.demo</string>
    <key>CFBundleExecutable</key>         <string>Demo</string>
    <key>CFBundlePackageType</key>        <string>APPL</string>
    <key>CFBundleVersion</key>            <string>0.1.0</string>
    <key>CFBundleShortVersionString</key> <string>0.1.0</string>
    <key>CFBundleIconFile</key>           <string>Demo</string>
    <key>LSMinimumSystemVersion</key>     <string>11.0</string>
    <key>NSHighResolutionCapable</key>    <true/>
</dict>
</plist>
```

| Key | Why it matters |
|-----|----------------|
| `CFBundleIdentifier` | reverse-DNS identity; the codesigning and notarization records key on it, and two applications sharing one are the same application as far as macOS is concerned |
| `CFBundleExecutable` | the filename under `Contents/MacOS/`; a mismatch is a bundle that will not launch |
| `CFBundleShortVersionString` | the version a user sees; `CFBundleVersion` is the build number and may differ |
| `CFBundleIconFile` | basename of the `.icns` in `Resources/`, conventionally without the extension |
| `NSHighResolutionCapable` | absent or false, the window server scales the app up from 1x and it looks blurry on every Mac sold in the last decade |

Archive the bundle at the root:

```sh
bsdtar -C out/darwin-x86_64/release -a -cf dist/demo-0.1.0-darwin-x86_64.zip Demo.app
```

The executable bit on `Contents/MacOS/Demo` has to survive the round trip or the
bundle will not launch. `bsdtar` records unix permissions in the zip; verify
rather than assume:

```
$ bsdtar -C /tmp/unz -xf dist/demo-0.1.0-darwin-x86_64.zip
$ ls -l /tmp/unz/Demo.app/Contents/MacOS/Demo
-rwxr-xr-x  ... /tmp/unz/Demo.app/Contents/MacOS/Demo
```

### Signing and notarization

**This is external tooling.** The compiler emits a Mach-O binary and stops; it
has no signing code, holds no keys, and will not grow a `mach sign`. Signing is
Apple's protocol, tied to Apple's certificate authority, and belongs to a tool
built for it.

From a linux host that tool is [`rcodesign`](https://github.com/indygreg/apple-platform-rs),
which implements Apple code signing and notarization without needing macOS:

```sh
cargo install apple-codesign
```

You need a **Developer ID Application** certificate from Apple (a paid developer
account) exported as a `.p12`, and an **App Store Connect API key** for
notarization. Neither is something the compiler can supply.

Sign the bundle — `rcodesign` recurses into nested Mach-O binaries by default,
unlike Apple's `codesign`:

```sh
rcodesign sign \
  --p12-file developer-id.p12 --p12-password-file ~/.certificate-password \
  --code-signature-flags runtime \
  Demo.app
```

`--code-signature-flags runtime` enables the hardened runtime. Notarization
rejects a bundle without it, with `The executable does not have the hardened
runtime enabled.` The flag applies to the main binary only; a bundle with several
binaries needs the scoped form (`--code-signature-flags Contents/MacOS/helper:runtime`)
for each additional one. `rcodesign sign --for-notarization` validates the whole
configuration against notarization's requirements up front and is worth using
while you are still getting a pipeline working.

Then notarize. A bundle can be submitted directly — `rcodesign` zips it for you,
so there is no separate archive step — and `--staple` waits for the result and
attaches the ticket on success:

```sh
rcodesign notary-submit \
  --api-key-file ~/.appstoreconnect/key.json \
  --staple \
  Demo.app
```

Encode the API key file once, beforehand:

```sh
rcodesign encode-app-store-connect-api-key \
  -o ~/.appstoreconnect/key.json \
  <issuer-id> <key-id> ~/Downloads/AuthKey_<key-id>.p8
```

Notarization is asynchronous and can take anywhere from seconds to much longer;
`rcodesign notary-list`, `notary-wait`, and `notary-log` inspect a submission
that was interrupted or that failed.

> The `rcodesign` commands in this section are a recipe, not a verified
> transcript. Signing requires a Developer ID certificate and notarization an App
> Store Connect key, so unlike everything else on this page they cannot be
> exercised from a credential-less build host. Their flags are quoted against
> `rcodesign` 0.29's documentation; re-check them with `rcodesign <command>
> --help` before you depend on them.

Zip the bundle **after** signing and stapling. Signing rewrites files inside the
bundle, so an archive made first ships an unsigned copy.

### The Gatekeeper reality

Signing and notarizing are not optional for public distribution, and it is worth
being precise about why. Gatekeeper evaluates a downloaded application against
the system policy database, which contains these rules among others:

```
11[Notarized Developer ID] P5 allow execute
        ... certificate leaf[...] exists and notarized
13[Unnotarized Developer ID] P0 deny execute
        ... certificate leaf[...] exists and (... timestamp >= "20190408000000Z")
```

A Developer ID signature that has *not* been notarized matches an explicit
**deny**. Signing alone does not get you there; since April 2019 the notarization
step is what moves an application from the deny rule to the allow rule.

An unsigned application is worse off still: it carries the
`com.apple.quarantine` attribute after download and is refused with a dialog
offering no obvious way past it. Users can clear it through System Settings or
`xattr -d`, but an application that requires a terminal command to launch is not
one you can hand to the public.

For a private tool, a team build, or CI, ad-hoc signing (`rcodesign sign
Demo.app` with no certificate) plus locally clearing quarantine is enough. The
moment a stranger downloads the archive, you need the certificate and the
notarization.

## Assets

An application's data files ship one of two ways.

A **loose asset directory** beside the executable is the right default. Files
stay patchable without a rebuild, a large asset does not inflate the binary or
its link time, and users can see what they installed. Everything on this page's
archive layouts assumes it. Resolve paths relative to the executable's own
location rather than the working directory, which you do not control.

Loose assets stop being the right answer when the deliverable is a **single
file** — a CLI tool people drop on their `PATH`, something distributed by copying
one binary — or when an asset is load-bearing for correctness and must not drift
out of sync with the code that reads it. Compiling those into the binary is what
`#[embed]` is for:

```mach
#[embed("../assets/logo.qoi")]
val LOGO: [_]u8;          # length taken from the file's byte count

#[embed("../assets/font.bin")]
val FONT: [65536]u8;      # length pinned; a size change fails the build
```

The bytes are placed in read-only data at compile time, exactly like any other
constant byte array — no runtime I/O, no copy, no asset path to resolve. Two
rules matter when you lay the project out. The path resolves relative to the
**declaring source file**, not the project root, hence the `../` above for a
`src/` module embedding from a project-root `assets/`. And the embedded file is
a build input whose content digest feeds incremental compilation, so editing
the asset rebuilds the embedding module — the binary cannot silently go stale
against the file it claims to contain. `[_]u8` infers the length; `[N]u8` pins
it, which is how a fixed-format asset (a boot sector, a ROM image, a header you
parse by offset) fails the build the moment it stops being that size. See
[language/decorators.md](language/decorators.md#embedstr--compile-time-file-embedding)
for the full rules.

The trade is the archive layouts above: an embedded asset cannot be patched
without a rebuild, and a large one inflates the binary and its link time. Keep
big, optional, or user-replaceable data loose; embed what the program cannot
correctly run without.

## Cross-building from one host

`--all-targets` builds every declared `[target.*]` in one invocation:

```sh
mach build . --profile release --all-targets
```

One cell per artifact × target, and three shipping binaries at the end of it:

```
$ file out/linux-x86_64/release/bin/demo
ELF 64-bit LSB executable, x86-64, version 1 (SYSV), statically linked, stripped
$ file out/windows-x86_64/release/bin/demo.exe
PE32+ executable for MS Windows 6.00 (console), x86-64, 11 sections
$ file out/darwin-x86_64/release/Demo.app/Contents/MacOS/Demo
Mach-O 64-bit x86_64 executable, flags:<NOUNDEFS>
```

For a **pure-Mach project this needs nothing installed.** Mach emits ELF, COFF,
and Mach-O itself and links each without a system linker, so there is no
cross-toolchain to acquire and no SDK to extract — a stock linux checkout
produces all three.

### Checking a cross-build

Two of those three cannot run on the machine that produced them, so verify a
cross-build by **reading its headers**, not by executing it. The questions worth
asking are: right container, right machine, right image type.

```
$ objdump -f out/windows-x86_64/release/bin/demo.exe
out/windows-x86_64/release/bin/demo.exe:     file format pei-x86-64
architecture: i386:x86-64, flags 0x0000010b:
HAS_RELOC, EXEC_P, HAS_DEBUG, D_PAGED
start address 0x0000000140001000

$ objdump -p out/windows-x86_64/release/bin/demo.exe | grep -E "^Magic|^Subsystem"
Magic			020b	(PE32+)
Subsystem		00000003	(Windows CUI)
```

```
$ llvm-objdump --macho --private-headers \
    out/darwin-x86_64/release/Demo.app/Contents/MacOS/Demo | head -4
out/darwin-x86_64/release/Demo.app/Contents/MacOS/Demo:
Mach header
      magic cputype cpusubtype  caps    filetype ncmds sizeofcmds      flags
MH_MAGIC_64  X86_64        ALL  0x00     EXECUTE     7        560   NOUNDEFS
```

`NOUNDEFS` on the Mach-O is the one worth pausing on: it says no symbol was left
unresolved at link time, so the vendored C archive really did go in and there is
nothing waiting to bind at load. The import directory (`objdump -p | grep "DLL
Name"`, above) is the PE equivalent — it enumerates exactly what the loader will
be asked for. Between them you know the binary is well-formed and complete
without ever launching it.

The Mach-O dump continues into every load command; the header is the part that
answers the question, hence the `head`. GNU `objdump` reads ELF and PE but not
Mach-O, while `llvm-objdump` reads all three, so one tool covers every target if
you prefer that.

> `mach run` and `mach test` accept `--runner <cmd>`, which hands a foreign
> binary to a host-side launcher instead of exec'ing it. That is a property of
> the environment a build is tested in, not of the build itself, and it is not
> needed to check that a cross-build is correct — see
> [cli.md](cli.md#mach-run).

### Vendored C is where it gets real

The moment a `[step]` shells out to a C compiler, that compiler — not mach — has
to produce an object in the target's format. The naive step runs the host `cc`
for every build cell and produces host ELF every time:

```
building demo-win (windows-x86_64)
error: coff: not an x86-64 object
building demo-mac (darwin-x86_64)
error: macho: not a 64-bit Mach-O object (bad magic; expected 0xFEEDFACF)
```

Every step process inherits `MACH_TARGET_ISA`, `MACH_TARGET_OS`, and
`MACH_TARGET_ABI` for exactly this reason. Branch on them and select a C target
per cell:

```toml
[step.qz]
cmd  = "vendor/build-qz.sh {project.out}/obj/qz/libqz.a"
in   = ["vendor/qz/*.c", "vendor/qz/*.h", "vendor/build-qz.sh"]
out  = ["{project.out}/obj/qz/libqz.a"]
need = []
```

```sh
#!/bin/sh
# compile the vendored C for this build cell's target and archive it.
# MACH_TARGET_ISA / MACH_TARGET_OS are exported by mach for every step.
set -eu
out="$1"
obj="${out%.a}.o"
case "$MACH_TARGET_OS" in
    linux)   triple="$MACH_TARGET_ISA-unknown-linux-gnu" ;;
    windows) triple="$MACH_TARGET_ISA-pc-windows-msvc"   ;;
    darwin)  triple="$MACH_TARGET_ISA-apple-darwin"      ;;
    *) echo "no C target configured for $MACH_TARGET_OS" >&2; exit 1 ;;
esac
mkdir -p "$(dirname "$out")"
clang --target="$triple" -c -O2 -Ivendor/qz -o "$obj" vendor/qz/qz.c
rm -f "$out"
ar rcs "$out" "$obj"
```

One `clang` covers all three object formats, so the only thing that varies per
cell is the triple — which is why it is the easy answer here; a per-target gcc
cross-toolchain works equally well. What you cannot do is skip the question.
**This is the one part of cross-building mach does not do for you**, and it
applies to every dependency that vendors C.

Note what the script above is getting away with: `qz.c` includes no system
headers, so a target triple is the whole story. C that includes system headers
needs those headers too, which is a different order of problem — a Windows SDK or
a macOS SDK, per target.

### The darwin limit: vendored C builds on macOS

**A darwin build that vendors C, or that touches any Apple framework, has to run
on a macOS machine.** This is the single most surprising constraint in planning a
release pipeline, so it is worth stating without hedging.

The macOS SDK — its headers, and the frameworks a real application links against
(Cocoa, Metal, CoreAudio) — is licensed for use on Apple hardware and is not
redistributable. You cannot lawfully stage it on a linux builder, which means no
amount of toolchain work makes this leg cross-buildable. It is not awkward, and
it is not a gap in mach waiting to be closed; it is a licensing boundary, and the
fix is a macOS runner.

What still cross-builds from linux, and what does not:

| Darwin build | Cross-buildable from linux? |
|---|---|
| pure Mach, no external link inputs | **yes** — mach emits and links Mach-O itself |
| vendored C with no system includes | yes, with a clang darwin triple (the script above) |
| vendored C that includes SDK headers | **no** — needs the macOS SDK |
| anything linking an Apple framework | **no** — needs the frameworks |

So the split is not linux-versus-macOS for the whole release; it is per leg. Mach
itself is the worked example: its CI cross-builds a darwin *seed* on an
`ubuntu-latest` runner — pure Mach, so nothing of Apple's is involved — and then
does the real fixpoint build on `macos-15` and `macos-15-intel` runners (see
`.github/workflows/darwin-lane.yml`).

Plan the pipeline accordingly. Linux and windows come off one linux host; darwin
gets its own macOS job the moment a framework or a vendored C dependency enters
the picture. Signing is a separate question and does *not* force the same
choice — `rcodesign` runs from linux, so a macOS build job can hand its bundle to
a linux signing step if that is how your CI is shaped.

### Mach-O prefixes C symbols

Mach-O decorates C symbols with a leading underscore; ELF and COFF do not. The
same C function is `qz_answer` in an ELF or COFF object and `_qz_answer` in a
Mach-O one, so one unadorned `ext fun` cannot bind on all three. Gate the
declaration:

```mach
$if ($mach.build.os == $mach.os.darwin) {
    #[symbol("_qz_answer")]
    ext fun qz_answer() i32;
} $or {
    ext fun qz_answer() i32;
}
```

The symptom when you forget is `error: undefined symbol: qz_answer` on the darwin
cell alone, while linux and windows link.

## See also

- [manifest.md](manifest.md) — `[artifact.*]` (including `subsystem`, `icon`,
  and `manifest`), `[link.*]`, and `[step.*]`
- [cli.md](cli.md) — `--all-targets`, `--runner`, and link-input resolution
- [language/ext-fun.md](language/ext-fun.md) — `ext fun`, `#[symbol]`, `#[library]`
- [language/decorators.md](language/decorators.md) — `#[embed]`
