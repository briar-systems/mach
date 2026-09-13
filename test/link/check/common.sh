# common.sh: the readers every link check shares, sourced by the check scripts.
#
# a check script turns a built case artifact into a normalized text observable on
# stdout, which test/run.sh diffs against the case's expect file. it is invoked as
#
#   check.sh <engine> <leg> <binary> [<g-binary>] [<profile>]
#
# engine is `native` or `qemu:<command>`, leg the machine the case runs on, and the
# `-g` twin is present when case.conf says `gbuild: yes`. a nonzero exit fails the
# cell and stderr is shown; stdout is the observable.

# is_signal_exit <status> — true when a wait status is a signal death rather than a
# deliberate return. a signal death is 128 + N with N in 1..64 (POSIX real-time
# signals cap there). the two are completely different defects - a process that
# faulted versus one that ran to completion and returned nonzero because it computed
# a wrong value - so nothing that reports a failed run may collapse them (#2593,
# #2369).
is_signal_exit() {
    [ "$1" -ge 129 ] && [ "$1" -le 192 ]
}

# describe_exit <status> — a nonzero wait status as a diagnostic phrase, naming a
# signal death separately from a deliberate return. for the golden-observable form
# of the same distinction see produce_panic_exit, whose `exit=signal(N)` token this
# deliberately does not share: that one is diffed text, this one is prose for a
# human reading a failure.
describe_exit() {
    if is_signal_exit "$1"; then
        echo "died on signal $(($1 - 128)) (exit $1)"
    else
        echo "returned $1"
    fi
}

# run_captured <engine> <leg> <binary> <stdout-file> [<stderr-file>]
# run a built binary under the leg's engines.conf engine, writing its stdout to
# <stdout-file> and its exit status to
# `run_status`. with a fifth argument stderr goes to that file, otherwise it is
# merged into <stdout-file>. `run_out` is the stdout as a string, for producers that
# compare it; a producer forwarding stdout as its observable must cat the file
# instead, since `$(...)` strips trailing newlines the golden may carry.
# always returns 0 unless the run could not be attempted: classifying the status is
# the caller's job, and every executed case has to be able to see it.
#
# captured to a file rather than through `$(...)` so the status read sits on the line
# directly after the command, the shape that survives this harness's `set -e`.
# `$(...)` cannot carry the status either: after
# `if ! v=$(cmd)`, `$?` inside the branch is the negation's 0, not the command's.
run_captured() {
    case "$1" in
        native)  set -- "" "$2" "$3" "$4" ${5+"$5"} ;;
        qemu:*)  set -- "${1#qemu:}" "$2" "$3" "$4" ${5+"$5"} ;;
        *) echo "link: '$2' declares engine '$1', which executes nothing" >&2; return 1 ;;
    esac
    if [ $# -ge 5 ]; then
        ${1:+"$1"} "$3" >"$4" 2>"$5"
    else
        ${1:+"$1"} "$3" >"$4" 2>&1
    fi
    run_status=$?
    run_out=$(cat "$4")
    return 0
}

# report_run_failure <label> <status> <output>
# report a failed execution of a built binary: what the status actually was, and
# everything the program printed before it stopped. the output is the evidence -
# #2586 was a program that printed four correct values and one wrong one, and the
# harness discarded the line naming the defect and said "execution failed" (#2593).
report_run_failure() {
    echo "link: $1: $(describe_exit "$2")" >&2
    if [ -n "$3" ]; then
        printf '%s\n' "$3" | sed 's/^/    /' >&2
    else
        echo "    (the program printed nothing)" >&2
    fi
}

# diff_expected_actual <expected> <actual>
# report a runtime contract mismatch as a diff. a producer that compares a program's
# output against a fixed contract must show WHICH line disagreed: #2586 was one wrong
# value among five correct ones, and naming it is the whole diagnosis (#2593).
diff_expected_actual() {
    _de=$(mktemp)
    _da=$(mktemp)
    printf '%s\n' "$1" >"$_de"
    printf '%s\n' "$2" >"$_da"
    diff -u --label expected --label actual "$_de" "$_da" | sed 's/^/    /' >&2
    rm -f "$_de" "$_da"
}

# read_le_uint <file> <offset> <size>
# print the unsigned little-endian integer of <size> bytes (1, 2, 4, or 8) at <offset>.
# od reads in host byte order; every CI runner is little-endian.
read_le_uint() {
    dd if="$1" bs=1 skip="$2" count="$3" 2>/dev/null | od -An -tu"$3" | tr -d ' \n'
}

