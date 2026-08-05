# produce.sh — observable producers, sourced by run.sh.
#
# a producer turns a built case artifact into a normalized text observable on
# stdout, which run.sh diffs against the golden. the producer is the only thing
# that varies between verification modes; the golden-diff core does not change.
#
# producers:
#   exec        — run the program, observe its stdout (native / qemu).
#   relro-fault — run the program and report whether its write to a RELRO'd .rodata
#                 slot faulted (SIGSEGV -> exit 139); the --pie RELRO runtime guard.
#   field       — coreutils (od/dd) reads of known header offsets, for format facts
#                 execution cannot observe (PE ASLR bit, macho PIE bit). dispatched
#                 on the artifact's own magic, so it is independent of how the case
#                 maps to a leg. no LLVM; reads little-endian fields (every runner is LE).
#   macho-signed— structurally resolve the three x86_64 immediate-store displacements
#                 and compare them with independent initialized-data markers, proving
#                 SIGNED_1/_2/_4 each linked to its exact target without a mac runner.
#   pe-imports  — like field, but walks the PE import directory and reports every
#                 `<dll>:<symbol>` binding (#2510). PE-only; the observable for
#                 two-level-namespace attribution, which execution cannot check.
#   pe-exceptions — walks the PE exception directory and verifies the linked
#                 clang fixture's runtime-function row and foreign UNWIND_INFO.
#   pe-codeview — verifies real clang CodeView SECREL+SECTION pairs name the
#                 exact final PE section and include its first-section prefix.
#   pe-dllimport — verifies a real Clang `__imp_X` rel32 targets X's IAT slot,
#                 with one undecorated import shared by a direct X call.
#   pe-local-import — verifies duplicate real Clang `__imp_X` references target
#                 one local pointer cell when X is defined in the same image,
#                 with no loader import and one DIR64 ASLR base relocation.
#   pe-resources — independently walks the PE resource tree and validates icon,
#                 version, manifest, metadata, and initialized-data accounting.
#   relro       — like field, but walks the ELF program headers for a PT_GNU_RELRO
#                 (the static-PIE RELRO region). ELF-only; used by the elf-relro guard.
#   flat-loader — load an os=freestanding, of=raw flat image via a tiny C loader
#                 (mmap + jump) and report the image's exit status.
#   built       — build-only: assert the tuple composed and emitted an artifact,
#                 for a cross-built target with no host runner (a freestanding
#                 aarch64/riscv64 image on the x86_64 leg). the observable is a
#                 constant, so the golden is the fact "it emitted".
#   panic-exit  — run a binary EXPECTED to call std.system.panic and report its
#                 stderr message plus its exit status, distinguishing a deliberate
#                 termination from a signal death (#2369). unlike relro-fault, the
#                 message is part of the observable: printing it and then faulting
#                 is exactly the regression this guards.
#   debuginfo   — build the case with and without `-g` (run.sh builds both) and
#                 assert over the artifacts: llvm-dwarfdump --verify accepts the `-g`
#                 image, `-g` is loadable-byte additive, discarded weak templates do
#                 not alias live line/location metadata, and live names symbolize.
#                 requires llvm-dwarfdump, llvm-symbolizer, and readelf; it runs only
#                 on the ELF debug-info legs, which install them.
#   spirv-val   — validate every `.spv` module a finished-module target delivered
#                 with the Khronos validator (spirv-tools). the target links
#                 nothing, so there is no binary to run and the module tree is the
#                 artifact; the external validator is what makes this a validity
#                 check rather than a round-trip through mach's own reader.
#   vector-emit — disassemble the case's own objects and report, per function,
#                 whether the compiler EMITTED packed SIMD (#2207). the observable
#                 execution cannot produce: a vectorizer that silently stops firing
#                 still computes the right answer, so a run-and-compare case stays
#                 green while the feature is dead. needs llvm-objdump.
#   vector-lanes— disassemble the case's own objects and report, per function, whether
#                 a vector LANE was accessed in a register and whether a whole 128-bit
#                 vector went to memory (#2236). the same blind spot as vector-emit:
#                 lane access through a stack round-trip computes the right answer, so
#                 only the emitted form distinguishes it. needs llvm-objdump.
#   float-emit  — disassemble the case's own objects and report, per function, how
#                 many emitted instructions name the back end's reserved FP scratch
#                 registers (#2237). the same argument as vector-emit: an encoder
#                 that stages every allocated float operand through scratch computes
#                 the right answer while emitting twice the instructions, so only a
#                 shape observable can see it. needs llvm-objdump.
#
# build-fails is a run-mode but not a producer: it asserts the compile is REJECTED
# and takes the compiler's 'error:' diagnostic as the observable. it is handled in
# run.sh (there is no artifact to run), noted here for discoverability.

# the directory this file lives in (int/lib), used to find flat_loader.c. resolved
# from the sourced path so it does not depend on run.sh's variables.
_produce_lib_dir=$(CDPATH= cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
_flat_loader_bin=

# qemu_bin <target> — the qemu-user interpreter for a harness target, keyed by ISA
# rather than sliced from the name. linux-riscv64's name suffix happens to match its
# qemu-user binary (qemu-riscv64), but linux-arm64's does not (qemu-aarch64, never
# qemu-arm64), and the bare `linux` leg carries no ISA suffix at all. keep in sync
# with the ISA each int/*/mach.toml's `[target.<leg>]` declares when a targets.conf
# row is added.
#
# ELF ONLY. qemu-user's loader understands the Linux ELF ABI and nothing else - a
# PE or Mach-O artifact can never load under it, on any host, no matter which
# qemu-<arch> binary is named. A target whose `[target.<leg>].os` is not `linux`
# has no qemu interpreter and never will (#2453, found by executing qemu-x86_64
# against a real windows PE artifact and qemu-aarch64 against a real darwin-aarch64
# Mach-O one: both fail `Exec format error` unconditionally). Naming that here
# explicitly is what keeps the next target addition from reintroducing a mapping
# that looks alive and cannot work - the same shape #2314 found for linux-arm64,
# except that one was dead-but-workable and these are dead-permanently.
qemu_bin() {
    case "$1" in
        linux)         echo qemu-x86_64 ;;
        linux-arm64)   echo qemu-aarch64 ;;
        linux-riscv64) echo qemu-riscv64 ;;
        windows|darwin-x86_64|darwin-aarch64)
            echo "int: '$1' is not linux - qemu-user loads ELF only, so a PE or Mach-O target has no qemu interpreter (#2453)" >&2
            return 1 ;;
        *) echo "int: no qemu interpreter mapping for target '$1'" >&2; return 1 ;;
    esac
}

# produce_exec <runmode> <target> <binary>
# runs the built binary and forwards its stdout as the observable. native mode runs
# it directly; qemu mode runs it under the matching qemu-user (qemu_bin). the
# producer's exit status is the program's, so a crash (nonzero) fails the case.
produce_exec() {
    runmode=$1
    target=$2
    bin=$3
    if [ "$runmode" = "qemu" ]; then
        interp=$(qemu_bin "$target") || return 1
        "$interp" "$bin"
    else
        "$bin"
    fi
}

# produce_relro_fault <runmode> <target> <binary>
# runs the built binary (expected to write to a relocated constant's RELRO'd .rodata
# storage) and reports whether that write faulted. after the --pie startup mprotects
# the region read-only, the write must raise SIGSEGV, which surfaces as exit 128+11=139
# both natively and under qemu-user; any other status means the region stayed writable.
# the program's own stdout is discarded - the observable is purely the fault fact - so
# this is a runtime (exec-like) producer sharing one target-independent golden.
produce_relro_fault() {
    runmode=$1
    target=$2
    bin=$3
    if [ "$runmode" = "qemu" ]; then
        interp=$(qemu_bin "$target") || return 1
        "$interp" "$bin" >/dev/null 2>&1
    else
        "$bin" >/dev/null 2>&1
    fi
    ec=$?
    if [ "$ec" -eq 139 ]; then
        echo "relro_write=faulted"
    else
        echo "relro_write=exit$ec"
    fi
}

# produce_panic_exit <runmode> <target> <binary>
# runs a binary EXPECTED to call std.system.panic and reports its stderr message
# followed by its exit status, distinguishing a deliberate PANIC_EXIT from a signal
# death (#2369): panic used to write its message and then execute a trap
# instruction with no exit syscall, which faulted (SIGSEGV, exit 139 on x86_64;
# SIGTRAP, exit 133, on aarch64 / riscv64) and made a correctly detected internal
# error indistinguishable from memory corruption. stdout+stderr are captured to a
# file rather than a command substitution, both to avoid stripping the message's
# own trailing newline (`$()` strips all of them) and to keep the exit-status read
# on the line directly after the command, matching produce_relro_fault - the
# proven-safe shape under this harness's `set -e`.
#
# a signal death is 128 + N with N in 1..64 (POSIX real-time signals cap there); the
# fact is reported as `signal(<n>)` rather than the raw number so a golden reviewer
# does not have to recompute N to see what regressed.
produce_panic_exit() {
    runmode=$1
    target=$2
    bin=$3
    out=$(mktemp)
    if [ "$runmode" = "qemu" ]; then
        interp=$(qemu_bin "$target") || { rm -f "$out"; return 1; }
        "$interp" "$bin" >"$out" 2>&1
    else
        "$bin" >"$out" 2>&1
    fi
    ec=$?
    cat "$out"
    rm -f "$out"
    if [ "$ec" -ge 129 ] && [ "$ec" -le 192 ]; then
        echo "exit=signal($((ec - 128)))"
    else
        echo "exit=$ec"
    fi
}

