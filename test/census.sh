#!/bin/sh
# census.sh: source-level structural censuses that no unit test can ask.
#
# usage: census.sh [name]...    (default: every census)
#
# each census greps the tree for a pattern that must not appear in production
# code and reports the offending file:line. a line inside a top-level
# `test "..." { }` block is a test line and never counts, and so is a line
# inside a `t_`- or `ut_`-prefixed helper function: the censuses are about what
# the shipped compiler does, not what its tests construct.

set -u
root=$(cd "$(dirname "$0")/.." && pwd)
status=0
censuses="$*"
tmp=$(mktemp)
trap 'rm -f "$tmp"' EXIT

want() {
    [ -z "$censuses" ] && return 0
    case " $censuses " in *" $1 "*) return 0;; esac
    return 1
}

# census <name> <pattern> <dir>...
census() {
    name=$1; shift
    pattern=$1; shift
    : > "$tmp"
    find "$@" -name '*.mach' | sort | while IFS= read -r f; do
        rel=${f#"$root"/}
        awk '
            BEGIN { depth = 0; intest = 0; pending = "" }
            {
                line = $0
                if (intest == 1 && line ~ /^(test[ \t]+"|(pub[ \t]+)?(fun|rec|tag|def|val|var|use)[ \t])/) { intest = 0 }
                if (intest == 0 && (line ~ /^test[ \t]+"/ || line ~ /^(pub[ \t]+)?fun[ \t]+u?t_/)) { intest = 1; depth = 0 }
                if (intest == 0) {
                    out = line
                    if (pending != "") { out = pending " " line; pending = "" }
                    if (out ~ /=[ \t]*$/) { pending = out }
                    else { print NR ":" out }
                }
                n = gsub(/\{/, "{", line)
                m = gsub(/\}/, "}", line)
                if (intest == 1) {
                    depth += n - m
                    if (depth <= 0) { intest = 0 }
                }
            }
        ' "$f" | grep -E "$pattern" | sed "s|^|$rel:|" >> "$tmp"
    done
    hits=$(wc -l < "$tmp" | tr -d ' ')
    if [ "$hits" -ne 0 ]; then
        sed 's|^|  |' "$tmp"
        echo "census $name: FAIL ($hits production hit(s))"
        status=1
    else
        echo "census $name: ok"
    fi
}

if want b-be-7; then
    # B-BE-7 / G-8: an ISA identity is known to the target layer and the registry
    # alone. shared codegen, the frontend, the middle end, the linker and the
    # driver read a target's declarations, never its name.
    census b-be-7 'ARCH_X86_64|ARCH_AARCH64|ARCH_RISCV|ARCH_SPIRV' \
        "$root/src/lang/be" "$root/src/lang/fe" "$root/src/lang/me" \
        "$root/src/lang/build" "$root/src/lang/driver" "$root/src/cli"
fi

if want b-fs-1; then
    # B-FS-1: a compiler-controlled artifact reaches its final path through the
    # publication primitive, never through a direct create-then-write. the
    # primitive is `std.filesystem.transaction` in mach-std; `target/of/bin.mach`
    # wraps it for the object writers.
    census b-fs-1 'fs\.create\(|fs\.write\(|fs\.write_bytes\(|fs\.create_new\(' \
        "$root/src/lang/target/of" "$root/src/lang/build" "$root/src/cli/cmd"
fi

if want b-fs-1-publication; then
    # B-FS-1: the compiler publishes through mach.lang.publication and nothing
    # else prepares or commits a filesystem transaction, except cli/cmd/init's
    # two-file swap protocol, which renames between prepare and commit and is
    # not the publication of one path.
    census b-fs-1-publication 'txn\.(prepare|prepare_bytes|prepare_subtree|commit)\(' \
        "$root/src/lang/target" "$root/src/lang/build" "$root/src/lang/driver" \
        "$root/src/lang/fe" "$root/src/lang/me" "$root/src/lang/be" \
        $(find "$root/src/cli" -name '*.mach' ! -name 'init.mach') \
        $(find "$root/src/lang" -maxdepth 1 -name '*.mach' ! -name 'publication.mach')
fi

if want b-diag-1; then
    # B-DIAG-1: a diagnostic append result is never discarded. a failed append
    # is a durable rejection of the phase, so every statement-position append
    # goes through a `diagnostic.record_*` form, which records the loss on the
    # store. the checked forms stay available where a caller propagates the
    # failure itself.
    census b-diag-1 '^[0-9]+:[[:space:]]*diagnostic\.(error|warning|info|help|commit|gate_error|gate_commit|attach_[a-z_]*)\(' \
        "$root/src"
fi

if want p-4-lower-scope; then
    # P-4 / B-ME-7: the module a lowering step is bound to, and the comptime
    # environment that binding carries, are values installed whole through
    # `scope_enter` and taken down by the LIFO-checked `scope_leave`. no site
    # assigns into a live binding, so no exit path can leave a context bound to
    # a foreign module.
    census p-4-lower-scope '\.(scope|env)\.[a-z_]+[ \t]*=[^=]' \
        "$root/src/lang/me"
fi

if want b-fe-4-scan; then
    # B-FE-4 / G-4: compile-time dependency detection goes through the one
    # visitor, `comptime.gate_depends_on`. a hand-written scanner is a
    # boolean predicate that recurses over an `id.ExprId` and consults
    # compile-time state; each one carries its own partial list of expression
    # kinds and answers "no dependency" for every kind it forgot.
    : > "$tmp"
    find "$root/src" -name '*.mach' | sort | while IFS= read -r f; do
        rel=${f#"$root"/}
        awk '
            BEGIN { fn = ""; depth = 0; sig = ""; insig = 0; selfcall = 0; leaf = 0; start = 0 }
            {
                line = $0
                if (fn == "" && insig == 0 && line ~ /^(pub[ \t]+)?fun[ \t]+[A-Za-z_][A-Za-z0-9_]*[ \t]*\(/) {
                    if (line ~ /^(pub[ \t]+)?fun[ \t]+u?t_/) { next }
                    name = line
                    sub(/^(pub[ \t]+)?fun[ \t]+/, "", name)
                    sub(/[ \t]*\(.*$/, "", name)
                    start = NR; sig = line; insig = 1; selfcall = 0; leaf = 0
                }
                if (insig == 1) {
                    if (line != sig) { sig = sig " " line }
                    if (line ~ /\{/) {
                        insig = 0
                        if (sig ~ /id\.ExprId/ && sig ~ /\)[ \t]*bool[ \t]*\{/) { fn = name; depth = 0 } else { fn = "" }
                    }
                    if (fn == "") { next }
                }
                if (fn != "") {
                    if (line ~ ("[^A-Za-z0-9_]" name "[ \t]*\\(") && NR != start) { selfcall = 1 }
                    if (line ~ /comptime\.|SYM_FLAG_COMPTIME|COMPTIME_IDENT|loopvar|field_descriptor/) { leaf = 1 }
                    n = gsub(/\{/, "{", line); m = gsub(/\}/, "}", line)
                    depth += n - m
                    if (depth <= 0) {
                        if (selfcall == 1 && leaf == 1) { print start ": " name }
                        fn = ""
                    }
                }
            }
        ' "$f" | sed "s|^|$rel:|" >> "$tmp"
    done
    hits=$(wc -l < "$tmp" | tr -d ' ')
    if [ "$hits" -ne 0 ]; then
        sed 's|^|  |' "$tmp"
        echo "census b-fe-4-scan: FAIL ($hits hand-written dependency scanner(s))"
        status=1
    else
        echo "census b-fe-4-scan: ok"
    fi
fi

if want b-fe-4-walk; then
    # B-FE-4 / G-4: the same rule from the other side. a compile-time leaf
    # predicate answers a question about one node; it is never consulted from
    # inside a recursive expression walk, because that walk would be a second
    # traversal with its own kind coverage.
    : > "$tmp"
    find "$root/src" -name '*.mach' | sort | while IFS= read -r f; do
        rel=${f#"$root"/}
        awk '
            BEGIN { fn = ""; depth = 0; sig = ""; insig = 0; selfcall = 0; leaf = 0; start = 0 }
            {
                line = $0
                if (fn == "" && insig == 0 && line ~ /^(pub[ \t]+)?fun[ \t]+[A-Za-z_][A-Za-z0-9_]*[ \t]*\(/) {
                    if (line ~ /^(pub[ \t]+)?fun[ \t]+u?t_/) { next }
                    name = line
                    sub(/^(pub[ \t]+)?fun[ \t]+/, "", name)
                    sub(/[ \t]*\(.*$/, "", name)
                    start = NR; sig = line; insig = 1; selfcall = 0; leaf = 0
                }
                if (insig == 1) {
                    if (line != sig) { sig = sig " " line }
                    if (line ~ /\{/) {
                        insig = 0
                        if (sig ~ /id\.ExprId/) { fn = name; depth = 0 } else { fn = "" }
                    }
                    if (fn == "") { next }
                }
                if (fn != "") {
                    if (line ~ ("[^A-Za-z0-9_]" name "[ \t]*\\(") && NR != start) { selfcall = 1 }
                    if (line ~ /is_type_comparison\(|is_type_query_call\(|is_layout_intrinsic_call\(|is_field_descriptor_member\(|is_field_type_member\(|is_comptime_path\(/) { leaf = 1 }
                    n = gsub(/\{/, "{", line); m = gsub(/\}/, "}", line)
                    depth += n - m
                    if (depth <= 0) {
                        if (selfcall == 1 && leaf == 1) { print start ": " name }
                        fn = ""
                    }
                }
            }
        ' "$f" | sed "s|^|$rel:|" >> "$tmp"
    done
    hits=$(wc -l < "$tmp" | tr -d ' ')
    if [ "$hits" -ne 0 ]; then
        sed 's|^|  |' "$tmp"
        echo "census b-fe-4-walk: FAIL ($hits leaf predicate(s) inside a recursive walk)"
        status=1
    else
        echo "census b-fe-4-walk: ok"
    fi
fi

if want q-kinds; then
    # query kinds: every `pub val Q_*: QueryKind = N` carries a distinct N. two
    # branches each adding "the next" kind auto-merge to the same value with no
    # conflict marker, so the block is re-checked mechanically.
    dupes=$(grep -oE 'pub val Q_[A-Z_]+: *QueryKind *= *[0-9]+' "$root/src/lang/query.mach" | awk '{print $NF}' | sort | uniq -d)
    if [ -n "$dupes" ]; then
        echo "census q-kinds: FAIL (duplicate query kind value(s): $(printf '%s' "$dupes" | tr '\n' ' '))"; status=1
    else
        echo "census q-kinds: ok"
    fi
fi

if want real-bools; then
    # B-DIAG-3: a res[bool, E] is a declared real outcome, never a success
    # that is always true. every declaration whose return type is
    # res[bool, fail.Fail] or res[bool, outcome.Fail] is listed in
    # test/census/real-bools.txt as path:function; a declaration off the list,
    # or a list entry with no declaration, fails. no function-pointer type
    # returns a bool result, and no error is an empty string standing in for a
    # kind.
    list="$root/test/census/real-bools.txt"
    found=$(mktemp)
    listed=$(mktemp)
    find "$root/src" -name '*.mach' | sort | while IFS= read -r f; do
        rel=${f#"$root"/}
        awk -v rel="$rel" '
            /^(pub[ \t]+)?fun[ \t]+[A-Za-z0-9_]+/ {
                match($0, /fun[ \t]+[A-Za-z0-9_]+/)
                name = substr($0, RSTART + 4, RLENGTH - 4)
                sub(/^[ \t]+/, "", name)
            }
            /\) (R\.Result\[bool, (str|outcome\.Fail)\]|res\[bool, (fail|outcome)\.Fail\])[ \t]*\{/ { print rel ":" name }
        ' "$f"
    done | sort > "$found"
    awk '{ sub(/\r$/, ""); print }' "$list" | sort > "$listed"
    : > "$tmp"
    comm -23 "$found" "$listed" | sed 's|^|  declared, not listed: |' >> "$tmp"
    comm -13 "$found" "$listed" | sed 's|^|  listed, not declared: |' >> "$tmp"
    grep -rnE 'fun\([^)]*\) (R\.Result\[bool, (str|outcome\.Fail)\]|res\[bool, (fail|outcome)\.Fail\])|^[ \t]*[^(]*\) (R\.Result\[bool, (str|outcome\.Fail)\]|res\[bool, (fail|outcome)\.Fail\]);' "$root/src" \
        | sed "s|^$root/||; s|^|  bool-result function type: |" >> "$tmp"
    grep -rnE 'R\.err\[[^]]*\]\(""\)' "$root/src" \
        | sed "s|^$root/||; s|^|  empty-string error: |" >> "$tmp"
    rm -f "$found" "$listed"
    hits=$(wc -l < "$tmp" | tr -d ' ')
    if [ "$hits" -ne 0 ]; then
        cat "$tmp"
        echo "census real-bools: FAIL ($hits finding(s))"
        status=1
    else
        echo "census real-bools: ok ($(wc -l < "$list" | tr -d ' ') listed real bools)"
    fi
fi

if want catalog-defaults; then
    # #3124: no closed catalog answers an unknown member with a default. a
    # catalog is every integer typedef under src/ with two or more literal
    # members; an id (sentinel members only) and a bit set (every member a
    # power of two) are not catalogs and are set apart by that shape. every
    # production function that takes a catalog type, branches on that
    # parameter, and ends with an unconditional literal return is a
    # default-picking site unless the same file records the function as a
    # partition: a test named `<fn>:partition_<Catalog>` that names every
    # member of the catalog, which the census checks, so a member added later
    # fails the census until the test classifies it.
    # test/census/catalog-exceptions.txt lists a catalog the census skips
    # (none today).
    skip=$(grep -vE '^(#|$)' "$root/test/census/catalog-exceptions.txt" | tr '\n' '|' | sed 's/|$//')
    types=$(find "$root/src" -name '*.mach' | sort | while IFS= read -r f; do
        awk '
            /^(pub[ \t]+)?def[ \t]+[A-Za-z_][A-Za-z0-9_]*[ \t]*:[ \t]*(u8|u16|u32|u64|i8|i16|i32|i64|usize)[ \t]*;/ {
                name=$0; sub(/^(pub[ \t]+)?def[ \t]+/,"",name); sub(/[ \t]*:.*$/,"",name); defs[name]=1
            }
            /^(pub[ \t]+)?val[ \t]+[A-Z][A-Z0-9_]*[ \t]*:[ \t]*[A-Za-z_][A-Za-z0-9_]*[ \t]*=[ \t]*[^;]+;/ {
                l=$0; sub(/^(pub[ \t]+)?val[ \t]+[A-Z][A-Z0-9_]*[ \t]*:[ \t]*/,"",l)
                t=l; sub(/[ \t]*=.*$/,"",t)
                v=l; sub(/^[^=]*=[ \t]*/,"",v); sub(/[ \t]*;.*$/,"",v)
                n[t]++
                if (v ~ /^0[xX]/) { hex[t]++; v=sprintf("%d", strtonum(v)) }
                if (v ~ /^[0-9]+$/) { lit[t]++; x=v+0; if (x>0 && int(x)==x) { p=x; while (p>1 && p%2==0) p=p/2; if (p==1) pow2[t]++ } }
            }
            END {
                for (t in defs) {
                    if (n[t] < 2) continue
                    if (hex[t]==n[t]) continue
                    if (n[t] >= 3 && pow2[t] == lit[t] && lit[t] == n[t]) continue
                    print t
                }
            }
        ' "$f"
    done | sort -u | grep -vE "^(${skip:-__none__})$" | tr '\n' '|' | sed 's/|$//')
    ncat=$(printf '%s' "$types" | tr '|' '\n' | wc -l | tr -d ' ')
    : > "$tmp"
    parts=$(mktemp)
    find "$root/src" -name '*.mach' | sort | while IFS= read -r f; do
        rel=${f#"$root"/}
        awk -v types="$types" -v parts="$parts" -v rel="$rel" '
            # a production function that takes a catalog type, branches on that
            # parameter, and ends with an unconditional literal return picks a
            # default; a `<fn>:partition_<Type>` test in the file records it as a
            # partition instead
            BEGIN { fn=""; depth=0; intest=0; sig=""; insig=0; nd=0 }
            {
                line=$0
                # a column-0 declaration ends any test block: brace counting drifts
                # over string literals that spell braces
                if (intest==1 && line ~ /^(test[ \t]+"|(pub[ \t]+)?(fun|rec|tag|def|val|var|use)[ \t])/) { intest=0; if (curtest!="") { tend[curtest]=NR-1; curtest="" } }
                if (intest==0 && line ~ /^test[ \t]+"/) {
                    tn=line; sub(/^test[ \t]+"/,"",tn); sub(/".*$/,"",tn)
                    curtest=""
                    if (tn ~ /(^|[.:])[A-Za-z_][A-Za-z0-9_]*:partition_[A-Za-z_][A-Za-z0-9_]*$/) {
                        key=tn; sub(/^.*[.]/,"",key); recorded[key]=1; curtest=key; tstart[key]=NR
                    }
                }
                if (intest==0 && (line ~ /^test[ \t]+"/ || line ~ /^(pub[ \t]+)?fun[ \t]+u?t_/)) { intest=1; depth=0 }
                if (intest==1) { n=gsub(/\{/,"{",line); m=gsub(/\}/,"}",line); depth+=n-m; if (depth<=0) { intest=0; if (curtest!="") { tend[curtest]=NR; curtest="" } }; next }
                if (fn=="" && insig==0 && line ~ /^(pub[ \t]+)?fun[ \t]+[A-Za-z_][A-Za-z0-9_]*[ \t]*\(/) {
                    name=line; sub(/^(pub[ \t]+)?fun[ \t]+/,"",name); sub(/[ \t]*\(.*$/,"",name)
                    start=NR; sig=line; insig=1
                }
                if (insig==1) {
                    if (line!=sig) sig=sig " " line
                    if (line ~ /\{/) {
                        insig=0; fn=""
                        params=sig; sub(/^[^(]*\(/,"",params); sub(/\)[^)]*$/,"",params)
                        np=split(params,ps,",")
                        pname=""; ptype=""
                        for (i=1;i<=np;i++) { p=ps[i]; if (match(p, "[A-Za-z_][A-Za-z0-9_]*[ \t]*:[ \t]*([A-Za-z_][A-Za-z0-9_]*\\.)?(" types ")[ \t]*$")) { q=substr(p,RSTART,RLENGTH); t=q; sub(/^.*[:.][ \t]*/,"",t); al=q; sub(/^[^:]*:[ \t]*/,"",al); if (al ~ /\./) { sub(/\..*$/,"",al) } else { al="" }; sub(/[ \t]*:.*$/,"",q); sub(/^[ \t]+/,"",q); pname=q; ptype=t; palias=al } }
                        if (pname!="") { fn=name; depth=0; branched=0; lastret=""; lastdepth=0 }
                    }
                    if (fn=="") next
                }
                if (fn!="") {
                    if (line ~ ("(if|or)[ \t]*\\([ \t]*" pname "[ \t]*==")) branched=1
                    n=gsub(/\{/,"{",line); m=gsub(/\}/,"}",line)
                    d0=depth; depth+=n-m
                    if (d0==1 && line ~ /^[ \t]*ret[ \t]/) { lastret=line; sub(/^[ \t]*/,"",lastret) }
                    if (depth<=0) {
                        if (branched==1 && lastret ~ /^ret[ \t]+((opt|res)\[[^\]]*\]\.(some|ok)\{)?("[^"]*"|[0-9]+|true|false|nil|[A-Z][A-Z0-9_.]*|[a-z_]+\.[A-Z][A-Z0-9_]*)\}?[ \t]*;/) { nd++; dl[nd]=start ": " name " (" pname ": " ptype ") " lastret; dk[nd]=name ":partition_" ptype; dt[nd]=ptype; da[nd]=palias }
                        fn=""
                    }
                }
            }
            END {
                for (i=1;i<=nd;i++) {
                    if (dk[i] in recorded) { print rel "|" dk[i] "|" dt[i] "|" da[i] "|" tstart[dk[i]] "|" tend[dk[i]] >> parts } else { print dl[i] }
                }
            }
        ' "$f" | sed "s|^|$rel:|" >> "$tmp"
    done
    # a recorded partition's test names every member of the catalog, so a member
    # added later is a census failure until the test classifies it. the catalog
    # is the file that defines the parameter's type: the same file for a bare
    # name, else the `use` line of its alias.
    while IFS='|' read -r rel key ptype palias ts te; do
        f="$root/$rel"
        if [ -z "$palias" ]; then
            deffile="$f"
        else
            # `use mach.a.b;` or `use alias: mach.a.b;` names src/a/b.mach
            path=$(grep -E "^use ($palias: )?mach(\.[a-z0-9_]+)*\.$palias;|^use $palias: mach(\.[a-z0-9_]+)+;" "$f" | head -n1 | sed -E "s/^use ($palias: )?mach\.//; s/;.*//; s/\./\//g")
            deffile="$root/src/$path.mach"
        fi
        if [ ! -f "$deffile" ] || ! grep -qE "^(pub[[:space:]]+)?def[[:space:]]+$ptype[[:space:]]*:" "$deffile"; then
            echo "$rel: $key: the catalog $ptype could not be located from the signature" >> "$tmp"
            continue
        fi
        body=$(sed -n "${ts},${te}p" "$f")
        grep -E "^(pub[[:space:]]+)?val[[:space:]]+[A-Z][A-Z0-9_]*[[:space:]]*:[[:space:]]*$ptype[[:space:]]*=" "$deffile" \
            | sed -E 's/^(pub[[:space:]]+)?val[[:space:]]+//; s/[[:space:]]*:.*//' | while IFS= read -r member; do
            printf '%s\n' "$body" | grep -qE "(^|[^A-Za-z0-9_])$member([^A-Za-z0-9_]|$)" \
                || echo "$rel: $key: the partition test does not name the member $member" >> "$tmp"
        done
    done < "$parts"
    hits=$(wc -l < "$tmp" | tr -d ' ')
    npart=$(wc -l < "$parts" | tr -d ' ')
    rm -f "$parts"
    if [ "$hits" -ne 0 ]; then
        sed 's|^|  |' "$tmp"
        echo "census catalog-defaults: FAIL ($hits default-picking site(s) on a closed catalog)"
        status=1
    else
        echo "census catalog-defaults: ok ($ncat catalogs, $npart recorded partitions)"
    fi
fi

exit $status
