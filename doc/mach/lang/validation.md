# mach.lang.validation

## tag EvidenceError

```mach
pub tag EvidenceError: u8 {
    alloc:   A.Error;
    release: A.Error;
    invalid_input;
    invalid_copy;
    no_allocator;
    truncated;
    truncated_schema;
    truncated_identity;
    truncated_profile;
    truncated_observation;
    envelope;
    schema;
    identity_mismatch;
    short_limit;
    capture_size;
    trailing;
    field;
}
```

an evidence record's refusal: the allocator's, an envelope, schema or
payload that does not decode, a field a platform cannot represent, or an
input that fails the same validity the decoder demands

## def ValidationGateResultKind

```mach
pub def ValidationGateResultKind: u8
```

## val RESULT_PASS

```mach
pub val RESULT_PASS:                   ValidationGateResultKind = 0
```

## val RESULT_SEMANTIC_MISMATCH

```mach
pub val RESULT_SEMANTIC_MISMATCH:      ValidationGateResultKind = 1
```

## val RESULT_UNSUPPORTED_INPUT

```mach
pub val RESULT_UNSUPPORTED_INPUT:      ValidationGateResultKind = 2
```

## val RESULT_TOOL_UNAVAILABLE

```mach
pub val RESULT_TOOL_UNAVAILABLE:       ValidationGateResultKind = 3
```

## val RESULT_INFRASTRUCTURE_FAILURE

```mach
pub val RESULT_INFRASTRUCTURE_FAILURE: ValidationGateResultKind = 4
```

## val RESULT_RESOURCE_EXHAUSTION

```mach
pub val RESULT_RESOURCE_EXHAUSTION:    ValidationGateResultKind = 5
```

## val RESULT_INVALID_INJECTION

```mach
pub val RESULT_INVALID_INJECTION:      ValidationGateResultKind = 6
```

## def ValidationCause

```mach
pub def ValidationCause: u8
```

## val CAUSE_PASS

```mach
pub val CAUSE_PASS: ValidationCause = 0
```

## val CAUSE_ORACLE_TIMEOUT

```mach
pub val CAUSE_ORACLE_TIMEOUT:      ValidationCause = 6
```

## val CAUSE_ORACLE_SPAWN_FAILURE

```mach
pub val CAUSE_ORACLE_SPAWN_FAILURE: ValidationCause = 9
```

## val CAUSE_ORACLE_CAPTURE

```mach
pub val CAUSE_ORACLE_CAPTURE:       ValidationCause = 10
```

## val CAUSE_ORACLE_SIGNAL

```mach
pub val CAUSE_ORACLE_SIGNAL:        ValidationCause = 11
```

## val CAUSE_ORACLE_EXIT

```mach
pub val CAUSE_ORACLE_EXIT:          ValidationCause = 12
```

## val CAUSE_INVALID_TERMINAL

```mach
pub val CAUSE_INVALID_TERMINAL:     ValidationCause = 13
```

## rec ValidationGateId

```mach
pub rec ValidationGateId;
```

## rec ValidationGateResult

```mach
pub rec ValidationGateResult;
```

## fun gate_result

```mach
pub fun gate_result(kind: ValidationGateResultKind, cause: ValidationCause,
detail_identity: str) ValidationGateResult;
```

## def InjectionDomain

```mach
pub def InjectionDomain: u8
```

## def InjectionEffect

```mach
pub def InjectionEffect: u8
```

## rec Injection

```mach
pub rec Injection;
```

## def IoAttemptKind

```mach
pub def IoAttemptKind: u8
```

## rec IoAttempt

```mach
pub rec IoAttempt;
```

## rec ControlledClock

```mach
pub rec ControlledClock;
```

## rec Deadline

```mach
pub rec Deadline;
```

## rec SeedControl

```mach
pub rec SeedControl;
```

## rec BoundedSchedule

```mach
pub rec BoundedSchedule;
```

## rec Replay

```mach
pub rec Replay;
```

## rec Minimizer

```mach
pub rec Minimizer;
```

## rec ResourceLimits

```mach
pub rec ResourceLimits;
```

## rec OracleProfile

```mach
pub rec OracleProfile;
```

## rec OracleInput

```mach
pub rec OracleInput;
```

## rec OracleObservation

```mach
pub rec OracleObservation;
```

## rec Reproducibility

```mach
pub rec Reproducibility;
```

## rec EvidenceInput

```mach
pub rec EvidenceInput;
```

## rec ValidationEvidenceView

```mach
pub rec ValidationEvidenceView;
```

## def ValidationEvidence

```mach
pub def ValidationEvidence: ptr
```

