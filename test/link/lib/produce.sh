# produce.sh — observable producers, sourced by run.sh.
#
# a producer turns a built case artifact into a normalized text observable on
# stdout, which run.sh diffs against the golden. the producer is the only thing
# that varies between verification modes; the golden-diff core does not change.
#
# CHOOSING ONE. this list says what each producer observes. doc/design/test-observability.md says
# what running a program structurally CANNOT observe, maps each of those classes to
# the surface that can, and states which of them needed an integration test at all -
# most of the historical cases did not. most producers below exist because of one of
# its entries, so read the two together, and read it first before adding a case.
#
# producers:
#   exec        — run the program, observe its stdout (native / qemu).
#   relro-fault — run the program and report whether its write to a RELRO'd .rodata
#                 slot faulted (SIGSEGV -> exit 139); the --pie RELRO runtime guard.
#   field       — coreutils (od/dd) reads of known header offsets, for format facts
#                 execution cannot observe (PE ASLR bit, macho PIE bit). dispatched
#                 on the artifact's own magic, so it is independent of how the case
#                 maps to a leg. no LLVM; reads little-endian fields (every runner is LE).
#   embed-dedup — count each embedded asset's byte sequence in the final image and
#                 then run the program. byte-identical #[embed] content from several
#                 modules must occupy ONE placement (#2541); an in-program address
#                 comparison cannot see the duplicate copies, only a file scan can.
#   macho-framing— walk a Mach-O executable's load commands and report how the image
#                 is framed: the __PAGEZERO span, __TEXT's base, and the section
#                 commands on every segment, plus which entry command it carries.
#                 the static and dyld-loaded layouts must agree on all of it except
#                 the entry command (#2599). no LLVM; od/dd reads only.
#   macho-mod-init— assert a __DATA,__mod_init_func section reaches the image with the
#                 section TYPE dyld dispatches on, and on a darwin runner run the image
#                 and require that dyld actually called it (#2637).
#   macho-sections— assert a linked darwin image kept its inputs' (segment, section)
#                 names, the identity a name-based runtime scan needs: __objc_classlist
#                 concatenated contiguously from two objects, __objc_imageinfo's flags
#                 intact, and a typed, rebased __mod_init_func entry (#2606). structural
#                 only: its objc metadata is synthetic, and libobjc faults on it (#2637).
#   macho-header-span — assert a Mach-O image whose section count outgrows the one-page
#                 header reservation still frames itself: the header and its load
#                 commands fit inside __TEXT's first mapped span, and every section
#                 command lands in the segment that declares it (#2606).
#   macho-signed— structurally resolve the three x86_64 immediate-store displacements
#                 and compare them with independent initialized-data markers, proving
#                 SIGNED_1/_2/_4 each linked to its exact target without a mac runner.
#   macho-got   — resolve real clang GOT_LOAD plus non-relaxable GOT32/GOT64 sites,
#                 prove local/import slot deduplication and exact dyld rebase/bind
#                 rows, and execute the same PIE on the native Intel macOS leg.
#   macho-abs-bind — resolve real clang absolute 64-bit references to libSystem
#                 data imports, prove each cell carries its own in-place dyld
#                 bind row (no GOT indirection), and execute the same PIE on the
#                 native Intel macOS leg.
#   pe-imports  — like field, but walks the PE import directory and reports every
#                 `<dll>:<symbol>` binding (#2510). PE-only; the observable for
#                 two-level-namespace attribution, which execution cannot check.
#   macho-imports — the Mach-O analogue: reports every `<dylib>:<symbol>` bind row
#                 from both the bind and lazy-bind tables. Mach-O is a two-level
#                 namespace too, so which dylib provides each import is a fact
#                 only the emitted metadata carries (#2636). Cross-built on the
#                 linux leg and read with llvm-objdump host-side, like macho-got.
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
#   debuginfo   — build the case with and without `-g` (run.sh builds both) and
#                 assert over the artifacts: llvm-dwarfdump --verify accepts the `-g`
#                 image, `-g` is loadable-byte additive, discarded weak templates do
#                 not alias live line/location metadata, and live names symbolize.
#                 requires llvm-dwarfdump, llvm-symbolizer, and readelf; it runs only
#                 on the ELF debug-info legs, which install them.
#   symtab      — read the linked image's `.symtab` and report which functions it
#                 names, at what addresses and with what binding (#2772). a plain
#                 build carries one; a profiler, a debugger and a crash dumper all
#                 read it, and nothing the program computes depends on it. requires
#                 llvm-readelf.
#   elf-needed  — read the ELF `.dynamic` section's DT_NEEDED entries and report
#                 the dependency list, sorted, or "none" for a fully static image.
#                 requires readelf.
#   link-provider - report whether a local link dependency's symbol is defined. requires nm.
#   gdb-session — drive a real gdb batch session over the `-g` artifact and report
#                 where it stopped, which frame it named, and what each local read
#                 back as (#2756). debuginfo checks the ENCODING; this checks what a
#                 consumer of that encoding actually does with it. requires gdb.
#   varloc-fbreg— build the case with and without `-g` (run.sh builds both) and cross
#                 the two halves of a frame-slot variable location against each other:
#                 every `DW_OP_fbreg` offset must name an address the function's own
#                 emitted code addresses from the same frame-base register. reading
#                 the offset alone cannot see #2759, because a producer that derives
#                 it a second way is self-consistent under the bug. requires
#                 llvm-dwarfdump and llvm-objdump.
#   riscv-pcrel — run the program, then read the linked riscv64 image and the foreign
#                 clang object it links: every psABI `auipc` + `%pcrel_lo(.Lpcrel_hiK)`
#                 pair must resolve to an aligned address outside the text range
#                 (#2797). mach's own objects spelled both halves the same way, so
#                 they were self-consistently wrong and only a foreign object could
#                 tell. requires llvm-objdump.
#   spirv-val-vulkan — validate every `.spv` module a finished-module target
#                 delivered with the Khronos validator (spirv-tools), under the
#                 Vulkan environment a module carrying entry points must satisfy.
#                 the target links nothing, so there is no binary to run and the
#                 module tree is the artifact; the external validator is what makes
#                 this a validity check rather than a round trip through mach's own
#                 reader.
#   spirv-image — validate the module tree AND report the image / sampler surface the
#                 emitter produced for it (#2794). spirv-val cannot see this case's
#                 subject either: an `OpTypeImage` whose Dim operand came out 2D
#                 where the program wrote `sampler3d`, or whose Sampled operand came
#                 out 2 rather than 1, is a perfectly valid module describing a
#                 different image, and one whose sample instruction reads some other
#                 handle validates as long as the types line up. So this prints every
#                 OpTypeImage with each of its seven operands, every
#                 OpTypeSampledImage and OpTypeSampler with what they wrap, every
#                 UniformConstant variable with the descriptor set and binding
#                 decorating it, and every OpSampledImage / OpImageSampleImplicitLod
#                 with its operand count and the TYPE of the handle it reads - which
#                 is the assertion that ties the sample back to the declaration.
#                 handle types carry no spirv-dis friendly name, so ids are replaced
#                 by the shape they were declared with; every id in the output is a
#                 description, never a number. needs spirv-val and spirv-dis.
# build-fails is a run mode but not a producer: it asserts the compile is REJECTED
# and takes the compiler's 'error:' diagnostic as the observable. it is handled in
# run.sh (there is no artifact to run), noted here for discoverability.

# the directory this file lives in (test/link/lib), used to find flat_loader.c. resolved
# from the sourced path so it does not depend on run.sh's variables.
_produce_lib_dir=$(CDPATH= cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
_flat_loader_bin=


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

# produce_exec <engine> <leg> <binary>
# runs the built binary and forwards its stdout as the observable. native mode runs
# it directly; qemu mode runs it under the matching qemu-user (qemu_bin). the
# producer's exit status is the program's, so a crash (nonzero) fails the case.
#
# on failure run.sh discards this producer's stdout and shows only its stderr, so a
# failing run reports the status and the program's own stdout there instead - without
# it a crashed exec case says only "producer exit 139" and throws away how far the
# program got (#2593).
produce_exec() {
    engine=$1
    target=$2
    bin=$3
    out=$(mktemp)
    err=$(mktemp)
    run_captured "$engine" "$target" "$bin" "$out" "$err" || { rm -f "$out" "$err"; return 1; }
    if [ "$run_status" -ne 0 ]; then
        report_run_failure "exec" "$run_status" "$run_out"
        [ -s "$err" ] && sed 's/^/    /' "$err" >&2
        rm -f "$out" "$err"
        return "$run_status"
    fi
    cat "$err" >&2
    cat "$out"
    rm -f "$out" "$err"
}

# produce_relro_fault <engine> <leg> <binary>
# runs the built binary (expected to write to a relocated constant's RELRO'd .rodata
# storage) and reports whether that write faulted. after the --pie startup mprotects
# the region read-only, the write must raise SIGSEGV, which surfaces as exit 128+11=139
# both natively and under qemu-user; any other status means the region stayed writable.
# the program's own stdout is discarded - the observable is purely the fault fact - so
# this is a runtime (exec-like) producer sharing one target-independent golden.
#
# on the expected fault the program's output is noise and stays out of the way. on any
# other status it is evidence - how far the program got before the write that should
# have faulted did not - so it is reported to stderr, leaving the observable the golden
# pins byte-identical (#2593).
produce_relro_fault() {
    engine=$1
    target=$2
    bin=$3
    out=$(mktemp)
    run_captured "$engine" "$target" "$bin" "$out" || { rm -f "$out"; return 1; }
    ec=$run_status
    rm -f "$out"
    if [ "$ec" -eq 139 ]; then
        echo "relro_write=faulted"
    else
        echo "relro_write=exit$ec"
        report_run_failure "relro-fault: expected the RELRO write to fault, but the program" \
            "$ec" "$run_out"
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

# produce_embed_dedup <engine> <leg> <binary>
# Count each embedded asset's byte sequence in the FINAL IMAGE, then run the
# program.
#
# The observable has to be the emitted bytes. Three modules embed byte-identical
# content, and #2518 already made their addresses compare equal WITHIN a module,
# so an in-program address comparison cannot see the defect #2541 is about: the
# object boundary lost the embed marker and the linker concatenated the same bytes
# once per module, leaving copies nobody references. Counting occurrences in the
# file is what distinguishes one placement from several.
#
# The fourth asset differs in its last byte only, so it also pins the other half of
# the contract: content that is not identical must stay distinct, and a scan that
# merged on length or section alone would report it missing.
produce_embed_dedup() {
    engine=$1
    target=$2
    bin=$3

    shared=9E2D41770BC35AE13684F21C60ABD94E73
    other=9E2D41770BC35AE13684F21C60ABD94E74

    shared_hits=$(count_byte_sequence "$bin" "$shared") || return 2
    other_hits=$(count_byte_sequence "$bin" "$other") || return 2

    [ "$shared_hits" -eq 1 ] || {
        echo "link: embed-dedup: the shared 17-byte asset occurs $shared_hits times, expected one placement for all three modules" >&2
        return 1
    }
    [ "$other_hits" -eq 1 ] || {
        echo "link: embed-dedup: the differing 17-byte asset occurs $other_hits times, expected exactly its own placement" >&2
        return 1
    }

    produce_exec "$engine" "$target" "$bin"
    echo "shared_placements=$shared_hits"
    echo "distinct_placements=$other_hits"
}

# produce_macho_framing <engine> <leg> <binary>
# Walk a Mach-O executable's load commands and report how the image is FRAMED: the
# __PAGEZERO span, __TEXT's base, whether __PAGEZERO ends exactly where __TEXT
# begins, then one line per LC_SEGMENT_64 naming its section commands, and finally
# which entry command the image carries.
#
# The framing is what a static (LC_UNIXTHREAD) and a dyld-loaded (LC_MAIN) image
# must have IN COMMON - the entry command is the only line that may differ between
# the two goldens (#2599, where the non-PIE image was based a page below
# DARWIN_BASE_ADDR, carried a __PAGEZERO one page short of 4 GiB, and emitted no
# section commands at all, leaving `llvm-objdump -d` with nothing to disassemble).
# Addresses other than the base are deliberately left out: they move with the
# program's size, and the fact under test is the framing, not the layout.
produce_macho_framing() {
    bin=$3
    magic=$(read_le_uint "$bin" 0 4)
    [ "$magic" = "4277009103" ] || { echo "link: macho-framing: not a 64-bit mach-o (magic $magic)" >&2; return 2; }
    ncmds=$(read_le_uint "$bin" 16 4)

    pagezero_size=
    text_vmaddr=
    entry=none
    segs=$(mktemp)
    off=32
    i=0
    while [ "$i" -lt "$ncmds" ]; do
        cmd=$(read_le_uint "$bin" "$off" 4)
        cmdsize=$(read_le_uint "$bin" $((off + 4)) 4)
        case "$cmd" in
            25)   # LC_SEGMENT_64
                name=$(dd if="$bin" bs=1 skip=$((off + 8)) count=16 2>/dev/null | tr '\0' '\n' | head -n 1)
                vmaddr=$(read_le_uint "$bin" $((off + 24)) 8)
                vmsize=$(read_le_uint "$bin" $((off + 32)) 8)
                nsects=$(read_le_uint "$bin" $((off + 64)) 4)
                [ "$name" = "__PAGEZERO" ] && pagezero_size=$vmsize
                [ "$name" = "__TEXT" ] && text_vmaddr=$vmaddr
                sects=
                k=0
                while [ "$k" -lt "$nsects" ]; do
                    sh=$((off + 72 + k * 80))
                    sname=$(dd if="$bin" bs=1 skip="$sh" count=16 2>/dev/null | tr '\0' '\n' | head -n 1)
                    if [ -z "$sects" ]; then sects=$sname; else sects="$sects,$sname"; fi
                    k=$((k + 1))
                done
                [ -n "$sects" ] || sects=-
                printf 'seg %s nsects=%s sects=%s\n' "$name" "$nsects" "$sects" >>"$segs"
                ;;
            5)          entry=LC_UNIXTHREAD ;;   # LC_UNIXTHREAD
            2147483688) entry=LC_MAIN ;;         # LC_MAIN (0x80000028)
        esac
        [ "$cmdsize" -ge 8 ] || { rm -f "$segs"; echo "link: macho-framing: zero-size load command" >&2; return 2; }
        off=$((off + cmdsize))
        i=$((i + 1))
    done

    [ -n "$pagezero_size" ] || { rm -f "$segs"; echo "link: macho-framing: no __PAGEZERO" >&2; return 2; }
    [ -n "$text_vmaddr" ]   || { rm -f "$segs"; echo "link: macho-framing: no __TEXT" >&2; return 2; }

    printf 'pagezero_vmsize=0x%x\n' "$pagezero_size"
    printf 'text_vmaddr=0x%x\n' "$text_vmaddr"
    printf 'pagezero_abuts_text=%s\n' "$(( pagezero_size == text_vmaddr ))"
    cat "$segs"
    rm -f "$segs"
    printf 'entry=%s\n' "$entry"
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

