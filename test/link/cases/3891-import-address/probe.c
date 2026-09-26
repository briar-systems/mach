/* a shared library the case links as a dynamic dependency. no libc, so it links
   the same on every leg without a target sysroot */
typedef long (*probe_fn)(long);

long probe_twice(long x) { return x * 2; }

long probe_apply(probe_fn f, long x) { return f(x) + 1; }
