// the C++ half of the compact personality case (mach #4211): a throw, and a
// catch that compact unwind alone describes. compiled by the runner's own
// toolchain.

// a mach #[symbol] name is the literal object symbol, and darwin's C++
// compiler prefixes an underscore to every C name, so the label asks for the
// literal one
extern "C" long long unwind_middle(long long depth) __asm__("unwind_middle");

extern "C" __attribute__((noinline)) long long cxx_throw(long long depth) {
    if (depth > 0) throw depth;
    return -1;
}

extern "C" __attribute__((noinline)) long long cxx_catch(long long depth) {
    try {
        return unwind_middle(depth);
    } catch (...) {
        return 100 + depth;
    }
}
