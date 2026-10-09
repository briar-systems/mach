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

