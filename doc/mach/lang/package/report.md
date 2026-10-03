# mach.lang.package.report

where dependency management reports while it works. a progress line, a note and a
failure the operation goes on past stream to the caller as they happen; `effects` collects
a note for each change an operation made before it failed, which the caller shows after
the failure, so a failed command still names the state it left

## rec Report

```mach
pub rec Report;
```

## fun init

```mach
pub fun init(a: *A.Allocator, ctx: ptr, on_line: LineFn, on_note: NoteFn, on_fault: FaultFn) Report;
```

a report over the given sink, whose effects `a` owns

## fun dnit

```mach
pub fun dnit(r: *Report);
```

## fun line

```mach
pub fun line(r: *Report, text: str);
```

## fun note

```mach
pub fun note(r: *Report, text: str);
```

## fun fault

```mach
pub fun fault(r: *Report, f: fail.Fail);
```

## fun effectf

```mach
pub fun effectf(r: *Report, fmt: str, va: ...);
```

keep an effect note formatted as std.format formats it; a note that cannot be kept is
dropped, since it only explains a failure already reported

## fun linef

```mach
pub fun linef(r: *Report, a: *A.Allocator, fmt: str, va: ...);
```

a progress line formatted as std.format formats it, through storage `a` lends for the call

## fun notef

```mach
pub fun notef(r: *Report, a: *A.Allocator, fmt: str, va: ...);
```

a note formatted as std.format formats it, through storage `a` lends for the call

## fun discard

```mach
pub fun discard(a: *A.Allocator) Report;
```

a report that keeps nothing, for work that has nothing to say or a test that checks state
rather than what is said

