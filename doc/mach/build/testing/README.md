[mach](../../README.md) / [build](../README.md) / **testing**

# mach.build.testing

a test build's tests: the run's selection and its dispatcher

a run selects the project's own tests, or its dependencies' too under `--include-deps`,
whose qualified name contains the filter. the dispatcher is the test executable's
`main`, and an artifact's own `main` yields to it.

The directory's root module is [`mach.build.testing`](../testing.md).

| Module | |
| :--- | :--- |
| [`pool`](pool.md) | the test process pool: every test of a run is its own process, up to a window of them at once, each bounded by the run's timeout and its output captured to a log beside the dispatcher |

