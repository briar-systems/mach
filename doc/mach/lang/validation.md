# mach.lang.validation

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

## rec ValidationGateResult

```mach
pub rec ValidationGateResult;
```

## fun gate_result

```mach
pub fun gate_result(kind: ValidationGateResultKind, cause: ValidationCause,
detail_identity: str) ValidationGateResult;
```