# produce_macho_mod_init <engine> <leg> <binary>
# Assert that a __DATA,__mod_init_func section reaches the image as one dyld will
# actually run, then on a darwin runner run it and require that dyld did.
#
# Presence and even a correct pointer are not enough. dyld finds an image's
# initializers by scanning for sections whose TYPE is S_MOD_INIT_FUNC_POINTERS, so
# a section carrying the right name, the right alignment, and a correctly rebased
# pointer at the right address is still never called if the link emitted the
# default S_REGULAR type - which is exactly what #2637 was. Every structural fact
# below held while the initializer silently did not run, so the structural half
# constrains the image and the darwin run is what settles it.
produce_macho_mod_init() {
    engine=$1
    target=$2
    bin=$3

    fields=$(macho_section_fields "$bin" __DATA __mod_init_func) || return 2
    set -- $fields
    mi_addr=$1; mi_size=$2; mi_align=$4; mi_flags=$5

    [ $((mi_flags & 0xFF)) -eq 9 ] || {
        echo "link: macho-mod-init: __mod_init_func has section type $((mi_flags & 0xFF)), expected 9 (S_MOD_INIT_FUNC_POINTERS); dyld dispatches on the type and never runs any other" >&2
        return 1
    }
    [ "$mi_size" -eq 8 ] || {
        echo "link: macho-mod-init: __mod_init_func is $mi_size bytes, expected one 8-byte entry" >&2
        return 1
    }
    [ "$mi_align" -ge 3 ] || {
        echo "link: macho-mod-init: __mod_init_func is 2^$mi_align aligned, expected at least pointer alignment" >&2
        return 1
    }

    # the entry is an in-image pointer, so a PIE must slide it: with no rebase row
    # dyld would call whatever the unslid address happens to land on.
    mi_hex=$(printf '0x%X' "$mi_addr")
    rebases=$(macho_objdump --macho --rebase "$bin") || return 2
    [ "$(printf '%s\n' "$rebases" | grep -F -c "$mi_hex")" -eq 1 ] || {
        echo "link: macho-mod-init: __mod_init_func has no rebase row at $mi_hex" >&2
        return 1
    }

    if [ "$target" = darwin-x86_64 ]; then
        out=$(mktemp)
        run_captured native "$target" "$bin" "$out" || { rm -f "$out"; return 1; }
        rm -f "$out"
        if [ "$run_status" -ne 0 ]; then
            report_run_failure "macho-mod-init: the native PIE" "$run_status" "$run_out"
            return 1
        fi
        [ "$run_out" = "init ok" ] || {
            echo "link: macho-mod-init: dyld did not run the __mod_init_func entry" >&2
            diff_expected_actual "init ok" "$run_out"
            return 1
        }
    fi

    echo "mod_init_func=type9-rebased"
}

# produce_macho_header_span <engine> <leg> <binary>
# Assert that a darwin image whose load commands exceed one page linked, and that
# its header agrees with the span the linker reserved below the first segment.
#
# The Mach-O header carries an 80-byte section_64 per named section and an
# LC_LOAD_DYLIB per dependency, so it grows with the image; `sizeofcmds` is a
# uint32 and the format caps it nowhere near a page. A fixed one-page reservation
# refused the link outright (#2690).
#
# Size alone would be a weak observable. The reservation is what places __TEXT:
# the header occupies the gap between the image base and the first segment, so a
# writer that widened the gap on its own would slide __TEXT, and __PAGEZERO with
# it, below the base - the defect of #2599, which every size check would still
# pass. The framing is therefore asserted alongside: __PAGEZERO spans the whole low
# region, __TEXT is based exactly where __PAGEZERO ends, and the header sits at
# file offset 0 inside it.
#
# The page size is read from the image's own cputype rather than passed in, since
# the leg here is linux and darwin maps 16 KiB pages on arm64 and 4 KiB on x86_64.
produce_macho_header_span() {
    bin=$3
    magic=$(read_le_uint "$bin" 0 4)
    [ "$magic" = "4277009103" ] || { echo "link: macho-header-span: not a 64-bit mach-o (magic $magic)" >&2; return 2; }

    cputype=$(read_le_uint "$bin" 4 4)
    case "$cputype" in
        16777223) page=4096  ;;   # CPU_TYPE_X86_64
        16777228) page=16384 ;;   # CPU_TYPE_ARM64
        *) echo "link: macho-header-span: unexpected cputype $cputype" >&2; return 2 ;;
    esac

    ncmds=$(read_le_uint "$bin" 16 4)
    sizeofcmds=$(read_le_uint "$bin" 20 4)

    pagezero_size=
    text_vmaddr=
    text_fileoff=
    sect_addr=
    sect_fileoff=
    dylibs=0
    off=32
    i=0
    while [ "$i" -lt "$ncmds" ]; do
        cmd=$(read_le_uint "$bin" "$off" 4)
        cmdsize=$(read_le_uint "$bin" $((off + 4)) 4)
        case "$cmd" in
            25)   # LC_SEGMENT_64
                name=$(dd if="$bin" bs=1 skip=$((off + 8)) count=16 2>/dev/null | tr '\0' '\n' | head -n 1)
                if [ "$name" = "__PAGEZERO" ]; then
                    pagezero_size=$(read_le_uint "$bin" $((off + 32)) 8)
                fi
                if [ "$name" = "__TEXT" ]; then
                    text_vmaddr=$(read_le_uint "$bin" $((off + 24)) 8)
                    text_fileoff=$(read_le_uint "$bin" $((off + 40)) 8)
                    nsects=$(read_le_uint "$bin" $((off + 64)) 4)
                    [ "$nsects" -gt 0 ] || { echo "link: macho-header-span: __TEXT carries no section" >&2; return 2; }
                    sect_addr=$(read_le_uint "$bin" $((off + 72 + 32)) 8)
                    sect_fileoff=$(read_le_uint "$bin" $((off + 72 + 48)) 4)
                fi
                ;;
            12) dylibs=$((dylibs + 1)) ;;   # LC_LOAD_DYLIB
        esac
        [ "$cmdsize" -ge 8 ] || { echo "link: macho-header-span: zero-size load command" >&2; return 2; }
        off=$((off + cmdsize))
        i=$((i + 1))
    done

    [ -n "$pagezero_size" ] || { echo "link: macho-header-span: no __PAGEZERO" >&2; return 2; }
    [ -n "$text_vmaddr" ]   || { echo "link: macho-header-span: no __TEXT" >&2; return 2; }

    # the reservation is the gap the header fills: from __TEXT's start to its first
    # section's content.
    reservation=$((sect_addr - text_vmaddr))

    printf 'sizeofcmds_over_one_page=%s\n' "$(( sizeofcmds > page ))"
    printf 'multiple_dylib_loads=%s\n' "$(( dylibs > 1 ))"
    printf 'reservation_whole_pages=%s\n' "$(( reservation > 0 && reservation % page == 0 ))"
    printf 'header_fits_reservation=%s\n' "$(( 32 + sizeofcmds <= reservation ))"
    printf 'first_section_fileoff_is_reservation=%s\n' "$(( sect_fileoff == reservation ))"
    printf 'pagezero_vmsize=0x%x\n' "$pagezero_size"
    printf 'text_vmaddr=0x%x\n' "$text_vmaddr"
    printf 'text_fileoff=%s\n' "$text_fileoff"
    printf 'pagezero_abuts_text=%s\n' "$(( pagezero_size == text_vmaddr ))"
}

