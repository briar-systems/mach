[mach](../README.md) / [report](README.md) / **record**

# mach.report.record

the report records: one emitter, two encodings

every record a command writes under `--report json` or `--report lines` is written here,
field by field in the order the record page lists them, and the encoding the mode chose
lays it out. json writes one object a line. lines writes one record a line: the record's
kind bare, then each field as `key=value` parted by tabs, a nested field's key the path of
keys to it joined by `.`, each element of a list a repeat of the list's key, a null field
left out, and in a value a tab, a newline and a backslash written `\t`, `\n` and `\\`.

Source: [`src/report/record.mach`](../../../src/report/record.mach)

| Type | | |
| :--- | :--- | :--- |
| [`Encoding`](#encoding) | `def` | how records are laid out |
| [`Record`](#record) | `rec` | an open record, or a record nested in one: its encoding, where it goes, the json object under json, and under lines the keys that lead to it |
| [`List`](#list) | `rec` | an open list field: as a record, its path ending in the list's key |

| On `Encoding` | |
| :--- | :--- |
| [`JSON`](#encoding), [`LINES`](#encoding) | |
| [`begin`](#begin) | open a record of `kind` on w: its schema and kind, the first fields every record has |

| On `Record` | |
| :--- | :--- |
| [`end`](#end) | close a record and its line |
| [`text`](#text) | a text field |
| [`text_or_null`](#text_or_null) | a text field, null when v is nil |
| [`null`](#null) | a null field, which lines leaves out |
| [`int`](#int) | an integer field |
| [`flag`](#flag) | a truth field, `true` or `false` in both encodings |
| [`nested_begin`](#nested_begin) | open a record-valued field `key` of r as child |
| [`nested_end`](#nested_end) | close a nested record |
| [`list_begin`](#list_begin) | open a list field `key` of r |

| On `List` | |
| :--- | :--- |
| [`list_end`](#list_end) | close a list |
| [`list_text`](#list_text) | a text element of a list |
| [`list_record_begin`](#list_record_begin) | open a record element of a list as child, closed with nested_end |

| Constants | |
| :--- | :--- |
| [`SCHEMA`](#constants) | the version every record carries in `schema` |

---

## Encoding

```mach
pub def Encoding: u8
```

how records are laid out

| Name | Value | |
| :--- | :--- | :--- |
| `JSON` | `0` |  |
| `LINES` | `1` |  |

### begin

```mach
pub fun begin(w: *std.io.writer.Writer, enc: Encoding, r: *Record, kind: str)
```

open a record of `kind` on w: its schema and kind, the first fields every record has

## Record

```mach
pub rec Record {
    enc:   Encoding;
    w:     *std.io.writer.Writer;
    json:  json.ndjson.Object;
    path:  [DEPTH]str;
    depth: usize;
}
```

an open record, or a record nested in one: its encoding, where it goes, the json object
under json, and under lines the keys that lead to it

### end

```mach
pub fun end(r: *Record)
```

close a record and its line

### text

```mach
pub fun text(r: *Record, key: str, v: str)
```

a text field

### text_or_null

```mach
pub fun text_or_null(r: *Record, key: str, v: str)
```

a text field, null when v is nil

### null

```mach
pub fun null(r: *Record, key: str)
```

a null field, which lines leaves out

### int

```mach
pub fun int(r: *Record, key: str, v: i64)
```

an integer field

### flag

```mach
pub fun flag(r: *Record, key: str, v: bool)
```

a truth field, `true` or `false` in both encodings

### nested_begin

```mach
pub fun nested_begin(r: *Record, key: str, child: *Record)
```

open a record-valued field `key` of r as child

### nested_end

```mach
pub fun nested_end(child: *Record)
```

close a nested record

### list_begin

```mach
pub fun list_begin(r: *Record, key: str, l: *List)
```

open a list field `key` of r

## List

```mach
pub rec List {
    enc:   Encoding;
    w:     *std.io.writer.Writer;
    json:  json.ndjson.Array;
    path:  [DEPTH]str;
    depth: usize;
}
```

an open list field: as a record, its path ending in the list's key

### list_end

```mach
pub fun list_end(l: *List)
```

close a list

### list_text

```mach
pub fun list_text(l: *List, v: str)
```

a text element of a list

### list_record_begin

```mach
pub fun list_record_begin(l: *List, child: *Record)
```

open a record element of a list as child, closed with nested_end

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `SCHEMA` | `i64` | `3` | the version every record carries in `schema` |

