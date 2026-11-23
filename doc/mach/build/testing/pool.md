[mach](../../README.md) / [build](../README.md) / [testing](README.md) / **pool**

# mach.build.testing.pool

the test process pool: every test of a run is its own process, up to a window
of them at once, each bounded by the run's timeout and its output captured to
a log beside the dispatcher. a test is classified and reported to the readout
the moment it is reaped, and the run closes with its summary

Source: [`src/build/testing/pool.mach`](../../../../src/build/testing/pool.mach)

Uses types from [`mach.base.fail`](../../base/fail.md), [`mach.manifest.runner`](../../manifest/runner.md), [`mach.report.readout`](../../report/readout.md), [`mach.report.testrun`](../../report/testrun.md).

| Type | | |
| :--- | :--- | :--- |
| [`Config`](#config) | `rec` | how a run executes its tests |
| [`Counts`](#counts) | `rec` | what a finished run decides: how many tests did not pass, and how many of those the run's own machinery failed rather than the test |

| On `Config` | |
| :--- | :--- |
| [`run`](#run) | run one set of tests, each reported to sink as it finishes, and close the run with its summary |

---

## Config

```mach
pub rec Config {
    runner:  *mach.manifest.runner.Runner;
    threads: u32;
    timeout: std.chrono.duration.Duration;
    level:   u8;
    bytes:   bool;
}
```

how a run executes its tests

| Field | Type | |
| :--- | :--- | :--- |
| `runner` | `*mach.manifest.runner.Runner` | what each test is launched through, or nil for the test to run itself |
| `threads` | `u32` | how many tests run at once, at least one |
| `timeout` | `std.chrono.duration.Duration` | each test's bound, positive |
| `level` | `u8` | the readout level; a passing test's output is kept only at the items level |
| `bytes` | `bool` | whether a test's captured output is kept for the renderer |

### run

```mach
pub fun run(a: *std.allocator.Allocator, sink: *mach.report.readout.Sink, arts: *mach.report.testrun.TestArtifact, n: u32, config: *Config) res[Counts, mach.base.fail.Fail]
```

run one set of tests, each reported to sink as it finishes, and close the run
with its summary

| Parameter | |
| :--- | :--- |
| `a` | backs the results, the logs' paths and every process |
| `sink` | the readout the run reports to |
| `arts` | the tests |
| `n` | how many there are |
| `config` | how they run |
| **returns** | the run's counts; err when the run could not start, before any test ran |

## Counts

```mach
pub rec Counts {
    failed:         u32;
    infrastructure: u32;
}
```

what a finished run decides: how many tests did not pass, and how many of
those the run's own machinery failed rather than the test

| Field | Type | |
| :--- | :--- | :--- |
| `failed` | `u32` | the tests whose result is neither a pass nor a skip |
| `infrastructure` | `u32` | of those, the ones whose result is an infrastructure failure |

