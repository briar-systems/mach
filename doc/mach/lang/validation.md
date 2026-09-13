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

## fun evidence_text

```mach
pub fun evidence_text(e: EvidenceError) str;
```

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
pub val CAUSE_PASS:                 ValidationCause = 0
```

## val CAUSE_MISMATCH

```mach
pub val CAUSE_MISMATCH:             ValidationCause = 1
```

## val CAUSE_UNSUPPORTED

```mach
pub val CAUSE_UNSUPPORTED:          ValidationCause = 2
```

## val CAUSE_TOOL_UNAVAILABLE

```mach
pub val CAUSE_TOOL_UNAVAILABLE:     ValidationCause = 3
```

## val CAUSE_INVALID_INJECTOR

```mach
pub val CAUSE_INVALID_INJECTOR:     ValidationCause = 4
```

## val CAUSE_UNREACHED_INJECTION

```mach
pub val CAUSE_UNREACHED_INJECTION:  ValidationCause = 5
```

## val CAUSE_ORACLE_TIMEOUT

```mach
pub val CAUSE_ORACLE_TIMEOUT:       ValidationCause = 6
```

## val CAUSE_ORACLE_OUTPUT_LIMIT

```mach
pub val CAUSE_ORACLE_OUTPUT_LIMIT:  ValidationCause = 7
```

## val CAUSE_ORACLE_CANCELED

```mach
pub val CAUSE_ORACLE_CANCELED:      ValidationCause = 8
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

## val CAUSE_MALFORMED_OUTPUT

```mach
pub val CAUSE_MALFORMED_OUTPUT:     ValidationCause = 14
```

## rec ValidationGateId

```mach
pub rec ValidationGateId;
```

## rec ValidationGateResult

```mach
pub rec ValidationGateResult;
```

## fun gate_id_valid

```mach
pub fun gate_id_valid(id: *ValidationGateId) bool;
```

## fun gate_result_valid

```mach
pub fun gate_result_valid(r: *ValidationGateResult) bool;
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

## val INJECTION_ALLOCATION

```mach
pub val INJECTION_ALLOCATION: InjectionDomain = 0
```

## val INJECTION_READ

```mach
pub val INJECTION_READ:       InjectionDomain = 1
```

## val INJECTION_WRITE

```mach
pub val INJECTION_WRITE:      InjectionDomain = 2
```

## val INJECTION_CLOCK

```mach
pub val INJECTION_CLOCK:      InjectionDomain = 3
```

## val INJECTION_SCHEDULE

```mach
pub val INJECTION_SCHEDULE:   InjectionDomain = 4
```

## def InjectionEffect

```mach
pub def InjectionEffect: u8
```

## val INJECT_FAIL

```mach
pub val INJECT_FAIL:        InjectionEffect = 0
```

## val INJECT_SHORT

```mach
pub val INJECT_SHORT:       InjectionEffect = 1
```

## val INJECT_INTERRUPTED

```mach
pub val INJECT_INTERRUPTED: InjectionEffect = 2
```

## rec Injection

```mach
pub rec Injection;
```

## fun injection

```mach
pub fun injection(domain: InjectionDomain, effect: InjectionEffect,
ordinal: u64, short_limit: usize) Injection;
```

## fun injection_hit

```mach
pub fun injection_hit(i: *Injection, domain: InjectionDomain) bool;
```

## fun injection_result

```mach
pub fun injection_result(i: *Injection) ValidationGateResult;
```

## def IoAttemptKind

```mach
pub def IoAttemptKind: u8
```

## val IO_COMPLETE

```mach
pub val IO_COMPLETE:    IoAttemptKind = 0
```

## val IO_SHORT

```mach
pub val IO_SHORT:       IoAttemptKind = 1
```

## val IO_INTERRUPTED

```mach
pub val IO_INTERRUPTED: IoAttemptKind = 2
```

## val IO_FAILED

```mach
pub val IO_FAILED:      IoAttemptKind = 3
```

## rec IoAttempt

```mach
pub rec IoAttempt;
```

## fun io_attempt

```mach
pub fun io_attempt(i: *Injection, domain: InjectionDomain, requested: usize) IoAttempt;
```

## rec ControlledClock

```mach
pub rec ControlledClock;
```

## rec Deadline

```mach
pub rec Deadline;
```

## fun clock

```mach
pub fun clock(start_ns: u64) ControlledClock;
```

## fun clock_set

```mach
pub fun clock_set(c: *ControlledClock, now_ns: u64) bool;
```

## fun clock_advance

```mach
pub fun clock_advance(c: *ControlledClock, elapsed_ns: u64) bool;
```

## fun deadline

```mach
pub fun deadline(c: *ControlledClock, budget_ns: u64) Deadline;
```

## fun deadline_expired

```mach
pub fun deadline_expired(c: *ControlledClock, d: *Deadline) bool;
```

## rec SeedControl

```mach
pub rec SeedControl;
```

## fun seed_control

```mach
pub fun seed_control(seed: u64) SeedControl;
```

## fun seed_next

```mach
pub fun seed_next(s: *SeedControl) u64;
```

## rec BoundedSchedule

```mach
pub rec BoundedSchedule;
```

## fun bounded_schedule

```mach
pub fun bounded_schedule(seed: u64, limit: u64) BoundedSchedule;
```

## fun schedule_next

```mach
pub fun schedule_next(s: *BoundedSchedule, choices: u32, selected: *u32) bool;
```

## rec Replay

```mach
pub rec Replay;
```

## fun replay

```mach
pub fun replay(choices: *u32, count: usize) Replay;
```

## fun replay_next

```mach
pub fun replay_next(r: *Replay, bound: u32, selected: *u32) bool;
```

## fun replay_exact

```mach
pub fun replay_exact(r: *Replay) bool;
```

## rec Minimizer

```mach
pub rec Minimizer;
```

## fun minimizer

```mach
pub fun minimizer(failing_value: u64, limit: u64) Minimizer;
```

## fun minimize_next

```mach
pub fun minimize_next(m: *Minimizer, candidate: *u64) bool;
```

## fun minimize_observe

```mach
pub fun minimize_observe(m: *Minimizer, still_fails: bool) bool;
```

## fun minimized_value

```mach
pub fun minimized_value(m: *Minimizer) u64;
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

## fun observe_oracle

```mach
pub fun observe_oracle(i: *OracleInput) OracleObservation;
```

## val VALIDATION_EVIDENCE_SCHEMA

```mach
pub val VALIDATION_EVIDENCE_SCHEMA: u16 = 2
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

## fun evidence_validate_bytes

```mach
pub fun evidence_validate_bytes(data: *u8, len: usize,
out: *ValidationEvidenceView) err[EvidenceError];
```

## fun evidence_build

```mach
pub fun evidence_build(a: *A.Allocator, input: *EvidenceInput) res[ValidationEvidence, EvidenceError];
```

## fun evidence_open

```mach
pub fun evidence_open(a: *A.Allocator, data: *u8,
len: usize) res[ValidationEvidence, EvidenceError];
```

## fun evidence_copy

```mach
pub fun evidence_copy(a: *A.Allocator, e: ValidationEvidence,
len: *usize) res[*u8, EvidenceError];
```

## fun evidence_content_identity

```mach
pub fun evidence_content_identity(e: ValidationEvidence, digest: *u8) bool;
```

## fun evidence_release

```mach
pub fun evidence_release(e: *ValidationEvidence) err[EvidenceError];
```

