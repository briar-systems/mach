# mach.lang.alloc.probe

the compiler's one allocation probe: a counting, tracking, refusing
allocator for its own tests

a probe sits in front of a backing allocator (a page allocator unless the
test hands one in) or in front of nothing. every allocate and reallocate
request is one ordinal in a single sequence, so a test that runs an
operation once per ordinal, from one to the number of requests the
operation makes, meets every refusal path the operation has. the probe
tracks every block it handed out, so after each run it can say whether the
operation released everything (`leaked`), released something twice or with
the wrong size (`mismatch`), and how many requests it made (`counted`).

## refusal modes

`fail_at_ordinal` refuses exactly one request; `fail_from_ordinal` refuses
that request and every later one; `make_refusing` builds a probe over no
backing that refuses everything and only counts, for the growth checks that
prove a capacity overflow is refused before any reallocate is attempted;
`refuse_reallocate` refuses every reallocate while allocates still succeed.
`arm` starts the counted window: the ordinal is measured from the arm
point, so a fixture built through the probe before the operation runs does
not shift the ordinal the operation is refused at.

## the walk

`walk` runs an attempt at ordinal zero to learn how many requests the
operation makes, then once per ordinal from one to that count with a fresh
probe each time. after every attempt it checks the probe: nothing
outstanding, no mismatch, tracking never overflowed, and when an ordinal
was armed the refusal fired exactly once. the attempt itself asserts what
the operation promised: the typed refusal, the input contract, the
teardown. an operation that makes no requests at ordinal zero is reported
as such; the test then classifies it nonallocating instead of probing it.

