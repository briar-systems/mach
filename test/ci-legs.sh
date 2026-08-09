#!/usr/bin/env bash
# ci-legs.sh: emit the GitHub Actions matrix for one cadence from engines.conf.
#
# usage: ci-legs.sh <pr|main>
#
# a CI leg is a RUNNER, not a target row, and the runner column is what assigns rows
# to it: the five ubuntu-latest rows (x86_64-linux, riscv64-linux, riscv32, spirv,
# mos6502) are one job that covers all five, and a job per row would build the same
# artifacts five times over.
#
# the workflow hands each job its runner LABEL and the driver reads this file itself
# (`run.sh --runner`), so no step names a target list it could get wrong.
#
# prints a compact JSON array the workflow feeds straight to `strategy.matrix.include`:
#
#   [{"runner":"ubuntu-latest","legs":"x86_64-linux riscv64-linux ...","qemu":"qemu-riscv64"}]
#
# keys avoid hyphens so `matrix.runner` resolves in the GitHub expression context.
# `legs` is the link suite's leg list for that runner; `qemu` names the interpreters
# the runner must install, empty when it needs none.
#
# a runner whose rows disagree about cadence is a refusal, not a choice: picking one
# would silently move a metered darwin row onto every pull request, or drop a pr row
# to the release merge, and neither is visible afterwards in the workflow.
set -eu

cadence=${1:-}
case "$cadence" in
    pr|main) : ;;
    *) echo "usage: ci-legs.sh <pr|main>" >&2; exit 2 ;;
esac

here=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)

awk -v want="$cadence" '
    /^[[:space:]]*#/ || /^[[:space:]]*$/ { next }
    {
        name = $1; engine = $8; runner = $10; rowcadence = $11
        if (runner == "-") next
        if (runner in seen && cad[runner] != rowcadence) {
            printf "ci-legs.sh: runner %s carries cadence %s and %s; one runner is one job, so the rows must agree\n", runner, cad[runner], rowcadence > "/dev/stderr"
            bad = 1
            next
        }
        seen[runner] = 1
        cad[runner] = rowcadence
        if (rowcadence != want) next
        if (runner in legs) legs[runner] = legs[runner] " " name; else { legs[runner] = name; order[++n] = runner }
        if (engine ~ /^qemu:/) {
            cmd = substr(engine, 6)
            if (!(runner in qemu)) qemu[runner] = cmd
            else if (index(" " qemu[runner] " ", " " cmd " ") == 0) qemu[runner] = qemu[runner] " " cmd
        }
    }
    END {
        if (bad) exit 1
        printf "["
        for (i = 1; i <= n; i++) {
            r = order[i]
            if (i > 1) printf ","
            printf "{\"runner\":\"%s\",\"legs\":\"%s\",\"qemu\":\"%s\"}", r, legs[r], (r in qemu ? qemu[r] : "")
        }
        printf "]\n"
    }
' "$here/engines.conf"
