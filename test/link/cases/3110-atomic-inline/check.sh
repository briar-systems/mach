#!/usr/bin/env bash
. "$(dirname "$0")/../../check/common.sh"

# atomic_inline_scan <mnemonic-regex> — read a linked-image disassembly and report
# the two facts above. the wrapper list is spelled out so a wrapper with no call
# reads as 0 rather than being absent, and any other `std.sync.atomic.*` symbol a
# case function calls is appended after the eight.
atomic_inline_scan() {
    awk -v atomic="$1" '
    BEGIN {
        n = split("load store cas fetch_add fetch_sub exchange fence spin_hint", names, " ")
        for (i = 1; i <= n; i++) { calls[names[i]] = 0; known[names[i]] = 1 }
        probe = 0; incase = 0; pending = ""
    }
    /^[0-9a-f]+ <.*>:$/ {
        fn = $0; sub(/^[0-9a-f]+ </, "", fn); sub(/>:$/, "", fn)
        incase = (fn == "main" || fn ~ /^case\./)
        probe = (fn == "case.main.probe")
        pending = ""
        next
    }
    /^[[:space:]]*[0-9a-f]+:/ {
        if (!incase) { next }
        text = $0
        sub(/^[[:space:]]*[0-9a-f]+:[[:space:]]*/, "", text)
        sub(/[[:space:]]*#.*$/, "", text)
        gsub(/[[:space:]]+/, " ", text)
        sub(/ $/, "", text)
        mnemonic = text; sub(/ .*$/, "", mnemonic)
        if (match(text, /<std\.sync\.atomic\.[a-z_]+>/)) {
            sym = substr(text, RSTART + 1, RLENGTH - 2)
            if (mnemonic ~ /^(call|bl|jal|jalr)/) {
                short = sym; sub(/^std\.sync\.atomic\./, "", short)
                if (!(short in known)) { names[++n] = short; known[short] = 1 }
                calls[short]++
                if (probe) { seq[++nseq] = "call " sym }
            }
            next
        }
        if (mnemonic == "lock") { pending = "lock "; next }
        if (mnemonic ~ atomic) {
            if (probe) { seq[++nseq] = pending text }
        }
        pending = ""
        next
    }
    END {
        line = "calls from case text:"
        for (i = 1; i <= n; i++) { line = line " " names[i] "=" calls[names[i]] }
        print line
        print "probe:"
        for (i = 1; i <= nseq; i++) { print "  " seq[i] }
    }
    '
}

# produce_atomic_inline <engine> <leg> <binary>
# the CROSS-MODULE INLINING observable for the std atomic wrappers (#3110): the
# program's own output first (its counts depend on the atomicity and the ordering the
# wrappers promise, from two threads), then two facts read from the linked image.
#
#   calls from case text   per wrapper, the direct calls (`call`/`bl`/`jal`/`jalr`
#                          naming the symbol) made from every function the case
#                          itself defines: `main` and `case.*`. std's own callers of
#                          the wrappers are deliberately outside this count; the
#                          question is what the IMPORTER's text does.
#   probe                  the atomic instructions of `case.main.probe`, one per
#                          line, in text order, with a direct wrapper call listed as
#                          `call <symbol>` so the debug cell's sequence is the eight
#                          calls and the release cell's is the instruction sequence
#                          those calls became. the x86-64 `lock` prefix, which
#                          llvm-objdump decodes as its own line, is joined onto the
#                          instruction it prefixes.
#
# the golden is per target and per profile: the instruction vocabulary is the ISA's
# and the counts are the profile's.
produce_atomic_inline() {
    engine=$1
    target=$2
    bin=$3
    out=$(mktemp)
    err=$(mktemp)
    run_captured "$engine" "$target" "$bin" "$out" "$err" || { rm -f "$out" "$err"; return 1; }
    if [ "$run_status" -ne 0 ]; then
        report_run_failure "atomic-inline" "$run_status" "$run_out"
        [ -s "$err" ] && sed 's/^/    /' "$err" >&2
        rm -f "$out" "$err"
        return "$run_status"
    fi
    cat "$err" >&2
    cat "$out"
    rm -f "$out" "$err"

    tool=$(resolve_objdump) || {
        echo "link: atomic-inline: llvm-objdump not found (install the 'llvm' package)" >&2
        return 2
    }
    case "$target" in
        x86_64-*)  atomic='^(lock|xchg|cmpxchg|xadd|mfence|lfence|sfence|pause)' ;;
        aarch64-*) atomic='^(ldar|stlr|ldaxr|stlxr|ldxr|stxr|dmb|dsb|isb|yield|cas|ldadd|swp)' ;;
        riscv64-*) atomic='^(fence|lr[.]|sc[.]|amo|pause)' ;;
        *) echo "link: atomic-inline: no atomic vocabulary for '$target'" >&2; return 2 ;;
    esac
    "$tool" -d --no-show-raw-insn "$bin" | atomic_inline_scan "$atomic"
}

produce_atomic_inline "$@"
