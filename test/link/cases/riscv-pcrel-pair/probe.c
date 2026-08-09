/* the C half of the riscv64 pc-relative pair case (mach #2797).
 *
 * Compiled by clang for riscv64, which reaches a global through the RISC-V psABI's
 * hi/lo pair: `auipc rN, %pcrel_hi(sym)` materializes the page, and a following
 * instruction carries `%pcrel_lo(.Lpcrel_hiK)` naming a LABEL AT THE AUIPC rather
 * than naming the target. The linker has to read the target off the high half that
 * label points at. mach never did, so the target dropped out of the arithmetic
 * entirely and every value below loaded from an address computed only from where the
 * two instructions sit. This is not conditional on how far apart they are - adjacent
 * halves resolve to a flat 0 displacement, which is the target being absent, not the
 * assumption holding.
 *
 * Two shapes, because they go wrong by different amounts and are read back
 * differently:
 *
 *   - `pcrel_doubles` / `pcrel_longs` materialize a base pointer in an `addi`, then
 *     index off it, so one bad pair corrupts four reads at once and the wrongness is
 *     visible as an integer.
 *
 *   - `scale` multiplies by a constant clang pools, the shape that made
 *     test/link/cases/narrow-stack-args read every float argument back as exactly 0: a
 *     scale factor loaded from the wrong address is 0.0, and 0.0 times anything is a
 *     plausible-looking zero rather than a crash.
 *
 * The globals are mutable on purpose. A `const` array of these values is folded into
 * immediates by clang at -O1 and the pair never appears, which would leave the case
 * green while testing nothing - `label-pairs` in the observable is what states that
 * this has not silently happened. */

double kd[4] = {0.125, 0.25, 0.375, 0.5};
long kl[4] = {11, 22, 33, 44};

/* four doubles read through one materialized base, scaled to exact integers */
long pcrel_doubles(long *out) {
    out[0] = (long)(kd[0] * 1000.0);
    out[1] = (long)(kd[1] * 1000.0);
    out[2] = (long)(kd[2] * 1000.0);
    out[3] = (long)(kd[3] * 1000.0);
    return 4;
}

/* the same through an integer base, so a wrong base is visible without the FPU */
long pcrel_longs(void) {
    return kl[0] + kl[1] * 10 + kl[2] * 100 + kl[3] * 1000;
}

/* a pooled float constant as a scale factor: reading it from the wrong address
 * yields 0.0, and every product is then exactly 0.
 *
 * This one is deliberately the SECOND pooled constant in its section, so its symbol
 * sits at a non-zero offset (`.LCPI2_0` at +8 where `.LCPI0_0` is at +0). A pairing
 * that recovered the target's section but dropped the symbol's own offset within it
 * would resolve the offset-0 constant correctly by coincidence and only this one
 * wrongly, so having both is what makes the case able to tell those apart. */
long scale(double v) { return (long)(v * 12.5); }

/* the STORE spelling, R_RISCV_PCREL_LO12_S. It shares the low-half resolution path
 * with the load form, and nothing else in this suite reaches it from a foreign
 * object - a shared path is only actually covered where something exercises it.
 *
 * Both stores also make clang put a NON-ZERO addend on the HI20 (`kl + 10`, `kd +
 * 18`, since the element is not the first), so they cover the other way a pair can
 * resolve correctly by luck: an addend that is dropped is invisible when it is 0. */
long store_long(long v) { kl[2] = v; return v; }
long store_double(double v) { kd[3] = v; return 1; }
