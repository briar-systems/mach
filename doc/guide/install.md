# Install

Mach is one `mach` binary with no other toolchain to install first. Take the latest release with the install script or as an archive, or build it from source.

## Install script

On Linux and macOS:

```sh
curl -fsSL https://machlang.org/install.sh | sh
```

On Windows, in PowerShell:

```sh
irm https://machlang.org/install.ps1 | iex
```

Both forward to the installer in the [mach repository](../../dist). It downloads the latest release for your host, checks the archive against the release's `SHA256SUMS`, and installs nothing on a mismatch. At a terminal it asks for the install directory.

| Host | Default directory |
|---|---|
| Linux, macOS | `~/.local/bin` |
| Windows | `%LOCALAPPDATA%\mach\bin` |

- `MACH_INSTALL_DIR` names the directory and skips the question.
- `MACH_VERSION` picks a release other than the latest.

The installer does not change `PATH`, so add the directory yourself.

## Precompiled binaries

Every [release](https://github.com/briar-systems/mach/releases) carries an archive for Linux, macOS and Windows, each on x86_64 and aarch64. Unpack yours and put `mach` on `PATH`.

## Building from source

Mach is written in Mach, so building it needs an installed `mach`.

```sh
git clone https://github.com/briar-systems/mach.git
cd mach
mach dep pull .
mach build . -o a
./a build .
```

The installed compiler builds `a`, and `a` builds the compiler to `out/<target>/<profile>/bin/mach`, such as `out/linux-x86_64/debug/bin/mach`.

## Checking the install

```sh
mach info
```

It prints the compiler's version and the host it builds for. `mach info targets` lists every target it can build.
