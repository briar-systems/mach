[mach](../README.md) / **base**

# mach.base

what every other area is written in: the allocation refusal, failures and diagnostics,
interned strings and handles, source locations, and plain utilities

| Module | |
| :--- | :--- |
| [`alloc`](alloc.md) | the compiler's allocation refusal, rendered once |
| [`catalog`](catalog.md) | the refusal of a catalog member that no arm of a dispatch names |
| [`checked`](checked.md) | checked arithmetic refusing overflow with a cause |
| [`diagnostic`](diagnostic.md) | diagnostics, the builder that assembles one and the store that collects them |
| [`duration`](duration.md) | durations written as `<n>ms`, `<n>s`, `<n>m` or `<n>h`, parsed and rendered back |
| [`fail`](fail.md) | the compiler's one failure type, and how a refusal of the machine becomes one |
| [`fingerprint`](fingerprint.md) | domain-separated fingerprint encoding, and the sha-256 digests of bytes and files |
| [`float`](float.md) | binary floating point as the compiler computes it: formats, rounding, NaNs and decimals |
| [`growth`](growth.md) | the compiler's one growth routine for arrays it keeps as a pointer and a capacity beside its own count |
| [`handle`](handle.md) | stores whose elements never move |
| [`intern`](intern.md) | interned strings: one stable id per distinct byte string |
| [`role`](role.md) | which side of a build the calling thread works on, the owner or a worker, and a check that a worker leaves the build's shared stores alone |
| [`scc`](scc.md) | strongly connected components of a directed graph over numbered nodes |
| [`source`](source.md) | the source map: the files of a compilation and the spans, locations and lines in them |
| [`stack`](stack.md) | the stack the compiler's recursive readers run on |
| [`textbuild`](textbuild.md) | a text builder that keeps its first failure |
| [`version`](version.md) | the compiler's version |
| [`wide`](wide.md) | 128-bit integer carrier over two u64 limbs |
| [`wire`](wire.md) | the compiler's one binary codec |

| Directory | |
| :--- | :--- |
| [`diagnostic/`](diagnostic/README.md) | diagnostics, the builder that assembles one and the store that collects them |
| [`source/`](source/README.md) | the source map: the files of a compilation and the spans, locations and lines in them |

