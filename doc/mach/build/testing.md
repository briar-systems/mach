[mach](../README.md) / [build](README.md) / **testing**

# mach.build.testing

a test build's tests: the run's selection and its dispatcher

a run selects the project's own tests, or its dependencies' too under `--include-deps`,
whose qualified name contains the filter. the dispatcher is the test executable's
`main`, and an artifact's own `main` yields to it.

Source: [`src/build/testing.mach`](../../../src/build/testing.mach)

Uses types from [`mach.base.diagnostic`](../base/diagnostic.md), [`mach.base.fail`](../base/fail.md), [`mach.base.wire`](../base/wire.md), [`mach.build.outcome`](outcome.md), [`mach.compile.backend`](../compile/backend.md), [`mach.compile.project`](../compile/project.md), [`mach.lower.testrunner`](../lower/testrunner.md), [`mach.target.input`](../target/input.md).

| Type | | |
| :--- | :--- | :--- |
| [`TestScope`](#testscope) | `rec` | the tests a test build collected and the ones its run selects |

| On `TestScope` | |
| :--- | :--- |
| [`collect_scope`](#collect_scope) | col is the unit's test declarations in module order; the scope owns it from here |
| [`scope_dnit`](#scope_dnit) | release what the scope holds and leave it empty, which a scope already empty survives |
| [`record_tests`](#record_tests) | record each selected test in `bo`, in selection order, as the runner reads it |
| [`link_dispatcher`](#link_dispatcher) | synthesize the dispatcher over the selected tests, write its object to `dispatcher`, and link it with the test objects and the normal objects into `artifact`, answering what the link read |
| [`fingerprint_scope`](#fingerprint_scope) | write the selection into a link configuration fingerprint, so a changed selection relinks the dispatcher |

---

## TestScope

```mach
pub rec TestScope {
    col:     mach.lower.testrunner.Collected;
    have:    bool;
    pick:    *mach.lower.testrunner.Test;
    sel_len: u32;
}
```

the tests a test build collected and the ones its run selects

| Field | Type | |
| :--- | :--- | :--- |
| `col` | `mach.lower.testrunner.Collected` | the test declarations collected, in module order, owned once `have` is set |
| `have` | `bool` | whether `col` is held and must be released |
| `pick` | `*mach.lower.testrunner.Test` | the selected tests, copied out of `col`, an array of `col.count` slots, nil for none |
| `sel_len` | `u32` | how many of `pick` are selected |

### collect_scope

```mach
pub fun collect_scope(p: *mach.compile.project.Project, col: mach.lower.testrunner.Collected, sc: *TestScope) err[mach.base.fail.Fail]
```

col is the unit's test declarations in module order; the scope owns it from here

### scope_dnit

```mach
pub fun scope_dnit(p: *mach.compile.project.Project, sc: *TestScope)
```

release what the scope holds and leave it empty, which a scope already empty survives

| Parameter | |
| :--- | :--- |
| `p` | the project whose allocator and session the scope's parts came from |
| `sc` | the scope |

### record_tests

```mach
pub fun record_tests(p: *mach.compile.project.Project, sc: *TestScope, exe: *u8, target: str, profile: str,
    bo: *mach.build.outcome.BuildOutcome, oa: *std.allocator.Allocator) err[mach.base.fail.Fail]
```

record each selected test in `bo`, in selection order, as the runner reads it

| Parameter | |
| :--- | :--- |
| `p` | the project the tests were collected from |
| `sc` | the scope holding the selection |
| `exe` | the dispatcher executable the tests run in, nil when they are only listed |
| `target` | the name of the target they were built for |
| `profile` | the profile they were built under |
| `bo` | the outcome that records them, keeping its own copies |
| `oa` | the outcome's allocator, which recording does not read |
| **returns** | ok, or the failure of the first test that could not be recorded |

### link_dispatcher

```mach
pub fun link_dispatcher(p: *mach.compile.project.Project, diags: *mach.base.diagnostic.DiagnosticStore, sc: *TestScope, links: *Vector[mach.target.input.Input],
    modules: *Given, module_len: u32, dispatcher: str, artifact: str) res[mach.compile.backend.LinkRead, mach.base.fail.Fail]
```

synthesize the dispatcher over the selected tests, write its object to `dispatcher`, and
link it with the test objects and the normal objects into `artifact`, answering what the link
read

| Parameter | |
| :--- | :--- |
| `modules` | the build's current objects: the test objects in topological order, then the normal objects in the link's order |

### fingerprint_scope

```mach
pub fun fingerprint_scope(s: *mach.base.wire.Sink, p: *mach.compile.project.Project, sc: *TestScope)
```

write the selection into a link configuration fingerprint, so a changed selection relinks
the dispatcher

| Parameter | |
| :--- | :--- |
| `s` | the sink the fingerprint is written to |
| `p` | the project whose names the tests carry |
| `sc` | the scope holding the selection |

