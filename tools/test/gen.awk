# gen.awk — deterministic generator of small, mostly-valid Mach programs.
#
# emits one complete program (a `main` returning i64) built from a constrained
# grammar of SAFE constructs: i64 locals, integer arithmetic, `if {} or {}`
# chains, bounded `for` loops, a `rec` with field access, and a helper call.
# randomness comes from a seeded LCG so a given seed always yields the same
# program; pass it with `-v seed=<n>`.
#
# safety guards keep programs mostly-valid and crash-free at runtime:
#   - division/modulo always use a non-zero constant divisor
#   - `for` loops run a fixed, statically-bounded number of iterations
#   - every local is declared and initialised before use
# the point is to exercise the compiler, not to require every program compile.

BEGIN {
    if (seed == "") { seed = 1 }
    state = seed % 2147483647
    if (state <= 0) { state += 2147483646 }

    nlocals = 0
    emit_header()
    emit_record()
    emit_helper()
    emit_main()
}

# park-miller minimal-standard LCG; returns a value in [0, 2147483646].
function rnd() {
    state = (state * 16807) % 2147483647
    return state
}

# uniform integer in [lo, hi].
function rint(lo, hi) {
    return lo + (rnd() % (hi - lo + 1))
}

function emit_header() {
    print "use std.runtime;"
    print "$main.symbol = \"main\";"
    print ""
}

# a small record so generated programs exercise aggregate layout + field access.
function emit_record() {
    print "rec Box {"
    print "    a: i64;"
    print "    b: i64;"
    print "}"
    print ""
}

# a pure helper taking two i64 and returning one, called from main.
function emit_helper() {
    print "fun mix(p: i64, q: i64) i64 {"
    print "    var r: i64 = p;"
    emit_arith_stmt("    ", "r", "p", "q")
    print "    ret r;"
    print "}"
    print ""
}

# choose a random already-declared local name, or a small constant literal.
function rand_operand(   pick) {
    if (nlocals > 0 && rint(0, 2) > 0) {
        pick = rint(0, nlocals - 1)
        return locals[pick]
    }
    return rint(1, 9) ""
}

# emit `dst = <op> <binop> <op>;` using only safe divisors.
function emit_arith_stmt(ind, dst, lhs, rhs,   op, l, r) {
    op = rint(0, 4)
    l = (lhs == "") ? rand_operand() : lhs
    r = (rhs == "") ? rand_operand() : rhs
    if (op == 0) { print ind dst " = " l " + " r ";" }
    else if (op == 1) { print ind dst " = " l " - " r ";" }
    else if (op == 2) { print ind dst " = " l " * " r ";" }
    else if (op == 3) { print ind dst " = " l " / " rint(1, 9) ";" }  # nonzero divisor
    else { print ind dst " = " l " % " rint(1, 9) ";" }              # nonzero modulus
}

# emit a boolean condition over a declared local and a constant.
function rand_cond(   v, cmp, k) {
    v = (nlocals > 0) ? locals[rint(0, nlocals - 1)] : "0"
    cmp = rint(0, 5)
    k = rint(0, 20)
    if (cmp == 0) return v " < " k
    if (cmp == 1) return v " > " k
    if (cmp == 2) return v " <= " k
    if (cmp == 3) return v " >= " k
    if (cmp == 4) return v " == " k
    return v " != " k
}

function emit_main(   nstmts, i, kind, name, lim, idx, target) {
    print "fun main(argc: i64, argv: **u8) i64 {"

    # always seed a couple of locals so later statements have operands.
    declare_local("    ", "x", rint(0, 9))
    declare_local("    ", "y", rint(0, 9))

    nstmts = rint(4, 9)
    for (i = 0; i < nstmts; i++) {
        kind = rint(0, 5)
        if (kind == 0) {
            # new local from arithmetic
            name = "v" i
            declare_local("    ", name, 0)
            emit_arith_stmt("    ", name, "", "")
        } else if (kind == 1) {
            # reassign an existing local
            if (nlocals > 0) {
                target = locals[rint(0, nlocals - 1)]
                emit_arith_stmt("    ", target, "", "")
            }
        } else if (kind == 2) {
            # if {} or {} chain
            target = (nlocals > 0) ? locals[rint(0, nlocals - 1)] : "x"
            print "    if (" rand_cond() ") {"
            emit_arith_stmt("        ", target, "", "")
            print "    }"
            print "    or {"
            emit_arith_stmt("        ", target, "", "")
            print "    }"
        } else if (kind == 3) {
            # bounded for loop: fixed iteration count, mutates a local
            target = (nlocals > 0) ? locals[rint(0, nlocals - 1)] : "x"
            idx = "i" i
            lim = rint(1, 6)
            print "    var " idx ": i64 = 0;"
            print "    for (" idx " < " lim ") {"
            emit_arith_stmt("        ", target, "", "")
            print "        " idx " = " idx " + 1;"
            print "    }"
        } else if (kind == 4) {
            # record construction + field access
            name = "box" i
            print "    var " name ": Box = Box{ a: " rint(0, 9) ", b: " rint(0, 9) " };"
            target = (nlocals > 0) ? locals[rint(0, nlocals - 1)] : "x"
            if (rint(0, 1) == 0) { print "    " target " = " name ".a;" }
            else { print "    " target " = " name ".b;" }
        } else {
            # call the helper with two operands
            name = "c" i
            declare_local("    ", name, 0)
            print "    " name " = mix(" rand_operand() ", " rand_operand() ");"
        }
    }

    # return a value derived from a declared local, masked into the 0..255
    # exit-code range so the differential check compares a stable byte.
    target = (nlocals > 0) ? locals[rint(0, nlocals - 1)] : "0"
    print "    var ret_code: i64 = " target ";"
    print "    if (ret_code < 0) { ret_code = -ret_code; }"
    print "    ret_code = ret_code % 256;"
    print "    ret ret_code;"
    print "}"
}

# declare `var <name>: i64 = <init>;` and record it as available.
function declare_local(ind, name, init) {
    print ind "var " name ": i64 = " init ";"
    locals[nlocals] = name
    nlocals++
}