# produce_macho_sections <engine> <leg> <binary>
# Assert that a linked darwin image kept the (segment, section) identity of its
# inputs, which is what a name-based runtime scan needs: libobjc walks
# __DATA,__objc_classlist and reads __DATA,__objc_imageinfo, and dyld calls every
# pointer in __DATA,__mod_init_func (#2606). Merging inputs by KIND alone drops the
# names and the sections do not exist in the output at all, even though their bytes
# are still mapped somewhere inside __data.
#
# Presence is not enough, so this also reads the bytes back. Two clang objects each
# contribute one __objc_classlist pointer, and the section must be ONE contiguous
# 16-byte pointer-aligned run holding both markers in link order - not two sections
# sharing a name, which is what a per-input section would produce and what libobjc's
# single-array walk cannot read.
#
# This image is inspected and never RUN, deliberately. Its objc metadata is
# synthetic - a class list entry has to point at a real Objective-C class object,
# and building one needs the macOS SDK - so the markers that make concatenation
# order checkable here are not addresses at all. libobjc reads that list for real
# on darwin and dereferences every entry, so running this image faults before main
# (#2637). Executing a genuine __mod_init_func initializer is `macho-mod-init`,
# whose payload is valid content a runtime can act on correctly.
produce_macho_sections() {
    target=$2
    bin=$3

    fields=$(macho_section_fields "$bin" __DATA __objc_classlist) || return 2
    set -- $fields
    cl_size=$2; cl_off=$3; cl_align=$4
    [ "$cl_size" -eq 16 ] || {
        echo "link: macho-sections: __objc_classlist is $cl_size bytes, expected the two inputs concatenated into 16" >&2
        return 1
    }
    [ "$cl_align" -ge 3 ] || {
        echo "link: macho-sections: __objc_classlist is 2^$cl_align aligned, expected at least pointer alignment" >&2
        return 1
    }
    [ $((cl_off % 8)) -eq 0 ] || {
        echo "link: macho-sections: __objc_classlist lands at file offset $cl_off, not pointer-aligned" >&2
        return 1
    }
    marker_a=$(read_le_uint "$bin" "$cl_off" 8)
    marker_b=$(read_le_uint "$bin" $((cl_off + 8)) 8)
    [ "$marker_a" -eq 9734 ] && [ "$marker_b" -eq 9739 ] || {
        echo "link: macho-sections: __objc_classlist holds $marker_a,$marker_b; expected the probe-a then probe-b markers 9734,9739" >&2
        return 1
    }

    fields=$(macho_section_fields "$bin" __DATA __objc_imageinfo) || return 2
    set -- $fields
    ii_size=$2; ii_off=$3
    [ "$ii_size" -eq 8 ] || {
        echo "link: macho-sections: __objc_imageinfo is $ii_size bytes, expected 8" >&2
        return 1
    }
    ii_version=$(read_le_uint "$bin" "$ii_off" 4)
    ii_flags=$(read_le_uint "$bin" $((ii_off + 4)) 4)
    [ "$ii_version" -eq 0 ] && [ "$ii_flags" -eq 64 ] || {
        echo "link: macho-sections: __objc_imageinfo holds version=$ii_version flags=$ii_flags; libobjc validates these and the input set 0/64" >&2
        return 1
    }

    fields=$(macho_section_fields "$bin" __DATA __mod_init_func) || return 2
    set -- $fields
    mi_addr=$1; mi_size=$2; mi_align=$4; mi_flags=$5
    # the section TYPE is the low byte of the flags word, and dyld dispatches on it:
    # S_MOD_INIT_FUNC_POINTERS is 9 (#2637).
    [ $((mi_flags & 0xFF)) -eq 9 ] || {
        echo "link: macho-sections: __mod_init_func has section type $((mi_flags & 0xFF)), expected 9 (S_MOD_INIT_FUNC_POINTERS)" >&2
        return 1
    }
    [ "$mi_size" -eq 8 ] || {
        echo "link: macho-sections: __mod_init_func is $mi_size bytes, expected one 8-byte entry" >&2
        return 1
    }
    [ "$mi_align" -ge 3 ] || {
        echo "link: macho-sections: __mod_init_func is 2^$mi_align aligned, expected at least pointer alignment" >&2
        return 1
    }
    # the entry is an in-image pointer, so a PIE must slide it: without a rebase row
    # dyld would call whatever the unslid address happens to land on.
    mi_hex=$(printf '0x%X' "$mi_addr")
    rebases=$(macho_objdump --macho --rebase "$bin") || return 2
    [ "$(printf '%s\n' "$rebases" | grep -F -c "$mi_hex")" -eq 1 ] || {
        echo "link: macho-sections: __mod_init_func has no rebase row at $mi_hex" >&2
        return 1
    }

    echo "objc_classlist=concatenated-16-pointer-aligned"
    echo "objc_imageinfo=version0-flags64"
    echo "mod_init_func=present-rebased"
}

# produce_macho_signed <engine> <leg> <binary>
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
                print "link: macho-signed: expected one initialized target marker, found " found > "/dev/stderr"
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
                print "link: macho-signed: expected one store of each width, found " n1 "/" n2 "/" n4 > "/dev/stderr"
                exit 2
            }
            printf "SIGNED_1 %.0f SIGNED_2 %.0f SIGNED_4 %.0f\n", v1, v2, v4
        }
    ') || return 2
    set -- $stores
    [ "$1" = SIGNED_1 ] && [ "$3" = SIGNED_2 ] && [ "$5" = SIGNED_4 ] || return 2

    [ "$2" -eq "$target1" ] || { echo "link: macho-signed: SIGNED_1 resolved to $2, expected $target1" >&2; return 1; }
    [ "$4" -eq "$target2" ] || { echo "link: macho-signed: SIGNED_2 resolved to $4, expected $target2" >&2; return 1; }
    [ "$6" -eq "$target4" ] || { echo "link: macho-signed: SIGNED_4 resolved to $6, expected $target4" >&2; return 1; }
    echo "SIGNED_1=exact"
    echo "SIGNED_2=exact"
    echo "SIGNED_4=exact"
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

