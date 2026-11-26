[mach](../README.md) / [report](README.md) / **layout**

# mach.report.layout

how every line a command prints for a person is laid out: result lines, rows of columns,
bars, and the one written form of a duration, a size, a count and a commit

a row is cells in columns. each column has a width, sits its cells left or right, and has
the spaces that part it from the column before. a blank cell keeps its column, so the cells
after it stay in theirs, and a row never ends in spaces: padding is written only when text
follows it. a block of lines shares one set of columns, measured from what it shows or from
the tables it draws its labels from, so a new line cannot drift out of them.

a number is rounded to the nearest of its last digit, and shows a tenth while it is under ten
of its unit: a size is exact bytes below a KiB, then KiB, MiB or GiB (`1.5 KiB`, `51 KiB`); a
measured duration is microseconds below a millisecond, milliseconds below a second (`1.5ms`,
`468ms`), seconds to a tenth below a minute (`18.5s`), then minutes and whole seconds. a count
of one takes the singular noun, and a count shown among others groups its digits by three
(`30,634`). a commit is its first COMMIT_SHORT hex digits.

a result line is a verb from one closed table, left-aligned in a gutter of GUTTER, then what
it acted on and its details, each GAP apart. a bar is `#` up to BAR long, scaled to the
largest of its table.

Source: [`src/report/layout.mach`](../../../src/report/layout.mach)

| Type | | |
| :--- | :--- | :--- |
| [`Align`](#align) | `def` | where a cell sits in its column |
| [`Column`](#column) | `rec` | one column of a block |
| [`Text`](#text) | `rec` | storage one written value lives in while its line is written, and how much a text writer has put in it |
| [`Verb`](#verb) | `def` | what a result line says was done, the word in its gutter |

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
| [`text_writer`](#text_writer) | a writer into t, from its start: what does not fit is dropped, and text_end closes it |
| [`text_end`](#text_end) | what a text writer wrote into t |
| [`count`](#count) | a count in decimal |
| [`duration`](#duration) | a measured duration, rounded to the nearest of its last digit: a tenth below ten of microseconds or milliseconds and whole above, seconds to a tenth below a minute, then minutes and whole seconds |
| [`size`](#size) | a size in bytes: exact below a KiB, then the largest binary unit it holds one of, rounded to the nearest tenth below ten of the unit and to the nearest whole above |
| [`commit`](#commit) | a commit as it is shown: its first COMMIT_SHORT digits |
| [`grouped`](#grouped) | a count with a comma between each group of three digits |
| [`bar_text`](#bar_text) | a bar of n `#`, nil when n is zero |

| On `Verb` | |
| :--- | :--- |
| [`CREATED`](#verb), [`REALIZED`](#verb), [`UPDATED`](#verb), [`UNCHANGED`](#verb), [`ADDED`](#verb), [`REMOVED`](#verb), [`BUILT`](#verb), [`CHECKED`](#verb), [`FORMATTED`](#verb), [`DIFFERS`](#verb), [`DOCUMENTED`](#verb), [`VERIFIED`](#verb), [`FETCHED`](#verb), [`PASSED`](#verb), [`FAILED`](#verb), [`SKIPPED`](#verb), [`MIGRATED`](#verb) | |
| [`verb_word`](#verb_word) | the word a verb says, as `formatted` |
| [`said`](#said) | the lead of a result line: its verb left-aligned in the gutter, then the gap before what it acted on |

| Functions | |
| :--- | :--- |
| [`aside`](#aside) | write a remark after a row's cells, parted from them by GAP |
| [`spaces`](#spaces) | write n spaces |
| [`noun`](#noun) | the noun a count takes |
| [`bar_length`](#bar_length) | the length of the bar of `part` against the largest `most`, rounded to nearest, one for any part above zero and zero for none |

| Constants | |
| :--- | :--- |
| [`GAP`](#constants) | the spaces between two columns, unless a column states its own |
| [`TEXT_CAP`](#constants) | the room one written number, duration, size or commit takes, its terminator included |
| [`COMMIT_SHORT`](#constants) | the hex digits a commit is shown by, unless a listing asks for the whole of it |
| [`GUTTER`](#constants) | the gutter's width: the widest verb, left-aligned in it |
| [`BAR`](#constants) | the room a bar may take |

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
    len:   usize;
}
```

storage one written value lives in while its line is written, and how much a text writer
has put in it

### text_writer

```mach
pub fun text_writer(t: *Text) std.io.writer.Writer
```

a writer into t, from its start: what does not fit is dropped, and text_end closes it

### text_end

```mach
pub fun text_end(t: *Text) str
```

what a text writer wrote into t

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

### grouped

```mach
pub fun grouped(t: *Text, n: u64) str
```

a count with a comma between each group of three digits

### bar_text

```mach
pub fun bar_text(t: *Text, n: usize) str
```

a bar of n `#`, nil when n is zero

## Verb

```mach
pub def Verb: u8
```

what a result line says was done, the word in its gutter

| Name | Value | |
| :--- | :--- | :--- |
| `CREATED` | `0` |  |
| `REALIZED` | `1` |  |
| `UPDATED` | `2` |  |
| `UNCHANGED` | `3` |  |
| `ADDED` | `4` |  |
| `REMOVED` | `5` |  |
| `BUILT` | `6` |  |
| `CHECKED` | `7` |  |
| `FORMATTED` | `8` |  |
| `DIFFERS` | `9` |  |
| `DOCUMENTED` | `10` |  |
| `VERIFIED` | `11` |  |
| `FETCHED` | `12` |  |
| `PASSED` | `13` |  |
| `FAILED` | `14` |  |
| `SKIPPED` | `15` |  |
| `MIGRATED` | `16` |  |

### verb_word

```mach
pub fun verb_word(v: Verb) str
```

the word a verb says, as `formatted`

### said

```mach
pub fun said(w: *std.io.writer.Writer, v: Verb)
```

the lead of a result line: its verb left-aligned in the gutter, then the gap before what it
acted on

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

### bar_length

```mach
pub fun bar_length(part: u64, most: u64) usize
```

the length of the bar of `part` against the largest `most`, rounded to nearest, one for any
part above zero and zero for none

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `GAP` | `usize` | `2` | the spaces between two columns, unless a column states its own |
| `TEXT_CAP` | `usize` | `48` | the room one written number, duration, size or commit takes, its terminator included |
| `COMMIT_SHORT` | `usize` | `12` | the hex digits a commit is shown by, unless a listing asks for the whole of it |
| `GUTTER` | `usize` | `10` | the gutter's width: the widest verb, left-aligned in it |
| `BAR` | `usize` | `24` | the room a bar may take |

