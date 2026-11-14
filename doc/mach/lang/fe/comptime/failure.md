# mach.lang.fe.comptime.failure

why a comptime evaluation has no value: refused for good, waiting on a later phase or
an instance, or failed inside the compiler

## def EvalFailKind

```mach
pub def EvalFailKind: u8
```

## val EVAL_FAIL_NONE

```mach
pub val EVAL_FAIL_NONE:           EvalFailKind = 0
```

## val EVAL_FAIL_REJECTED

```mach
pub val EVAL_FAIL_REJECTED:       EvalFailKind = 1
```

## val EVAL_FAIL_UNBOUND

```mach
pub val EVAL_FAIL_UNBOUND:        EvalFailKind = 2
```

## val EVAL_FAIL_NEEDS_MEMBER

```mach
pub val EVAL_FAIL_NEEDS_MEMBER:   EvalFailKind = 3
```

## val EVAL_FAIL_NEEDS_TYPES

```mach
pub val EVAL_FAIL_NEEDS_TYPES:    EvalFailKind = 4
```

## val EVAL_FAIL_NEEDS_LAYOUT

```mach
pub val EVAL_FAIL_NEEDS_LAYOUT:   EvalFailKind = 5
```

## val EVAL_FAIL_INTERNAL

```mach
pub val EVAL_FAIL_INTERNAL:       EvalFailKind = 6
```

## val EVAL_FAIL_NEEDS_INSTANCE

```mach
pub val EVAL_FAIL_NEEDS_INSTANCE: EvalFailKind = 7
```

## val EVAL_FAIL_NOT_INTEGER

```mach
pub val EVAL_FAIL_NOT_INTEGER:    EvalFailKind = 8
```

## rec EvalFail

```mach
pub rec EvalFail;
```

diag: the diagnostic kind the failure is reported as, where it reaches the user

## fun eval_error

```mach
pub fun eval_error(kind: EvalFailKind, diag: diagnostic_kind.Kind, message: str) EvalFail;
```

## fun eval_internal

```mach
pub fun eval_internal(message: str) EvalFail;
```

an internal failure of the evaluator, a compiler defect wherever it surfaces

## fun eval_from_fail

```mach
pub fun eval_from_fail(f: fail.Fail, diag: diagnostic_kind.Kind, rejected_message: str) EvalFail;
```

## fun eval_from_refusal

```mach
pub fun eval_from_refusal(f: fail.Fail, diag: diagnostic_kind.Kind, rejected_message: str) EvalFail;
```

a failure a walk with no place of its own returned to an evaluation: a refusal of the input
is rejected under its own kind, which the evaluation reports at the node it stands on, and
any other failure reads as eval_from_fail reads it

## fun stands_alone

```mach
pub fun stands_alone(f: EvalFail) bool;
```

whether a failure of a global initializer's own evaluation means it is no constant: it needs a
value, a member, the types or a layout no constant gives it
a refusal reported as itself wherever a compile-time value was asked for, since it says why no
value exists better than the asker can: a secret, or a constant bound per target tuple

## fun not_constant

```mach
pub fun not_constant(kind: EvalFailKind) bool;
```

## fun init_not_constant_report

```mach
pub fun init_not_constant_report(alloc: *A.Allocator, diags: *diagnostic.DiagnosticStore, file: lang_source.FileId, span: lang_source.Span, name: opt[str]) err[fail.Fail];
```

reports the initializer at `span` of the global `name` as no constant, the one report check
and lowering both give it; a decorator argument has no name

