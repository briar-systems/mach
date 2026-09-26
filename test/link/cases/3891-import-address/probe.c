/* a shared library the case links as a dynamic dependency. no libc, so it links
   the same on every leg without a target sysroot */
typedef long (*probe_fn)(long);

long probe_twice(long x) { return x * 2; }

long probe_apply(probe_fn f, long x) { return f(x) + 1; }

/* nonzero when f is this library's own pointer to probe_twice: an address mach
   takes of an import is the function's own, not a stub in the executable (mach #3942) */
long probe_is_twice(probe_fn f) { return f == probe_twice; }
