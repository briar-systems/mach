# mach.lang.alloc.probe

## val POISON

```mach
pub val POISON: u8 = 0xAA
```

every fresh byte is filled with this, so an uninitialized read is visible

## val MAX_TRACKED

```mach
pub val MAX_TRACKED: usize = 4096
```

how many live blocks one probe tracks; the table is open addressed, so this
is a hard cap on live blocks, not on blocks ever handed out

## val WALK_NONALLOCATING

```mach
pub val WALK_NONALLOCATING: i32 = -1
```

the attempt made no request at ordinal zero

## val WALK_LEAK

```mach
pub val WALK_LEAK: i32 = -2
```

the attempt left a block outstanding

## val WALK_MISMATCH

```mach
pub val WALK_MISMATCH: i32 = -3
```

the attempt released a block it did not own, or with the wrong size

## val WALK_OVERFLOW

```mach
pub val WALK_OVERFLOW: i32 = -4
```

the attempt held more live blocks than the probe tracks

## val WALK_UNFIRED

```mach
pub val WALK_UNFIRED: i32 = -5
```

the armed ordinal was never reached, or was refused more than once

## rec Probe

```mach
pub rec Probe;
```

## fun make

```mach
pub fun make(t: *Probe) err[A.Error];
```

a probe over a fresh page allocator

## fun make_over

```mach
pub fun make_over(t: *Probe, backing: *A.Allocator) err[A.Error];
```

a probe over the given backing, copied

## fun make_refusing

```mach
pub fun make_refusing(t: *Probe);
```

a probe over nothing: every request is refused and counted

## fun allocator_of

```mach
pub fun allocator_of(t: *Probe) *A.Allocator;
```

## fun fail_at_ordinal

```mach
pub fun fail_at_ordinal(t: *Probe, n: usize);
```

refuse exactly the nth request from now, counting from one; zero never

## fun fail_from_ordinal

```mach
pub fun fail_from_ordinal(t: *Probe, n: usize);
```

refuse the nth request from now and every one after it

## fun clear_failure

```mach
pub fun clear_failure(t: *Probe);
```

## fun arm

```mach
pub fun arm(t: *Probe);
```

start the counted window: the walk's ordinal is measured from here

## fun counted

```mach
pub fun counted(t: *Probe) usize;
```

requests since `arm`

## fun fired

```mach
pub fun fired(t: *Probe) bool;
```

whether an injected refusal has fired

## fun owns

```mach
pub fun owns(t: *Probe, p: ptr) bool;
```

## fun size_of

```mach
pub fun size_of(t: *Probe, p: ptr) usize;
```

## fun leaked

```mach
pub fun leaked(t: *Probe) bool;
```

## fun clean

```mach
pub fun clean(t: *Probe) bool;
```

nothing outstanding, nothing released wrongly, nothing untracked

## fun dnit

```mach
pub fun dnit(t: *Probe) usize;
```

release everything still outstanding to the backing and reset the counters

ret: how many live blocks had to be reclaimed, which is the leak count

## rec Outcome

```mach
pub rec Outcome;
```

## fun walk

```mach
pub fun walk[C](context: *C, attempt: fun(*C, *Probe) i32) Outcome;
```

run `attempt` once per ordinal: zero first, then one to the count the
baseline made. the attempt builds its fixture through the probe, calls
`arm`, runs the operation, asserts its promises and tears the fixture down,
returning zero on a pass. the walk stops at the first failing ordinal.

