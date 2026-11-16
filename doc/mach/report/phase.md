[mach](../README.md) / [report](README.md) / **phase**

# mach.report.phase

the status a phase ends with: accepted, rejected through diagnostics, or failed

a status counts its rejections, keeps the first failure that is not the input's, and merges
into another.

<sub>source: [`src/report/phase.mach`](../../../src/report/phase.mach) · imported by 45 modules</sub>

<sub>`fail` is [`mach.base.fail`](../base/fail.md)</sub>

| Type | | |
| :--- | :--- | :--- |
| [`PhaseKind`](#phasekind) | `tag` | phase status is independent of diagnostic storage and rendering: accepted, rejected through recorded diagnostics, or failed with the first failure that is not the input's, a defect or the machine's refusal, which borrows its text and keeps where it points |
| [`PhaseStatus`](#phasestatus) | `rec` |  |

| On `PhaseStatus` | |
| :--- | :--- |
| [`accepted`](#accepted) · [`reject`](#reject) | |
| [`internal`](#internal) | fail the phase with the defect `f` names, or with `f` itself when the machine refused, which is no defect, unless it failed so already |
| [`is_accepted`](#is_accepted) | |
| [`internal_failure`](#internal_failure) | the internal failure, absent unless the phase failed internally |
| [`phase_failure`](#phase_failure) · [`merge`](#merge) | |

---

## PhaseKind

```mach
pub tag PhaseKind: u8 {
    accepted;
    rejected;
    internal: fail.Fail;
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

### accepted

```mach
pub fun accepted() PhaseStatus
```

### reject

```mach
pub fun reject(s: *PhaseStatus)
```

### internal

```mach
pub fun internal(s: *PhaseStatus, f: fail.Fail)
```

fail the phase with the defect `f` names, or with `f` itself when the machine refused, which
is no defect, unless it failed so already

### is_accepted

```mach
pub fun is_accepted(s: PhaseStatus) bool
```

### internal_failure

```mach
pub fun internal_failure(s: PhaseStatus) opt[fail.Fail]
```

the internal failure, absent unless the phase failed internally

### phase_failure

```mach
pub fun phase_failure(s: PhaseStatus) fail.Fail
```

### merge

```mach
pub fun merge(into: *PhaseStatus, from: PhaseStatus)
```