# read_le_uint <file> <offset> <size>
# print the unsigned little-endian integer of <size> bytes (1, 2, 4, or 8) at <offset>.
# od reads in host byte order; every CI runner is little-endian.
read_le_uint() {
    dd if="$1" bs=1 skip="$2" count="$3" 2>/dev/null | od -An -tu"$3" | tr -d ' \n'
}

# field_pe <binary> — the PE ASLR fact. DllCharacteristics is a u16 in the optional
# header (at e_lfanew + 4 PE-sig + 20 COFF + 0x46 = e_lfanew + 0x5e); the
# DYNAMIC_BASE bit (0x40) is IMAGE_DLLCHARACTERISTICS_DYNAMIC_BASE.
field_pe() {
    bin=$1
    elfanew=$(read_le_uint "$bin" 60 4)
    dllchar=$(read_le_uint "$bin" $((elfanew + 0x5e)) 2)
    echo "DYNAMIC_BASE=$(( (dllchar & 0x40) != 0 ))"
}

# field_macho <binary> — the macho PIE fact. the mach_header's flags is a u32 at
# offset 24 (after magic/cputype/cpusubtype/filetype/ncmds/sizeofcmds); MH_PIE is
# 0x200000.
field_macho() {
    bin=$1
    flags=$(read_le_uint "$bin" 24 4)
    echo "PIE=$(( (flags & 0x200000) != 0 ))"
}

# macho_segment_fields <file> <segname> — print a segment's
# "vmaddr vmsize fileoff filesize" tuple from LC_SEGMENT_64, or fail when absent.
# The executable writer emits section-header-less load segments, so the load
# commands are the independent file-offset-to-VA map structural checks must use.
macho_segment_fields() {
    bin=$1
    want=$2
    ncmds=$(read_le_uint "$bin" 16 4)
    off=32
    i=0
    while [ "$i" -lt "$ncmds" ]; do
        cmd=$(read_le_uint "$bin" "$off" 4)
        cmdsize=$(read_le_uint "$bin" $((off + 4)) 4)
        if [ "$cmd" -eq 25 ]; then
            name=$(dd if="$bin" bs=1 skip=$((off + 8)) count=16 2>/dev/null | tr '\0' '\n' | head -n 1)
            if [ "$name" = "$want" ]; then
                vmaddr=$(read_le_uint "$bin" $((off + 24)) 8)
                vmsize=$(read_le_uint "$bin" $((off + 32)) 8)
                fileoff=$(read_le_uint "$bin" $((off + 40)) 8)
                filesize=$(read_le_uint "$bin" $((off + 48)) 8)
                printf '%s %s %s %s\n' "$vmaddr" "$vmsize" "$fileoff" "$filesize"
                return 0
            fi
        fi
        [ "$cmdsize" -ge 8 ] || return 2
        off=$((off + cmdsize))
        i=$((i + 1))
    done
    echo "int: macho-signed: segment '$want' not found" >&2
    return 2
}