# produce_macho_got <engine> <leg> <binary>
#
# Decode the checked-in fixture's five instruction signatures from __TEXT. The
# two clang GOT_LOAD fields and the assembly GOT32/GOT64 fields independently
# resolve to one local and one imported slot. Then inspect the exact dyld rows:
# the initialized local slot must be rebased, while the zero-on-disk import slot
# must bind ___stderrp to libSystem. On the native Intel macOS leg the same PIE is
# also executed and its values compared byte-for-byte with the runtime contract.
#
# The two non-relaxable local signatures (GOT32 / GOT64) carry a `movq (%rax), %rax`
# the import ones do not: X86_64_RELOC_GOT resolves to the slot address, so reading
# the local int takes two loads where testing the imported pointer takes one (#2586).
produce_macho_got() {
    target=$2
    bin=$3

    flags=$(read_le_uint "$bin" 24 4)
    [ $((flags & 0x200000)) -ne 0 ] || {
        echo "link: macho-got: executable is not PIE" >&2
        return 1
    }

    text_fields=$(macho_segment_fields "$bin" __TEXT) || return 2
    set -- $text_fields
    text_vm=$1; text_file=$3; text_size=$4

    sites=$(od -An -v -tu1 -j "$text_file" -N "$text_size" "$bin" | awk -v vm="$text_vm" '
        function s32(i, u) {
            u = b[i] + 256*b[i+1] + 65536*b[i+2] + 16777216*b[i+3]
            return u >= 2147483648 ? u - 4294967296 : u
        }
        function s64(i, lo, hi, u) {
            lo = b[i] + 256*b[i+1] + 65536*b[i+2] + 16777216*b[i+3]
            hi = b[i+4] + 256*b[i+5] + 65536*b[i+6] + 16777216*b[i+7]
            u = lo + 4294967296*hi
            return hi >= 2147483648 ? u - 18446744073709551616 : u
        }
        { for (i = 1; i <= NF; i++) b[++n] = $i }
        END {
            nl = ni = gl = gi = g64 = 0
            for (i = 1; i <= n; i++) {
                pc = vm + i - 1
                if (i + 10 <= n && b[i] == 72 && b[i+1] == 139 && b[i+2] == 5 &&
                    b[i+7] == 139 && b[i+8] == 0 && b[i+9] == 93 && b[i+10] == 195) {
                    nl++; vl = pc + 7 + s32(i + 3)
                }
                if (i + 16 <= n && b[i] == 72 && b[i+1] == 139 && b[i+2] == 13 &&
                    b[i+7] == 49 && b[i+8] == 192 && b[i+9] == 72 && b[i+10] == 131 &&
                    b[i+11] == 57 && b[i+12] == 0 && b[i+13] == 15 && b[i+14] == 149 &&
                    b[i+15] == 192) {
                    ni++; vi = pc + 7 + s32(i + 3)
                }
                if (i + 12 <= n && b[i] == 72 && b[i+1] == 141 && b[i+2] == 5 &&
                    b[i+7] == 72 && b[i+8] == 139 && b[i+9] == 0 &&
                    b[i+10] == 139 && b[i+11] == 0 && b[i+12] == 195) {
                    gl++; vgl = pc + 7 + s32(i + 3)
                }
                if (i + 17 <= n && b[i] == 72 && b[i+1] == 141 && b[i+2] == 5 &&
                    b[i+7] == 72 && b[i+8] == 131 && b[i+9] == 56 && b[i+10] == 0 &&
                    b[i+11] == 15 && b[i+12] == 149 && b[i+13] == 192 && b[i+14] == 15 &&
                    b[i+15] == 182 && b[i+16] == 192 && b[i+17] == 195) {
                    gi++; vgi = pc + 7 + s32(i + 3)
                }
                if (i + 25 <= n && b[i] == 72 && b[i+1] == 184 && b[i+10] == 72 &&
                    b[i+11] == 141 && b[i+12] == 13 && b[i+13] == 249 && b[i+14] == 255 &&
                    b[i+15] == 255 && b[i+16] == 255 && b[i+17] == 72 && b[i+18] == 1 &&
                    b[i+19] == 200 && b[i+20] == 72 && b[i+21] == 139 && b[i+22] == 0 &&
                    b[i+23] == 139 && b[i+24] == 0 && b[i+25] == 195) {
                    g64++; vg64 = pc + 10 + s64(i + 2)
                }
            }
            if (nl != 1 || ni != 1 || gl != 1 || gi != 1 || g64 != 1) {
                print "link: macho-got: expected GOT sites 1/1/1/1/1, found " nl "/" ni "/" gl "/" gi "/" g64 > "/dev/stderr"
                exit 2
            }
            printf "%.0f %.0f %.0f %.0f %.0f\n", vl, vi, vgl, vgi, vg64
        }
    ') || return 2
    set -- $sites
    local_load=$1; import_load=$2; local_got=$3; import_got=$4; local_got64=$5

    [ "$local_load" -eq "$local_got" ] && [ "$local_load" -eq "$local_got64" ] || {
        echo "link: macho-got: local references did not deduplicate to one slot" >&2
        return 1
    }
    [ "$import_load" -eq "$import_got" ] || {
        echo "link: macho-got: import references did not deduplicate to one slot" >&2
        return 1
    }
    [ "$local_load" -ne "$import_load" ] || {
        echo "link: macho-got: local and imported symbols share a slot" >&2
        return 1
    }

    relro_fields=$(macho_segment_fields "$bin" __DATA_CONST) || return 2
    set -- $relro_fields
    relro_vm=$1; relro_file=$3; relro_size=$4
    [ "$local_load" -ge "$relro_vm" ] && [ "$local_load" -lt $((relro_vm + relro_size)) ] || {
        echo "link: macho-got: local slot is not in __DATA_CONST" >&2
        return 1
    }
    local_file=$((relro_file + local_load - relro_vm))
    local_value=$(read_le_uint "$bin" "$local_file" 8)
    [ "$local_value" -ne 0 ] || {
        echo "link: macho-got: local slot was not initialized" >&2
        return 1
    }

    got_fields=$(macho_segment_fields "$bin" __GOT) || return 2
    set -- $got_fields
    got_vm=$1; got_file=$3; got_size=$4
    [ "$import_load" -ge "$got_vm" ] && [ "$import_load" -lt $((got_vm + got_size)) ] || {
        echo "link: macho-got: import slot is not in __GOT" >&2
        return 1
    }
    import_file=$((got_file + import_load - got_vm))
    import_value=$(read_le_uint "$bin" "$import_file" 8)
    [ "$import_value" -eq 0 ] || {
        echo "link: macho-got: import slot is not zero before dyld binding" >&2
        return 1
    }

    local_hex=$(printf '0x%x' "$local_load")
    import_hex=$(printf '0x%x' "$import_load")
    rebases=$(macho_objdump --macho --rebase "$bin" | tr 'A-F' 'a-f') || return 2
    [ "$(printf '%s\n' "$rebases" | grep -F -c "$local_hex")" -eq 1 ] || {
        echo "link: macho-got: local slot has no unique rebase row" >&2
        return 1
    }
    binds=$(macho_objdump --macho --bind "$bin" | tr 'A-F' 'a-f') || return 2
    bind_row=$(printf '%s\n' "$binds" | grep -F "$import_hex" || true)
    [ "$(printf '%s\n' "$bind_row" | grep -c 'libSystem.*___stderrp')" -eq 1 ] || {
        echo "link: macho-got: import slot has no unique libSystem ___stderrp bind" >&2
        return 1
    }

    if [ "$target" = darwin-x86_64 ]; then
        out=$(mktemp)
        run_captured native "$target" "$bin" "$out" || { rm -f "$out"; return 1; }
        rm -f "$out"
        expected=$(printf '%s\n' \
            'local-load=40' 'local-got=40' 'local-got64=40' \
            'import-load=1' 'import-got=1')
        if [ "$run_status" -ne 0 ]; then
            report_run_failure "macho-got: the native PIE" "$run_status" "$run_out"
            return 1
        fi
        [ "$run_out" = "$expected" ] || {
            echo "link: macho-got: the native PIE ran to completion but computed wrong values" >&2
            diff_expected_actual "$expected" "$run_out"
            return 1
        }
    fi

    echo "PIE=1"
    echo "local_slots=deduplicated"
    echo "local_slot=rebase"
    echo "import_slots=deduplicated"
    echo "import_slot=libSystem-bind"
}

# produce_macho_imports <engine> <leg> <binary>
#
# Every `<dylib>:<symbol>` the image asks dyld to bind, from both the bind and the
# lazy-bind table, sorted. Mach-O records imports per dependency exactly as PE
# does, so this is the observable for attribution on darwin: a claim that failed
# to attribute does not merely bind to the wrong dylib, it fails the link, and a
# claim attributed to the wrong entry shows up here as the wrong dylib name.
#
# Both tables carry the dylib in the second-to-last column and the symbol in the
# last, so one rule reads either; a data row is recognized by its leading segment
# name, which no header line has.
#
# The sort is pinned to the C collation, as produce_pe_imports's is: a dylib list
# mixing cases (`libSystem`, `libcurses`) orders differently under a UTF-8 locale
# than under C, so an unpinned sort makes the golden depend on the runner's
# environment rather than on the emitted image.
produce_macho_imports() {
    _engine=$1
    _target=$2
    bin=$3

    binds=$(macho_objdump --macho --bind "$bin") || return 2
    lazy=$(macho_objdump --macho --lazy-bind "$bin") || return 2
    printf '%s\n%s\n' "$binds" "$lazy" |
        awk '$1 ~ /^__/ && NF >= 5 { print $(NF-1) ":" $NF }' |
        LC_ALL=C sort -u
}

# produce_macho_abs_bind <engine> <leg> <binary>
#
# Walk the dyld bind rows for the fixture's two absolute 64-bit references to
# libSystem's __stdoutp/__stderrp. Each must bind in place inside __DATA (the
# cell itself is bound - no __GOT slot), one row per site, and the on-disk cell
# content must be the relocation's zero addend. On the native Intel macOS leg
# the same PIE is executed and its dereference contract compared exactly.
produce_macho_abs_bind() {
    target=$2
    bin=$3

    flags=$(read_le_uint "$bin" 24 4)
    [ $((flags & 0x200000)) -ne 0 ] || {
        echo "link: macho-abs-bind: executable is not PIE" >&2
        return 1
    }

    data_fields=$(macho_segment_fields "$bin" __DATA) || return 2
    set -- $data_fields
    data_vm=$1; data_file=$3; data_size=$4

    binds=$(macho_objdump --macho --bind "$bin" | tr 'A-F' 'a-f') || return 2

    check_slot() {
        sym=$1
        rows=$(printf '%s\n' "$binds" | grep -F "$sym" | grep -v "__got" || true)
        [ "$(printf '%s\n' "$rows" | grep -c .)" -eq 1 ] || {
            echo "link: macho-abs-bind: $sym does not have exactly one in-place bind row" >&2
            return 1
        }
        addr=$(printf '%s\n' "$rows" | awk '{for (i = 1; i <= NF; i++) if ($i ~ /^0x[0-9a-f]+$/) { print $i; exit }}')
        [ -n "$addr" ] || {
            echo "link: macho-abs-bind: $sym bind row carries no address" >&2
            return 1
        }
        [ $((addr)) -ge "$data_vm" ] && [ $((addr)) -lt $((data_vm + data_size)) ] || {
            echo "link: macho-abs-bind: $sym bind address is outside __DATA" >&2
            return 1
        }
        cell=$(read_le_uint "$bin" $((data_file + addr - data_vm)) 8)
        [ "$cell" -eq 0 ] || {
            echo "link: macho-abs-bind: $sym cell is not the zero addend on disk" >&2
            return 1
        }
        return 0
    }

    check_slot ___stdoutp || return 1
    check_slot ___stderrp || return 1

    if [ "$target" = darwin-x86_64 ]; then
        out=$(mktemp)
        run_captured native "$target" "$bin" "$out" || { rm -f "$out"; return 1; }
        rm -f "$out"
        if [ "$run_status" -ne 0 ]; then
            report_run_failure "macho-abs-bind: the native PIE" "$run_status" "$run_out"
            return 1
        fi
        [ "$run_out" = "abs-bind=1" ] || {
            echo "link: macho-abs-bind: the native PIE ran to completion but computed wrong values" >&2
            diff_expected_actual "abs-bind=1" "$run_out"
            return 1
        }
    fi

    echo "PIE=1"
    echo "out_slot=libSystem-bind-in-place"
    echo "err_slot=libSystem-bind-in-place"
    echo "slots=addend-zero"
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

# produce_pe_imports <engine> <leg> <binary>
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
        echo "link: pe-imports: not a PE32+ image (optional-header magic $magic)" >&2
        return 2
    fi

    imp_rva=$(read_le_uint "$bin" $((elfanew + 24 + 112 + 8)) 4)
    if [ "$imp_rva" -eq 0 ]; then
        echo "no-import-table"
        return 0
    fi
    imp_off=$(pe_rva_to_off "$bin" "$sec" "$nsec" "$imp_rva") || {
        echo "link: pe-imports: import directory RVA $imp_rva is in no section" >&2
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

# produce_pe_dllimport <engine> <leg> <binary>
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
        echo "link: pe-dllimport: not a PE32+ image (optional-header magic $magic)" >&2
        return 2
    fi

    imports=$(produce_pe_imports "$@") || return $?
    want_imports='kernel32.dll:GetTickCount
kernel32.dll:Sleep'
    if [ "$imports" != "$want_imports" ]; then
        echo "link: pe-dllimport: imports are '$imports', want two canonical exports" >&2
        return 2
    fi

    iat_dir=$((elfanew + 24 + 112 + 12 * 8))
    iat_rva=$(read_le_uint "$bin" "$iat_dir" 4)
    iat_size=$(read_le_uint "$bin" $((iat_dir + 4)) 4)
    if [ "$iat_rva" -eq 0 ] || [ "$iat_size" -ne 24 ]; then
        echo "link: pe-dllimport: IAT is RVA=$iat_rva size=$iat_size, want two entries" >&2
        return 2
    fi

    sleep_off=$(find_unique_hex "$bin" 4883ec28b907000000ff15) || {
        echo "link: pe-dllimport: qz_indirect Sleep signature is not unique" >&2
        return 2
    }
    tick_off=$(find_unique_hex "$bin" 904883c42848ff25) || {
        echo "link: pe-dllimport: qz_indirect GetTickCount signature is not unique" >&2
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
        echo "link: pe-dllimport: foreign targets $sleep_target/$tick_target miss IAT $iat_rva/$second_iat" >&2
        return 2
    fi

    echo "$imports"
    echo "iat_entries=2"
    echo "foreign_indirect_target=iat"
}

# produce_pe_local_import <engine> <leg> <binary>
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
        echo "link: pe-local-import: not a PE32+ image (optional-header magic $magic)" >&2
        return 2
    fi

    imports=$(produce_pe_imports "$@") || return $?
    iat_dir=$((elfanew + 24 + 112 + 12 * 8))
    iat_rva=$(read_le_uint "$bin" "$iat_dir" 4)
    iat_size=$(read_le_uint "$bin" $((iat_dir + 4)) 4)
    if [ "$imports" != "no-import-table" ] || [ "$iat_rva" -ne 0 ] || [ "$iat_size" -ne 0 ]; then
        echo "link: pe-local-import: local_add leaked into loader imports/IAT ($imports, $iat_rva/$iat_size)" >&2
        return 2
    fi

    first_suffix=$(find_unique_hex "$bin" ffc04883c428c3) || {
        echo "link: pe-local-import: first indirect-call suffix is not unique" >&2
        return 2
    }
    second_suffix=$(find_unique_hex "$bin" 05785634124883c428c3) || {
        echo "link: pe-local-import: second indirect-call suffix is not unique" >&2
        return 2
    }
    direct_suffix=$(find_unique_hex "$bin" 83c0024883c428c3) || {
        echo "link: pe-local-import: direct-call suffix is not unique" >&2
        return 2
    }
    local_off=$(find_unique_hex "$bin" 8d4123c36666662e0f1f840000000000) || {
        echo "link: pe-local-import: local definition marker is not unique" >&2
        return 2
    }

    first_patch=$((first_suffix - 4))
    second_patch=$((second_suffix - 4))
    direct_patch=$((direct_suffix - 4))
    if [ "$(read_le_uint "$bin" $((first_patch - 2)) 2)" -ne 5631 ] \
        || [ "$(read_le_uint "$bin" $((second_patch - 2)) 2)" -ne 5631 ] \
        || [ "$(read_le_uint "$bin" $((direct_patch - 1)) 1)" -ne 232 ]; then
        echo "link: pe-local-import: recovered displacement is not owned by the expected call opcode" >&2
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
        echo "link: pe-local-import: duplicate aliases target different cells $first_target/$second_target" >&2
        return 2
    fi
    if [ "$direct_target" -ne "$local_rva" ] || [ "$first_target" -eq "$local_rva" ]; then
        echo "link: pe-local-import: direct/indirect targets do not preserve one level of indirection" >&2
        return 2
    fi

    cell_off=$(pe_rva_to_off "$bin" "$sec" "$nsec" "$first_target") || {
        echo "link: pe-local-import: pointer-cell RVA $first_target is in no section" >&2
        return 2
    }
    image_base=$(read_le_uint "$bin" $((elfanew + 24 + 24)) 8)
    cell_value=$(read_le_uint "$bin" "$cell_off" 8)
    if [ "$cell_value" -ne $((image_base + local_rva)) ]; then
        echo "link: pe-local-import: cell contains $cell_value, expected local VA $((image_base + local_rva))" >&2
        return 2
    fi

    reloc_dir=$((elfanew + 24 + 112 + 5 * 8))
    reloc_rva=$(read_le_uint "$bin" "$reloc_dir" 4)
    reloc_size=$(read_le_uint "$bin" $((reloc_dir + 4)) 4)
    reloc_off=$(pe_rva_to_off "$bin" "$sec" "$nsec" "$reloc_rva") || {
        echo "link: pe-local-import: base-relocation directory is missing" >&2
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
            echo "link: pe-local-import: malformed base-relocation block" >&2
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
                echo "link: pe-local-import: unexpected base-relocation type $type" >&2
                return 2
            fi
            i=$((i + 1))
        done
        cursor=$((cursor + block_size))
    done
    if [ "$cursor" -ne "$reloc_size" ] || [ "$dir64_count" -ne 1 ] \
        || [ "$dir64_rva" -ne "$first_target" ]; then
        echo "link: pe-local-import: DIR64 set is $dir64_count at $dir64_rva, expected the one cell $first_target" >&2
        return 2
    fi

    echo "imports=0"
    echo "local_cells=1"
    echo "indirect_targets=local_cell"
    echo "direct_target=local_definition"
    echo "cell_value=local_definition"
    echo "base_reloc=local_cell"
}

# produce_pe_exceptions <engine> <leg> <binary>
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
        echo "link: pe-exceptions: not a PE32+ image (optional-header magic $magic)" >&2
        return 2
    fi

    exc_rva=$(read_le_uint "$bin" $((elfanew + 24 + 112 + 3 * 8)) 4)
    exc_size=$(read_le_uint "$bin" $((elfanew + 24 + 112 + 3 * 8 + 4)) 4)
    if [ "$exc_rva" -eq 0 ] || [ "$exc_size" -eq 0 ] || [ $((exc_size % 12)) -ne 0 ]; then
        echo "link: pe-exceptions: missing or malformed exception directory" >&2
        return 2
    fi
    exc_off=$(pe_rva_to_off "$bin" "$sec" "$nsec" "$exc_rva") || {
        echo "link: pe-exceptions: exception RVA $exc_rva is in no section" >&2
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
            echo "link: pe-exceptions: begin RVA $begin is in no section" >&2
            return 2
        }
        sig=$(dd if="$bin" bs=1 skip="$begin_off" count=9 2>/dev/null | od -An -tx1 | tr -d ' \n')
        if [ "$sig" = "4883ec28b928000000" ]; then
            found=$((found + 1))
            if [ $((end - begin)) -ne 22 ]; then
                echo "link: pe-exceptions: qz_answer row has length $((end - begin)), want 22" >&2
                return 2
            fi
            unwind_off=$(pe_rva_to_off "$bin" "$sec" "$nsec" "$unwind") || {
                echo "link: pe-exceptions: unwind RVA $unwind is in no section" >&2
                return 2
            }
            unwind_bytes=$(dd if="$bin" bs=1 skip="$unwind_off" count=8 2>/dev/null | od -An -tx1 | tr -d ' \n')
            if [ "$unwind_bytes" != "0104010004420000" ]; then
                echo "link: pe-exceptions: qz_answer UNWIND_INFO is $unwind_bytes" >&2
                return 2
            fi
        fi
        i=$((i + 1))
    done
    if [ "$found" -ne 1 ]; then
        echo "link: pe-exceptions: found $found qz_answer runtime rows, want 1" >&2
        return 2
    fi

    echo "exception_directory=present"
    echo "foreign_runtime=published"
    echo "foreign_length=22"
    echo "foreign_unwind=valid"
}

# produce_pe_codeview <engine> <leg> <binary>
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
        echo "link: pe-codeview: not a PE32+ image (optional-header magic $magic)" >&2
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
        echo 'link: pe-codeview: missing .text or complete .debug$S section' >&2
        return 2
    fi

    code_off=$(find_unique_hex "$bin" 4883ec28b928000000) || {
        echo "link: pe-codeview: codeview_answer signature is not unique" >&2
        return 2
    }
    code_rva=$(pe_off_to_rva "$bin" "$sec" "$nsec" "$code_off") || {
        echo "link: pe-codeview: codeview_answer is outside the PE sections" >&2
        return 2
    }
    if [ "$code_rva" -lt "$text_rva" ]; then
        echo "link: pe-codeview: code precedes its .text section" >&2
        return 2
    fi
    expected=$((code_rva - text_rva))

    secrel_a=$(read_le_uint "$bin" $((debug_off + 0x68)) 4)
    section_a=$(read_le_uint "$bin" $((debug_off + 0x6c)) 2)
    secrel_b=$(read_le_uint "$bin" $((debug_off + 0xac)) 4)
    section_b=$(read_le_uint "$bin" $((debug_off + 0xb0)) 2)
    if [ "$secrel_a" -ne "$expected" ] || [ "$secrel_b" -ne "$expected" ]; then
        echo "link: pe-codeview: SECREL fields $secrel_a/$secrel_b, want $expected" >&2
        return 2
    fi
    if [ "$section_a" -ne "$text_index" ] || [ "$section_b" -ne "$text_index" ]; then
        echo "link: pe-codeview: SECTION fields $section_a/$section_b, want $text_index" >&2
        return 2
    fi
    if [ "$text_rva" -ne 4096 ] || [ "$code_rva" -lt 8192 ] || [ "$expected" -lt 4096 ]; then
        echo "link: pe-codeview: fixture no longer covers first-section alignment padding" >&2
        return 2
    fi

    echo "codeview_section=present"
    echo "secrel_fields=exact"
    echo "section_fields=exact"
    echo "alignment_prefix=covered"
}

# produce_field <engine> <leg> <binary>
# emits the canonical structural fact for the artifact's format, dispatched on its
# leading magic bytes so the reader is independent of the leg the case ran on.
produce_field() {
    bin=$3
    magic=$(dd if="$bin" bs=1 count=4 2>/dev/null | od -An -tx1 | tr -d ' \n')
    case "$magic" in
        7f454c46)  field_elf "$bin" ;;      # 0x7F 'E' 'L' 'F' -> ELF
        4d5a*)     field_pe "$bin" ;;       # 'MZ' DOS stub -> PE/COFF
        cffaedfe*) field_macho "$bin" ;;    # MH_MAGIC_64 (little-endian)
        *) echo "link: field: unrecognized binary format (magic $magic)" >&2; return 2 ;;
    esac
}

