[mach](../../README.md) / [front](../README.md) / [comptime](README.md) / **failure**

# mach.front.comptime.failure

why a comptime evaluation has no value: refused for good, waiting on a later phase or
an instance, or failed inside the compiler

Source: [`src/front/comptime/failure.mach`](../../../../src/front/comptime/failure.mach)

Uses types from [`mach.base.diagnostic`](../../base/diagnostic.md), [`mach.base.diagnostic.kind`](../../base/diagnostic/kind.md), [`mach.base.fail`](../../base/fail.md), [`mach.base.source`](../../base/source.md).

| Type | | |
| :--- | :--- | :--- |
| [`EvalFailKind`](#evalfailkind) | `def` |  |
| [`EvalFail`](#evalfail) | `rec` | diag: the diagnostic kind the failure is reported as, where it reaches the user |

| On `EvalFailKind` | |
| :--- | :--- |
| [`EVAL_FAIL_NONE`](#evalfailkind), [`EVAL_FAIL_REJECTED`](#evalfailkind), [`EVAL_FAIL_UNBOUND`](#evalfailkind), [`EVAL_FAIL_NEEDS_MEMBER`](#evalfailkind), [`EVAL_FAIL_NEEDS_TYPES`](#evalfailkind), [`EVAL_FAIL_NEEDS_LAYOUT`](#evalfailkind), [`EVAL_FAIL_INTERNAL`](#evalfailkind), [`EVAL_FAIL_NEEDS_INSTANCE`](#evalfailkind), [`EVAL_FAIL_NOT_INTEGER`](#evalfailkind) | |
| [`EVAL_FAIL_TYPED`](#evalfailkind) | a refusal typing already reported, so no reader reports it again |
| [`eval_error`](#eval_error) | a failure of the given kind |
| [`typed_alone`](#typed_alone) | whether a refusal is one typing reports itself wherever the expression is typed, naming more than the evaluator can: an expression typing refused, or a suffixed literal its suffix's type does not hold |
| [`not_constant`](#not_constant) | whether a failure of a global initializer's own evaluation means it is no constant: it needs a value, a member, the types or a layout no constant gives it |
| [`no_value`](#no_value) | whether a failure of kind `kind` reported as `diag` says the expression has no compile-time value at all, as a run-time binding, a call or a name no constant binds gives none, rather than a fault in a value it has |

| On `EvalFail` | |
| :--- | :--- |
| [`eval_internal`](#eval_internal) | an internal failure of the evaluator, a compiler defect wherever it surfaces |
| [`eval_from_fail`](#eval_from_fail) | the evaluation failure a compiler failure means: a failure already reported is a rejection under `diag`, any other an internal failure carrying its description |
| [`eval_from_refusal`](#eval_from_refusal) | a failure a walk with no place of its own returned to an evaluation: a refusal of the input is rejected under its own kind, which the evaluation reports at the node it stands on, and any other failure reads as eval_from_fail reads it |
| [`stands_alone`](#stands_alone) | a refusal reported as itself wherever a compile-time value was asked for, since it says why no value exists better than the asker can: a secret |

| Functions | |
| :--- | :--- |
| [`init_not_constant_report`](#init_not_constant_report) | reports the initializer at `span` of the global `name` as no constant, as lowering meets it; a decorator argument has no name |
| [`init_no_value_report`](#init_no_value_report) | reports the initializer at `span` of the global `name` as no constant, as init_not_constant_report does, with the evaluator's reason it has no value as a note |

---

## EvalFailKind

```mach
pub def EvalFailKind: u8
```

| Name | Value | |
| :--- | :--- | :--- |
| `EVAL_FAIL_NONE` | `0` |  |
| `EVAL_FAIL_REJECTED` | `1` |  |
| `EVAL_FAIL_UNBOUND` | `2` |  |
| `EVAL_FAIL_NEEDS_MEMBER` | `3` |  |
| `EVAL_FAIL_NEEDS_TYPES` | `4` |  |
| `EVAL_FAIL_NEEDS_LAYOUT` | `5` |  |
| `EVAL_FAIL_INTERNAL` | `6` |  |
| `EVAL_FAIL_NEEDS_INSTANCE` | `7` |  |
| `EVAL_FAIL_NOT_INTEGER` | `8` |  |
| `EVAL_FAIL_TYPED` | `9` | a refusal typing already reported, so no reader reports it again |

### eval_error

```mach
pub fun eval_error(kind: EvalFailKind, diag: mach.base.diagnostic.kind.Kind, message: str) EvalFail
```

a failure of the given kind

| Parameter | |
| :--- | :--- |
| `kind` | why the evaluation has no value |
| `diag` | the diagnostic kind it is reported as |
| `message` | the text reported, borrowed |
| **returns** | the failure |

### typed_alone

```mach
pub fun typed_alone(kind: EvalFailKind, diag: mach.base.diagnostic.kind.Kind) bool
```

whether a refusal is one typing reports itself wherever the expression is typed, naming
more than the evaluator can: an expression typing refused, or a suffixed literal its suffix's type does not hold

### not_constant

```mach
pub fun not_constant(kind: EvalFailKind) bool
```

whether a failure of a global initializer's own evaluation means it is no constant: it needs a
value, a member, the types or a layout no constant gives it

### no_value

```mach
pub fun no_value(kind: EvalFailKind, diag: mach.base.diagnostic.kind.Kind) bool
```

whether a failure of kind `kind` reported as `diag` says the expression has no compile-time
value at all, as a run-time binding, a call or a name no constant binds gives none, rather
than a fault in a value it has. a context that asks for a constant refuses it under its own kind

## EvalFail

```mach
pub rec EvalFail {
    kind:    EvalFailKind;
    diag:    mach.base.diagnostic.kind.Kind;
    message: str;
}
```

diag: the diagnostic kind the failure is reported as, where it reaches the user

### eval_internal

```mach
pub fun eval_internal(message: str) EvalFail
```

an internal failure of the evaluator, a compiler defect wherever it surfaces

### eval_from_fail

```mach
pub fun eval_from_fail(f: mach.base.fail.Fail, diag: mach.base.diagnostic.kind.Kind, rejected_message: str) EvalFail
```

the evaluation failure a compiler failure means: a failure already reported is a rejection
under `diag`, any other an internal failure carrying its description

| Parameter | |
| :--- | :--- |
| `f` | the failure |
| `diag` | the diagnostic kind a rejection is reported as |
| `rejected_message` | the text of a rejection |
| **returns** | the evaluation failure |

### eval_from_refusal

```mach
pub fun eval_from_refusal(f: mach.base.fail.Fail, diag: mach.base.diagnostic.kind.Kind, rejected_message: str) EvalFail
```

a failure a walk with no place of its own returned to an evaluation: a refusal of the input
is rejected under its own kind, which the evaluation reports at the node it stands on, and
any other failure reads as eval_from_fail reads it

### stands_alone

```mach
pub fun stands_alone(f: EvalFail) bool
```

a refusal reported as itself wherever a compile-time value was asked for, since it says why no
value exists better than the asker can: a secret

## Functions

### init_not_constant_report

```mach
pub fun init_not_constant_report(alloc: *std.allocator.Allocator, diags: *mach.base.diagnostic.DiagnosticStore, file: mach.base.source.FileId, span: mach.base.source.Span, name: opt[str]) err[mach.base.fail.Fail]
```

reports the initializer at `span` of the global `name` as no constant, as lowering meets
it; a decorator argument has no name

### init_no_value_report

```mach
pub fun init_no_value_report(diags: *mach.base.diagnostic.DiagnosticStore, file: mach.base.source.FileId, span: mach.base.source.Span, name: str, why: str) err[mach.base.fail.Fail]
```

reports the initializer at `span` of the global `name` as no constant, as
init_not_constant_report does, with the evaluator's reason it has no value as a note