# produce_macho_signed <runmode> <target> <binary>
# Locate the checked-in clang fixture's initialized target bytes in __DATA, then
# decode the linked disp32 fields of its byte/word/dword immediate stores in
# __TEXT. The instruction-tail constants (7, 9, 10 bytes from instruction start)
# independently embody SIGNED_1/_2/_4's P+4+N convention. Exact equality with
# the three marker VAs proves more than a successful cross-link: every relocation
# selected the intended target byte.
produce_macho_signed() {
    bin=$3
    text_fields=$(macho_segment_fields "$bin" __TEXT) || return 2
    set -- $text_fields
    text_vm=$1; text_file=$3; text_size=$4
    data_fields=$(macho_segment_fields "$bin" __DATA) || return 2
    set -- $data_fields
    data_vm=$1; data_file=$3; data_size=$4

    marker_file=$(od -An -v -tu1 -j "$data_file" -N "$data_size" "$bin" | awk -v base="$data_file" '
        { for (i = 1; i <= NF; i++) b[++n] = $i }
        END {
            found = 0
            for (i = 1; i + 7 <= n; i++) {
                if (b[i] == 165 && b[i+1] == 0 && b[i+2] == 239 && b[i+3] == 190 &&
                    b[i+4] == 190 && b[i+5] == 186 && b[i+6] == 254 && b[i+7] == 202) {
                    found++; pos = base + i - 1
                }
            }
            if (found != 1) {
                print "int: macho-signed: expected one initialized target marker, found " found > "/dev/stderr"
                exit 2
            }
            printf "%.0f\n", pos
        }
    ') || return 2
    target1=$((data_vm + marker_file - data_file))
    target2=$((target1 + 2))
    target4=$((target1 + 4))

    stores=$(od -An -v -tu1 -j "$text_file" -N "$text_size" "$bin" | awk -v vm="$text_vm" '
        function disp32(i, u) {
            u = b[i] + 256*b[i+1] + 65536*b[i+2] + 16777216*b[i+3]
            return u >= 2147483648 ? u - 4294967296 : u
        }
        { for (i = 1; i <= NF; i++) b[++n] = $i }
        END {
            n1 = n2 = n4 = 0
            for (i = 1; i <= n; i++) {
                insn = vm + i - 1
                if (i + 6 <= n && b[i] == 198 && b[i+1] == 5 && b[i+6] == 1) {
                    n1++; v1 = insn + 7 + disp32(i + 2)
                }
                if (i + 8 <= n && b[i] == 102 && b[i+1] == 199 && b[i+2] == 5 && b[i+7] == 52 && b[i+8] == 18) {
                    n2++; v2 = insn + 9 + disp32(i + 3)
                }
                if (i + 9 <= n && b[i] == 199 && b[i+1] == 5 && b[i+6] == 120 && b[i+7] == 86 && b[i+8] == 52 && b[i+9] == 18) {
                    n4++; v4 = insn + 10 + disp32(i + 2)
                }
            }
            if (n1 != 1 || n2 != 1 || n4 != 1) {
                print "int: macho-signed: expected one store of each width, found " n1 "/" n2 "/" n4 > "/dev/stderr"
                exit 2
            }
            printf "SIGNED_1 %.0f SIGNED_2 %.0f SIGNED_4 %.0f\n", v1, v2, v4
        }
    ') || return 2
    set -- $stores
    [ "$1" = SIGNED_1 ] && [ "$3" = SIGNED_2 ] && [ "$5" = SIGNED_4 ] || return 2

    [ "$2" -eq "$target1" ] || { echo "int: macho-signed: SIGNED_1 resolved to $2, expected $target1" >&2; return 1; }
    [ "$4" -eq "$target2" ] || { echo "int: macho-signed: SIGNED_2 resolved to $4, expected $target2" >&2; return 1; }
    [ "$6" -eq "$target4" ] || { echo "int: macho-signed: SIGNED_4 resolved to $6, expected $target4" >&2; return 1; }
    echo "SIGNED_1=exact"
    echo "SIGNED_2=exact"
    echo "SIGNED_4=exact"
}

# field_elf <binary> — the ELF position-independence fact. e_type is a u16 at offset
# 16; ET_DYN (3) is a position-independent (PIE) executable, ET_EXEC (2) a
# fixed-address one.
field_elf() {
    bin=$1
    etype=$(read_le_uint "$bin" 16 2)
    echo "e_type=$etype"
}

# read_cstr <file> <offset> — print the NUL-terminated string at <offset>. splitting
# the byte window on NUL and taking the first record ends the string exactly where
# the format does; a name longer than the window is not a case this suite writes.
read_cstr() {
    dd if="$1" bs=1 skip="$2" count=256 2>/dev/null | tr '\0' '\n' | head -n 1
}

# pe_rva_to_off <file> <sec_table_off> <nsec> <rva>
# map an image RVA to a file offset through the section table. a section covers
# [VirtualAddress, VirtualAddress + max(VirtualSize, SizeOfRawData)); the raw
# window is the larger bound because a section whose VirtualSize rounds below its
# raw size still owns those bytes on disk.
pe_rva_to_off() {
    bin=$1; sec=$2; nsec=$3; rva=$4
    i=0
    while [ "$i" -lt "$nsec" ]; do
        base=$((sec + i * 40))
        vaddr=$(read_le_uint "$bin" $((base + 12)) 4)
        vsize=$(read_le_uint "$bin" $((base + 8)) 4)
        rsize=$(read_le_uint "$bin" $((base + 16)) 4)
        praw=$(read_le_uint "$bin" $((base + 20)) 4)
        span=$vsize
        [ "$rsize" -gt "$span" ] && span=$rsize
        if [ "$rva" -ge "$vaddr" ] && [ "$rva" -lt $((vaddr + span)) ]; then
            echo $((praw + rva - vaddr))
            return 0
        fi
        i=$((i + 1))
    done
    return 1
}

# pe_off_to_rva <file> <sec_table_off> <nsec> <offset>
# inverse of pe_rva_to_off for bytes backed by a section's raw-data window.
pe_off_to_rva() {
    bin=$1; sec=$2; nsec=$3; off=$4
    i=0
    while [ "$i" -lt "$nsec" ]; do
        base=$((sec + i * 40))
        vaddr=$(read_le_uint "$bin" $((base + 12)) 4)
        rsize=$(read_le_uint "$bin" $((base + 16)) 4)
        praw=$(read_le_uint "$bin" $((base + 20)) 4)
        if [ "$off" -ge "$praw" ] && [ "$off" -lt $((praw + rsize)) ]; then
            echo $((vaddr + off - praw))
            return 0
        fi
        i=$((i + 1))
    done
    return 1
}

# count exact byte-sequence occurrences in a file from lowercase hex
hex_count() {
    od -An -v -tx1 "$1" | awk -v want="$2" '
        BEGIN { width = length(want) }
        { for (i = 1; i <= NF; i++) {
            tail = tail $i
            if (length(tail) > width) tail = substr(tail, length(tail) - width + 1)
            if (tail == want) count++
        } }
        END { print count + 0 }
    '
}

# pe_resource_leaf <file> <section-table> <nsec> <resource-base-off> <type> <name>
# print "payload-file-offset size" for language id 0, or fail when absent.
pe_resource_leaf() {
    bin=$1; sec=$2; nsec=$3; root=$4; want_type=$5; want_name=$6
    type_count=$(read_le_uint "$bin" $((root + 14)) 2)
    ti=0
    while [ "$ti" -lt "$type_count" ]; do
        te=$((root + 16 + ti * 8))
        tid=$(read_le_uint "$bin" "$te" 4)
        if [ "$tid" -eq "$want_type" ]; then
            tchild=$(read_le_uint "$bin" $((te + 4)) 4)
            [ $((tchild & 0x80000000)) -ne 0 ] || return 1
            l2=$((root + (tchild & 0x7fffffff)))
            name_count=$(read_le_uint "$bin" $((l2 + 14)) 2)
            ni=0
            while [ "$ni" -lt "$name_count" ]; do
                ne=$((l2 + 16 + ni * 8))
                nid=$(read_le_uint "$bin" "$ne" 4)
                if [ "$nid" -eq "$want_name" ]; then
                    nchild=$(read_le_uint "$bin" $((ne + 4)) 4)
                    [ $((nchild & 0x80000000)) -ne 0 ] || return 1
                    l3=$((root + (nchild & 0x7fffffff)))
                    lang_count=$(read_le_uint "$bin" $((l3 + 14)) 2)
                    [ "$lang_count" -gt 0 ] || return 1
                    le=$((l3 + 16))
                    [ "$(read_le_uint "$bin" "$le" 4)" -eq 0 ] || return 1
                    de_rel=$(read_le_uint "$bin" $((le + 4)) 4)
                    [ $((de_rel & 0x80000000)) -eq 0 ] || return 1
                    de=$((root + de_rel))
                    data_rva=$(read_le_uint "$bin" "$de" 4)
                    data_size=$(read_le_uint "$bin" $((de + 4)) 4)
                    data_off=$(pe_rva_to_off "$bin" "$sec" "$nsec" "$data_rva") || return 1
                    echo "$data_off $data_size"
                    return 0
                fi
                ni=$((ni + 1))
            done
        fi
        ti=$((ti + 1))
    done
    return 1
}

# independently validate the PE resources produced from the fixture manifest
produce_pe_resources() {
    bin=$3
    elfanew=$(read_le_uint "$bin" 60 4)
    nsec=$(read_le_uint "$bin" $((elfanew + 6)) 2)
    optsize=$(read_le_uint "$bin" $((elfanew + 20)) 2)
    oh=$((elfanew + 24))
    sec=$((oh + optsize))
    [ "$(read_le_uint "$bin" "$oh" 2)" -eq 523 ] || return 2
    resource_rva=$(read_le_uint "$bin" $((oh + 112 + 2 * 8)) 4)
    resource_size=$(read_le_uint "$bin" $((oh + 112 + 2 * 8 + 4)) 4)
    [ "$resource_rva" -gt 0 ] && [ "$resource_size" -gt 0 ] || return 2
    root=$(pe_rva_to_off "$bin" "$sec" "$nsec" "$resource_rva") || return 2

    set -- $(pe_resource_leaf "$bin" "$sec" "$nsec" "$root" 3 1) || return 2
    icon_off=$1; icon_size=$2
    [ "$icon_size" -eq 4 ] || return 2
    [ "$(read_le_uint "$bin" "$icon_off" 4)" -eq 1144201745 ] || return 2

    set -- $(pe_resource_leaf "$bin" "$sec" "$nsec" "$root" 14 1) || return 2
    group_off=$1; group_size=$2
    [ "$group_size" -eq 20 ] || return 2
    [ "$(read_le_uint "$bin" $((group_off + 4)) 2)" -eq 1 ] || return 2
    [ "$(read_le_uint "$bin" $((group_off + 18)) 2)" -eq 1 ] || return 2

    set -- $(pe_resource_leaf "$bin" "$sec" "$nsec" "$root" 16 1) || return 2
    version_off=$1
    [ "$(read_le_uint "$bin" $((version_off + 40)) 4)" -eq 4277077181 ] || return 2
    [ "$(read_le_uint "$bin" $((version_off + 48)) 4)" -eq 196612 ] || return 2
    [ "$(read_le_uint "$bin" $((version_off + 52)) 4)" -eq 327680 ] || return 2

    set -- $(pe_resource_leaf "$bin" "$sec" "$nsec" "$root" 24 1) || return 2
    manifest_off=$1; manifest_size=$2
    case_dir=$(CDPATH= cd -- "$(dirname -- "$bin")/../.." && pwd)
    _pr_tmp=$(mktemp)
    dd if="$bin" of="$_pr_tmp" bs=1 skip="$manifest_off" count="$manifest_size" 2>/dev/null
    cmp -s "$_pr_tmp" "$case_dir/assets/app.manifest" || { rm -f "$_pr_tmp"; return 2; }
    rm -f "$_pr_tmp"

    [ "$(hex_count "$bin" 670061006d0065002d0061007000700000)" -eq 2 ] || return 2
    [ "$(hex_count "$bin" 700072006f0067000000)" -eq 1 ] || return 2
    [ "$(hex_count "$bin" 33002e0034002e0035000000)" -eq 2 ] || return 2
    [ "$(hex_count "$bin" 460069006c0065004400650073006300720069007000740069006f006e000000)" -eq 0 ] || return 2

    initialized=0
    i=0
    while [ "$i" -lt "$nsec" ]; do
        sh=$((sec + i * 40))
        chars=$(read_le_uint "$bin" $((sh + 36)) 4)
        if [ $((chars & 0x40)) -ne 0 ]; then
            initialized=$((initialized + $(read_le_uint "$bin" $((sh + 16)) 4)))
        fi
        i=$((i + 1))
    done
    [ "$(read_le_uint "$bin" $((oh + 8)) 4)" -eq "$initialized" ] || return 2

    echo "icon=present"
    echo "group_icon=present"
    echo "version=3.4.5"
    echo "manifest=preserved"
    echo "metadata=game-app/prog"
    echo "initialized_data=complete"
}

# find_unique_hex <file> <lowercase-hex-bytes>
# print the byte offset when the exact byte sequence occurs once in the file.
find_unique_hex() {
    bin=$1; want=$2
    od -An -v -tx1 "$bin" | awk -v want="$want" '
        BEGIN { width = length(want); bytes = width / 2 }
        {
            for (i = 1; i <= NF; i++) {
                tail = tail $i
                if (length(tail) > width) tail = substr(tail, length(tail) - width + 1)
                if (tail == want) { count++; found = n - bytes + 1 }
                n++
            }
        }
        END { if (count == 1) print found; else exit 1 }
    '
}

# produce_pe_imports <runmode> <target> <binary>
# emit the PE import table's symbol->DLL bindings, one `<dll>:<symbol>` line per
# imported function, sorted. this is the fact the two-level-namespace attribution
# rules decide (#2510) and the one execution cannot observe: a wrongly attributed
# import still links and still runs on the host that happens to export it, so only
# the emitted descriptor distinguishes a correct binding from a lucky one.
#
# walks the PE32+ headers with coreutils alone (no LLVM on the linux legs): the
# import data directory is entry 1 of the optional header's directory array, which
# for PE32+ begins at optional-header offset 112, i.e. e_lfanew + 24 + 112.
produce_pe_imports() {
    bin=$3
    elfanew=$(read_le_uint "$bin" 60 4)
    nsec=$(read_le_uint "$bin" $((elfanew + 6)) 2)
    optsize=$(read_le_uint "$bin" $((elfanew + 20)) 2)
    sec=$((elfanew + 24 + optsize))

    magic=$(read_le_uint "$bin" $((elfanew + 24)) 2)
    if [ "$magic" != "523" ]; then
        echo "int: pe-imports: not a PE32+ image (optional-header magic $magic)" >&2
        return 2
    fi

    imp_rva=$(read_le_uint "$bin" $((elfanew + 24 + 112 + 8)) 4)
    if [ "$imp_rva" -eq 0 ]; then
        echo "no-import-table"
        return 0
    fi
    imp_off=$(pe_rva_to_off "$bin" "$sec" "$nsec" "$imp_rva") || {
        echo "int: pe-imports: import directory RVA $imp_rva is in no section" >&2
        return 2
    }

    out=$(mktemp)
    d=0
    while :; do
        desc=$((imp_off + d * 20))
        ilt_rva=$(read_le_uint "$bin" "$desc" 4)
        name_rva=$(read_le_uint "$bin" $((desc + 12)) 4)
        iat_rva=$(read_le_uint "$bin" $((desc + 16)) 4)
        # the descriptor array ends at an all-zero entry
        if [ "$ilt_rva" -eq 0 ] && [ "$name_rva" -eq 0 ] && [ "$iat_rva" -eq 0 ]; then break; fi

        name_off=$(pe_rva_to_off "$bin" "$sec" "$nsec" "$name_rva") || return 2
        dll=$(read_cstr "$bin" "$name_off")

        # prefer the ILT; a linker may leave it zero and describe imports through
        # the IAT alone, which holds the same thunk encoding before the loader runs
        thunks=$ilt_rva
        [ "$thunks" -eq 0 ] && thunks=$iat_rva
        thunk_off=$(pe_rva_to_off "$bin" "$sec" "$nsec" "$thunks") || return 2

        t=0
        while :; do
            thunk=$((thunk_off + t * 8))
            entry_lo=$(read_le_uint "$bin" "$thunk" 4)
            entry_hi=$(read_le_uint "$bin" $((thunk + 4)) 4)
            [ "$entry_lo" -eq 0 ] && [ "$entry_hi" -eq 0 ] && break
            # Inspect bit 63 as a byte. A by-ordinal thunk is an unsigned u64
            # above the shell's signed arithmetic range, so reading the whole
            # value into `$((...))` would overflow before the test.
            top=$(read_le_uint "$bin" $((thunk + 7)) 1)
            if [ $((top & 128)) -ne 0 ]; then
                ordinal=$(read_le_uint "$bin" "$thunk" 2)
                echo "$dll:#$ordinal" >>"$out"
            else
                hn_off=$(pe_rva_to_off "$bin" "$sec" "$nsec" "$entry_lo") || return 2
                echo "$dll:$(read_cstr "$bin" $((hn_off + 2)))" >>"$out"
            fi
            t=$((t + 1))
        done
        d=$((d + 1))
    done

    LC_ALL=C sort "$out"
    rm -f "$out"
}

# produce_pe_dllimport <runmode> <target> <binary>
# Verify qz.o's IMAGE_REL_AMD64_REL32 sites against `__imp_Sleep` and indirect-only
# `__imp_GetTickCount` target their IAT cells. Stable byte signatures locate each
# four-byte displacement, so S = P + 4 + disp proves the recovered COFF -4 addend
# as well as the target. Mach also calls Sleep directly; an IAT size of three
# thunks (two bindings + one descriptor terminator) proves that pair deduplicated
# while the indirect-only export still receives a slot and no call stub.
produce_pe_dllimport() {
    bin=$3
    elfanew=$(read_le_uint "$bin" 60 4)
    nsec=$(read_le_uint "$bin" $((elfanew + 6)) 2)
    optsize=$(read_le_uint "$bin" $((elfanew + 20)) 2)
    sec=$((elfanew + 24 + optsize))
    magic=$(read_le_uint "$bin" $((elfanew + 24)) 2)
    if [ "$magic" != "523" ]; then
        echo "int: pe-dllimport: not a PE32+ image (optional-header magic $magic)" >&2
        return 2
    fi

    imports=$(produce_pe_imports "$@") || return $?
    want_imports='kernel32.dll:GetTickCount
kernel32.dll:Sleep'
    if [ "$imports" != "$want_imports" ]; then
        echo "int: pe-dllimport: imports are '$imports', want two canonical exports" >&2
        return 2
    fi

    iat_dir=$((elfanew + 24 + 112 + 12 * 8))
    iat_rva=$(read_le_uint "$bin" "$iat_dir" 4)
    iat_size=$(read_le_uint "$bin" $((iat_dir + 4)) 4)
    if [ "$iat_rva" -eq 0 ] || [ "$iat_size" -ne 24 ]; then
        echo "int: pe-dllimport: IAT is RVA=$iat_rva size=$iat_size, want two entries" >&2
        return 2
    fi

    sleep_off=$(find_unique_hex "$bin" 4883ec28b907000000ff15) || {
        echo "int: pe-dllimport: qz_indirect Sleep signature is not unique" >&2
        return 2
    }
    tick_off=$(find_unique_hex "$bin" 904883c42848ff25) || {
        echo "int: pe-dllimport: qz_indirect GetTickCount signature is not unique" >&2
        return 2
    }

    sleep_patch=$((sleep_off + 11))
    tick_patch=$((tick_off + 8))
    sleep_rva=$(pe_off_to_rva "$bin" "$sec" "$nsec" "$sleep_patch") || return 2
    tick_rva=$(pe_off_to_rva "$bin" "$sec" "$nsec" "$tick_patch") || return 2
    sleep_disp=$(read_le_uint "$bin" "$sleep_patch" 4)
    tick_disp=$(read_le_uint "$bin" "$tick_patch" 4)
    [ "$sleep_disp" -ge 2147483648 ] && sleep_disp=$((sleep_disp - 4294967296))
    [ "$tick_disp" -ge 2147483648 ] && tick_disp=$((tick_disp - 4294967296))
    sleep_target=$((sleep_rva + 4 + sleep_disp))
    tick_target=$((tick_rva + 4 + tick_disp))
    second_iat=$((iat_rva + 8))
    if ! { [ "$sleep_target" -eq "$iat_rva" ] && [ "$tick_target" -eq "$second_iat" ]; } \
       && ! { [ "$tick_target" -eq "$iat_rva" ] && [ "$sleep_target" -eq "$second_iat" ]; }; then
        echo "int: pe-dllimport: foreign targets $sleep_target/$tick_target miss IAT $iat_rva/$second_iat" >&2
        return 2
    fi

    echo "$imports"
    echo "iat_entries=2"
    echo "foreign_indirect_target=iat"
}

# produce_pe_local_import <runmode> <target> <binary>
# Verify the DefinedLocalImport shape from real Clang objects. Duplicate live
# `__imp_local_add` sites share one cell even though an earlier import archive was
# selected; a losing SELECT_ANY body's `__imp_dead_local` site leaves no cell.
# provider.o's direct local_add call still addresses the function. The sole cell
# stores that function's image VA, receives the sole DIR64 base relocation, and
# produces neither an import directory nor an IAT (#2552).
produce_pe_local_import() {
    bin=$3
    elfanew=$(read_le_uint "$bin" 60 4)
    nsec=$(read_le_uint "$bin" $((elfanew + 6)) 2)
    optsize=$(read_le_uint "$bin" $((elfanew + 20)) 2)
    sec=$((elfanew + 24 + optsize))
    magic=$(read_le_uint "$bin" $((elfanew + 24)) 2)
    if [ "$magic" != "523" ]; then
        echo "int: pe-local-import: not a PE32+ image (optional-header magic $magic)" >&2
        return 2
    fi

    imports=$(produce_pe_imports "$@") || return $?
    iat_dir=$((elfanew + 24 + 112 + 12 * 8))
    iat_rva=$(read_le_uint "$bin" "$iat_dir" 4)
    iat_size=$(read_le_uint "$bin" $((iat_dir + 4)) 4)
    if [ "$imports" != "no-import-table" ] || [ "$iat_rva" -ne 0 ] || [ "$iat_size" -ne 0 ]; then
        echo "int: pe-local-import: local_add leaked into loader imports/IAT ($imports, $iat_rva/$iat_size)" >&2
        return 2
    fi

    first_suffix=$(find_unique_hex "$bin" ffc04883c428c3) || {
        echo "int: pe-local-import: first indirect-call suffix is not unique" >&2
        return 2
    }
    second_suffix=$(find_unique_hex "$bin" 05785634124883c428c3) || {
        echo "int: pe-local-import: second indirect-call suffix is not unique" >&2
        return 2
    }
    direct_suffix=$(find_unique_hex "$bin" 83c0024883c428c3) || {
        echo "int: pe-local-import: direct-call suffix is not unique" >&2
        return 2
    }
    local_off=$(find_unique_hex "$bin" 8d4123c36666662e0f1f840000000000) || {
        echo "int: pe-local-import: local definition marker is not unique" >&2
        return 2
    }

    first_patch=$((first_suffix - 4))
    second_patch=$((second_suffix - 4))
    direct_patch=$((direct_suffix - 4))
    if [ "$(read_le_uint "$bin" $((first_patch - 2)) 2)" -ne 5631 ] \
        || [ "$(read_le_uint "$bin" $((second_patch - 2)) 2)" -ne 5631 ] \
        || [ "$(read_le_uint "$bin" $((direct_patch - 1)) 1)" -ne 232 ]; then
        echo "int: pe-local-import: recovered displacement is not owned by the expected call opcode" >&2
        return 2
    fi

    first_rva=$(pe_off_to_rva "$bin" "$sec" "$nsec" "$first_patch") || return 2
    second_rva=$(pe_off_to_rva "$bin" "$sec" "$nsec" "$second_patch") || return 2
    direct_rva=$(pe_off_to_rva "$bin" "$sec" "$nsec" "$direct_patch") || return 2
    local_rva=$(pe_off_to_rva "$bin" "$sec" "$nsec" "$local_off") || return 2
    first_disp=$(read_le_uint "$bin" "$first_patch" 4)
    second_disp=$(read_le_uint "$bin" "$second_patch" 4)
    direct_disp=$(read_le_uint "$bin" "$direct_patch" 4)
    [ "$first_disp" -ge 2147483648 ] && first_disp=$((first_disp - 4294967296))
    [ "$second_disp" -ge 2147483648 ] && second_disp=$((second_disp - 4294967296))
    [ "$direct_disp" -ge 2147483648 ] && direct_disp=$((direct_disp - 4294967296))
    first_target=$((first_rva + 4 + first_disp))
    second_target=$((second_rva + 4 + second_disp))
    direct_target=$((direct_rva + 4 + direct_disp))
    if [ "$first_target" -ne "$second_target" ]; then
        echo "int: pe-local-import: duplicate aliases target different cells $first_target/$second_target" >&2
        return 2
    fi
    if [ "$direct_target" -ne "$local_rva" ] || [ "$first_target" -eq "$local_rva" ]; then
        echo "int: pe-local-import: direct/indirect targets do not preserve one level of indirection" >&2
        return 2
    fi

    cell_off=$(pe_rva_to_off "$bin" "$sec" "$nsec" "$first_target") || {
        echo "int: pe-local-import: pointer-cell RVA $first_target is in no section" >&2
        return 2
    }
    image_base=$(read_le_uint "$bin" $((elfanew + 24 + 24)) 8)
    cell_value=$(read_le_uint "$bin" "$cell_off" 8)
    if [ "$cell_value" -ne $((image_base + local_rva)) ]; then
        echo "int: pe-local-import: cell contains $cell_value, expected local VA $((image_base + local_rva))" >&2
        return 2
    fi

    reloc_dir=$((elfanew + 24 + 112 + 5 * 8))
    reloc_rva=$(read_le_uint "$bin" "$reloc_dir" 4)
    reloc_size=$(read_le_uint "$bin" $((reloc_dir + 4)) 4)
    reloc_off=$(pe_rva_to_off "$bin" "$sec" "$nsec" "$reloc_rva") || {
        echo "int: pe-local-import: base-relocation directory is missing" >&2
        return 2
    }
    cursor=0
    dir64_count=0
    dir64_rva=0
    while [ "$cursor" -lt "$reloc_size" ]; do
        block=$((reloc_off + cursor))
        page=$(read_le_uint "$bin" "$block" 4)
        block_size=$(read_le_uint "$bin" $((block + 4)) 4)
        if [ "$block_size" -lt 8 ] || [ $((block_size % 2)) -ne 0 ] \
            || [ $((cursor + block_size)) -gt "$reloc_size" ]; then
            echo "int: pe-local-import: malformed base-relocation block" >&2
            return 2
        fi
        entries=$(((block_size - 8) / 2))
        i=0
        while [ "$i" -lt "$entries" ]; do
            entry=$(read_le_uint "$bin" $((block + 8 + i * 2)) 2)
            type=$((entry >> 12))
            if [ "$type" -eq 10 ]; then
                dir64_count=$((dir64_count + 1))
                dir64_rva=$((page + (entry & 4095)))
            elif [ "$type" -ne 0 ]; then
                echo "int: pe-local-import: unexpected base-relocation type $type" >&2
                return 2
            fi
            i=$((i + 1))
        done
        cursor=$((cursor + block_size))
    done
    if [ "$cursor" -ne "$reloc_size" ] || [ "$dir64_count" -ne 1 ] \
        || [ "$dir64_rva" -ne "$first_target" ]; then
        echo "int: pe-local-import: DIR64 set is $dir64_count at $dir64_rva, expected the one cell $first_target" >&2
        return 2
    fi

    echo "imports=0"
    echo "local_cells=1"
    echo "indirect_targets=local_cell"
    echo "direct_target=local_definition"
    echo "cell_value=local_definition"
    echo "base_reloc=local_cell"
}

# produce_pe_exceptions <runmode> <target> <binary>
# verify that the real clang COFF fixture in pe-foreign-unwind is represented by
# one valid RUNTIME_FUNCTION row in the published PE exception directory. the
# fixture's stable first nine instruction bytes identify qz_answer without a
# linked symbol table; its patched call displacement begins immediately after
# that signature. the row's 22-byte extent and exact UNWIND_INFO distinguish the
# foreign entry from mach-generated unwind rows in the same sorted table.
produce_pe_exceptions() {
    bin=$3
    elfanew=$(read_le_uint "$bin" 60 4)
    nsec=$(read_le_uint "$bin" $((elfanew + 6)) 2)
    optsize=$(read_le_uint "$bin" $((elfanew + 20)) 2)
    sec=$((elfanew + 24 + optsize))

    magic=$(read_le_uint "$bin" $((elfanew + 24)) 2)
    if [ "$magic" != "523" ]; then
        echo "int: pe-exceptions: not a PE32+ image (optional-header magic $magic)" >&2
        return 2
    fi

    exc_rva=$(read_le_uint "$bin" $((elfanew + 24 + 112 + 3 * 8)) 4)
    exc_size=$(read_le_uint "$bin" $((elfanew + 24 + 112 + 3 * 8 + 4)) 4)
    if [ "$exc_rva" -eq 0 ] || [ "$exc_size" -eq 0 ] || [ $((exc_size % 12)) -ne 0 ]; then
        echo "int: pe-exceptions: missing or malformed exception directory" >&2
        return 2
    fi
    exc_off=$(pe_rva_to_off "$bin" "$sec" "$nsec" "$exc_rva") || {
        echo "int: pe-exceptions: exception RVA $exc_rva is in no section" >&2
        return 2
    }

    found=0
    i=0
    while [ $((i * 12)) -lt "$exc_size" ]; do
        row=$((exc_off + i * 12))
        begin=$(read_le_uint "$bin" "$row" 4)
        end=$(read_le_uint "$bin" $((row + 4)) 4)
        unwind=$(read_le_uint "$bin" $((row + 8)) 4)
        begin_off=$(pe_rva_to_off "$bin" "$sec" "$nsec" "$begin") || {
            echo "int: pe-exceptions: begin RVA $begin is in no section" >&2
            return 2
        }
        sig=$(dd if="$bin" bs=1 skip="$begin_off" count=9 2>/dev/null | od -An -tx1 | tr -d ' \n')
        if [ "$sig" = "4883ec28b928000000" ]; then
            found=$((found + 1))
            if [ $((end - begin)) -ne 22 ]; then
                echo "int: pe-exceptions: qz_answer row has length $((end - begin)), want 22" >&2
                return 2
            fi
            unwind_off=$(pe_rva_to_off "$bin" "$sec" "$nsec" "$unwind") || {
                echo "int: pe-exceptions: unwind RVA $unwind is in no section" >&2
                return 2
            }
            unwind_bytes=$(dd if="$bin" bs=1 skip="$unwind_off" count=8 2>/dev/null | od -An -tx1 | tr -d ' \n')
            if [ "$unwind_bytes" != "0104010004420000" ]; then
                echo "int: pe-exceptions: qz_answer UNWIND_INFO is $unwind_bytes" >&2
                return 2
            fi
        fi
        i=$((i + 1))
    done
    if [ "$found" -ne 1 ]; then
        echo "int: pe-exceptions: found $found qz_answer runtime rows, want 1" >&2
        return 2
    fi

    echo "exception_directory=present"
    echo "foreign_runtime=published"
    echo "foreign_length=22"
    echo "foreign_unwind=valid"
}

# produce_pe_codeview <runmode> <target> <binary>
# verify that the real clang COFF fixture in pe-foreign-codeview retained its
# `.debug$S` records and that both SECREL+SECTION pairs resolve to the exact
# linked code address and final PE section. The fixture's `.text` requires 8 KiB
# alignment while PE's first section begins at RVA 0x1000, so deriving the
# expected offset from the section header (rather than linker content) exercises
# the alignment prefix that originally made the two bases differ.
produce_pe_codeview() {
    bin=$3
    elfanew=$(read_le_uint "$bin" 60 4)
    nsec=$(read_le_uint "$bin" $((elfanew + 6)) 2)
    optsize=$(read_le_uint "$bin" $((elfanew + 20)) 2)
    sec=$((elfanew + 24 + optsize))

    magic=$(read_le_uint "$bin" $((elfanew + 24)) 2)
    if [ "$magic" != "523" ]; then
        echo "int: pe-codeview: not a PE32+ image (optional-header magic $magic)" >&2
        return 2
    fi

    text_index=0
    text_rva=0
    debug_off=0
    debug_size=0
    i=0
    while [ "$i" -lt "$nsec" ]; do
        base=$((sec + i * 40))
        name=$(dd if="$bin" bs=1 skip="$base" count=8 2>/dev/null | od -An -tx1 | tr -d ' \n')
        case "$name" in
            2e74657874000000)
                text_index=$((i + 1))
                text_rva=$(read_le_uint "$bin" $((base + 12)) 4)
                ;;
            2e64656275672453)
                debug_size=$(read_le_uint "$bin" $((base + 16)) 4)
                debug_off=$(read_le_uint "$bin" $((base + 20)) 4)
                ;;
        esac
        i=$((i + 1))
    done
    if [ "$text_index" -eq 0 ] || [ "$debug_off" -eq 0 ] || [ "$debug_size" -lt 178 ]; then
        echo 'int: pe-codeview: missing .text or complete .debug$S section' >&2
        return 2
    fi

    code_off=$(find_unique_hex "$bin" 4883ec28b928000000) || {
        echo "int: pe-codeview: codeview_answer signature is not unique" >&2
        return 2
    }
    code_rva=$(pe_off_to_rva "$bin" "$sec" "$nsec" "$code_off") || {
        echo "int: pe-codeview: codeview_answer is outside the PE sections" >&2
        return 2
    }
    if [ "$code_rva" -lt "$text_rva" ]; then
        echo "int: pe-codeview: code precedes its .text section" >&2
        return 2
    fi
    expected=$((code_rva - text_rva))

    secrel_a=$(read_le_uint "$bin" $((debug_off + 0x68)) 4)
    section_a=$(read_le_uint "$bin" $((debug_off + 0x6c)) 2)
    secrel_b=$(read_le_uint "$bin" $((debug_off + 0xac)) 4)
    section_b=$(read_le_uint "$bin" $((debug_off + 0xb0)) 2)
    if [ "$secrel_a" -ne "$expected" ] || [ "$secrel_b" -ne "$expected" ]; then
        echo "int: pe-codeview: SECREL fields $secrel_a/$secrel_b, want $expected" >&2
        return 2
    fi
    if [ "$section_a" -ne "$text_index" ] || [ "$section_b" -ne "$text_index" ]; then
        echo "int: pe-codeview: SECTION fields $section_a/$section_b, want $text_index" >&2
        return 2
    fi
    if [ "$text_rva" -ne 4096 ] || [ "$code_rva" -lt 8192 ] || [ "$expected" -lt 4096 ]; then
        echo "int: pe-codeview: fixture no longer covers first-section alignment padding" >&2
        return 2
    fi

    echo "codeview_section=present"
    echo "secrel_fields=exact"
    echo "section_fields=exact"
    echo "alignment_prefix=covered"
}

