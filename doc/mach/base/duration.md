[mach](../README.md) / [base](README.md) / **duration**

# mach.base.duration

durations written as `<n>ms`, `<n>s`, `<n>m` or `<n>h`, parsed and rendered back

the form the command line's `--timeout` and a manifest step's `timeout` take.

Source: [`src/base/duration.mach`](../../../src/base/duration.mach)

| Type | | |
| :--- | :--- | :--- |
| [`Text`](#text) | `rec` | a duration in the largest unit of parse's that divides it exactly n:    the count unit: "h", "m", "s" or "ms"; "ns" when d is not a whole number of milliseconds |

| On `Text` | |
| :--- | :--- |
| [`text`](#text-1) | render d in the form parse reads back |

| Functions | |
| :--- | :--- |
| [`parse`](#parse) | parse a duration: a positive decimal integer followed by one of the units ms, s, m or h |

| Constants | |
| :--- | :--- |
| [`FORMS`](#constants) | the accepted forms of a duration, for error messages |

---

## Text

```mach
pub rec Text {
    n:    i64;
    unit: str;
}
```

a duration in the largest unit of parse's that divides it exactly
n:    the count
unit: "h", "m", "s" or "ms"; "ns" when d is not a whole number of milliseconds

### text

```mach
pub fun text(d: std.chrono.duration.Duration) Text
```

render d in the form parse reads back

| Parameter | |
| :--- | :--- |
| `d` | the duration |
| **returns** | the count and its unit |

## Functions

### parse

```mach
pub fun parse(s: *u8) opt[std.chrono.duration.Duration]
```

parse a duration: a positive decimal integer followed by one of the
units ms, s, m or h

| Parameter | |
| :--- | :--- |
| `s` | the argument |
| **returns** | the duration; none when s is malformed, zero, or overflows |

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `FORMS` | `str` | `"<n>ms, <n>s, <n>m or <n>h, with n a positive integer"` | the accepted forms of a duration, for error messages |

