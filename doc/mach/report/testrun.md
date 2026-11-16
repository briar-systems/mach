# mach.report.testrun

## rec TestArtifact

```mach
pub rec TestArtifact;
```

a collected test: its qualified name, where it is declared, the test object
that holds it, the dispatcher that runs it as `<exe> <idx>`, and the target and
profile that dispatcher was built for

## val EXIT_SKIP

```mach
pub val EXIT_SKIP: i32 = 77
```

the exit status a test reports a skip with: 0 is a pass, this is a skip, and
any other status is a failure

## def GateResultKind

```mach
pub def GateResultKind: u8
```

how a test ran: passed, skipped, disagreed with what it checks, or could not be judged

## val RESULT_PASS

```mach
pub val RESULT_PASS:                   GateResultKind = 0
```

## val RESULT_SEMANTIC_MISMATCH

```mach
pub val RESULT_SEMANTIC_MISMATCH:      GateResultKind = 1
```

## val RESULT_TOOL_UNAVAILABLE

```mach
pub val RESULT_TOOL_UNAVAILABLE:       GateResultKind = 3
```

## val RESULT_INFRASTRUCTURE_FAILURE

```mach
pub val RESULT_INFRASTRUCTURE_FAILURE: GateResultKind = 4
```

## val RESULT_RESOURCE_EXHAUSTION

```mach
pub val RESULT_RESOURCE_EXHAUSTION:    GateResultKind = 5
```

## val RESULT_SKIP

```mach
pub val RESULT_SKIP:                   GateResultKind = 7
```

## def GateCause

```mach
pub def GateCause: u8
```

what decided a result's kind

## val CAUSE_PASS

```mach
pub val CAUSE_PASS:                 GateCause = 0
```

## val CAUSE_ORACLE_TIMEOUT

```mach
pub val CAUSE_ORACLE_TIMEOUT:       GateCause = 4
```

## val CAUSE_ORACLE_SPAWN_FAILURE

```mach
pub val CAUSE_ORACLE_SPAWN_FAILURE: GateCause = 6
```

## val CAUSE_ORACLE_CAPTURE

```mach
pub val CAUSE_ORACLE_CAPTURE:       GateCause = 7
```

## val CAUSE_ORACLE_SIGNAL

```mach
pub val CAUSE_ORACLE_SIGNAL:        GateCause = 8
```

## val CAUSE_ORACLE_EXIT

```mach
pub val CAUSE_ORACLE_EXIT:          GateCause = 9
```

## val CAUSE_INVALID_TERMINAL

```mach
pub val CAUSE_INVALID_TERMINAL:     GateCause = 10
```

## val CAUSE_SKIP

```mach
pub val CAUSE_SKIP:                 GateCause = 11
```

## rec GateResult

```mach
pub rec GateResult;
```

one test's result; the tally owns a recorded result's detail

## fun gate_result

```mach
pub fun gate_result(kind: GateResultKind, cause: GateCause, detail_identity: str) GateResult;
```

## rec GateTally

```mach
pub rec GateTally;
```

## fun gate_tally_init

```mach
pub fun gate_tally_init(a: *A.Allocator) GateTally;
```

## fun record_gate

```mach
pub fun record_gate(gt: *GateTally, a: *A.Allocator, r: GateResult) err[A.Error];
```