# produce_field <runmode> <target> <binary>
# emits the canonical structural fact for the artifact's format, dispatched on its
# leading magic bytes so the reader is independent of the leg the case ran on.
produce_field() {
    bin=$3
    magic=$(dd if="$bin" bs=1 count=4 2>/dev/null | od -An -tx1 | tr -d ' \n')
    case "$magic" in
        7f454c46)  field_elf "$bin" ;;      # 0x7F 'E' 'L' 'F' -> ELF
        4d5a*)     field_pe "$bin" ;;       # 'MZ' DOS stub -> PE/COFF
        cffaedfe*) field_macho "$bin" ;;    # MH_MAGIC_64 (little-endian)
        *) echo "int: field: unrecognized binary format (magic $magic)" >&2; return 2 ;;
    esac
}

# produce_relro <runmode> <target> <binary>
# emits the ELF RELRO fact: relro=1 when a PT_GNU_RELRO program header (p_type
# 0x6474e552) is present, else relro=0. read host-side from the program headers
# (e_phoff u64 @32, e_phentsize u16 @54, e_phnum u16 @56), never executing the binary,
# so it works on every leg including qemu. ELF-only (RELRO is an ELF concept).
produce_relro() {
    bin=$3
    magic=$(dd if="$bin" bs=1 count=4 2>/dev/null | od -An -tx1 | tr -d ' \n')
    if [ "$magic" != "7f454c46" ]; then
        echo "int: relro: not an ELF binary (magic $magic)" >&2; return 2
    fi
    phoff=$(read_le_uint "$bin" 32 8)
    phentsize=$(read_le_uint "$bin" 54 2)
    phnum=$(read_le_uint "$bin" 56 2)
    relro=0
    i=0
    while [ "$i" -lt "$phnum" ]; do
        ptype=$(read_le_uint "$bin" $((phoff + i * phentsize)) 4)
        if [ "$ptype" = "1685382482" ]; then relro=1; break; fi   # PT_GNU_RELRO = 0x6474e552
        i=$((i + 1))
    done
    echo "relro=$relro"
}

