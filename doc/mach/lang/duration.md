# mach.lang.duration

## val FORMS

```mach
pub val FORMS: str = "<n>ms, <n>s, <n>m or <n>h, with n a positive integer"
```

the accepted forms of a duration, for error messages

## fun parse

```mach
pub fun parse(s: *u8) opt[du.Duration];
```

parse a duration: a positive decimal integer followed by one of the
units ms, s, m or h

s: the argument
ret: the duration; none when s is malformed, zero, or overflows

## rec Text

```mach
pub rec Text;
```

a duration in the largest unit of parse's that divides it exactly
n:    the count
unit: "h", "m", "s" or "ms"; "ns" when d is not a whole number of milliseconds

## fun text

```mach
pub fun text(d: du.Duration) Text;
```

render d in the form parse reads back

d: the duration
ret: the count and its unit

