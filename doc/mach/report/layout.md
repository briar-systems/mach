[mach](../README.md) / [report](README.md) / **layout**

# mach.report.layout

how every line a command prints for a person is laid out: rows of columns, and the one
written form of a duration, a size, a count's noun and a commit

a row is cells in columns. each column has a width, sits its cells left or right, and has
the spaces that part it from the column before. a blank cell keeps its column, so the cells
after it stay in theirs, and a row never ends in spaces: padding is written only when text
follows it. a block of lines shares one set of columns, measured from what it shows or from
the tables it draws its labels from, so a new line cannot drift out of them.

a number is rounded to the nearest of its last digit, and shows a tenth while it is under ten
of its unit: a size is exact bytes below a KiB, then KiB, MiB or GiB (`1.5 KiB`, `51 KiB`); a
measured duration is microseconds below a millisecond, milliseconds below a second (`1.5ms`,
`468ms`), seconds to a tenth below a minute (`18.5s`), then minutes and whole seconds. a count
of one takes the singular noun. a commit is its first COMMIT_SHORT hex digits.

Source: [`src/report/layout.mach`](../../../src/report/layout.mach)

| Type | | |
| :--- | :--- | :--- |
| [`Align`](#align) | `def` | where a cell sits in its column |
| [`Column`](#column) | `rec` | one column of a block |
| [`Text`](#text) | `rec` | storage one written value lives in while its line is written |

| On `Align` | |
| :--- | :--- |
| [`LEFT`](#align), [`RIGHT`](#align) | |

| On `Column` | |
| :--- | :--- |
| [`fit`](#fit) | widen a column to hold `s` |
| [`cells`](#cells) | write a row's cells in their columns and no line end, so an aside can follow on the line |
| [`row`](#row) | write a row's cells in their columns and end the line |

| On `Text` | |
| :--- | :--- |
| [`count`](#count) | a count in decimal |
| [`duration`](#duration) | a measured duration, rounded to the nearest of its last digit: a tenth below ten of microseconds or milliseconds and whole above, seconds to a tenth below a minute, then minutes and whole seconds |
| [`size`](#size) | a size in bytes: exact below a KiB, then the largest binary unit it holds one of, rounded to the nearest tenth below ten of the unit and to the nearest whole above |
| [`commit`](#commit) | a commit as it is shown: its first COMMIT_SHORT digits |

| Functions | |
| :--- | :--- |
| [`aside`](#aside) | write a remark after a row's cells, parted from them by GAP |
| [`spaces`](#spaces) | write n spaces |
| [`noun`](#noun) | the noun a count takes |

| Constants | |
| :--- | :--- |
| [`GAP`](#constants) | the spaces between two columns, unless a column states its own |
| [`TEXT_CAP`](#constants) | the room one written number, duration, size or commit takes, its terminator included |
| [`COMMIT_SHORT`](#constants) | the hex digits a commit is shown by, unless a listing asks for the whole of it |

---

## Align

```mach
pub def Align: u8
```

where a cell sits in its column

| Name | Value | |
| :--- | :--- | :--- |
| `LEFT` | `0` |  |
| `RIGHT` | `1` |  |

## Column

```mach
pub rec Column {
    width: usize;
    align: Align;
    gap:   usize;
}
```

one column of a block

| Field | Type | |
| :--- | :--- | :--- |
| `width` | `usize` | the cells' room; a longer cell takes more and pushes the rest of its row |
| `align` | `Align` | LEFT or RIGHT |
| `gap` | `usize` | the spaces before the column; the first column's gap indents the row |

### fit

```mach
pub fun fit(c: *Column, s: str)
```

widen a column to hold `s`

### cells

```mach
pub fun cells(w: *std.io.writer.Writer, cols: *Column, texts: *str, n: usize)
```

write a row's cells in their columns and no line end, so an aside can follow on the line

| Parameter | |
| :--- | :--- |
| `w` | where the row goes |
| `cols` | the block's columns |
| `texts` | one text per column, nil or empty for a blank cell |
| `n` | how many columns |

### row

```mach
pub fun row(w: *std.io.writer.Writer, cols: *Column, texts: *str, n: usize)
```

write a row's cells in their columns and end the line

## Text

```mach
pub rec Text {
    bytes: [TEXT_CAP]u8;
}
```

storage one written value lives in while its line is written

### count

```mach
pub fun count(t: *Text, n: u64) str
```

a count in decimal

### duration

```mach
pub fun duration(t: *Text, d: std.chrono.duration.Duration) str
```

a measured duration, rounded to the nearest of its last digit: a tenth below ten of
microseconds or milliseconds and whole above, seconds to a tenth below a minute, then minutes
and whole seconds. zero is `0us`

### size

```mach
pub fun size(t: *Text, bytes: u64) str
```

a size in bytes: exact below a KiB, then the largest binary unit it holds one of, rounded to
the nearest tenth below ten of the unit and to the nearest whole above

### commit

```mach
pub fun commit(t: *Text, full: str) str
```

a commit as it is shown: its first COMMIT_SHORT digits

## Functions

### aside

```mach
pub fun aside(w: *std.io.writer.Writer, text: str)
```

write a remark after a row's cells, parted from them by GAP

### spaces

```mach
pub fun spaces(w: *std.io.writer.Writer, n: usize)
```

write n spaces

### noun

```mach
pub fun noun(n: u64, one: str, many: str) str
```

the noun a count takes

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `GAP` | `usize` | `2` | the spaces between two columns, unless a column states its own |
| `TEXT_CAP` | `usize` | `32` | the room one written number, duration, size or commit takes, its terminator included |
| `COMMIT_SHORT` | `usize` | `12` | the hex digits a commit is shown by, unless a listing asks for the whole of it |