# produce_flat_loader <runmode> <target> <binary>
# loads the freestanding raw image through the C loader (built once, cached) and
# reports the image's exit status as the observable. any stdout the image writes
# flows first. a loader-infrastructure failure (no cc, mmap denied) returns nonzero.
produce_flat_loader() {
    bin=$3
    if [ -z "$_flat_loader_bin" ]; then
        _flat_loader_bin=$(mktemp -d)/flat_loader
        if ! cc -O2 -o "$_flat_loader_bin" "$_produce_lib_dir/flat_loader.c" 2>&1; then
            echo "int: flat-loader: could not build the C loader (cc required)" >&2
            _flat_loader_bin=
            return 2
        fi
    fi
    if "$_flat_loader_bin" "$bin"; then ec=0; else ec=$?; fi
    printf 'exit=%d\n' "$ec"
}

# produce_built <runmode> <target> <binary>
# a build-only observable: prove the tuple composes and emits an artifact without
# running it. for a cross-built target with no host runner (a freestanding aarch64
# / riscv64 flat image on the x86_64 linux leg) running is impossible, but the
# emit path is exactly what must not regress. run.sh has already failed the case if
# the build failed; this asserts the artifact exists and is non-empty.
produce_built() {
    bin=$3
    if [ -s "$bin" ]; then
        echo "built=1"
    else
        echo "int: built: artifact missing or empty" >&2
        return 2
    fi
}

