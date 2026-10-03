# mach.lang.build.testing.pool

the test process pool: every test of a run is its own process, up to a window
of them at once, each bounded by the run's timeout and its output captured to
a log beside the dispatcher. a test is classified and reported to the readout
the moment it is reaped, and the run closes with its summary

## rec Config

```mach
pub rec Config;
```

how a run executes its tests

runner: the program each test runs under, or nil to run the test itself
jobs: how many tests run at once, at least one
timeout: each test's bound, 0 for none
level: the readout level; a passing test's output is kept only at the items level
bytes: whether a test's captured output is kept for the renderer

## rec Counts

```mach
pub rec Counts;
```

what a finished run decides: how many tests did not pass, and how many of
those the run's own machinery failed rather than the test

## fun run

```mach
pub fun run(a: *A.Allocator, sink: *readout.Sink, arts: *outcome.TestArtifact, n: u32, config: *Config) res[Counts, fail.Fail];
```

run one set of tests, each reported to sink as it finishes, and close the run
with its summary

a: backs the results, the logs' paths and every process
sink: the readout the run reports to
arts: the tests
n: how many there are
config: how they run
ret: the run's counts; err when the run could not start, before any test ran

