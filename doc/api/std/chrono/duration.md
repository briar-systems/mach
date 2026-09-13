# std.chrono.duration

## def Duration

```mach
pub def Duration: i64
```

elapsed time in nanoseconds

## val NANOSECOND

```mach
pub val NANOSECOND:  Duration = 1
```

## val MICROSECOND

```mach
pub val MICROSECOND: Duration = 1000
```

## val MILLISECOND

```mach
pub val MILLISECOND: Duration = 1000000
```

## val SECOND

```mach
pub val SECOND:      Duration = 1000000000
```

## val MINUTE

```mach
pub val MINUTE:      Duration = 60 * SECOND
```

## val HOUR

```mach
pub val HOUR:        Duration = 60 * MINUTE
```

## fun duration_nsec

```mach
pub fun duration_nsec(d: Duration) i64;
```

return the duration as nanoseconds

d: duration to convert
ret: duration in nanoseconds

## fun duration_usec

```mach
pub fun duration_usec(d: Duration) i64;
```

return the duration as microseconds

d: duration to convert
ret: duration in microseconds

## fun duration_msec

```mach
pub fun duration_msec(d: Duration) i64;
```

return the duration as milliseconds

d: duration to convert
ret: duration in milliseconds

## fun duration_sec

```mach
pub fun duration_sec(d: Duration) i64;
```

return the duration as whole seconds

d: duration to convert
ret: duration in seconds

## fun duration_min

```mach
pub fun duration_min(d: Duration) i64;
```

return the duration as whole minutes

d: duration to convert
ret: duration in minutes

## fun duration_hour

```mach
pub fun duration_hour(d: Duration) i64;
```

return the duration as whole hours

d: duration to convert
ret: duration in hours

## fun duration_abs

```mach
pub fun duration_abs(d: Duration) Duration;
```

return the absolute value of d

d: duration to take absolute value of
ret: |d|

## fun format_duration

```mach
pub fun format_duration(d: Duration, buf: *u8, cap: usize) str;
```

render d as a compact ASCII human-readable string into buf

d: duration to render
buf: caller-provided byte buffer
cap: capacity of buf in bytes
ret: buf as a null-terminated str, or nil if cap is insufficient