# produce_spirv_val <runmode> <target> <binary>
# validate every SPIR-V module the build delivered, with the Khronos validator.
# a finished-module target has no linked binary: the delivery is the module tree,
# so this globs the case's output root (the directory <binary> would have been
# written into) and runs spirv-val over each `.spv`. an EXTERNAL validator is the
# point — mach reading back its own bytes proves self-consistency, not validity.
# the observable is the module count plus the verdict, so a build that silently
# stopped emitting fails on the count rather than passing vacuously.
produce_spirv_val() {
    out_dir=$(dirname "$3")
    if ! command -v spirv-val >/dev/null 2>&1; then
        echo "int: spirv-val: the validator is not installed (spirv-tools)" >&2
        return 2
    fi
    n=0
    for m in $(find "$out_dir" -name '*.spv' | sort); do
        spirv-val "$m" || return 1
        n=$((n + 1))
    done
    if [ "$n" -eq 0 ]; then
        echo "int: spirv-val: the build delivered no .spv module" >&2
        return 2
    fi
    printf 'modules=%d validator=clean\n' "$n"
}

# resolve_dwarfdump — print an llvm-dwarfdump on PATH, preferring the unversioned
# name and falling back to the highest-versioned one (ubuntu ships llvm-dwarfdump-NN).
# empty output (return 1) when none is installed.
resolve_dwarfdump() {
    if command -v llvm-dwarfdump >/dev/null 2>&1; then echo llvm-dwarfdump; return 0; fi
    newest=$(compgen -c 'llvm-dwarfdump-' 2>/dev/null | sort -t- -k3 -n | tail -1)
    [ -n "$newest" ] && { echo "$newest"; return 0; }
    return 1
}

# resolve_symbolizer — print an llvm-symbolizer on PATH, preferring the unversioned
# name and falling back to the highest-versioned one from the llvm package.
resolve_symbolizer() {
    if command -v llvm-symbolizer >/dev/null 2>&1; then echo llvm-symbolizer; return 0; fi
    newest=$(compgen -c 'llvm-symbolizer-' 2>/dev/null | sort -t- -k3 -n | tail -1)
    [ -n "$newest" ] && { echo "$newest"; return 0; }
    return 1
}

# _norm_shdr_fields <in> <out> — copy <in> to <out> zeroing the ELF header's
# section-table bookkeeping (e_shoff @40 8B, e_shnum @60 2B, e_shstrndx @62 2B), which
# legitimately differs once `-g` adds named debug sections. everything else — every
# loadable byte — must stay identical.
_norm_shdr_fields() {
    cp "$1" "$2"
    printf '\0\0\0\0\0\0\0\0' | dd of="$2" bs=1 seek=40 count=8 conv=notrunc status=none
    printf '\0\0'             | dd of="$2" bs=1 seek=60 count=2 conv=notrunc status=none
    printf '\0\0'             | dd of="$2" bs=1 seek=62 count=2 conv=notrunc status=none
}

# elf_seg_identical <g> <nog> — 0 when every PT_LOAD segment of the `-g` image has
# byte-identical file content in the no-`-g` image (after normalizing the header
# section-table fields), else 1. the additive-only guard: `-g` must not perturb one
# byte of the loaded program. PT_LOAD file extents come from `readelf -lW` (offset,
# filesz); a p_filesz of 0 (a pure .bss LOAD) carries no file bytes to compare.
elf_seg_identical() {
    an=$(mktemp); bn=$(mktemp)
    _norm_shdr_fields "$1" "$an"; _norm_shdr_fields "$2" "$bn"
    rc=0
    while read -r off fsz; do
        [ "$fsz" -eq 0 ] && continue
        if ! cmp -s \
            <(dd if="$an" bs=1M iflag=skip_bytes,count_bytes skip="$off" count="$fsz" status=none) \
            <(dd if="$bn" bs=1M iflag=skip_bytes,count_bytes skip="$off" count="$fsz" status=none); then
            rc=1; break
        fi
    done < <(readelf -lW "$1" 2>/dev/null | awk '/LOAD/{print strtonum($2), strtonum($5)}')
    rm -f "$an" "$bn"
    return $rc
}

