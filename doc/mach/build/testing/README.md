[mach](../../README.md) / [build](../README.md) / **testing**

# mach.build.testing

a test build's tests: the run's selection, its dispatcher, and a test process's verdict

a run selects the project's own tests, or its dependencies' too under `--include-deps`,
whose qualified name contains the filter. the dispatcher is the test executable's
`main`, and an artifact's own `main` yields to it.

<sub>[`mach.build.testing`](../testing.md) · 1 module · imports [`base`](../../base/README.md), [`compile`](../../compile/README.md), [`host`](../../host/README.md), [`lower`](../../lower/README.md), [`manifest`](../../manifest/README.md), [`report`](../../report/README.md), [`target`](../../target/README.md) · imported by [`cli`](../../cli/README.md)</sub>

| Module | |
| :--- | :--- |
| [`pool`](pool.md) | the test process pool: every test of a run is its own process, up to a window of them at once, each bounded by the run's timeout and its output captured to a log beside the dispatcher |

