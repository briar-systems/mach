/* the C half of the orphan-ext case (mach #3523): one symbol under its C name,
 * built by the leg's own toolchain, that an `ext` declared behind a layout gate
 * has to reach. */
long long probe_answer(long long x) { return x * 2 + 1; }
