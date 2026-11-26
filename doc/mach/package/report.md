[mach](../README.md) / [package](README.md) / **report**

# mach.package.report

where dependency management reports while it works. a result (a verb from mach.report.layout's
table, what it acted on and its fields), a note, a failure the operation goes on
past, and each remote whose releases it fetches stream to the caller as they happen; `effects`
collects a note for each change an operation made before it failed, which the caller shows
after the failure, so a failed command still names the state it left

Source: [`src/package/report.mach`](../../../src/package/report.mach)

Uses types from [`mach.base.fail`](../base/fail.md), [`mach.report.layout`](../report/layout.md).

| Type | | |
| :--- | :--- | :--- |
| [`Sink`](#sink) | `rec` | what a report's sink does with each thing reported |
| [`Report`](#report) | `rec` | a sink for results, notes, faults and fetches, with the effect notes an operation collected and the fetches it made |

| On `Sink` | |
| :--- | :--- |
| [`init`](#init) | a report over the given sink, whose effects `a` owns |

| On `Report` | |
| :--- | :--- |
| [`dnit`](#dnit) | free the effect notes the report kept |
| [`said`](#said) | write a result to the sink: verb v, what it acted on, then the n fields at `fields`, each borrowed for the call |
| [`note`](#note) | write a note to the sink |
| [`fault`](#fault) | report a failure the operation goes on past |
| [`stage`](#stage) | the caller opened a stage over the fetches from here, which it expects `n` of, 0 when it cannot count them; the sink counts them as they begin and shows each as an item of that stage |
| [`fetching`](#fetching) | a fetch from the remote at `url` begins |
| [`fetched`](#fetched) | the fetch from `url` ended after `d` |
| [`effectf`](#effectf) | keep an effect note formatted as std.format formats it; a note that cannot be kept is dropped, since it only explains a failure already reported |
| [`notef`](#notef) | a note formatted as std.format formats it, through storage `a` lends for the call |
| [`discard`](#discard) | a report that keeps nothing, for work that has nothing to say or a test that checks state rather than what is said |

---

## Sink

```mach
pub rec Sink {
    said:     SaidFn;
    note:     NoteFn;
    fault:    FaultFn;
    fetching: FetchingFn;
    fetched:  FetchedFn;
}
```

what a report's sink does with each thing reported

| Field | Type | |
| :--- | :--- | :--- |
| `said` | `SaidFn` | writes a result |
| `note` | `NoteFn` | writes a note |
| `fault` | `FaultFn` | reports a failure the operation goes on past |
| `fetching` | `FetchingFn` | shows a fetch beginning |
| `fetched` | `FetchedFn` | shows a fetch ended |

### init

```mach
pub fun init(a: *std.allocator.Allocator, ctx: ptr, sink: Sink) Report
```

a report over the given sink, whose effects `a` owns

## Report

```mach
pub rec Report {
    ctx:      ptr;
    sink:     Sink;
    effects:  Vector[str];
    staged:   bool;
    expected: u32;
    fetches:  u32;
    fetching: std.chrono.duration.Duration;
}
```

a sink for results, notes, faults and fetches, with the effect notes an operation
collected and the fetches it made

| Field | Type | |
| :--- | :--- | :--- |
| `ctx` | `ptr` | the context each callback receives |
| `sink` | `Sink` | what the callbacks are |
| `effects` | `Vector[str]` | the effect notes kept so far, owned by the report |
| `staged` | `bool` | whether the caller opened a stage over the fetches, which then show as its items |
| `expected` | `u32` | the fetches the caller expects in its stage, 0 when it cannot count them |
| `fetches` | `u32` | the fetches ended since the stage opened |
| `fetching` | `std.chrono.duration.Duration` | the time they took together |

### dnit

```mach
pub fun dnit(r: *Report)
```

free the effect notes the report kept

| Parameter | |
| :--- | :--- |
| `r` | the report to release |

### said

```mach
pub fun said(r: *Report, v: mach.report.layout.Verb, subject: str, fields: *mach.report.layout.Field, n: usize)
```

write a result to the sink: verb v, what it acted on, then the n fields at `fields`, each
borrowed for the call

### note

```mach
pub fun note(r: *Report, text: str)
```

write a note to the sink

| Parameter | |
| :--- | :--- |
| `r` | the report |
| `text` | the note, borrowed for the call |

### fault

```mach
pub fun fault(r: *Report, f: mach.base.fail.Fail)
```

report a failure the operation goes on past

| Parameter | |
| :--- | :--- |
| `r` | the report |
| `f` | the failure |

### stage

```mach
pub fun stage(r: *Report, n: u32)
```

the caller opened a stage over the fetches from here, which it expects `n` of, 0 when it cannot
count them; the sink counts them as they begin and shows each as an item of that stage

### fetching

```mach
pub fun fetching(r: *Report, url: str)
```

a fetch from the remote at `url` begins

### fetched

```mach
pub fun fetched(r: *Report, url: str, d: std.chrono.duration.Duration)
```

the fetch from `url` ended after `d`

### effectf

```mach
pub fun effectf(r: *Report, fmt: str, va: ...)
```

keep an effect note formatted as std.format formats it; a note that cannot be kept is
dropped, since it only explains a failure already reported

### notef

```mach
pub fun notef(r: *Report, a: *std.allocator.Allocator, fmt: str, va: ...)
```

a note formatted as std.format formats it, through storage `a` lends for the call

### discard

```mach
pub fun discard(a: *std.allocator.Allocator) Report
```

a report that keeps nothing, for work that has nothing to say or a test that checks state
rather than what is said

