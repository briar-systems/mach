[mach](../README.md) / [report](README.md) / **testrun**

# mach.report.testrun

collected tests, the result of each run and the tally over a run's results

Source: [`src/report/testrun.mach`](../../../src/report/testrun.mach)

| Type | | |
| :--- | :--- | :--- |
| [`TestArtifact`](#testartifact) | `rec` | a collected test: its qualified name, where it is declared, the test object that holds it, the dispatcher that runs it as `<exe> <idx>`, and the target and profile that dispatcher was built for |
| [`GateResultKind`](#gateresultkind) | `def` | how a test ran: passed, skipped, disagreed with what it checks, or could not be judged |
| [`GateCause`](#gatecause) | `def` | what decided a result's kind |
| [`GateResult`](#gateresult) | `rec` | one test's result; the tally owns a recorded result's detail |
| [`GateTally`](#gatetally) | `rec` | the count of results by kind over a run, and every result recorded |

| On `GateResultKind` | |
| :--- | :--- |
| [`RESULT_PASS`](#gateresultkind), [`RESULT_SEMANTIC_MISMATCH`](#gateresultkind), [`RESULT_INFRASTRUCTURE_FAILURE`](#gateresultkind), [`RESULT_RESOURCE_EXHAUSTION`](#gateresultkind), [`RESULT_SKIP`](#gateresultkind) | |
| [`gate_result`](#gate_result) | a result from its parts |

| On `GateCause` | |
| :--- | :--- |
| [`CAUSE_PASS`](#gatecause), [`CAUSE_ORACLE_TIMEOUT`](#gatecause), [`CAUSE_ORACLE_SPAWN_FAILURE`](#gatecause), [`CAUSE_ORACLE_CAPTURE`](#gatecause), [`CAUSE_ORACLE_SIGNAL`](#gatecause), [`CAUSE_ORACLE_EXIT`](#gatecause), [`CAUSE_INVALID_TERMINAL`](#gatecause), [`CAUSE_SKIP`](#gatecause) | |

| On `GateTally` | |
| :--- | :--- |
| [`gate_tally_init`](#gate_tally_init) | an empty tally |
| [`record_gate`](#record_gate) | record one result and count it under its kind |

| Constants | |
| :--- | :--- |
| [`EXIT_SKIP`](#constants) | the exit status a test reports a skip with: 0 is a pass, this is a skip, and any other status is a failure |

---

## TestArtifact

```mach
pub rec TestArtifact {
    module:  *u8;
    name:    *u8;
    file:    *u8;
    line:    u32;
    object:  *u8;
    exe:     *u8;
    idx:     u32;
    target:  *u8;
    profile: *u8;
}
```

a collected test: its qualified name, where it is declared, the test object
that holds it, the dispatcher that runs it as `<exe> <idx>`, and the target and
profile that dispatcher was built for

## GateResultKind

```mach
pub def GateResultKind: u8
```

how a test ran: passed, skipped, disagreed with what it checks, or could not be judged

| Name | Value | |
| :--- | :--- | :--- |
| `RESULT_PASS` | `0` |  |
| `RESULT_SEMANTIC_MISMATCH` | `1` |  |
| `RESULT_INFRASTRUCTURE_FAILURE` | `2` |  |
| `RESULT_RESOURCE_EXHAUSTION` | `3` |  |
| `RESULT_SKIP` | `4` |  |

### gate_result

```mach
pub fun gate_result(kind: GateResultKind, cause: GateCause, detail_identity: str) GateResult
```

a result from its parts

| Parameter | |
| :--- | :--- |
| `kind` | how the test ran |
| `cause` | what decided the kind |
| `detail_identity` | the detail text, borrowed until the result is recorded, or nil |
| **returns** | the result |

## GateCause

```mach
pub def GateCause: u8
```

what decided a result's kind

| Name | Value | |
| :--- | :--- | :--- |
| `CAUSE_PASS` | `0` |  |
| `CAUSE_ORACLE_TIMEOUT` | `4` |  |
| `CAUSE_ORACLE_SPAWN_FAILURE` | `6` |  |
| `CAUSE_ORACLE_CAPTURE` | `7` |  |
| `CAUSE_ORACLE_SIGNAL` | `8` |  |
| `CAUSE_ORACLE_EXIT` | `9` |  |
| `CAUSE_INVALID_TERMINAL` | `10` |  |
| `CAUSE_SKIP` | `11` |  |

## GateResult

```mach
pub rec GateResult {
    kind:            GateResultKind;
    cause:           GateCause;
    detail_identity: str;
}
```

one test's result; the tally owns a recorded result's detail

## GateTally

```mach
pub rec GateTally {
    pass:                   u32;
    semantic_mismatch:      u32;
    infrastructure_failure: u32;
    resource_exhaustion:    u32;
    skip:                   u32;
    results:                Vector[GateResult];
}
```

the count of results by kind over a run, and every result recorded

| Field | Type | |
| :--- | :--- | :--- |
| `pass` | `u32` | tests that passed |
| `semantic_mismatch` | `u32` | tests that disagreed with what they check |
| `infrastructure_failure` | `u32` | tests the harness failed to run |
| `resource_exhaustion` | `u32` | tests that ran out of a resource |
| `skip` | `u32` | tests that reported a skip |
| `results` | `Vector[GateResult]` | every recorded result, whose detail the tally owns |

### gate_tally_init

```mach
pub fun gate_tally_init(a: *std.allocator.Allocator) GateTally
```

an empty tally

| Parameter | |
| :--- | :--- |
| `a` | the allocator the result list grows in |
| **returns** | the tally with every count zero |

### record_gate

```mach
pub fun record_gate(gt: *GateTally, a: *std.allocator.Allocator, r: GateResult) err[std.allocator.Error]
```

record one result and count it under its kind

| Parameter | |
| :--- | :--- |
| `gt` | the tally |
| `a` | allocates the tally's copy of the result's detail |
| `r` | the result, whose detail is copied |
| **returns** | err when the detail cannot be copied or the result cannot be stored, the tally then unchanged |

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `EXIT_SKIP` | `i32` | `77` | the exit status a test reports a skip with: 0 is a pass, this is a skip, and any other status is a failure |

