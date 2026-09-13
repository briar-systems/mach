#!/usr/bin/env bash
# cc.sh — resolve a C compiler for the leg a case's [step] is building against.
#
# A [step] that invokes a bare `cc` assumes the host toolchain targets the
# leg being built. That holds on a leg's own native CI runner (every engines.conf
# row builds natively there) but not on a developer's cross-build from a
# different host - `mach --target linux-arm64` on an x86-64 box still runs the
# host's x86-64 `cc`, so the step silently emits an x86-64 object that gets
# linked into the leg's image. The result surfaces downstream as a relocation
# overflow or a wrong-code crash, a diagnostic that names nothing about its own
# cause (mach#2741).
#
# The fix is not "always cross-compile": several cases (narrow-stack-args,
# c-variadic) exist SPECIFICALLY to catch a leg's real toolchain diverging from
# mach's model of its ABI - Apple's arm64 stack-argument rule, for one - and
# that fact is only provable by the leg's own native compiler. Substituting a
# cross compiler there would silently stop testing what the case exists to
# test. So: use the host's own `cc` when the host already targets the leg
# (native build, including every CI runner), and fall back to a cross-capable
# `clang -target <triple>` only when it does not (a local cross-build). Either
# path only ever needs to emit an object (`-c`) - mach does the link itself -
# so the cross path needs no cross LINKER. It does need the target's libc
# HEADERS the moment a probe includes one, which is what the riscv64 branch's
# `--sysroot` supplies; see the triple table below.
#
# usage: cc.sh <compiler args...>  (same argv a bare `cc` would take)
#
# a case's [step] invokes this instead of `cc` directly and declares
# MACH_TARGET_ISA / MACH_TARGET_OS / MACH_TARGET_ABI in its environment from
# the active build cell's templates.
set -eu

: "${MACH_TARGET_ISA:?cc.sh: MACH_TARGET_ISA is not set - this must run as a mach build [step]}"
: "${MACH_TARGET_OS:?cc.sh: MACH_TARGET_OS is not set - this must run as a mach build [step]}"

# host_isa / host_os — this host's own toolchain target, normalized to the
# same vocabulary engines.conf and every case's [target.*] block use.
host_isa() {
    case "$(uname -m)" in
        x86_64|amd64)  echo x86_64 ;;
        aarch64|arm64) echo aarch64 ;;
        riscv64)       echo riscv64 ;;
        *)             echo "$(uname -m)" ;;
    esac
}
host_os() {
    case "$(uname -s)" in
        Linux)                echo linux ;;
        Darwin)                echo darwin ;;
        MINGW*|MSYS*|CYGWIN*) echo windows ;;
        *)                     echo "$(uname -s)" ;;
    esac
}

# host_cc - the host's own C compiler, by whatever name it answers to.
#
# `cc` is the POSIX spelling and every unix runner has one. WINDOWS DOES NOT: neither
# Git Bash nor the image's Visual Studio install provides that name. The candidates
# below resolve gcc or clang directly from the step's declared PATH.
#
# gcc and clang accept the same flags a bare `cc` does, so resolving to either needs no
# argv translation and no case has to know which one answered. MSVC's `cl` does not
# (`/c`, `/Fo:`), so it is NAMED IN THE FAILURE rather than half-supported: a second
# flag dialect for one call site is a permanent maintenance cost, and every candidate
# ahead of it produces the same object for this purpose.
host_cc() {
    for candidate in cc gcc clang; do
        if command -v "$candidate" >/dev/null 2>&1; then
            echo "$candidate"
            return 0
        fi
    done
    if command -v cl >/dev/null 2>&1; then
        echo "cc.sh: the only host C compiler on PATH is MSVC 'cl', whose flags are spelled differently from a bare 'cc' ('/c', '/Fo:') - this script passes a case's argv through unchanged. put a gcc or clang on the step's declared PATH." >&2
    else
        echo "cc.sh: no host C compiler on PATH (tried cc, gcc, clang). a case's [step] that compiles C needs one on every leg that runs it." >&2
    fi
    return 1
}