# produce_debuginfo <runmode> <target> <nog_binary> <g_binary>
# the binary-inspection producer for the debuginfo case kind (#2039): asserts, purely
# host-side over the artifacts run.sh built with and without `-g`, that (1) the
# standard structural validator accepts the whole `-g` image, (2) `-g` is loadable-
# byte additive, and (3) duplicate generic, comptime-value, and pack instances retain
# one live, symbolizable DIE while each discarded copy carries DWARF's dead-code address,
# has no line-table sequence at the winner, and has no location list at the winner.
# the facts are ISA-independent, so the golden is shared. requires llvm-dwarfdump,
# llvm-symbolizer, and readelf on the leg; a missing validator is a hard error.
produce_debuginfo() {
    nog=$3
    g=$4
    dd_tool=$(resolve_dwarfdump) || {
        echo "int: debuginfo: llvm-dwarfdump not found (install the 'llvm' package)" >&2; return 2
    }
    sym_tool=$(resolve_symbolizer) || {
        echo "int: debuginfo: llvm-symbolizer not found (install the 'llvm' package)" >&2; return 2
    }
    command -v readelf >/dev/null 2>&1 || {
        echo "int: debuginfo: readelf not found (install 'binutils')" >&2; return 2
    }

    if "$dd_tool" --verify "$g" >/dev/null 2>&1; then
        echo "dwarfdump_verify=clean"
    else
        echo "dwarfdump_verify=errors"
    fi

    if elf_seg_identical "$g" "$nog"; then
        echo "g_additive=yes"
    else
        echo "g_additive=no"
    fi

    # helper and main instantiate all three weak template forms. each winner must
    # symbolize by source name while the losing atom's DIE retains a dead low_pc.
    info=$("$dd_tool" --debug-info "$g") || return 1
    lines=$("$dd_tool" --debug-line "$g") || return 1
    locations=$("$dd_tool" --debug-loclists "$g") || return 1
    for spec in ident:ident value:add_n pack:pack_sum; do
        label=${spec%%:*}
        want=${spec#*:}
        counts=$(printf '%s\n' "$info" | awk -v want="$want" '
            index($0, "DW_AT_name") && index($0, "(\"" want "\")") {
                getline
                if ($0 ~ /dead code/) { dead++ }
                else if ($0 ~ /DW_AT_low_pc.*0x/) {
                    live++
                    if (addr == "" && match($0, /0x[0-9a-fA-F]+/)) {
                        addr = substr($0, RSTART, RLENGTH)
                    }
                }
            }
            END { printf "%d %d %s", live, dead, addr }
        ')
        set -- $counts
        printf 'weak_%s_dies=live:%s,dead:%s\n' "$label" "$1" "$2"
        symbol=missing
        if [ -n "$3" ]; then
            resolved=$("$sym_tool" --obj="$g" "$3" | sed -n '1p')
            if [ "$resolved" = "$want" ]; then symbol=$resolved; fi
        fi
        printf 'weak_%s_symbol=%s\n' "$label" "$symbol"

        # every live template has exactly one sequence beginning at its entry. a
        # losing weak set_address that resolves to the winner creates a second
        # prologue_end row at that address while remaining validator-clean.
        line_starts=$(printf '%s\n' "$lines" | awk -v addr="$3" '
            $1 == addr && /prologue_end/ { n++ }
            END { print n + 0 }
        ')
        line_state="count:$line_starts"
        [ "$line_starts" -eq 1 ] && line_state=unique
        printf 'weak_%s_lines=%s\n' "$label" "$line_state"

        # pack_sum's changing accumulator home gives it a location list in debug
        # builds. release may optimize that list away, so the invariant is that at
        # most one list starts at the winner; a losing base_address alias makes two.
        if [ "$label" = pack ]; then
            loc_starts=$(printf '%s\n' "$locations" | awk -v addr="$3" '
                index($0, "[" addr ",") { n++ }
                END { print n + 0 }
            ')
            loc_state="aliased:$loc_starts"
            [ "$loc_starts" -le 1 ] && loc_state=not-aliased
            printf 'weak_pack_locations=%s\n' "$loc_state"
        fi
    done
}

# resolve_objdump — print an llvm-objdump on PATH, preferring the unversioned name
# and falling back to the highest-versioned one (ubuntu ships llvm-objdump-NN, from
# the same `llvm` package as llvm-dwarfdump). llvm-objdump decodes every ISA mach
# targets regardless of the runner's own, which GNU objdump does not. empty output
# (return 1) when none is installed.
resolve_objdump() {
    if command -v llvm-objdump >/dev/null 2>&1; then echo llvm-objdump; return 0; fi
    newest=$(compgen -c 'llvm-objdump-' 2>/dev/null | sort -t- -k3 -n | tail -1)
    [ -n "$newest" ] && { echo "$newest"; return 0; }
    return 1
}

# produce_vector_emit <runmode> <target> <binary>
# the auto-vectorizer's EMISSION observable (#2207): for each function of the case's
# own module, whether the emitted machine code contains packed SIMD instructions.
#
# a vectorize case that only runs the program and compares against a `#[scalar]`
# reference is vacuously green when the pass declines — both sides are scalar and
# agree — so this is the only assertion that can see the pass stop firing, or start
# firing on a shape it must refuse. it disassembles the OBJECTS rather than the
# linked binary: mach's linker emits no symbol table, so per-function attribution
# exists only before the link. the objects sit beside the artifact because the case
# pins `out = "out/int/build"` in its own manifest; the project id (the directory
# under obj/ holding the case's modules, as opposed to its dependencies') is read
# from that same manifest rather than assumed.
#
# the per-function verdict, not the instruction count, is the observable: the count
# moves with every unrelated backend change, while `simd` / `scalar` moves only when
# the vectorizer's own decision does.
produce_vector_emit() {
    dis_case_objects vector-emit "$3" | vector_emit_scan
}

# dis_case_objects <producer> <binary>
# disassemble the case's OWN module objects (not its dependencies') to stdout, the
# shared front half of every emitted-shape producer.
#
# the objects rather than the linked binary: mach's linker emits no symbol table, so
# per-function attribution exists only before the link. they sit beside the artifact
# because such a case pins `out = "out/int/build"` in its own manifest; the project
# id (the directory under obj/ holding the case's modules) is read from that same
# manifest rather than assumed. <producer> only names the caller in diagnostics.
dis_case_objects() {
    who=$1
    bin=$2
    dir=$(dirname "$(dirname "$(dirname "$bin")")")
    id=$(sed -n 's/^[[:space:]]*id[[:space:]]*=[[:space:]]*"\([^"]*\)".*/\1/p' "$dir/mach.toml" | head -1)
    if [ -z "$id" ]; then
        echo "int: $who: no project id in ${dir}/mach.toml" >&2; return 2
    fi
    objdir="$dir/out/int/build/obj/$id"
    if [ ! -d "$objdir" ]; then
        echo "int: $who: no objects at $objdir (the case must pin out = \"out/int/build\")" >&2; return 2
    fi
    tool=$(resolve_objdump) || {
        echo "int: $who: llvm-objdump not found (install the 'llvm' package)" >&2; return 2
    }
    find "$objdir" -name '*.o' | sort | while IFS= read -r o; do
        "$tool" -d --no-show-raw-insn "$o"
    done
}

# produce_float_emit <runmode> <target> <binary>
# the scalar-float EMISSION observable (#2237): for each function of the case's own
# module, how many emitted instructions name the back end's reserved FP scratch
# registers.
#
# the same argument as vector-emit. an encoder that stages every already-allocated
# float operand through a fixed scratch pair — which is what x86-64 did for every
# operand of every scalar float op before #2237 — computes exactly the right answer
# while emitting four instructions where two suffice, so a run-and-compare case is
# vacuously green against it. the scratch count is what changes.
#
# the count IS the observable here, unlike vector-emit's verdict: zero is a
# statement ("no allocated operand was staged"), and the number a regression
# reintroduces is the diagnostic. it is stable against unrelated backend work
# because only a genuine spill legitimately reaches scratch, which is why the case
# pins the release profile and keeps its kernels under register pressure.
produce_float_emit() {
    dis_case_objects float-emit "$3" | float_emit_scan
}

# vector_emit_scan — `<function> simd|scalar` per function (see emit_scan)
vector_emit_scan() { emit_scan simd; }

# float_emit_scan — `<function> scratch=<count>` per function (see emit_scan)
float_emit_scan() { emit_scan fpscratch; }

# frame_elision_scan — `<function> framed|frameless` per function (see emit_scan)
frame_elision_scan() { emit_scan frame; }

# vector_lanes_scan — `<function> lane=<0|1> vecmem=<0|1>` per function (see emit_scan)
vector_lanes_scan() { emit_scan lanes; }

# emit_scan <mode> — read a disassembly on stdin, print one line per function,
# sorted by function, preceded by the ISA the dispatch resolved. one scanner for
# every emitted-shape observable: the demangler, the symbol attribution and the
# sort are the same question regardless of what is being counted, and <mode>
# selects only the per-instruction predicate and the line format.
#
#   simd      — `<function> simd|scalar`, whether the function contains packed SIMD
#               (#2207). packed recognition is per ISA and deliberately narrow, so
#               a scalar function can never read as vectorized:
#     x86_64  — the packed SSE opcodes, plus MOVUPS / MOVAPS / MOVDQU / MOVDQA,
#               which the backend emits only for a 128-bit value. the scalar `*ss` /
#               `*sd` forms and the bank-crossing MOVD / MOVQ are excluded, as is
#               XORPS, which zeroes an XMM for scalar float negation, and PXOR,
#               which clears one.
#     aarch64 — any operand naming a NEON register: a `vN.<arrangement>` or a
#               128-bit `qN`. scalar float uses `sN` / `dN` and never matches.
#     riscv64 — any RVV mnemonic (`v...`); no scalar riscv64 mnemonic starts with
#               `v`. riscv64 has no 128-bit vector model, so every verdict there is
#               scalar.
#     a case whose source writes SIMD types directly would defeat this (its packed
#     ops are not the vectorizer's); the vectorize cases use scalar element types
#     only.
#
#   fpscratch — `<function> scratch=<count>`, how many instructions name a register
#               the back end reserves from FP allocation (#2237). each back end
#               holds out one pair, so a value only appears there when the encoder
#               STAGED it rather than operating on it where the allocator put it:
#     x86_64  — xmm14 / xmm15
#     aarch64 — v30 / v31, under any of their scalar or vector spellings
#     riscv64 — f30 / f31, spelled ft10 / ft11 in the ABI names the disassembler
#               prints
#     a genuine spill legitimately reaches these, so a case asserting zero must keep
#     its kernels under register pressure and pin the release profile.
#
#   frame     — `<function> framed|frameless`, whether the emitted prologue builds a
#               frame (#1940). the only mode that reads the FIRST instruction and no
#               other, which is what makes the observable stable: a function's body
#               moves with every unrelated backend change, while its first
#               instruction is the frame decision and nothing else.
#     x86_64  — `push %rbp` opens every framed prologue.
#     aarch64 — `stp x29, x30, [sp, #-N]!` is the record-at-top prologue.
#     riscv64 — `addi sp, sp, -N` opens the frame allocation.
#
#   lanes     — `<function> lane=<0|1> vecmem=<0|1>`, how a vector lane was accessed
#               (#2236). the only mode reporting TWO independent facts, and both
#               matter: `lane` is the win, a scalar entering or leaving a vector
#               register directly; `vecmem` is the cost it replaces, a whole 128-bit
#               vector moved through memory, which was once the only way to name a
#               lane and costs a store-to-load forwarding stall on every read.
#     aarch64 — lane is a lane-indexed operand (`vN.<t>[K]`, what UMOV / DUP-element
#               / INS render); vecmem is a `qN` memory move (LDR/STR Q).
#     x86_64  — lane is PEXTR / PINSR (no lane form is selected there yet, so it
#               reads 0 today and flips when x64 opts in); vecmem is MOVUP[SD] /
#               MOVAP[SD] / MOVDQ[AU].
#     riscv64 — no 128-bit vector model at all: every vector is scalarized into
#               ordinary integer loads and stores, so both facts are 0 - the
#               target's own golden, not an exemption.
emit_scan() {
    awk -v mode="$1" '
    function demangle(s,   i, len, c, out) {
        if (substr(s, 1, 2) != "_M") { return s }
        i = 3
        out = ""
        while (i <= length(s)) {
            c = substr(s, i, 1)
            if (c == "N") { i++; continue }
            if (c !~ /[0-9]/) { return s }
            len = 0
            while (i <= length(s) && substr(s, i, 1) ~ /[0-9]/) { len = len * 10 + substr(s, i, 1); i++ }
            out = substr(s, i, len)
            i += len
        }
        if (out == "") { return s }
        return out
    }
    function packed(m, rest) {
        if (isa == "x86_64") {
            if (m ~ /^(movup[sd]|movap[sd]|movdq[au])$/) { return 1 }
            if (m ~ /^(add|sub|mul|div|min|max|sqrt|rcp|rsqrt|hadd|hsub|unpckh|unpckl|shuf|cmp[a-z]*)p[sd]$/) { return 1 }
            if (m ~ /^p(add|sub|and|andn|or|cmp|mul|mull|mulh|madd|min|max|avg|abs|sign|sll|srl|sra|shuf|unpck|ack)[a-z0-9]*$/) { return 1 }
            return 0
        }
        if (isa == "aarch64") {
            if (rest ~ /(^|[ ,[])v[0-9]+\.[0-9]+[bhsd]/) { return 1 }
            if (rest ~ /(^|[ ,[])q[0-9]+([ ,\]]|$)/)     { return 1 }
            return 0
        }
        if (isa == "riscv64") { return m ~ /^v[a-z]/ }
        return 0
    }
    function fp_scratch(m, rest) {
        if (isa == "x86_64")  { return rest ~ /xmm1[45]/ }
        if (isa == "aarch64") { return rest ~ /(^|[^0-9a-zA-Z_])[bhsdqv]3[01]([^0-9]|$)/ }
        if (isa == "riscv64") { return rest ~ /(^|[^0-9a-zA-Z_])ft1[01]([^0-9]|$)/ }
        return 0
    }
    function framed(m, rest) {
        if (isa == "x86_64")  { return m ~ /^push/ && rest ~ /%rbp/ }
        if (isa == "aarch64") { return m == "stp" && rest ~ /x29,[[:space:]]*x30/ }
        if (isa == "riscv64") { return m == "addi" && rest ~ /^sp,[[:space:]]*sp,[[:space:]]*-/ }
        return 0
    }
    function is_lane(m, rest) {
        if (isa == "aarch64") { return rest ~ /v[0-9]+\.[bhsd]\[[0-9]+\]/ }
        if (isa == "x86_64")  { return m ~ /^p(extr|insr)[bwdq]$/ }
        return 0
    }
    function is_vecmem(m, rest) {
        if (isa == "aarch64") {
            if (m !~ /^(ldr|str|ldur|stur|ldp|stp)$/) { return 0 }
            return rest ~ /(^|[ ,])q[0-9]+([ ,]|$)/
        }
        if (isa == "x86_64") { return m ~ /^(movup[sd]|movap[sd]|movdq[au])$/ }
        return 0
    }
    # the SECOND, independent fact. a mode reporting two facts keeps them in separate
    # accumulators rather than folding one into the other, so neither can mask it.
    function counts2(m, rest) {
        if (mode == "lanes") { return is_vecmem(m, rest) }
        return 0
    }
    function counts(m, rest) {
        if (mode == "simd")      { return packed(m, rest) }
        if (mode == "fpscratch") { return fp_scratch(m, rest) }
        if (mode == "frame")     { return framed(m, rest) }
        if (mode == "lanes")     { return is_lane(m, rest) }
        return 0
    }
    /file format/ {
        if      ($0 ~ /x86-64/)   { isa = "x86_64" }
        else if ($0 ~ /aarch64/)  { isa = "aarch64" }
        else if ($0 ~ /riscv/)    { isa = "riscv64" }
        else                      { bad = $0 }
        next
    }
    /^[0-9a-f]+ <.*>:$/ {
        sym = $0
        sub(/^[0-9a-f]+ </, "", sym)
        sub(/>:$/, "", sym)
        sym = demangle(sym)
        if (!(sym in count)) { names[++n] = sym; count[sym] = 0; count2[sym] = 0 }
        cur = sym
        first = 1
        next
    }
    cur != "" && /^[[:space:]]*[0-9a-f]+:/ {
        # the frame verdict is the FIRST instruction and nothing else; every other
        # mode reads the whole function.
        if (mode == "frame" && !first) { next }
        first = 0
        line = $0
        sub(/^[[:space:]]*[0-9a-f]+:[[:space:]]*/, "", line)
        split(line, f, /[[:space:]]+/)
        rest = line
        sub(/^[^[:space:]]+[[:space:]]*/, "", rest)
        if (counts(f[1], rest))  { count[cur]++ }
        if (counts2(f[1], rest)) { count2[cur]++ }
    }
    END {
        if (bad != "") { print "int: emit-scan: " mode " scan hit an unrecognized object format (" bad ")" > "/dev/stderr"; exit 2 }
        # insertion-sort the names so the observable does not depend on emission order
        for (i = 2; i <= n; i++) {
            k = names[i]
            j = i - 1
            while (j >= 1 && names[j] > k) { names[j + 1] = names[j]; j-- }
            names[j + 1] = k
        }
        print "arch=" isa
        for (i = 1; i <= n; i++) {
            if (mode == "simd")       { print names[i] " " (count[names[i]] > 0 ? "simd" : "scalar") }
            else if (mode == "frame") { print names[i] " " (count[names[i]] > 0 ? "framed" : "frameless") }
            else if (mode == "lanes") { print names[i] " lane=" (count[names[i]] > 0 ? 1 : 0) " vecmem=" (count2[names[i]] > 0 ? 1 : 0) }
            else                      { print names[i] " scratch=" count[names[i]] }
        }
    }
    '
}

# produce_vector_lanes <runmode> <target> <binary>
# disassembles the case's own objects and reports, per function, how a vector LANE
# was accessed (#2236). Shares produce_vector_emit's object discovery: the case pins
# `out = "out/int/build"` and the project id comes from its manifest.
produce_vector_lanes() {
    dis_case_objects vector-lanes "$3" | vector_lanes_scan
}



# produce_frame_elision <runmode> <target> <binary>
# the frame-elision observable (#1940): for each function of the case's own module,
# whether the emitted prologue builds a frame.
#
# execution cannot see this. a frameless leaf and a framed one compute the same
# answer, so a case that only runs the program is green whether the elision fires,
# stops firing, or fires on a function that must keep its frame - the last of which
# is a miscompile (an incoming stack parameter addressed off a frame pointer that
# was never established). the verdict is read from the bytes for that reason.
#
# it disassembles the OBJECTS, not the linked binary: mach's linker emits no symbol
# table, so per-function attribution exists only before the link. the objects sit
# beside the artifact because the case pins `out = "out/int/build"` in its own
# manifest, and the project id is read from that manifest rather than assumed - the
# same arrangement produce_vector_emit uses.
produce_frame_elision() {
    dis_case_objects frame-elision "$3" | frame_elision_scan
}


# produce <run> <runmode> <target> <binary> [<g_binary>]
# dispatches to the producer named by <run>, forwarding the remaining arguments. the
# debuginfo producer takes an extra `-g` artifact path run.sh built alongside the
# default (no-`-g`) one; every other producer inspects the single default artifact.
produce() {
    run=$1
    shift
    case "$run" in
        exec)        produce_exec "$@" ;;
        relro-fault) produce_relro_fault "$@" ;;
        panic-exit)  produce_panic_exit "$@" ;;
        field)       produce_field "$@" ;;
        macho-signed) produce_macho_signed "$@" ;;
        pe-imports)  produce_pe_imports "$@" ;;
        pe-exceptions) produce_pe_exceptions "$@" ;;
        pe-codeview) produce_pe_codeview "$@" ;;
        pe-dllimport) produce_pe_dllimport "$@" ;;
        pe-local-import) produce_pe_local_import "$@" ;;
        pe-resources) produce_pe_resources "$@" ;;
        relro)       produce_relro "$@" ;;
        flat-loader) produce_flat_loader "$@" ;;
        built)       produce_built "$@" ;;
        debuginfo)   produce_debuginfo "$@" ;;
        spirv-val)   produce_spirv_val "$@" ;;
        vector-emit) produce_vector_emit "$@" ;;
        vector-lanes) produce_vector_lanes "$@" ;;
        frame-elision) produce_frame_elision "$@" ;;
        float-emit)  produce_float_emit "$@" ;;
        *) echo "int: unknown run mode '$run'" >&2; return 2 ;;
    esac
}