# produce_relro <engine> <leg> <binary>
# emits the ELF RELRO fact: relro=1 when a PT_GNU_RELRO program header (p_type
# 0x6474e552) is present, else relro=0. read host-side from the program headers
# (e_phoff u64 @32, e_phentsize u16 @54, e_phnum u16 @56), never executing the binary,
# so it works on every leg including qemu. ELF-only (RELRO is an ELF concept).
produce_relro() {
    bin=$3
    magic=$(dd if="$bin" bs=1 count=4 2>/dev/null | od -An -tx1 | tr -d ' \n')
    if [ "$magic" != "7f454c46" ]; then
        echo "link: relro: not an ELF binary (magic $magic)" >&2; return 2
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

# produce_flat_loader <engine> <leg> <binary>
# loads the freestanding raw image through the C loader (built once, cached) and
# reports the image's exit status as the observable. any stdout the image writes
# flows first. a loader-infrastructure failure (no cc, mmap denied) returns nonzero.
produce_flat_loader() {
    bin=$3
    if [ -z "$_flat_loader_bin" ]; then
        _flat_loader_bin=$(mktemp -d)/flat_loader
        if ! cc -O2 -o "$_flat_loader_bin" "$_produce_lib_dir/flat_loader.c" 2>&1; then
            echo "link: flat-loader: could not build the C loader (cc required)" >&2
            _flat_loader_bin=
            return 2
        fi
    fi
    if "$_flat_loader_bin" "$bin"; then ec=0; else ec=$?; fi
    printf 'exit=%d\n' "$ec"
}

# produce_built <engine> <leg> <binary>
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
        echo "link: built: artifact missing or empty" >&2
        return 2
    fi
}

# produce_spirv_val_vulkan <engine> <leg> <binary>
# as produce_spirv_val, but validates against the VULKAN environment rather than
# the universal one. the two are different contracts and a module cannot satisfy
# both: a library module declares the Linkage capability so a consumer can find its
# exported functions, and Vulkan forbids that capability outright; a shader module
# carries entry points and no linkage at all. so a case picks the environment its
# module is actually meant for, and the stricter Vulkan rules — the fragment
# stage's mandatory origin, the compute stage's mandatory workgroup size — are
# genuinely checked rather than skipped by validating everything universally.
produce_spirv_val_vulkan() {
    spirv_val_env "$3" vulkan1.3
}

# spirv_val_env <binary> <target-env>
# shared body: glob the case's output root (a finished-module target has no linked
# binary, so the delivery is the module tree) and run spirv-val over each `.spv`.
# an EXTERNAL validator is the point — mach reading back its own bytes proves
# self-consistency, not validity. the observable is the module count plus the
# verdict, so a build that silently stopped emitting fails on the count rather than
# passing vacuously.
spirv_val_env() {
    out_dir=$(dirname "$1")
    env_arg=$2
    if ! command -v spirv-val >/dev/null 2>&1; then
        echo "link: spirv-val: the validator is not installed (spirv-tools)" >&2
        return 2
    fi
    n=0
    for m in $(find "$out_dir" -name '*.spv' | sort); do
        if [ -n "$env_arg" ]; then
            spirv-val --target-env "$env_arg" "$m" || return 1
        else
            spirv-val "$m" || return 1
        fi
        n=$((n + 1))
    done
    if [ "$n" -eq 0 ]; then
        echo "link: spirv-val: the build delivered no .spv module" >&2
        return 2
    fi
    if [ -n "$env_arg" ]; then
        printf 'modules=%d validator=clean env=%s\n' "$n" "$env_arg"
        return 0
    fi
    printf 'modules=%d validator=clean\n' "$n"
}