if [ "$(host_isa)" = "$MACH_TARGET_ISA" ] && [ "$(host_os)" = "$MACH_TARGET_OS" ]; then
    hostcc=$(host_cc) || exit 1
    exec "$hostcc" "$@"
fi

# cross build: the host cannot produce this leg's ISA/OS natively, so route
# through clang with an explicit target triple.
#
# COMPILE-ONLY NEEDS NO CROSS LINKER. It DOES need the target's libc headers as
# soon as a probe includes one, and this file used to claim otherwise - "needs no
# sysroot or cross-linker for any of these, verified against every registered
# leg (mach#2741)". That claim was verified against a set that could not disprove
# it: `c-variadic` skipped riscv64-linux, so no probe that cross-built here
# had ever included a libc header. Restoring that leg (mach#2771) reached the case
# that falsifies it, and clang resolved the HOST's /usr/include/stdint.h while
# targeting riscv64 - failing inside it on a file no case mentions
# ("bits/libc-header-start.h file not found", a multiarch host's glibc looking for
# its own arch subdirectory). Silently succeeding would have been worse: an object
# compiled against the wrong architecture's libc headers, which is the same class
# of quiet divergence mach#2741 named. So the riscv64 branch names a sysroot, and
# no branch pretends the headers are free.
#
# extend this table exactly when engines.conf gains a leg (same axis
# check-target-matrix.sh already enforces one block per [target.*] on).
triple=
extra=
sysroot_arg=
case "$MACH_TARGET_OS-$MACH_TARGET_ISA" in
    linux-x86_64)    triple=x86_64-linux-gnu ;;
    linux-aarch64)   triple=aarch64-linux-gnu ;;
    # riscv64 needs five flags to keep clang's output inside what mach's ELF
    # reader consumes and matching the ABI mach's own riscv64 backend implements,
    # none of which lose anything these case probes need:
    #   -march=rv64gc -mabi=lp64d     PINS the architecture profile AND the hard-
    #                                  float calling convention mach's lp64.mach
    #                                  implements (float/double args in the fa
    #                                  registers). the bare `riscv64-linux-gnu`
    #                                  triple's DEFAULT for both is a clang-version /
    #                                  packaging fact, not a stable one: -mabi=lp64d
    #                                  alone (the first attempt here) still failed in
    #                                  CI, on an older clang (18) whose default
    #                                  -march for the bare triple does not carry the
    #                                  D extension, so -mabi=lp64d silently produced
    #                                  a soft-float-shaped object rather than an
    #                                  error - the SAME toolchain-default hazard as
    #                                  the original -mabi gap, one knob over, found
    #                                  the same way: passed on this developer host's
    #                                  newer clang (whose default -march already
    #                                  carries +d) and failed in CI (mach#2771's own
    #                                  first CI run after that PR was already open -
    #                                  narrow-stack-args's variadic-free floats still
    #                                  read back as flat zero). pinning both removes
    #                                  the toolchain dependence outright rather than
    #                                  hoping every clang defaults the same way.
    #   -mno-relax                    clang's default codegen emits R_RISCV_RELAX
    #                                  (type 51) hints for the linker's optional
    #                                  relaxation pass; mach has no such pass and
    #                                  rejects the type outright.
    #   -fno-asynchronous-unwind-tables / -fno-unwind-tables
    #                                  drops .eh_frame, whose length deltas clang
    #                                  encodes as R_RISCV_ADD32/SUB32 pairs; mach's
    #                                  reader does not fold arithmetic relocation
    #                                  pairs, and nothing here throws or unwinds.
    #   -fno-jump-tables               a switch-derived jump table in .rodata
    #                                  hits the exact same ADD32/SUB32 pairing.
    # suppressing emission at the compiler is the fix, not teaching mach's reader
    # relocation arithmetic it will never otherwise need (mach#2741).
    #
    # ...and a SYSROOT, because this is the one leg CI cross-builds: linux-riscv64
    # is the only engines.conf row whose runner is not its own ISA, so it is the
    # only leg whose probes need target libc headers in CI. `--sysroot` alone is
    # sufficient and is what makes the resolution honest - clang finds
    # <sysroot>/include and stops searching the host's /usr/include, so a probe
    # either compiles against riscv64's own headers or fails naming the one it
    # wanted. The path is where Ubuntu's `libc6-dev-riscv64-cross` installs, which
    # ci.yml installs on this leg. Absent, the build stops here naming the package
    # rather than falling back to the host's headers, which is exactly
    # the silent wrong-headers outcome this exists to prevent.
    linux-riscv64)
        triple=riscv64-linux-gnu
        sysroot=/usr/riscv64-linux-gnu
        if [ ! -d "$sysroot/include" ]; then
            echo "cc.sh: cross-building linux-riscv64 needs riscv64 libc headers, and none are at '$sysroot/include' - install Ubuntu's libc6-dev-riscv64-cross. compiling against the host's headers instead would emit an object built to the wrong architecture's libc (mach#2741, mach#2771)" >&2
            exit 1
        fi
        extra="-march=rv64gc -mabi=lp64d -mno-relax -fno-asynchronous-unwind-tables -fno-unwind-tables -fno-jump-tables"
        sysroot_arg=$sysroot
        ;;
    windows-x86_64)  triple=x86_64-pc-windows-msvc ;;
    darwin-x86_64)   triple=x86_64-apple-darwin ;;
    darwin-aarch64)  triple=arm64-apple-darwin ;;
    *)
        echo "cc.sh: no cross-compile triple known for $MACH_TARGET_OS/$MACH_TARGET_ISA (host is $(host_os)/$(host_isa)); add one to test/link/lib/cc.sh's triple table" >&2
        exit 1
        ;;