# read_cstr <file> <offset> — print the NUL-terminated string at <offset>. splitting
# the byte window on NUL and taking the first record ends the string exactly where
# the format does; a name longer than the window is not a case this suite writes.
read_cstr() {
    dd if="$1" bs=1 skip="$2" count=256 2>/dev/null | tr '\0' '\n' | head -n 1
}

# field_elf <binary> — the ELF position-independence fact. e_type is a u16 at offset
# 16; ET_DYN (3) is a position-independent (PIE) executable, ET_EXEC (2) a
# fixed-address one.
field_elf() {
    bin=$1
    etype=$(read_le_uint "$bin" 16 2)
    echo "e_type=$etype"
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
# The load commands are the independent file-offset-to-VA map structural checks
# must use; a section command's own offsets are derived from them.
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
    echo "link: macho: segment '$want' not found" >&2
    return 2
}

# macho_section_fields <file> <segname> <sectname> — print a section's
# "addr size fileoff align flags" tuple from its section_64 entry, or fail when
# absent. `flags` carries the section TYPE in its low byte, which is what dyld
# dispatches on (S_MOD_INIT_FUNC_POINTERS = 9).
macho_section_fields() {
    bin=$1
    want_seg=$2
    want_sect=$3
    ncmds=$(read_le_uint "$bin" 16 4)
    off=32
    i=0
    while [ "$i" -lt "$ncmds" ]; do
        cmd=$(read_le_uint "$bin" "$off" 4)
        cmdsize=$(read_le_uint "$bin" $((off + 4)) 4)
        if [ "$cmd" -eq 25 ]; then
            segname=$(dd if="$bin" bs=1 skip=$((off + 8)) count=16 2>/dev/null | tr '\0' '\n' | head -n 1)
            nsects=$(read_le_uint "$bin" $((off + 64)) 4)
            k=0
            while [ "$k" -lt "$nsects" ]; do
                sh=$((off + 72 + k * 80))
                sectname=$(dd if="$bin" bs=1 skip="$sh" count=16 2>/dev/null | tr '\0' '\n' | head -n 1)
                if [ "$segname" = "$want_seg" ] && [ "$sectname" = "$want_sect" ]; then
                    printf '%s %s %s %s %s\n' \
                        "$(read_le_uint "$bin" $((sh + 32)) 8)" \
                        "$(read_le_uint "$bin" $((sh + 40)) 8)" \
                        "$(read_le_uint "$bin" $((sh + 48)) 4)" \
                        "$(read_le_uint "$bin" $((sh + 52)) 4)" \
                        "$(read_le_uint "$bin" $((sh + 64)) 4)"
                    return 0
                fi
                k=$((k + 1))
            done
        fi
        [ "$cmdsize" -ge 8 ] || return 2
        off=$((off + cmdsize))
        i=$((i + 1))
    done
    echo "link: macho: section '$want_seg,$want_sect' not found" >&2
    return 2
}

# invoke LLVM's Mach-O inspector from either an ordinary PATH installation
# (Linux integration legs) or Xcode's selected toolchain (native macOS legs)
macho_objdump() {
    if command -v llvm-objdump >/dev/null 2>&1; then
        llvm-objdump "$@"
    elif command -v xcrun >/dev/null 2>&1; then
        xcrun llvm-objdump "$@"
    else
        echo "link: macho: llvm-objdump is required" >&2
        return 2
    fi
}

# count_byte_sequence <file> <hex-bytes> — print how many times the byte sequence
# (given as contiguous hex digit pairs) occurs in the file. od reads the whole
# file once; awk slides a window over it, so overlapping occurrences all count.
count_byte_sequence() {
    od -An -v -tu1 "$1" | awk -v want="$2" '
        BEGIN {
            n = length(want) / 2
            for (i = 1; i <= n; i++) {
                pair = substr(want, i * 2 - 1, 2)
                need[i] = strtonum("0x" pair)
            }
        }
        { for (i = 1; i <= NF; i++) b[++len] = $i }
        END {
            hits = 0
            for (i = 1; i + n - 1 <= len; i++) {
                ok = 1
                for (j = 1; j <= n && ok; j++) { if (b[i + j - 1] != need[j]) ok = 0 }
                if (ok) hits++
            }
            print hits
        }
    '
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

# resolve_readobj — the llvm-readobj twin of resolve_objdump
resolve_readobj() {
    if command -v llvm-readobj >/dev/null 2>&1; then echo llvm-readobj; return 0; fi
    newest=$(compgen -c 'llvm-readobj-' 2>/dev/null | sort -t- -k3 -n | tail -1)
    [ -n "$newest" ] && { echo "$newest"; return 0; }
    return 1
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