# produce_spirv_image <engine> <leg> <binary>
# validate the delivered module tree and report its image / sampler surface.
#
# THE OPERANDS ARE THE OBSERVABLE. `OpTypeImage` carries seven of them, and every
# one of the wrong values a bug produces is a module spirv-val accepts: a Dim of 2D
# where the source wrote `sampler3d`, an Arrayed of 0 where it wrote `array`, a
# Sampled of 2 (a storage image) where the sampled form was meant. So each is
# printed by name rather than counted, and so is the descriptor the handle is bound
# at - a set or binding that came out 0 binds the wrong descriptor and validates.
#
# Ids are replaced by the shape behind them throughout. An image type has no
# spirv-dis friendly name, so `%22` in a sample instruction says nothing on its own;
# resolving it to `sampledimage(image(float,2D,...))` is what makes "this sample
# reads THAT handle" an assertion rather than a claim about two numbers that
# renumber whenever anything else in the module changes.
produce_spirv_image() {
    out_dir=$(dirname "$3")
    if ! command -v spirv-val >/dev/null 2>&1; then
        echo "link: spirv-image: the validator is not installed (spirv-tools)" >&2
        return 2
    fi
    if ! command -v spirv-dis >/dev/null 2>&1; then
        echo "link: spirv-image: the disassembler is not installed (spirv-tools)" >&2
        return 2
    fi
    n=0
    for m in $(find "$out_dir" -name '*.spv' | sort); do
        dis=$(spirv-dis --no-header "$m") || return 1
        rel=${m#"$out_dir"/}
        if printf '%s\n' "$dis" | grep -q '^ *OpEntryPoint '; then
            kind=shader
            spirv-val --target-env vulkan1.3 "$m" || return 1
            env=vulkan1.3
        else
            kind=library
            spirv-val "$m" || return 1
            env=universal
        fi
        printf 'module=%s kind=%s env=%s validator=clean\n' "$rel" "$kind" "$env"
        printf '%s\n' "$dis" | awk '
            # the capabilities a dimensionality demands. a 1D or arrayed-cube image
            # declared without its capability is an invalid module, and one declared
            # WITH a capability nothing needs is a module a driver may refuse to
            # load, so both directions are in the observable.
            $1 == "OpCapability" { printf "  capability %s\n", $2; next }

            # the descriptor a handle is bound at. decorations precede the types and
            # variables they apply to, so one pass suffices.
            $1 == "OpDecorate" && $3 == "DescriptorSet" { dset[$2] = $4; next }
            $1 == "OpDecorate" && $3 == "Binding"       { dbind[$2] = $4; next }

            # "%4 = OpTypeImage %float 2D 0 0 0 1 Unknown" - all seven operands.
            $3 == "OpTypeImage" {
                d = "image(" $4 "," $5 ",depth=" $6 ",arrayed=" $7 ",ms=" $8 ",sampled=" $9 "," $10 ")"
                desc[$1] = d
                printf "  type %s\n", d
                next
            }
            $3 == "OpTypeSampledImage" {
                u = desc[$4]; if (u == "") { u = $4 }
                desc[$1] = "sampledimage(" u ")"
                printf "  type %s\n", desc[$1]
                next
            }
            $3 == "OpTypeSampler" { desc[$1] = "sampler"; printf "  type sampler\n"; next }

            # a pointer to a handle, so a variable can be reported by what it points
            # at rather than by spirv-dis, which names it after an id.
            $3 == "OpTypePointer" {
                u = desc[$5]
                if (u != "") { desc[$1] = "ptr(" $4 "," u ")"; ptr[$1] = u }
                next
            }

            # "%9 = OpVariable %_ptr_UniformConstant_7 UniformConstant" - only the
            # handle-typed ones; every other variable belongs to another feature.
            $3 == "OpVariable" && ptr[$4] != "" {
                printf "  binding set=%s binding=%s storage=%s type=%s\n",
                    (($1 in dset) ? dset[$1] : "<none>"),
                    (($1 in dbind) ? dbind[$1] : "<none>"),
                    $5, ptr[$4]
                next
            }

            # "%22 = OpLoad %5 %7" - remember the result type so a sample can be
            # reported by the handle it reads rather than by an operand id.
            $3 == "OpLoad" { rty[$1] = $4; next }

            $3 == "OpSampledImage" {
                rty[$1] = $4
                i = desc[rty[$5]]; if (i == "") { i = "<unknown>" }
                s = desc[rty[$6]]; if (s == "") { s = "<unknown>" }
                printf "  combine image=%s sampler=%s operands=%d\n", i, s, NF - 4
                next
            }
            $3 == "OpImageSampleImplicitLod" {
                h = desc[rty[$5]]; if (h == "") { h = "<unknown>" }
                printf "  sample result=%s handle=%s operands=%d\n", $4, h, NF - 4
                next
            }
        '
        n=$((n + 1))
    done
    if [ "$n" -eq 0 ]; then
        echo "link: spirv-image: the build delivered no .spv module" >&2
        return 2
    fi
    printf 'modules=%d\n' "$n"
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

# produce_debuginfo <engine> <leg> <nog_binary> <g_binary>
# the binary-inspection producer for the debuginfo case kind (#2039): asserts, purely
# host-side over the artifacts run.sh built with and without `-g`, that (1) the
# standard structural validator accepts the whole `-g` image and which warning classes
# it reports while doing so, (1b) a real consumer
# (addr2line, i.e. libbfd) decodes the line table without a diagnostic and resolves the
# entry point to a name, (2) `-g` is loadable-byte additive, and (3) duplicate generic,
# comptime-value, and pack instances retain
# one live, symbolizable DIE while each discarded copy carries DWARF's dead-code address,
# has no line-table sequence at the winner, and has no location list at the winner.
# the facts are ISA-independent, so the golden is shared. requires llvm-dwarfdump,
# llvm-symbolizer, readelf, and addr2line on the leg; a missing tool is a hard error.
produce_debuginfo() {
    nog=$3
    g=$4
    dd_tool=$(resolve_dwarfdump) || {
        echo "link: debuginfo: llvm-dwarfdump not found (install the 'llvm' package)" >&2; return 2
    }
    sym_tool=$(resolve_symbolizer) || {
        echo "link: debuginfo: llvm-symbolizer not found (install the 'llvm' package)" >&2; return 2
    }
    command -v readelf >/dev/null 2>&1 || {
        echo "link: debuginfo: readelf not found (install 'binutils')" >&2; return 2
    }
    command -v addr2line >/dev/null 2>&1 || {
        echo "link: debuginfo: addr2line not found (install 'binutils')" >&2; return 2
    }

    # --verify EXITS ZERO ON WARNINGS (#2755), so reading only its status collapsed "no
    # diagnostics at all" onto "no errors, and a wall of warnings". That is how a
    # validator stops validating: the next real warning lands in a stream nobody reads.
    # The observable is therefore the stream. `errors` is reported first because an
    # error subsumes a warning, and each residual warning TEXT is listed - sorted,
    # deduplicated, with the per-CU `[0x...]` offset elided - so a failure names itself
    # in the diff instead of reading `warnings`.
    #
    # ONE warning class is expected and filtered, with its reason: DWARF 5 §6.2.4
    # numbers file entries from 0 while §6.2.2 still gives the line state machine's
    # `file` register an initial value of 1, and binutils resolves that in favour of
    # §6.2.2. So a single-file CU must declare its source at slot 0 AND at slot 1 or
    # libbfd rejects the whole section ("mangled line number section (bad file number)",
    # #2582) - measured on binutils 2.47 against clang's own single-entry `-gdwarf-5`
    # output as well as ours. gcc emits the duplicate and llvm-dwarfdump warns on gcc's
    # output identically. It is also validator-version dependent: llvm-dwarfdump 18 does
    # not report it and 22 does, so leaving it in the stream would make the golden a
    # statement about the runner's llvm package. Every OTHER warning, of any class,
    # still fails the case.
    dd_expected='\.debug_line\[.*\]\.prologue\.file_names\[1\] is a duplicate of file_names\[0\]'
    dd_out=$("$dd_tool" --verify "$g" 2>&1)
    # `|| true` because an empty result is the expected outcome of a filter rather than
    # a failure, and run.sh runs under `set -e`.
    dd_warn=$(printf '%s\n' "$dd_out" | sed -n 's/^warning: //p' \
        | { grep -v -E "$dd_expected" || true; } \
        | sed -e 's/\[0x[0-9a-fA-F]*\]/[]/g' | sort -u)
    if printf '%s\n' "$dd_out" | grep -q '^error:'; then
        echo "dwarfdump_verify=errors"
    elif [ -n "$dd_warn" ]; then
        echo "dwarfdump_verify=warnings"
    else
        echo "dwarfdump_verify=clean"
    fi
    if [ -n "$dd_warn" ]; then
        printf '%s\n' "$dd_warn" | while IFS= read -r w; do echo "dwarfdump_warning=$w"; done
    fi

    # CONSUMER-SIDE DECODE (#2582). --verify above checks structural and reference
    # integrity; it does NOT check that the line program decodes against the file table
    # the way a consumer reads it, so it accepted a .debug_line that binutils rejected
    # outright ("mangled line number section (bad file number)") on every CU. addr2line
    # is the cheapest standard consumer of that decode - the same libbfd path perf, gdb,
    # and most crash symbolizers reach - so its stderr is the observable, verbatim when
    # non-empty so a regression names itself in the diff rather than reading `errors`.
    # the entry point is the address because every leg's image has one at a known place.
    entry=$(readelf -hW "$g" 2>/dev/null | awk '/Entry point address:/{print $NF}')
    a2l_err=$(addr2line -f -e "$g" "$entry" 2>&1 >/dev/null | sed -n '1p')
    a2l_fn=$(addr2line -f -e "$g" "$entry" 2>/dev/null | sed -n '1p')
    if [ -n "$a2l_err" ]; then
        echo "addr2line_stderr=$a2l_err"
    else
        echo "addr2line_stderr=clean"
    fi
    if [ -n "$a2l_fn" ] && [ "$a2l_fn" != "??" ]; then
        echo "addr2line_entry=resolved"
    else
        echo "addr2line_entry=unresolved"
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
        # a substring match, not equality: once a `.symtab` exists (#2772) a real
        # symbolizer prefers the ELF symbol table's linkage name over DWARF's
        # DW_AT_name for the function-name field, so the resolved text is the
        # mangled form (e.g. `_M7dbgcase7genericN11identI3i64E`), not the bare
        # source identifier - and the mangling scheme itself is due to change
        # (the dotted-name rewrite). either way the source identifier is still
        # IN there, so that is the fact this asserts, printed back as the
        # semantic label rather than the raw resolved text so the golden names
        # what was checked instead of freezing today's mangling spelling.
        symbol=missing
        if [ -n "$3" ]; then
            resolved=$("$sym_tool" --obj="$g" "$3" | sed -n '1p')
            case "$resolved" in *"$want"*) symbol=$want ;; esac
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

# produce_elf-needed <engine> <leg> <binary>
# the ELF DT_NEEDED observable: one `name` line per shared-library dependency the
# `.dynamic` section actually carries, sorted, or the literal line "none" when the
# image has no PT_DYNAMIC segment at all (a fully static link). mach's own ELF
# writer does not emit a conventional `.dynstr`/`.dynsym` section pair the way a
# hand-parsed byte reader could walk directly (`.dynamic` is the only dynamic-linking
# section it emits), so this shells out to `readelf -d` rather than reimplementing
# ELF dynamic-section parsing; the same tool dependency `symtab` already requires.
produce_elf_needed() {
    b=$3
    command -v readelf >/dev/null 2>&1 || {
        echo "link: elf-needed: readelf not found (install the 'binutils' package)" >&2; return 2
    }
    # readelf prints an explanatory sentence (exit 0) rather than empty output
    # when a static image has no PT_DYNAMIC segment at all, so the absence check
    # is on the filtered NEEDED lines, not on readelf's raw output.
    needed=$(readelf -d "$b" 2>/dev/null | awk -F'[][]' '/\(NEEDED\)/ { print $2 }' | sort)
    if [ -z "$needed" ]; then
        echo "none"
        return 0
    fi
    printf '%s\n' "$needed"
}

# produce_link_provider <engine> <leg> <binary>
produce_link_provider() {
    b=$3
    command -v nm >/dev/null 2>&1 || {
        echo "link: link-provider: nm not found (install the 'binutils' package)" >&2; return 2
    }
    if nm "$b" 2>/dev/null | grep -qE ' T provider_marker$'; then
        echo "provider_marker=defined"
    else
        echo "provider_marker=missing"
    fi
}

# produce_symtab <engine> <leg> <binary>
# the ELF `.symtab` observable (#2772): a PLAIN build (no `-g`, no special flag -
# the shape a shipped release binary actually has) now carries a real function
# symbol table, which test/link/cases/debuginfo cannot speak to at all (DWARF is a
# `-g`-only concern). requires nm, readelf, and addr2line; a missing tool is a
# hard error, the same contract produce_debuginfo already uses for its own
# validators.
produce_symtab() {
    b=$3
    command -v nm >/dev/null 2>&1 || {
        echo "link: symtab: nm not found (install the 'binutils' package)" >&2; return 2
    }
    command -v readelf >/dev/null 2>&1 || {
        echo "link: symtab: readelf not found (install the 'binutils' package)" >&2; return 2
    }
    command -v addr2line >/dev/null 2>&1 || {
        echo "link: symtab: addr2line not found (install the 'binutils' package)" >&2; return 2
    }

    sh_out=$(readelf -SW "$b" 2>/dev/null)
    if printf '%s\n' "$sh_out" | grep -qE '\.symtab +SYMTAB'; then
        echo "symtab_present=yes"
    else
        echo "symtab_present=no"
    fi
    if printf '%s\n' "$sh_out" | grep -qE '\.strtab +STRTAB'; then
        echo "strtab_present=yes"
    else
        echo "strtab_present=no"
    fi

    # `nm` (the standard "does this binary have symbols at all" tool) finds both
    # the fixture's functions as defined (T) symbols: `main` (an explicit
    # `#[symbol("main")]`, unmangled) and `burn` (mangled - matched by substring,
    # not exact name, since the mangling scheme is not this case's concern).
    nm_out=$(nm "$b" 2>/dev/null)
    if printf '%s\n' "$nm_out" | grep -qE ' T main$'; then
        echo "nm_main=defined"
    else
        echo "nm_main=missing"
    fi
    if printf '%s\n' "$nm_out" | grep -qE ' T .*burn'; then
        echo "nm_burn=defined"
    else
        echo "nm_burn=missing"
    fi

    # mid-function resolution: the address at burn's st_value PLUS HALF of its
    # st_size must still resolve to burn - the check `st_size` is right, not just
    # present, and the one most likely to be skipped (a symbol table with entries
    # and no real sizes looks fine under `nm` and only fails a profiler later).
    # readelf -sW dumps EVERY symbol table in the file, and a shared object's
    # `.dynsym` (#2807) carries the same global function with `st_size` always 0
    # (that table has never had a size writer - out of this case's scope) ahead
    # of `.symtab` in the listing; matching the first hit anywhere would silently
    # grab the wrong table's zero-size entry and read as "no symbol" rather than
    # the real fact under test, so the scan is confined to the `.symtab` table by
    # its own "Symbol table '.symtab'" banner line.
    burn_line=$(readelf -sW "$b" 2>/dev/null | awk -v want="'.symtab'" '
        /^Symbol table / { insym = ($0 ~ want); next }
        insym && /burn/ && / FUNC / { print; exit }
    ')
    burn_val=$(printf '%s\n' "$burn_line" | awk '{ print $2 }')
    burn_size=$(printf '%s\n' "$burn_line" | awk '{ print $3 }')
    if [ -n "$burn_val" ] && [ -n "$burn_size" ] && [ "$burn_size" -gt 0 ]; then
        mid=$(( 0x$burn_val + burn_size / 2 ))
        resolved=$(addr2line -f -e "$b" "$(printf '0x%x' "$mid")" 2>/dev/null | sed -n '1p')
        case "$resolved" in
            *burn*) echo "midfunc_resolve=burn" ;;
            *)      echo "midfunc_resolve=other" ;;
        esac
    else
        echo "midfunc_resolve=no-symbol"
    fi

    # byte-additivity (the property test/link/cases/debuginfo's elf_seg_identical
    # proves for DWARF, here read directly off the load segments rather than
    # from a before/after diff, since this symbol table is unconditional - there
    # is no "before" build to compare against): every PT_LOAD's file extent must
    # end at or before .symtab's file offset, so a loader - which reads only
    # PT_LOAD - never sees a byte the symbol table touched.
    # readelf -SW's leading "[ N]" is two whitespace-split fields ("[" and "N]"),
    # so the section name is $3 and the file offset is $6.
    symtab_off_hex=$(printf '%s\n' "$sh_out" | awk '$3 == ".symtab" { print $6; exit }')
    last_load_end=0
    while read -r typ off _va _pa filesz _memsz _flg _align; do
        [ "$typ" = "LOAD" ] || continue
        seg_end=$(( off + filesz ))
        if [ "$seg_end" -gt "$last_load_end" ]; then last_load_end=$seg_end; fi
    done <<PHDRS
$(readelf -lW "$b" 2>/dev/null | awk '/^  LOAD/ { print }')
PHDRS
    if [ -n "$symtab_off_hex" ] && [ "$last_load_end" -le "$(( 0x$symtab_off_hex ))" ]; then
        echo "symtab_after_loadable=yes"
    else
        echo "symtab_after_loadable=no"
    fi
}

# produce_gdb_session <engine> <leg> <nog_binary> <g_binary> <profile>
# the BEHAVIOURAL debugger observable (#2756): test/link/cases/debuginfo proves the `-g`
# image is structurally valid; it cannot prove gdb reports the right thing when a
# user actually breaks into it. this producer drives one real `gdb --batch` session
# over the `-g` artifact and normalizes its transcript into stop/frame/value facts -
# every one of them read off the fixture by hand before it went into the golden (see
# test/link/cases/debugger-gdb/src/main.mach and case.conf for the arithmetic).
#
# a missing gdb is a hard error, the same contract produce_debuginfo already uses for
# its own validators (llvm-dwarfdump, addr2line, ...): every runner this case's
# case.conf names (`linux` only - see that file for why the others are not) is
# expected to carry one, so a silent skip would hide a real coverage gap instead of
# reporting it.
#
# gdb's own text is not the observable: it carries a pid in the exit line, a
# `Breakpoint N` or `Breakpoint N.M` counter that depends on how many candidate
# addresses gdb resolved for a source line (an inlined body and its dead out-of-line
# twin both claim the same line, so this varies between profiles for reasons that
# have nothing to do with correctness), and - a real gap this case does NOT assert
# on - the caller frame's OWN argument list, which this compiler currently populates
# from unrelated inlined locals rather than `main`'s real parameters (found while
# building this case; tracked separately, not asserted here because it is not what
# #2756 asks this case to prove). the gdb script below prints a `SENTINEL:<name>`
# echo ahead of each fact so the normalizer below can key off it instead of gdb's own
# formatting, and extracts only the function name and source line from a frame
# line, never its argument list.
#
# THE #2779 PROBES ARE DEBUG-ONLY. Both bugs #2779 fixed are specific to opt0's own
# location and line-table construction and do not reproduce at opt2 (verified by
# hand: `dies`/`staysalive` auto-inline at release exactly like `addone` does, and a
# breakpoint's `next` there steps clean out to the caller frame - a same-frame
# before/after `v` comparison is meaningless once that happens, the same reason
# `addone` itself never carried this probe). So the golden this run compares against
# is now PER-PROFILE (`expect.$profile.txt`, see run.sh), and only a debug-profile
# session sets the extra breakpoints / walks the extra steps below - a release
# session's transcript, and its golden, are exactly what they were before #2779.
produce_gdb_session() {
    g=$4
    profile=${5:-}
    command -v gdb >/dev/null 2>&1 || {
        echo "link: gdb-session: gdb not found (install the 'gdb' package)" >&2; return 2
    }

    gdbtmp=$(mktemp -d)
    script=$gdbtmp/session.gdb
    {
        cat <<'GDBEOF'
set debuginfod enabled off
set pagination off
break main.mach:11
break main.mach:20
break main.mach:27
GDBEOF
        # line 22 is accumulate's own `ret total;` (Bug B); lines 39 / 50 are
        # `dies` / `staysalive`'s `val r: i64 = v + 1;` (Bug A pair). set before
        # `run` like every other breakpoint so their numbering (4/5/6) is stable
        # regardless of when execution first reaches them.
        if [ "$profile" = debug ]; then
            cat <<'GDBEOF'
break main.mach:22
break main.mach:39
break main.mach:50
GDBEOF
        fi
        cat <<'GDBEOF'
run
echo SENTINEL:addone_stop\n
frame 0
echo SENTINEL:addone_v\n
print v
echo SENTINEL:addone_caller\n
frame 1
continue
continue
continue
continue
continue
echo SENTINEL:accum_total\n
print total
echo SENTINEL:accum_i\n
print i
echo SENTINEL:step1\n
step
echo SENTINEL:step2\n
step
delete 2
continue
GDBEOF
        # Bug B: this `continue` reaches accumulate's OWN ret statement before
        # deadlocal's breakpoint, iff line 22 resolves to its real address rather
        # than accumulate's entry (the bug: a duplicate row AT the entry, which
        # this call already passed long before reaching this line, so a broken
        # build never stops here at all and falls through straight to deadlocal -
        # a wrong `accum_ret_stop_func` is exactly as loud a failure as a wrong
        # `accum_ret_total`).
        if [ "$profile" = debug ]; then
            cat <<'GDBEOF'
echo SENTINEL:accum_ret_stop\n
frame 0
echo SENTINEL:accum_ret_total\n
print total
continue
GDBEOF
        fi
        cat <<'GDBEOF'
echo SENTINEL:dead_stop\n
frame 0
echo SENTINEL:dead_v\n
print v
echo SENTINEL:dead_unused\n
print unused
echo SENTINEL:dead_caller\n
frame 1
delete 3
continue
GDBEOF
        # `delete 3` above (line 27, deadlocal's `ret`) before the `continue` that
        # runs past it: at release, adding `dies` / `staysalive` to this file gives
        # the optimizer more to fold, and it can unify a fragment of one of their
        # tail sequences with deadlocal's own - gdb then resolves breakpoint 3 to a
        # THIRD location inside `main`'s inlined call to `dies`, which stops the
        # session there instead of letting the program finish (found by hand while
        # adding these probes: the release transcript went from "exited normally"
        # to no exit line and empty stdout at all, with no other change). deadlocal
        # is called exactly once, so this deletion loses no coverage - the same
        # reasoning `delete 2` already applies to the loop breakpoint above.
        #
        # Bug A pair: `next` past `val r = v + 1;` and re-read `v`. `dies` must
        # report it unavailable once `r`'s storage takes over; `staysalive` - v
        # read again on its own `ret` line - must keep reading the real value.
        # Asserted together on purpose (see main.mach): a fix that reported
        # `<optimized out>` for BOTH would pass either probe run alone.
        if [ "$profile" = debug ]; then
            cat <<'GDBEOF'
echo SENTINEL:dies_v_before\n
print v
next
echo SENTINEL:dies_v_after\n
print v
continue
echo SENTINEL:stays_v_before\n
print v
next
echo SENTINEL:stays_v_after\n
print v
continue
GDBEOF
        fi
    } >"$script"

    transcript=$gdbtmp/transcript.txt
    gdb --batch -q -x "$script" "$g" >"$transcript" 2>&1

    # the program's own stdout is ground truth for the values gdb is asked to read
    # back: a=addone's result, b=accumulate's, c=deadlocal's, d=dies's, e=staysalive's
    # (main.mach calls all five at both profiles, so this line needs no profile
    # branch even though only debug walks the gdb-side d/e probes below). cross-
    # checking it against the golden's hand-computed values is what would catch a
    # normalizer bug that made every gdb-side assertion vacuously agree with itself.
    stdout=$(grep -E '^[a-e]=[0-9]+$' "$transcript" | paste -sd, -)
    echo "program_stdout=$stdout"
    if grep -q 'exited normally' "$transcript"; then
        echo "exit=normal"
    else
        echo "exit=abnormal"
    fi

    # state machine over the transcript: a `SENTINEL:<name>` line names the fact the
    # NEXT matching line carries. frame lines (`_stop`/`_caller`) yield two facts,
    # function and line, deliberately excluding the argument list (see header). a
    # `print` result is the text after `$N = `. a `step` target is the source line
    # number gdb echoes ahead of the line's own text.
    cur=
    while IFS= read -r line; do
        case "$line" in
            SENTINEL:*) cur=${line#SENTINEL:}; continue ;;
        esac
        [ -n "$cur" ] || continue
        case "$cur" in
            *_stop|*_caller)
                m=$(printf '%s\n' "$line" | sed -E -n 's/^#[01]  (0x[0-9a-f]+ in )?([A-Za-z_][A-Za-z0-9_]*) \(.*\) at .*:([0-9]+)$/\2 \3/p')
                if [ -n "$m" ]; then
                    echo "${cur}_func=${m% *}"
                    echo "${cur}_line=${m#* }"
                    cur=
                fi
                ;;
            step1|step2)
                m=$(printf '%s\n' "$line" | sed -E -n 's/^([0-9]+)\t.*/\1/p')
                if [ -n "$m" ]; then
                    echo "${cur}_line=$m"
                    cur=
                fi
                ;;
            *)
                m=$(printf '%s\n' "$line" | sed -E -n 's/^\$[0-9]+ = (.*)$/\1/p')
                if [ -n "$m" ]; then
                    echo "${cur}=$m"
                    cur=
                fi
                ;;
        esac
    done <"$transcript"

    rm -rf "$gdbtmp"
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

