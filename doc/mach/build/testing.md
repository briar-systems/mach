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
| [`TestScope`](#testscope) | `rec` |  |

| On `TestScope` | |
| :--- | :--- |
| [`collect_scope`](#collect_scope) | col is the unit's test declarations in module order; the scope owns it from here |
| [`scope_dnit`](#scope_dnit), [`record_tests`](#record_tests) | |
| [`link_dispatcher`](#link_dispatcher) | synthesize the dispatcher over the selected tests, write its object to `dispatcher`, and link it with the test objects and the normal objects into `artifact`, answering what the link read |
| [`fingerprint_scope`](#fingerprint_scope) | |

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

### collect_scope

```mach
pub fun collect_scope(p: *mach.compile.project.Project, col: mach.lower.testrunner.Collected, sc: *TestScope) err[mach.base.fail.Fail]
```

col is the unit's test declarations in module order; the scope owns it from here

### scope_dnit

```mach
pub fun scope_dnit(p: *mach.compile.project.Project, sc: *TestScope)
```

### record_tests

```mach
pub fun record_tests(p: *mach.compile.project.Project, sc: *TestScope, exe: *u8, target: str, profile: str,
    bo: *mach.build.outcome.BuildOutcome, oa: *std.allocator.Allocator) err[mach.base.fail.Fail]
```

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
| `modules` | the build's current objects: test and normal objects in topological order |

### fingerprint_scope

```mach
pub fun fingerprint_scope(s: *mach.base.wire.Sink, p: *mach.compile.project.Project, sc: *TestScope)
```

