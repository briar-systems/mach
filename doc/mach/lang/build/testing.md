# mach.lang.build.testing

## fun classify_subprocess

```mach
pub fun classify_subprocess(term: *subprocess.SubprocessTerminal, expired: bool, cap_ok: bool, cap: *subprocess.CaptureIoResult) validation.ValidationGateResult;
```

## fun classify_setup_failure

```mach
pub fun classify_setup_failure() validation.ValidationGateResult;
```

## fun classify_timeout

```mach
pub fun classify_timeout() validation.ValidationGateResult;
```

## fun classify_wait_failure

```mach
pub fun classify_wait_failure() validation.ValidationGateResult;
```

## rec TestScope

```mach
pub rec TestScope;
```

## fun collect_scope

```mach
pub fun collect_scope(p: *driver.Project, col: testrunner.Collected, sc: *TestScope) err[outcome.Fail];
```

col is the unit's test declarations in module order; the scope owns it from here

## fun free_scope

```mach
pub fun free_scope(p: *driver.Project, sc: *TestScope);
```

## fun record_tests

```mach
pub fun record_tests(p: *driver.Project, sc: *TestScope, exe: *u8,
bo: *outcome.BuildOutcome, oa: *A.Allocator) err[outcome.Fail];
```

## rec DispatchInputs

```mach
pub rec DispatchInputs;
```

what the dispatcher links beside the build's images: the external objects and
the dynamic libraries the artifact names

## fun collect_dispatch_inputs

```mach
pub fun collect_dispatch_inputs(p: *driver.Project, unit: *plan.BuildUnit) res[DispatchInputs, outcome.Fail];
```

## fun free_dispatch_inputs

```mach
pub fun free_dispatch_inputs(p: *driver.Project, di: *DispatchInputs);
```

## fun link_dispatcher

```mach
pub fun link_dispatcher(p: *driver.Project, sc: *TestScope, di: *DispatchInputs,
modules: *of.ObjectImage, module_len: u32, dispatcher: str, artifact: str) err[outcome.Fail];
```

synthesize the dispatcher over the selected tests, write it to `dispatcher`, and
link it with the test objects and the normal objects into `artifact`. the images
are compiler-produced, so the link keeps only what the selected tests reach

modules: the build's current images: normal and test objects in topological order

## fun fingerprint_scope

```mach
pub fun fingerprint_scope(fb: *dq.FpBuf, p: *driver.Project, sc: *TestScope) err[outcome.Fail];
```

