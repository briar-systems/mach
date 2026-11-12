# mach.lang.phase

## tag PhaseKind

```mach
pub tag PhaseKind: u8 {
    accepted;
    rejected;
    internal: fail.Fail;
}
```

phase status is independent of diagnostic storage and rendering: accepted,
rejected through recorded diagnostics, or internally failed with the first
internal failure, which borrows its text and keeps where it points

## rec PhaseStatus

```mach
pub rec PhaseStatus;
```

## fun accepted

```mach
pub fun accepted() PhaseStatus;
```

## fun reject

```mach
pub fun reject(s: *PhaseStatus);
```

## fun internal

```mach
pub fun internal(s: *PhaseStatus, f: fail.Fail);
```

fail the phase internally with the defect `f` names, unless it failed internally already

## fun is_accepted

```mach
pub fun is_accepted(s: PhaseStatus) bool;
```

## fun internal_failure

```mach
pub fun internal_failure(s: PhaseStatus) opt[fail.Fail];
```

the internal failure, absent unless the phase failed internally

## fun phase_failure

```mach
pub fun phase_failure(s: PhaseStatus) fail.Fail;
```

## fun merge

```mach
pub fun merge(into: *PhaseStatus, from: PhaseStatus);
```

