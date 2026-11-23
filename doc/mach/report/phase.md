[mach](../README.md) / [report](README.md) / **phase**

# mach.report.phase

the status a phase ends with: accepted, rejected through diagnostics, or failed

a status counts its rejections, keeps the first failure that is not the input's, and merges
into another.

Source: [`src/report/phase.mach`](../../../src/report/phase.mach)

Uses types from [`mach.base.fail`](../base/fail.md).

| Type | | |
| :--- | :--- | :--- |
| [`PhaseKind`](#phasekind) | `tag` | phase status is independent of diagnostic storage and rendering: accepted, rejected through recorded diagnostics, or failed with the first failure that is not the input's, a defect or the machine's refusal, which borrows its text and keeps where it points |
| [`PhaseStatus`](#phasestatus) | `rec` | the status of one phase and how many rejections it recorded |

| On `PhaseStatus` | |
| :--- | :--- |
| [`accepted`](#accepted) | a status that is accepted with no rejections |
| [`reject`](#reject) | count one rejection, turning an accepted status rejected and leaving an internal failure as it is |
| [`internal`](#internal) | fail the phase with the defect `f` names, or with `f` itself when the machine refused, which is no defect, unless it failed so already |
| [`is_accepted`](#is_accepted) | whether the phase is still accepted |
| [`internal_failure`](#internal_failure) | the internal failure, absent unless the phase failed internally |
| [`phase_failure`](#phase_failure) | the failure a phase ends with |
| [`merge`](#merge) | fold one status into another, adding its rejections and keeping the first internal failure |

---

## PhaseKind

```mach
pub tag PhaseKind: u8 {
    accepted;
    rejected;
    internal: mach.base.fail.Fail;
}
```

phase status is independent of diagnostic storage and rendering: accepted,
rejected through recorded diagnostics, or failed with the first failure that
is not the input's, a defect or the machine's refusal, which borrows its text
and keeps where it points

## PhaseStatus

```mach
pub rec PhaseStatus {
    kind:       PhaseKind;
    rejections: u64;
}
```

the status of one phase and how many rejections it recorded

| Field | Type | |
| :--- | :--- | :--- |
| `kind` | `PhaseKind` | accepted, rejected or failed internally |
| `rejections` | `u64` | the number of rejections recorded against the phase |

### accepted

```mach
pub fun accepted() PhaseStatus
```

a status that is accepted with no rejections

| Parameter | |
| :--- | :--- |
| **returns** | the fresh status |

### reject

```mach
pub fun reject(s: *PhaseStatus)
```

count one rejection, turning an accepted status rejected and leaving an internal failure as it is

| Parameter | |
| :--- | :--- |
| `s` | the status to update |

### internal

```mach
pub fun internal(s: *PhaseStatus, f: mach.base.fail.Fail)
```

fail the phase with the defect `f` names, or with `f` itself when the machine refused, which
is no defect, unless it failed so already

### is_accepted

```mach
pub fun is_accepted(s: PhaseStatus) bool
```

whether the phase is still accepted

| Parameter | |
| :--- | :--- |
| `s` | the status to inspect |
| **returns** | true when it was never rejected and never failed internally |

### internal_failure

```mach
pub fun internal_failure(s: PhaseStatus) opt[mach.base.fail.Fail]
```

the internal failure, absent unless the phase failed internally

### phase_failure

```mach
pub fun phase_failure(s: PhaseStatus) mach.base.fail.Fail
```

the failure a phase ends with

| Parameter | |
| :--- | :--- |
| `s` | the status to read |
| **returns** | the reported marker when it was rejected, the internal failure when it failed, and a message failure when it was accepted, which callers never ask |

### merge

```mach
pub fun merge(into: *PhaseStatus, from: PhaseStatus)
```

fold one status into another, adding its rejections and keeping the first internal failure

| Parameter | |
| :--- | :--- |
| `into` | the status that takes the result |
| `from` | the status merged in |