# produce_riscv_pcrel <engine> <leg> <binary>
# the riscv64 pc-relative HI/LO PAIR observable (#2797): the values a clang-built
# object loads through psABI hi/lo pairs, plus two facts about what was emitted.
#
# the values first, because the defect's signature is a believable zero: a pooled
# scale factor read from the wrong address is 0.0, and every product is then exactly
# 0. a case that only checked the exit status was green against it, which is how this
# survived as an unexplained CI-only failure of two other cases.
#
# then two emitted facts, each answering a way the value check could go quiet:
#
#   label-pairs=yes|no  whether the case's OWN C object still spells a low half the
#                       psABI way (`%pcrel_lo` naming a local label at the auipc,
#                       rather than naming the target the way mach's back end does).
#                       clang folds a `const` array into immediates and emits no pair
#                       at all; without this the fixture could stop containing the
#                       shape it exists to test and stay green forever
#   text-relative=<n>   over the WHOLE linked image: float loads reached through an
#                       `auipc`-materialized base whose resolved address lands inside
#                       the text range. a float constant is never in .text, so the
#                       answer is 0 for any correct link, and it is a property rather
#                       than an assertion about these three calls - it keeps meaning
#                       the same thing under whatever the next relaxation change does.
#                       the pre-fix linker resolved these pairs a few bytes off their
#                       own auipc, which is squarely inside .text
produce_riscv_pcrel() {
    engine=$1
    target=$2
    bin=$3
    out=$(mktemp)
    err=$(mktemp)
    run_captured "$engine" "$target" "$bin" "$out" "$err" || { rm -f "$out" "$err"; return 1; }
    if [ "$run_status" -ne 0 ]; then
        report_run_failure "riscv-pcrel" "$run_status" "$run_out"
        [ -s "$err" ] && sed 's/^/    /' "$err" >&2
        rm -f "$out" "$err"
        return "$run_status"
    fi
    cat "$err" >&2
    cat "$out"
    rm -f "$out" "$err"

    tool=$(resolve_objdump) || {
        echo "link: riscv-pcrel: llvm-objdump not found (install the 'llvm' package)" >&2
        return 2
    }
    dir=$(dirname "$(dirname "$(dirname "$bin")")")
    probe="$dir/out/link/build/obj/probe.o"
    if [ ! -f "$probe" ]; then
        echo "link: riscv-pcrel: no probe object at $probe" >&2; return 2
    fi
    "$tool" -d -r --no-show-raw-insn "$probe" | riscv_pcrel_label_scan
    # no --mattr override: the image now describes itself completely enough for the
    # disassembler to configure itself. it needed one twice over - e_flags did not
    # advertise the compressed extension the image contained (mach#2813), and the
    # `.riscv.attributes` arch string was derived from a per-ISA constant describing
    # what mach's own encoder emits rather than from what the linked image ended up
    # holding, which is what the compressed FLOAT loads (`c.fld`) read (mach#2828).
    # both are now derived from the image, so an override here would be hiding whether
    # they still are. without a correct answer from one of them those loads decode as
    # `<unknown>` and the property silently measures nothing
    "$tool" -d --no-show-raw-insn "$bin" | riscv_pcrel_image_scan
}

# riscv_pcrel_label_scan — read a `-d -r` object disassembly and report whether any
# `R_RISCV_PCREL_LO12_*` in it uses the psABI's label spelling.
#
# clang names the low half `%pcrel_lo(.Lpcrel_hiK)` - a label AT the paired auipc,
# LLVM's fixed prefix for it - where mach's own back end names the target on both
# halves. so the prefix is exactly the discriminator, and no cross-referencing of
# relocation sites is needed to tell the two spellings apart.
riscv_pcrel_label_scan() {
    awk '
    /R_RISCV_PCREL_LO12/ && /\.Lpcrel_hi/ { found = 1 }
    END { print "label-pairs=" (found ? "yes" : "no") }
    '
}

