# std.data.toml

## val TYPE_STRING

```mach
pub val TYPE_STRING:  u8 = 0
```

## val TYPE_INTEGER

```mach
pub val TYPE_INTEGER: u8 = 1
```

## val TYPE_FLOAT

```mach
pub val TYPE_FLOAT:   u8 = 2
```

## val TYPE_BOOL

```mach
pub val TYPE_BOOL:    u8 = 3
```

## val TYPE_ARRAY

```mach
pub val TYPE_ARRAY:   u8 = 4
```

## val TYPE_TABLE

```mach
pub val TYPE_TABLE:   u8 = 5
```

## tag TomlError

```mach
pub tag TomlError: u8 {
    syntax:   usize;
    overflow: usize;
    depth:    usize;
    conflict: usize;
    alloc:    Error;
}
```

every way a parse can fail

syntax: the byte at the payload offset is not valid TOML
overflow: an integer literal at the payload offset does not fit i64
depth: a value at the payload offset nests deeper than MAX_VALUE_DEPTH
conflict: the key at the payload offset collides with an existing value of
          another shape (a table where a scalar was set, an array of tables
          over a table, and the like)
alloc: an acquisition was refused; nothing the parse built survives

## rec Value

```mach
pub rec Value;
```

tagged TOML value

tag: type discriminator (TYPE_*)

## rec Array

```mach
pub rec Array;
```

ordered list of TOML values

items: pointer to Value array (ptr to break recursion)
len: number of elements
cap: allocated capacity

## rec Table

```mach
pub rec Table;
```

ordered map of string keys to TOML values

keys: pointer to str array (parallel with values)
values: pointer to Value array
len: number of entries
cap: allocated capacity

## fun dnit

```mach
pub fun dnit(a: *Allocator, t: *Table) err[Error];
```

release a parsed table and everything under it

The teardown `parse` has always needed and never had. A `Table` owns its key
copies, its value payloads, and its two parallel arrays, all from the allocator
`parse` was handed, so one recursive walk returns the whole tree. Callers that
outlive a single parse - anything holding a Session across reloads - must call
this or the parse is a permanent allocation.

The table is reset to the empty state rather than left dangling, so a
double-dnit is a no-op rather than a double free. every buffer is released
even when one refusal is reported; the first refusal is the outcome.

a: allocator the table was parsed with
t: table to tear down
ret: ok, or the first refusal the allocator reported

## fun get

```mach
pub fun get(t: *Table, path: str) opt[*Value];
```

look up a value by dotted path in a table

walks nested tables for each dot-separated segment. for keys that
contain literal dots, use get_table() then iterate with table_key().
the pointer borrows the table and expires with dnit.

t: root table
path: dotted key path (e.g. "project.name")
ret: the value, or none when a segment of the path is not present or an
      inner segment is not a table

## fun get_str

```mach
pub fun get_str(t: *Table, path: str) opt[str];
```

look up a string value by dotted path

t: root table
path: dotted key path
ret: the string value, or none if not found or wrong type

## fun get_int

```mach
pub fun get_int(t: *Table, path: str) opt[i64];
```

look up an integer value by dotted path

t: root table
path: dotted key path
ret: the integer value, or none if not found or wrong type

## fun get_float

```mach
pub fun get_float(t: *Table, path: str) opt[f64];
```

look up a float value by dotted path

t: root table
path: dotted key path
ret: the float value, or none if not found or wrong type

## fun get_bool

```mach
pub fun get_bool(t: *Table, path: str) opt[bool];
```

look up a boolean value by dotted path

t: root table
path: dotted key path
ret: the boolean value, or none if not found or wrong type

## fun get_table

```mach
pub fun get_table(t: *Table, path: str) opt[*Table];
```

look up a sub-table by dotted path

t: root table
path: dotted key path
ret: the sub-table, or none if not found or wrong type

## fun get_array

```mach
pub fun get_array(t: *Table, path: str) opt[*Array];
```

look up an array by dotted path

t: root table
path: dotted key path
ret: the array, or none if not found or wrong type

## fun array_get

```mach
pub fun array_get(arr: *Array, index: usize) opt[*Value];
```

get a value from an array by index

arr: array to index
index: element index
ret: the element, or none past the last element

## fun table_len

```mach
pub fun table_len(t: *Table) usize;
```

get the number of entries in a table

t: table to query
ret: number of key-value pairs

## fun table_key

```mach
pub fun table_key(t: *Table, index: usize) opt[str];
```

get a key by positional index

t: table to query
index: position (0-based)
ret: the key, or none past the last entry

## fun table_value

```mach
pub fun table_value(t: *Table, index: usize) opt[*Value];
```

get a value by positional index

t: table to query
index: position (0-based)
ret: the value, or none past the last entry

## fun array_len

```mach
pub fun array_len(arr: *Array) usize;
```

get the number of elements in an array

arr: array to query
ret: number of elements

## val MAX_VALUE_DEPTH

```mach
pub val MAX_VALUE_DEPTH: usize = 64
```

how many containers a value may nest inside before the reader refuses

arrays and inline tables descend through parse_value, so nesting depth is a
count taken straight from the document: without a stated bound the only limit
is whatever stack the process happens to have, and a document deep enough to
exhaust it kills the reader rather than being rejected by it. sixty-four is
far past anything a hand-written document reaches and far short of any stack
the reader runs on.

## fun parse

```mach
pub fun parse(a: *Allocator, src: str) res[Table, TomlError];
```

parse a TOML document into a table tree

supports TOML 1.0: basic and literal strings (single and multi-line),
integers (decimal, hex, octal, binary), floats, booleans, arrays,
tables, inline tables, array of tables, and dotted keys.

a: allocator for all internal allocations
src: null-terminated TOML source string
ret: the root table, or the failure; a failed parse strands nothing

