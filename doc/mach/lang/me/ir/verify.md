# mach.lang.me.ir.verify

## def VerifyCheck

```mach
pub def VerifyCheck: u8
```

## val VC_SSA

```mach
pub val VC_SSA:             VerifyCheck = 0
```

## val VC_TERM_PRESENCE

```mach
pub val VC_TERM_PRESENCE:   VerifyCheck = 1
```

## val VC_TERM_UNIQUE

```mach
pub val VC_TERM_UNIQUE:     VerifyCheck = 2
```

## val VC_SUCC_EXISTS

```mach
pub val VC_SUCC_EXISTS:     VerifyCheck = 3
```

## val VC_PRED_CONSISTENT

```mach
pub val VC_PRED_CONSISTENT: VerifyCheck = 4
```

## val VC_PHI_COVERAGE

```mach
pub val VC_PHI_COVERAGE:    VerifyCheck = 5
```

## val VC_OPERAND_TYPE

```mach
pub val VC_OPERAND_TYPE:    VerifyCheck = 6
```

## val VC_CONST_TYPE

```mach
pub val VC_CONST_TYPE:      VerifyCheck = 7
```

## val VC_DOMINANCE

```mach
pub val VC_DOMINANCE:       VerifyCheck = 8
```

## val VC_REACHABILITY

```mach
pub val VC_REACHABILITY:    VerifyCheck = 9
```

## val VC_AGG_ADDRESS

```mach
pub val VC_AGG_ADDRESS: VerifyCheck = 10
```

## val VC_TYPE_RESOLVE

```mach
pub val VC_TYPE_RESOLVE: VerifyCheck = 11
```

## val VC_DANGLING_ID

```mach
pub val VC_DANGLING_ID: VerifyCheck = 12
```

## val VC_TYPE_AGREE

```mach
pub val VC_TYPE_AGREE: VerifyCheck = 13
```

## val VC_CALL_ARG_TYPE

```mach
pub val VC_CALL_ARG_TYPE: VerifyCheck = 14
```

## val VC_BITCAST_CLASS

```mach
pub val VC_BITCAST_CLASS: VerifyCheck = 15
```

## val VC_CONVERT_WIDTH

```mach
pub val VC_CONVERT_WIDTH: VerifyCheck = 16
```

## val VC_DANGLING_OPERAND

```mach
pub val VC_DANGLING_OPERAND: VerifyCheck = 17
```

## val VC_VOLATILE_FORM

```mach
pub val VC_VOLATILE_FORM: VerifyCheck = 18
```

## val VC_OPCODE_KNOWN

```mach
pub val VC_OPCODE_KNOWN: VerifyCheck = 19
```

## val VC_VALUE_KNOWN

```mach
pub val VC_VALUE_KNOWN:  VerifyCheck = 20
```

## rec Violation

```mach
pub rec Violation;
```

## rec VerifyReport

```mach
pub rec VerifyReport;
```

## rec VerifyOpts

```mach
pub rec VerifyOpts;
```

## fun opts

```mach
pub fun opts(tgt: *resolved.Target, full: bool, repr: bool) VerifyOpts;
```

## fun verify_module_ext

```mach
pub fun verify_module_ext(m: *ir.Module, alloc: *A.Allocator, o: VerifyOpts) res[VerifyReport, fail.Fail];
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
pub fun violation_loc(m: *ir.Module, v: *Violation) source.SrcLoc;
```

## fun describe_located

```mach
pub fun describe_located(m: *ir.Module, v: *Violation, itn: *intern.Interner,
srcmap: *source.SourceMap, alloc: *A.Allocator) res[str, fail.Fail];
```

successful text is caller-owned, and allocation failures never produce partial descriptions

