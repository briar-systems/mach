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
rejected through recorded diagnostics, or failed with the first failure that
is not the input's, a defect or the machine's refusal, which borrows its text
and keeps where it points

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

fail the phase with the defect `f` names, or with `f` itself when the machine refused, which
is no defect, unless it failed so already

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

