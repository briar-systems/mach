# mach.lang.readout

## val PH_LOAD

```mach
pub val PH_LOAD:     u8 = 0
```

## val PH_RESOLVE

```mach
pub val PH_RESOLVE:  u8 = 1
```

## val PH_SEMA

```mach
pub val PH_SEMA:     u8 = 2
```

## val PH_LOWER

```mach
pub val PH_LOWER:    u8 = 3
```

## val PH_OPTIMIZE

```mach
pub val PH_OPTIMIZE: u8 = 4
```

## val PH_CODEGEN

```mach
pub val PH_CODEGEN:  u8 = 5
```

## val PH_EMIT

```mach
pub val PH_EMIT:     u8 = 6
```

## val PH_LINK

```mach
pub val PH_LINK:     u8 = 7
```

## val LEVEL_PHASES

```mach
pub val LEVEL_PHASES: u8 = 1
```

## val LEVEL_ITEMS

```mach
pub val LEVEL_ITEMS:  u8 = 2
```

## fun phase_valid

```mach
pub fun phase_valid(ph: u8) bool;
```

## def Instant

```mach
pub def Instant: opt[ctime.Time]
```

an instant a readout measures from: absent when the platform clock refused
the sample, so a duration measured from it is zero rather than invented

## fun sample

```mach
pub fun sample() Instant;
```

## fun elapsed

```mach
pub fun elapsed(start: Instant) cdur.Duration;
```

## rec PhaseMetrics

```mach
pub rec PhaseMetrics;
```

## def ProgressEventKind

```mach
pub def ProgressEventKind: u8
```

## val PROGRESS_EVENT_PHASE

```mach
pub val PROGRESS_EVENT_PHASE:   ProgressEventKind = 0
```

## val PROGRESS_EVENT_ITEM

```mach
pub val PROGRESS_EVENT_ITEM:    ProgressEventKind = 1
```

## val PROGRESS_EVENT_SUMMARY

```mach
pub val PROGRESS_EVENT_SUMMARY: ProgressEventKind = 2
```

## rec PhaseEvent

```mach
pub rec PhaseEvent;
```

## rec ItemEvent

```mach
pub rec ItemEvent;
```

## rec SummaryEvent

```mach
pub rec SummaryEvent;
```

## rec ProgressEvent

```mach
pub rec ProgressEvent;
```

## rec Progress

```mach
pub rec Progress;
```

## fun init

```mach
pub fun init(pr: *Progress, alloc: *A.Allocator, level: u8);
```

## fun dnit

```mach
pub fun dnit(pr: *Progress);
```

## fun progress_valid

```mach
pub fun progress_valid(pr: *Progress) bool;
```

## fun unit_begin

```mach
pub fun unit_begin(pr: *Progress);
```

## fun phase_begin

```mach
pub fun phase_begin(pr: *Progress, ph: u8);
```

## fun phase_carved

```mach
pub fun phase_carved(pr: *Progress, ph: u8);
```

## fun phase_workers

```mach
pub fun phase_workers(pr: *Progress, ph: u8, n: u32);
```

## fun phase_dropped

```mach
pub fun phase_dropped(pr: *Progress, ph: u8, n: u32);
```

## fun span_begin

```mach
pub fun span_begin(pr: *Progress) Instant;
```

## fun span_end

```mach
pub fun span_end(pr: *Progress, ph: u8, host: u8, start: Instant);
```

## fun item_begin

```mach
pub fun item_begin(pr: *Progress) Instant;
```

## fun item

```mach
pub fun item(pr: *Progress, ph: u8, name: str, start: Instant);
```

## fun item_dur

```mach
pub fun item_dur(pr: *Progress, ph: u8, name: str, d: cdur.Duration);
```

## fun phase_end

```mach
pub fun phase_end(pr: *Progress, ph: u8, count: u32, noun_one: str, noun_many: str);
```

## fun summary

```mach
pub fun summary(pr: *Progress, out: str, modules: u32, size: i64, unit: str, has_size: bool);
```

## fun render

```mach
pub fun render(pr: *Progress);
```

## fun size_split

```mach
pub fun size_split(bytes: usize, unit: *str) i64;
```

