# mach.lang.phase

## tag PhaseKind

```mach
pub tag PhaseKind: u8 {
    accepted;
    rejected;
    internal: str;
}
```

phase status is independent of diagnostic storage and rendering: accepted,
rejected through recorded diagnostics, or internally failed with the text
of the first internal failure

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
pub fun internal(s: *PhaseStatus, text: str);
```

## fun is_accepted

```mach
pub fun is_accepted(s: PhaseStatus) bool;
```

## fun internal_text

```mach
pub fun internal_text(s: PhaseStatus) opt[str];
```

the internal failure's text, absent unless the phase failed internally

## fun phase_failure

```mach
pub fun phase_failure(s: PhaseStatus) fail.Fail;
```

## fun merge

```mach
pub fun merge(into: *PhaseStatus, from: PhaseStatus);
```