# riscv_pcrel_image_scan — read a linked-image disassembly and report two independent
# properties of every load reached through an `auipc`-materialized base.
#
#   misaligned      the resolved address is not a multiple of the access width. this
#                   is the one that CATCHES the #2797 defect: a low half resolved
#                   against the wrong pc lands an arbitrary byte count off its symbol,
#                   and the fixture's own repros report 8 (debug) and 4 (release)
#   text-relative   the resolved address lands inside the text range. a float or data
#                   constant is never in .text, so a correct link answers 0 - but so
#                   does the #2797 defect, which resolves to a wrong address in .data
#                   rather than onto code. it is kept as a true invariant that fails
#                   closed for a DIFFERENT mistake (a pair resolved onto instructions,
#                   which is what a bad relaxation or a wrong section base produces),
#                   and it is reported honestly as a second number rather than being
#                   described as the one doing the work
#
# a group is tracked per destination register and ends when that register is written
# again, which is the whole lifetime a hi/lo pair has. hex is parsed by hand rather
# than through gawk's strtonum: the runners' `awk` is mawk, which has no such
# function, and every other scan here is written to the same constraint.
riscv_pcrel_image_scan() {
    awk '
    function hex(s,   i, c, v, p) {
        sub(/^0x/, "", s)
        v = 0
        for (i = 1; i <= length(s); i++) {
            c = substr(s, i, 1)
            p = index("0123456789abcdef", tolower(c))
            if (p == 0) { return v }
            v = v * 16 + (p - 1)
        }
        return v
    }
    # a disassembled immediate, printed as an already-signed `-0x2bc` or `0x18`
    function signed(s) {
        if (substr(s, 1, 1) == "-") { return -hex(substr(s, 2)) }
        return hex(s)
    }
    /^[[:space:]]*[0-9a-f]+:/ {
        addr = hex(substr($1, 1, length($1) - 1))
        if (seen == 0 || addr < lo_addr) { lo_addr = addr }
        if (addr > hi_addr) { hi_addr = addr }
        seen = 1
        op = $2
        if (op == "auipc") {
            rd = $3; sub(/,$/, "", rd)
            base[rd] = addr + hex($4) * 4096
            live[rd] = 1
            next
        }
        # `addi a1, a1, 0x18` / `mv a1, a1` - the low half that finishes a base
        # POINTER. the pair materializes an address here rather than completing an
        # access, and every load off the result belongs to the same pair.
        if (op == "addi" || op == "mv") {
            rd = $3; sub(/,$/, "", rd)
            rs = $4; sub(/,$/, "", rs)
            if (live[rs] == 1) {
                d = (op == "mv") ? "0" : $5
                base[rd] = base[rs] + signed(d)
                live[rd] = 1
                next
            }
            if (rd in live) { live[rd] = 0 }
            next
        }
        # any load reached through such a base: the pair resolves to where it reads
        if (op == "fld" || op == "ld" || op == "flw" || op == "lw" || op == "lwu") {
            m = $4
            if (match(m, /\(.*\)$/)) {
                r = substr(m, RSTART + 1, RLENGTH - 2)
                d = substr(m, 1, RSTART - 1)
                if (live[r] == 1) {
                    a = base[r] + signed(d)
                    w = (op == "fld" || op == "ld") ? 8 : 4
                    np++
                    if (a % w != 0) { bad++ }
                    resolved[np] = a
                }
            }
            rd = $3; sub(/,$/, "", rd)
            if (rd in live) { live[rd] = 0 }
            next
        }
        # anything else that writes a register ends its group. the destination is the
        # first operand of every RV64 form that writes one; a store or a branch names
        # a source there instead, which only ends a group early and never extends one.
        rd = $3
        sub(/,$/, "", rd)
        if (rd in live) { live[rd] = 0 }
        next
    }
    END {
        intext = 0
        for (i = 1; i <= np; i++) {
            if (resolved[i] >= lo_addr && resolved[i] <= hi_addr) { intext++ }
        }
        print "pcrel-loads=" (np + 0 > 0 ? "yes" : "no")
        print "misaligned=" bad + 0
        print "text-relative=" intext
    }
    '
}



# produce_varloc_fbreg <engine> <leg> <nog_binary> <g_binary>
# the FRAME-SLOT VARIABLE LOCATION observable (#2759): does a `DW_OP_fbreg` offset name
# an address the emitted code actually uses for that slot.
#
# the offset alone is not evidence. the producer and the encoder each turn one layout
# fact - the bytes the prologue reserves between the frame pointer and the slot region -
# into an address, and a producer that derives it a second way is perfectly
# self-consistent while pointing at the wrong bytes. it agreed with the encoder on
# x86-64 and aarch64, whose reservation is 0, and was wrong by 16 to 200 bytes on every
# riscv64 function, which is why nothing saw it. so the observable crosses the two:
# for each subprogram, every `DW_OP_fbreg` offset must appear as a displacement in some
# instruction of that same function that names the register `DW_AT_frame_base` names.
#
# the two counts are ISA-independent by construction, so the golden is shared:
#   checked=<n>   offsets crossed. zero would make `unbacked` vacuous, so it is stated
#   unbacked=<n>  offsets no emitted access backs. the invariant is 0 on every target
#
# the displacement set is read coarsely - every integer literal on an instruction line
# mentioning the frame-base register - because the three ISAs spell a frame access three
# ways and a large offset is spelled a fourth (materialized into a scratch register
# first). coarse in the permissive direction only: it can accept an offset it should
# have rejected, never reject a correct one, and #2759's offsets miss the real set by
# the whole reservation rather than narrowly. requires llvm-dwarfdump and llvm-objdump.
produce_varloc_fbreg() {
    g=$4
    dd_tool=$(resolve_dwarfdump) || {
        echo "link: varloc-fbreg: llvm-dwarfdump not found (install the 'llvm' package)" >&2; return 2
    }
    od_tool=$(resolve_objdump) || {
        echo "link: varloc-fbreg: llvm-objdump not found (install the 'llvm' package)" >&2; return 2
    }

    # one record per subprogram that has a machine range and at least one fbreg
    # variable: "lo hi framebase-register off,off,..."
    "$dd_tool" --debug-info "$g" 2>/dev/null | awk '
        function flush(   i) {
            if (lo != "" && hi != "" && fb != "" && offs != "") { print lo, hi, fb, offs }
            lo = ""; hi = ""; fb = ""; offs = ""
        }
        /DW_TAG_subprogram/ { flush(); next }
        # only the subprogram own range: an inlined-subroutine or lexical-block DIE
        # nested inside it carries a low_pc too, and it is printed after the frame base.
        /DW_AT_low_pc/  { if (fb == "" && match($0, /0x[0-9a-f]+/)) { lo = substr($0, RSTART, RLENGTH) } next }
        /DW_AT_high_pc/ { if (fb == "" && match($0, /0x[0-9a-f]+/)) { hi = substr($0, RSTART, RLENGTH) } next }
        # the DWARF register NUMBER, not the name llvm-dwarfdump prints beside it: it
        # spells the aarch64 frame pointer W29 while the disassembler spells it x29.
        /DW_AT_frame_base/ { if (match($0, /DW_OP_reg[0-9]+/)) { fb = substr($0, RSTART + 9, RLENGTH - 9) } next }
        /DW_OP_fbreg/ {
            s = $0
            while (match(s, /DW_OP_fbreg [+-]?[0-9]+/)) {
                o = substr(s, RSTART, RLENGTH); sub(/^DW_OP_fbreg /, "", o); sub(/^\+/, "", o)
                offs = (offs == "") ? o : offs "," o
                s = substr(s, RSTART + RLENGTH)
            }
            next
        }
        END { flush() }
    ' > "$g.fns" || return 1

    "$od_tool" -d --no-show-raw-insn "$g" 2>/dev/null > "$g.dis" || return 1

    case "$2" in
        *riscv64*) fb_isa=riscv64 ;;
        *arm64*|*aarch64*) fb_isa=aarch64 ;;
        *) fb_isa=x86_64 ;;
    esac

    awk -v fns="$g.fns" -v isa="$fb_isa" '
        function hex2dec(h,   v, i, c, d) {
            sub(/^0x/, "", h); v = 0
            for (i = 1; i <= length(h); i++) {
                c = tolower(substr(h, i, 1)); d = index("0123456789abcdef", c) - 1
                v = v * 16 + d
            }
            return v
        }
        # the frame-base register as the disassembler spells it, from the DWARF number.
        # only two can appear: the frame pointer, and the stack pointer for a function
        # with no frame record (`dwarf.frame_base_omit_reg`).
        function disname(n) {
            if (isa == "riscv64") { if (n == 8)  { return "s0"   } if (n == 2)  { return "sp"   } }
            if (isa == "aarch64") { if (n == 29) { return "x29"  } if (n == 31) { return "sp"   } }
            if (isa == "x86_64")  { if (n == 6)  { return "%rbp" } if (n == 7)  { return "%rsp" } }
            return ""
        }
        BEGIN {
            n = 0
            while ((getline line < fns) > 0) {
                split(line, f, " ")
                r = disname(f[3] + 0)
                if (r == "") { continue }
                n++; flo[n] = hex2dec(f[1]); fhi[n] = hex2dec(f[2]); freg[n] = r; foffs[n] = f[4]
            }
            close(fns)
        }
        # "  4076a0: sd a0, -0x68(s0)"
        /^[ ]*[0-9a-f]+:/ {
            addr = $1; sub(/:$/, "", addr); a = hex2dec("0x" addr)
            # multiple debug descriptions can share the same machine range
            for (i = 1; i <= n; i++) {
                if (a >= flo[i] && a < fhi[i]) {
                    if (index($0, freg[i]) == 0) { continue }
                    s = $0
                    while (match(s, /-?0x[0-9a-f]+/)) {
                        t = substr(s, RSTART, RLENGTH)
                        neg = (substr(t, 1, 1) == "-")
                        v = hex2dec(neg ? substr(t, 2) : t); if (neg) { v = -v }
                        seen[i "/" v] = 1
                        s = substr(s, RSTART + RLENGTH)
                    }
                }
            }
        }
        END {
            checked = 0; unbacked = 0
            for (i = 1; i <= n; i++) {
                c = split(foffs[i], o, ",")
                for (j = 1; j <= c; j++) {
                    checked++
                    if (!((i "/" (o[j] + 0)) in seen)) { unbacked++ }
                }
            }
            # the count itself is ISA-dependent (the three back ends put different
            # variables in slots), so only its being nonzero is stated - without which
            # `unbacked=0` would be vacuous. the invariant is exact.
            print "varloc_fbreg_checked=" (checked > 0 ? "nonzero" : "zero")
            print "varloc_fbreg_unbacked=" unbacked
        }
    ' "$g.dis"
    rc=$?
    rm -f "$g.fns" "$g.dis"
    return $rc
}


# produce <run> <engine> <leg> <binary> [<g_binary>] [<profile>]
# dispatches to the producer named by <run>, forwarding the remaining arguments. the
# debuginfo producer takes an extra `-g` artifact path run.sh built alongside the
# default (no-`-g`) one; every other producer inspects the single default artifact.
# <profile> is appended (not inserted) so every EXISTING producer's positional
# reading is untouched by its addition; only gdb-session reads it, since only its
# observable is a real function of the active profile's own codegen (#2779).
produce() {
    run=$1
    shift
    case "$run" in
        exec)        produce_exec "$@" ;;
        relro-fault) produce_relro_fault "$@" ;;
        field)       produce_field "$@" ;;
        macho-signed) produce_macho_signed "$@" ;;
        macho-got)   produce_macho_got "$@" ;;
        embed-dedup) produce_embed_dedup "$@" ;;
        macho-framing) produce_macho_framing "$@" ;;
        macho-sections) produce_macho_sections "$@" ;;
        macho-header-span) produce_macho_header_span "$@" ;;
        macho-imports) produce_macho_imports "$@" ;;
        macho-mod-init) produce_macho_mod_init "$@" ;;
        macho-abs-bind) produce_macho_abs_bind "$@" ;;
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
        symtab)      produce_symtab "$@" ;;
        elf-needed)  produce_elf_needed "$@" ;;
        link-provider) produce_link_provider "$@" ;;
        gdb-session) produce_gdb_session "$@" ;;
        spirv-val-vulkan) produce_spirv_val_vulkan "$@" ;;
        spirv-image)  produce_spirv_image "$@" ;;
        varloc-fbreg) produce_varloc_fbreg "$@" ;;
        riscv-pcrel) produce_riscv_pcrel "$@" ;;
        *) echo "link: unknown run mode '$run'" >&2; return 2 ;;
    esac
}
