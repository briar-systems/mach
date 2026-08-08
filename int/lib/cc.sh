#!/usr/bin/env bash
# cc.sh — resolve a C compiler for the leg a case's [step] is building against.
#
# A [step] that shells out to a bare `cc` assumes the host toolchain targets the
# leg being built. That holds on a leg's own native CI runner (every targets.conf
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
# so `clang -target` needs no sysroot or cross-linker to be sufficient.
#
# usage: cc.sh <compiler args...>  (same argv a bare `cc` would take)
#
# a case's [step] invokes this instead of `cc` directly; MACH_TARGET_ISA /
# MACH_TARGET_OS / MACH_TARGET_ABI are already injected into every step's
# environment for the active build cell (#1964), which is what makes this
# possible without threading the target through the step's own cmd string.
set -eu

: "${MACH_TARGET_ISA:?cc.sh: MACH_TARGET_ISA is not set - this must run as a mach build [step]}"
: "${MACH_TARGET_OS:?cc.sh: MACH_TARGET_OS is not set - this must run as a mach build [step]}"

# host_isa / host_os — this host's own toolchain target, normalized to the
# same vocabulary targets.conf and every case's [target.*] block use.
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

if [ "$(host_isa)" = "$MACH_TARGET_ISA" ] && [ "$(host_os)" = "$MACH_TARGET_OS" ]; then
    exec "${CC:-cc}" "$@"
fi

# cross build: the host cannot produce this leg's ISA/OS natively, so route
# through clang with an explicit target triple. compile-only (-c) needs no
# sysroot or cross-linker for any of these, verified against every
# targets.conf leg (mach#2741) - only a case that asked cc.sh to LINK a full
# binary would need one, and none does today.
#
# extend this table exactly when targets.conf gains a leg (same axis
# check-target-matrix.sh already enforces one block per [target.*] on).
triple=
extra=
case "$MACH_TARGET_OS-$MACH_TARGET_ISA" in
    linux-x86_64)    triple=x86_64-linux-gnu ;;
    linux-aarch64)   triple=aarch64-linux-gnu ;;
    # riscv64 needs three flags to keep clang's output inside what mach's ELF
    # reader consumes, none of which lose anything these int-case probes need:
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
    linux-riscv64)
        triple=riscv64-linux-gnu
        extra="-mno-relax -fno-asynchronous-unwind-tables -fno-unwind-tables -fno-jump-tables"
        ;;
    windows-x86_64)  triple=x86_64-pc-windows-msvc ;;
    darwin-x86_64)   triple=x86_64-apple-darwin ;;
    darwin-aarch64)  triple=arm64-apple-darwin ;;
    *)
        echo "cc.sh: no cross-compile triple known for $MACH_TARGET_OS/$MACH_TARGET_ISA (host is $(host_os)/$(host_isa)); add one to int/lib/cc.sh's triple table" >&2
        exit 1
        ;;
esac

if ! command -v clang >/dev/null 2>&1; then
    echo "cc.sh: host is $(host_os)/$(host_isa), leg needs $MACH_TARGET_OS/$MACH_TARGET_ISA, and no cross toolchain is available - clang (for '-target $triple') is not installed" >&2
    exit 1
fi

# $extra is deliberately unquoted: it is always either empty or a fixed set of
# single-word flags built above, never user input, so word-splitting it here is
# the intended expansion, not a quoting bug.
exec clang -target "$triple" $extra "$@"
