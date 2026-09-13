# std.chrono.time

## def Clock

```mach
pub def Clock: res[Time, io_error.Error]
```

the outcome of reading a clock

## def Elapsed

```mach
pub def Elapsed: res[Duration, io_error.Error]
```

the outcome of measuring against a clock

## rec Time

```mach
pub rec Time;
```

an instant in time with nanosecond precision

sec: seconds since Unix epoch (1970-01-01 00:00:00 UTC)
nsec: nanoseconds within second [0, 999999999]

## fun now

```mach
pub fun now() Clock;
```

return the current wall-clock time

a failed clock is reported, never replaced by the zero time.

ret: current Time from the system clock, or the native refusal

## fun monotonic

```mach
pub fun monotonic() Clock;
```

return the current monotonic clock time

ret: current Time from the monotonic clock (for measuring elapsed time), or
     the native refusal

## fun unix

```mach
pub fun unix(sec: i64, nsec: i64) Time;
```

create a Time from a Unix timestamp

sec: seconds since Unix epoch
nsec: nanoseconds within second
ret: Time with nsec normalized to [0, 999999999]

## fun since

```mach
pub fun since(t: Time) Elapsed;
```

return the time elapsed since t

t: start time
ret: duration since t, or the wall clock's refusal

## fun until

```mach
pub fun until(t: Time) Elapsed;
```

return the duration until t

t: target time
ret: duration until t, or the wall clock's refusal

## fun time_unix_sec

```mach
pub fun time_unix_sec(t: Time) i64;
```

return seconds since Unix epoch

t: time to query
ret: seconds component

## fun time_unix_nsec

```mach
pub fun time_unix_nsec(t: Time) i64;
```

return nanoseconds since Unix epoch

t: time to query
ret: total nanoseconds since epoch

## fun time_unix_usec

```mach
pub fun time_unix_usec(t: Time) i64;
```

return microseconds since Unix epoch

t: time to query
ret: total microseconds since epoch

## fun time_unix_msec

```mach
pub fun time_unix_msec(t: Time) i64;
```

return milliseconds since Unix epoch

t: time to query
ret: total milliseconds since epoch

## fun time_add

```mach
pub fun time_add(t: Time, d: Duration) Time;
```

return t + d

t: base time
d: duration to add
ret: new Time offset by d

## fun time_sub

```mach
pub fun time_sub(t: Time, u: Time) Duration;
```

return the duration t - u

t: minuend time
u: subtrahend time
ret: duration between t and u

## fun time_before

```mach
pub fun time_before(t: Time, u: Time) bool;
```

report whether t is before u

t: time to test
u: time to compare against
ret: true if t < u

## fun time_after

```mach
pub fun time_after(t: Time, u: Time) bool;
```

report whether t is after u

t: time to test
u: time to compare against
ret: true if t > u

## fun time_equal

```mach
pub fun time_equal(t: Time, u: Time) bool;
```

report whether t and u represent the same instant

t: first time
u: second time
ret: true if t == u

## fun time_is_zero

```mach
pub fun time_is_zero(t: Time) bool;
```

report whether t represents the zero time instant

t: time to test
ret: true if t is the zero time (Unix epoch)

