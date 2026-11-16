[mach](../README.md) / [build](README.md) / **testing**

# mach.build.testing

a test build's tests: the run's selection, its dispatcher, and a test process's verdict

a run selects the project's own tests, or its dependencies' too under `--include-deps`,
whose qualified name contains the filter. the dispatcher is the test executable's
`main`, and an artifact's own `main` yields to it.

<sub>source: [`src/build/testing.mach`](../../../src/build/testing.mach) · imported by 2 modules</sub>

<sub>`testrunner` is [`mach.lower.testrunner`](../lower/testrunner.md) · `project` is [`mach.compile.project`](../compile/project.md) · `fail` is [`mach.base.fail`](../base/fail.md) · `outcome` is [`mach.build.outcome`](outcome.md) · `A` is `std.allocator` · `diagnostic` is [`mach.base.diagnostic`](../base/diagnostic.md) · `input` is [`mach.target.input`](../target/input.md) · `backend` is [`mach.compile.backend`](../compile/backend.md) · `wire` is [`mach.base.wire`](../base/wire.md) · `subprocess` is [`mach.host.subprocess`](../host/subprocess.md) · `testrun` is [`mach.report.testrun`](../report/testrun.md)</sub>

| Type | | |
| :--- | :--- | :--- |
| [`TestScope`](#testscope) | `rec` |  |

| On `TestScope` | |
| :--- | :--- |
| [`collect_scope`](#collect_scope) | col is the unit's test declarations in module order; the scope owns it from here |
| [`scope_dnit`](#scope_dnit) · [`record_tests`](#record_tests) | |
| [`link_dispatcher`](#link_dispatcher) | synthesize the dispatcher over the selected tests, write its object to `dispatcher`, and link it with the test objects and the normal objects into `artifact`, answering what the link read |
| [`fingerprint_scope`](#fingerprint_scope) | |

| Functions | |
| :--- | :--- |
| [`classify_subprocess`](#classify_subprocess) · [`classify_setup_failure`](#classify_setup_failure) · [`classify_wait_failure`](#classify_wait_failure) | |

---

## TestScope

```mach
pub rec TestScope {
    col:     testrunner.Collected;
    have:    bool;
    pick:    *testrunner.Test;
    sel_len: u32;
}
```

### collect_scope

```mach
pub fun collect_scope(p: *project.Project, col: testrunner.Collected, sc: *TestScope) err[fail.Fail]
```

col is the unit's test declarations in module order; the scope owns it from here

### scope_dnit

```mach
pub fun scope_dnit(p: *project.Project, sc: *TestScope)
```

### record_tests

```mach
pub fun record_tests(p: *project.Project, sc: *TestScope, exe: *u8, target: str, profile: str,
    bo: *outcome.BuildOutcome, oa: *A.Allocator) err[fail.Fail]
```

### link_dispatcher

```mach
pub fun link_dispatcher(p: *project.Project, diags: *diagnostic.DiagnosticStore, sc: *TestScope, links: *Vector[input.Input],
    modules: *Given, module_len: u32, dispatcher: str, artifact: str) res[backend.LinkRead, fail.Fail]
```

synthesize the dispatcher over the selected tests, write its object to `dispatcher`, and
link it with the test objects and the normal objects into `artifact`, answering what the link
read

| Parameter | |
| :--- | :--- |
| `modules` | the build's current objects: test and normal objects in topological order |

### fingerprint_scope

```mach
pub fun fingerprint_scope(s: *wire.Sink, p: *project.Project, sc: *TestScope)
```

## Functions

### classify_subprocess

```mach
pub fun classify_subprocess(term: *subprocess.SubprocessTerminal, expired: bool, cap_ok: bool, cap: *subprocess.CaptureIoResult) testrun.GateResult
```

### classify_setup_failure

```mach
pub fun classify_setup_failure() testrun.GateResult
```

### classify_wait_failure

```mach
pub fun classify_wait_failure() testrun.GateResult
```