esac

if ! command -v clang >/dev/null 2>&1; then
    echo "cc.sh: host is $(host_os)/$(host_isa), leg needs $MACH_TARGET_OS/$MACH_TARGET_ISA, and no cross toolchain is available - clang (for '-target $triple') is not installed" >&2
    exit 1
fi

# $extra is deliberately unquoted: it is always either empty or a fixed set of
# single-word flags built above, never user input, so word-splitting it here is
# the intended expansion, not a quoting bug. the sysroot rides its own quoted
# expansion and is present only for a branch that set one.
clang -target "$triple" ${sysroot_arg:+--sysroot="$sysroot_arg"} $extra "$@" || exit $?

# the -mabi/-march pin above is a REQUEST, and a toolchain that does not honour it
# fails silently in the direction that looks like a mach bug: a soft-float probe
# linked against mach's hard-float code reads every float argument back as zero,
# which is indistinguishable from a broken classifier. that exact confusion cost
# real time on mach#2771 twice, once per knob. so verify the produced object
# rather than trusting the flag, and name the toolchain when it disagrees.
#
# riscv64 records the float ABI in the ELF header's e_flags (offset 0x30, 4 bytes
# LE): mask 0x6 is 0 soft, 2 single, 4 double. mach's lp64.mach implements lp64d,
# so anything but 4 is a mismatch this suite cannot paper over.
if [ "$MACH_TARGET_OS-$MACH_TARGET_ISA" = "linux-riscv64" ]; then
    out=
    prev=
    for arg in "$@"; do
        [ "$prev" = "-o" ] && out=$arg
        prev=$arg
    done
    if [ -n "$out" ] && [ -f "$out" ] && [ "$(head -c 4 "$out" | od -An -tx1 | tr -d ' \n')" = "7f454c46" ]; then
        flags=$(od -An -tu4 -j 48 -N 4 "$out" | tr -d ' \n')
        abi=$((flags & 6))
        if [ "$abi" != "4" ]; then
            case "$abi" in
                0) got="soft-float (lp64)" ;;
                2) got="single-float (lp64f)" ;;
                *) got="unknown ($abi)" ;;
            esac
            echo "cc.sh: $(clang --version | head -1) produced a $got object for '$out' despite -mabi=lp64d -march=rv64gc; mach's riscv64 backend implements lp64d, so every float argument across this boundary would read back as zero. pin a toolchain that honours the flags rather than treating the resulting zeros as a mach defect (mach#2771, mach#2777)" >&2
            exit 1
        fi
    fi
fi
