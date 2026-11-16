[mach](../README.md) / [package](README.md) / **report**

# mach.package.report

where dependency management reports while it works. a progress line, a note and a
failure the operation goes on past stream to the caller as they happen; `effects` collects
a note for each change an operation made before it failed, which the caller shows after
the failure, so a failed command still names the state it left

Source: [`src/package/report.mach`](../../../src/package/report.mach)

Uses types from [`mach.base.fail`](../base/fail.md).

| Type | | |
| :--- | :--- | :--- |
| [`Report`](#report) | `rec` |  |

| On `Report` | |
| :--- | :--- |
| [`init`](#init) | a report over the given sink, whose effects `a` owns |
| [`dnit`](#dnit), [`line`](#line), [`note`](#note), [`fault`](#fault) | |
| [`effectf`](#effectf) | keep an effect note formatted as std.format formats it; a note that cannot be kept is dropped, since it only explains a failure already reported |
| [`linef`](#linef) | a progress line formatted as std.format formats it, through storage `a` lends for the call |
| [`notef`](#notef) | a note formatted as std.format formats it, through storage `a` lends for the call |
| [`discard`](#discard) | a report that keeps nothing, for work that has nothing to say or a test that checks state rather than what is said |

---

## Report

```mach
pub rec Report {
    ctx:     ptr;
    line:    LineFn;
    note:    NoteFn;
    fault:   FaultFn;
    effects: Vector[str];
}
```

### init

```mach
pub fun init(a: *std.allocator.Allocator, ctx: ptr, on_line: LineFn, on_note: NoteFn, on_fault: FaultFn) Report
```

a report over the given sink, whose effects `a` owns

### dnit

```mach
pub fun dnit(r: *Report)
```

### line

```mach
pub fun line(r: *Report, text: str)
```

### note

```mach
pub fun note(r: *Report, text: str)
```

### fault

```mach
pub fun fault(r: *Report, f: mach.base.fail.Fail)
```

### effectf

```mach
pub fun effectf(r: *Report, fmt: str, va: ...)
```

keep an effect note formatted as std.format formats it; a note that cannot be kept is
dropped, since it only explains a failure already reported

### linef

```mach
pub fun linef(r: *Report, a: *std.allocator.Allocator, fmt: str, va: ...)
```

a progress line formatted as std.format formats it, through storage `a` lends for the call

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

