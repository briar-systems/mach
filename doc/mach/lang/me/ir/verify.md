# mach.lang.me.ir.verify

## val VC_AGG_ADDRESS

```mach
pub val VC_AGG_ADDRESS: VerifyCheck = 10
```

## rec Violation

```mach
pub rec Violation;
```

## rec VerifyReport

```mach
pub rec VerifyReport;
```

## fun opts

```mach
pub fun opts(tgt: *resolved.Target, full: bool, repr: bool) VerifyOpts;
```

## fun verify_module_ext

```mach
pub fun verify_module_ext(m: *me_ir.Module, alloc: *A.Allocator, o: VerifyOpts) res[VerifyReport, fail.Fail];
```

## fun dnit_report

```mach
pub fun dnit_report(r: *VerifyReport);
```

## fun describe

```mach
pub fun describe(check: VerifyCheck) opt[str];
```

the text of a check; absent for a tag outside the catalog

## fun violation_loc

```mach
pub fun violation_loc(m: *me_ir.Module, v: *Violation) lang_source.SrcLoc;
```

## fun describe_located

```mach
pub fun describe_located(m: *me_ir.Module, v: *Violation, itn: *intern.Interner,
srcmap: *lang_source.SourceMap, alloc: *A.Allocator) res[str, fail.Fail];
```

successful text is caller-owned, and allocation failures never produce partial descriptions

