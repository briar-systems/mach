[mach](../README.md) / [package](README.md) / **report**

# mach.package.report

where dependency management reports while it works. a progress line (a verb from
mach.report.layout's table and what it acted on), a remark, a note and a failure the
operation goes on past stream to the caller as they happen; `effects` collects
a note for each change an operation made before it failed, which the caller shows after
the failure, so a failed command still names the state it left

Source: [`src/package/report.mach`](../../../src/package/report.mach)

Uses types from [`mach.base.fail`](../base/fail.md), [`mach.report.layout`](../report/layout.md).

| Type | | |
| :--- | :--- | :--- |
| [`Report`](#report) | `rec` | a sink for progress lines, notes and faults, with the effect notes an operation collected |

| On `Report` | |
| :--- | :--- |
| [`init`](#init) | a report over the given sink, whose effects `a` owns |
| [`dnit`](#dnit) | free the effect notes the report kept |
| [`said`](#said) | write a progress line to the sink: verb v, then the n texts at `texts`, borrowed for the call |
| [`said1`](#said1) | a progress line of verb v and what it acted on |
| [`said2`](#said2) | a progress line of verb v, what it acted on and one detail |
| [`said3`](#said3) | a progress line of verb v, what it acted on and two details |
| [`line`](#line) | write a remark to the sink |
| [`note`](#note) | write a note to the sink |
| [`fault`](#fault) | report a failure the operation goes on past |
| [`effectf`](#effectf) | keep an effect note formatted as std.format formats it; a note that cannot be kept is dropped, since it only explains a failure already reported |
| [`notef`](#notef) | a note formatted as std.format formats it, through storage `a` lends for the call |
| [`discard`](#discard) | a report that keeps nothing, for work that has nothing to say or a test that checks state rather than what is said |

---

## Report

```mach
pub rec Report {
    ctx:     ptr;
    said:    SaidFn;
    line:    LineFn;
    note:    NoteFn;
    fault:   FaultFn;
    effects: Vector[str];
}
```

a sink for progress lines, notes and faults, with the effect notes an operation collected

| Field | Type | |
| :--- | :--- | :--- |
| `ctx` | `ptr` | the context each callback receives |
| `said` | `SaidFn` | writes a progress line |
| `line` | `LineFn` | writes a remark |
| `note` | `NoteFn` | writes a note |
| `fault` | `FaultFn` | reports a failure the operation goes on past |
| `effects` | `Vector[str]` | the effect notes kept so far, owned by the report |

### init

```mach
pub fun init(a: *std.allocator.Allocator, ctx: ptr, on_said: SaidFn, on_line: LineFn, on_note: NoteFn, on_fault: FaultFn) Report
```

a report over the given sink, whose effects `a` owns

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
pub fun said(r: *Report, v: mach.report.layout.Verb, texts: *str, n: usize)
```

write a progress line to the sink: verb v, then the n texts at `texts`, borrowed for the call

### said1

```mach
pub fun said1(r: *Report, v: mach.report.layout.Verb, a: str)
```

a progress line of verb v and what it acted on

### said2

```mach
pub fun said2(r: *Report, v: mach.report.layout.Verb, a: str, b: str)
```

a progress line of verb v, what it acted on and one detail

### said3

```mach
pub fun said3(r: *Report, v: mach.report.layout.Verb, a: str, b: str, c: str)
```

a progress line of verb v, what it acted on and two details

### line

```mach
pub fun line(r: *Report, text: str)
```

write a remark to the sink

| Parameter | |
| :--- | :--- |
| `r` | the report |
| `text` | the remark, borrowed for the call |

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

