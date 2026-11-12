# mach.lang.build.testing

## fun classify_subprocess

```mach
pub fun classify_subprocess(term: *subprocess.SubprocessTerminal, expired: bool, cap_ok: bool, cap: *subprocess.CaptureIoResult) testrun.GateResult;
```

## fun classify_setup_failure

```mach
pub fun classify_setup_failure() testrun.GateResult;
```

## fun classify_wait_failure

```mach
pub fun classify_wait_failure() testrun.GateResult;
```

## rec TestScope

```mach
pub rec TestScope;
```

## fun collect_scope

```mach
pub fun collect_scope(p: *project.Project, col: testrunner.Collected, sc: *TestScope) err[fail.Fail];
```

col is the unit's test declarations in module order; the scope owns it from here

## fun free_scope

```mach
pub fun free_scope(p: *project.Project, sc: *TestScope);
```

## fun record_tests

```mach
pub fun record_tests(p: *project.Project, sc: *TestScope, exe: *u8, target: str, profile: str,
bo: *outcome.BuildOutcome, oa: *A.Allocator) err[fail.Fail];
```

## fun link_dispatcher

```mach
pub fun link_dispatcher(p: *project.Project, diags: *diagnostic.DiagnosticStore, sc: *TestScope, links: *Vector[input.Input],
modules: *Given, module_len: u32, dispatcher: str, artifact: str) res[backend.LinkRead, fail.Fail];
```

synthesize the dispatcher over the selected tests, write its object to `dispatcher`, and
link it with the test objects and the normal objects into `artifact`, answering what the link
read

modules: the build's current objects: test and normal objects in topological order

## fun fingerprint_scope

```mach
pub fun fingerprint_scope(s: *wire.Sink, p: *project.Project, sc: *TestScope);
```

