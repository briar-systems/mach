# std.data.json

## def Kind

```mach
pub def Kind: u8
```

## val NULL

```mach
pub val NULL:   Kind = 0
```

## val BOOL

```mach
pub val BOOL:   Kind = 1
```

## val NUMBER

```mach
pub val NUMBER: Kind = 2
```

## val STRING

```mach
pub val STRING: Kind = 3
```

## val ARRAY

```mach
pub val ARRAY:  Kind = 4
```

## val OBJECT

```mach
pub val OBJECT: Kind = 5
```

## val MAX_DEPTH

```mach
pub val MAX_DEPTH: usize = 128
```

how many arrays and objects a value may nest inside before the parser
refuses

every container descends through parse_value, so the depth is a count taken
straight from the document; without a bound the only limit is the stack the
process happens to have. this is far past any hand-written document and far
short of any stack the parser runs on.

## tag JsonError

```mach
pub tag JsonError: u8 {
    syntax: usize;
    depth:  usize;
    alloc:  allocator.Error;
}
```

every way a parse or a string decode can fail

syntax: the byte at the payload offset is not valid JSON (or, from
        value_string_decode, not a valid escape); the offset is into the
        source given to parse, or into the raw string bytes being decoded
depth: a container at the payload offset nests deeper than MAX_DEPTH
alloc: growing a container's storage was refused; everything the parse had
        acquired is released before it reports

## rec Value

```mach
pub rec Value;
```

a parsed JSON value

kind: type discriminator (NULL, BOOL, NUMBER, STRING, ARRAY, OBJECT)
bool_val: boolean value (1 = true, 0 = false) when kind == BOOL
is_float: for a NUMBER, 1 if parsed as a float (had a fraction or exponent),
           else 0 (an integer); selects num_val vs float_val
num_val: integer value when kind == NUMBER and is_float == 0
float_val: float value when kind == NUMBER and is_float == 1
str_val: pointer into original input when kind == STRING; the RAW on-wire
           bytes with escapes intact (use value_string_decode for logical bytes)
str_len: byte length of string (not null-terminated)
children: heap-allocated array of child Values (ARRAY or OBJECT)
keys: heap-allocated array of key strings (OBJECT only, parallel with children;
           pointers into the original input, not null-terminated)
keys_len: heap-allocated array of key byte lengths (parallel with keys)
count: number of children/keys
cap: allocated capacity of children/keys arrays

## fun parse

```mach
pub fun parse(src: *u8, src_len: usize, alloc: *allocator.Allocator) res[Value, JsonError];
```

parse JSON text into a Value tree

the tree's container storage comes from `alloc`; release it with dnit. a
parse that fails part way releases everything it acquired before reporting.

src: pointer to JSON source bytes
src_len: length of source in bytes
alloc: allocator for array/object storage
ret: root Value on success, or the failure

## fun dnit

```mach
pub fun dnit(alloc: *allocator.Allocator, v: *Value) err[allocator.Error];
```

release the container storage under a value, leaving the value empty

walks arrays and objects to any depth and returns every children, keys and
keys_len buffer to the allocator the tree was parsed with. scalars and
strings own nothing (a string points into the caller's input). every buffer
is released even when one refusal is reported; the first refusal is the
outcome. a released value is empty, so a second dnit is a no-op.

alloc: the allocator the tree was parsed with
v: the value whose storage to release
ret: ok, or the first refusal the allocator reported

## fun value_is_null

```mach
pub fun value_is_null(v: *Value) bool;
```

check if a value is null

## fun value_is_bool

```mach
pub fun value_is_bool(v: *Value) bool;
```

check if a value is a boolean

## fun value_is_number

```mach
pub fun value_is_number(v: *Value) bool;
```

check if a value is a number

## fun value_is_string

```mach
pub fun value_is_string(v: *Value) bool;
```

check if a value is a string

## fun value_is_array

```mach
pub fun value_is_array(v: *Value) bool;
```

check if a value is an array

## fun value_is_object

```mach
pub fun value_is_object(v: *Value) bool;
```

check if a value is an object

## fun value_bool

```mach
pub fun value_bool(v: *Value) bool;
```

get the boolean value

ret: true if bool_val is nonzero

## fun value_is_float

```mach
pub fun value_is_float(v: *Value) bool;
```

whether a NUMBER value was parsed as a float

true for a number with a fraction or exponent, false for an integer-looking
number; selects whether to read it with value_float or value_number.

## fun value_number

```mach
pub fun value_number(v: *Value) i64;
```

get the integer value of an integer NUMBER

meaningful when the number was parsed as an integer (value_is_float is false);
read a float number with value_float.

## fun value_float

```mach
pub fun value_float(v: *Value) f64;
```

get the value of a NUMBER as a 64-bit float

returns the float value for a number parsed as a float, or the integer value
widened to f64 for an integer number, so it is defined for any NUMBER.

## fun value_string

```mach
pub fun value_string(v: *Value, len: *usize) *u8;
```

get the raw string pointer and length

for a parsed value these are the RAW on-wire bytes between the quotes, escapes
intact and zero-copy into the input (parse "a\nb" yields the 4 bytes a \ n b);
use value_string_decode to resolve the escapes into logical bytes.

len: receives the byte length of the string
ret: pointer to the first byte of the raw string content

## fun value_string_decode

```mach
pub fun value_string_decode(v: *Value, buf: *u8, len: usize) res[usize, JsonError];
```

decode the raw wire bytes of a string value into logical
bytes, resolving JSON escapes into a caller buffer.

the inverse of the emit escaper: '\"' '\\' '\/' become '"' '\' '/', '\b' '\f'
'\n' '\r' '\t' become their control bytes, and '\uXXXX' decodes to UTF-8 (a
high+low surrogate pair combines into one astral code point). unescaped bytes,
including raw UTF-8, pass through verbatim. the parser stores raw wire bytes
(see value_string), so this is where a parsed string becomes logical text.

bytes are written into `buf` up to `len`; the full decoded length is always
returned, so a caller can size the buffer by passing len 0 (or a short buffer)
and calling again once `buf` is large enough (as with emit).

v: the value whose raw string bytes to decode (str_val/str_len)
buf: destination buffer for the decoded bytes
len: capacity of buf in bytes
ret: the decoded byte length on success (may exceed len), or syntax at the
     offset of a malformed escape (unknown escape, a '\u' without four hex
     digits, or an ill-formed surrogate pair)

## fun value_count

```mach
pub fun value_count(v: *Value) usize;
```

get the number of children (array elements or object entries)

## fun value_get

```mach
pub fun value_get(v: *Value, index: usize) opt[*Value];
```

get an array element or object value by index

the pointer borrows the tree and expires with dnit.

index: zero-based index
ret: the child Value, or none past the last child

## fun value_key

```mach
pub fun value_key(v: *Value, index: usize) opt[str];
```

get an object key by index

the key points into the original input and is NOT null-terminated;
its byte length comes from value_key_len.

index: zero-based index
ret: the key, or none past the last entry or when v is not an object

## fun value_key_len

```mach
pub fun value_key_len(v: *Value, index: usize) usize;
```

get the byte length of an object key by index

index: zero-based index
ret: key length in bytes, or 0 if out of bounds

## fun value_find

```mach
pub fun value_find(v: *Value, key: str) opt[*Value];
```

find an object value by key name (linear search)

the pointer borrows the tree and expires with dnit.

key: null-terminated key to search for
ret: the Value, or none when no entry has that key or v is not an object

## fun emit

```mach
pub fun emit(v: *Value, buf: *u8, len: usize) usize;
```

emit a Value as JSON text into a buffer

strings and object keys are escaped under ESCAPE_VERBATIM: '"', '\', and
control bytes 0x00-0x1F are escaped (short escapes \b \t \n \f \r, else
\u00xx); other bytes, including valid UTF-8, are written verbatim. str_val
and keys are treated as logical bytes, so emitting a PARSED tree double-
escapes any escapes the source contained -- see the file header (mach-std#340).

v: value to emit
buf: destination buffer
len: size of destination buffer in bytes
ret: number of bytes written (may exceed len if buffer too small)

## fun write_value

```mach
pub fun write_value(w: *writer.Writer, v: *Value) err[WriteError];
```

emit a Value as JSON text through a writer

the same rendering as emit, to any sink. the outcome is the writer's, with
the bytes persisted before a failure in its payload.

w: the writer to emit through
v: value to emit
ret: ok, or the writer's failure with the persisted prefix

## rec Object

```mach
pub rec Object;
```

an open JSON object in the streaming NDJSON emitter, tracking the comma
boundary between its members.

w: the writer the object is emitted through (borrowed)
first: no member has been emitted into the object yet

## rec Array

```mach
pub rec Array;
```

an open JSON array in the streaming NDJSON emitter, tracking the comma
boundary between its elements.

w: the writer the array is emitted through (borrowed)
first: no element has been emitted into the array yet

## fun object_begin

```mach
pub fun object_begin(w: *writer.Writer, o: *Object) err[WriteError];
```

begin a JSON object, writing `{` and arming the first-member
state.

w: the writer to emit through (must outlive the object)
o: the object state to initialise
ret: ok, or the writer's failure with the persisted prefix

## fun object_end

```mach
pub fun object_end(o: *Object) err[WriteError];
```

end a JSON object, writing `}` and the newline that terminates the
NDJSON line.

o: the object to close
ret: ok, or the writer's failure with the persisted prefix

## fun object_end_value

```mach
pub fun object_end_value(o: *Object) err[WriteError];
```

close a nested object, writing `}` without the NDJSON line
terminator.

Used for an object that is a member value or an array element, where the
enclosing container - not this object - owns the line boundary.

o: the object to close
ret: ok, or the writer's failure with the persisted prefix

## fun field_object_begin

```mach
pub fun field_object_begin(o: *Object, key: *u8, child: *Object) err[WriteError];
```

open an object-valued member: the key lead-in, then `{`,
arming `child` over the parent's writer.

Fill `child` with the field_* helpers and close it with object_end_value; the
parent stays open and takes further members afterwards.

o: the open parent object
key: the member key
child: the nested object state to initialise
ret: ok, or the writer's failure with the persisted prefix

## fun field_array_begin

```mach
pub fun field_array_begin(o: *Object, key: *u8, arr: *Array) err[WriteError];
```

open an array-valued member: the key lead-in, then `[`,
arming `arr` over the parent's writer. Close it with array_end.

o: the open parent object
key: the member key
arr: the array state to initialise
ret: ok, or the writer's failure with the persisted prefix

## fun array_end

```mach
pub fun array_end(arr: *Array) err[WriteError];
```

close an array, writing `]`

arr: the array to close
ret: ok, or the writer's failure with the persisted prefix

## fun array_object_begin

```mach
pub fun array_object_begin(arr: *Array, child: *Object) err[WriteError];
```

open an object element of `arr`: the element separator,
then `{`, arming `child` over the array's writer.

Fill `child` with the field_* helpers and close it with object_end_value.

arr: the open array
child: the element object state to initialise
ret: ok, or the writer's failure with the persisted prefix

## fun field_str

```mach
pub fun field_str(o: *Object, key: *u8, v: *u8) err[WriteError];
```

emit a string-valued member (the value quoted and ASCII-escaped)

o: the open object
key: the member key (ASCII, emitted verbatim between quotes)
v: the null-terminated value, escaped; nil emits an empty string
ret: ok, or the writer's failure with the persisted prefix

## fun field_str_or_null

```mach
pub fun field_str_or_null(o: *Object, key: *u8, v: *u8) err[WriteError];
```

emit a string-or-null member: the escaped value, or JSON
`null` when `v` is nil.

o: the open object
key: the member key
v: the null-terminated value, or nil for a JSON null
ret: ok, or the writer's failure with the persisted prefix

## fun field_null

```mach
pub fun field_null(o: *Object, key: *u8) err[WriteError];
```

emit a JSON `null`-valued member

o: the open object
key: the member key
ret: ok, or the writer's failure with the persisted prefix

## fun field_i64

```mach
pub fun field_i64(o: *Object, key: *u8, v: i64) err[WriteError];
```

emit an integer-valued member

o: the open object
key: the member key
v: the integer value
ret: ok, or the writer's failure with the persisted prefix

## fun field_f64

```mach
pub fun field_f64(o: *Object, key: *u8, v: f64) err[WriteError];
```

emit a float-valued member

the value is written as the shortest decimal that round-trips through the
parser (value_float). an integer-valued float emits without a fractional part,
so the parser reads it back as an integer number (value_is_float false); read
it with value_number in that case.

o: the open object
key: the member key
v: the float value
ret: ok, or the writer's failure with the persisted prefix

## fun field_bool

```mach
pub fun field_bool(o: *Object, key: *u8, v: bool) err[WriteError];
```

emit a boolean-valued member

o: the open object
key: the member key
v: the boolean value
ret: ok, or the writer's failure with the persisted prefix

## fun value_f64

```mach
pub fun value_f64(w: *writer.Writer, v: f64) err[WriteError];
```

write a bare JSON float number through the writer

emits the shortest decimal that round-trips through the parser (value_float),
with no key or surrounding container - the value equivalent of
write_json_string for a float, for use as an array element or standalone
value. an integer-valued float emits without a fractional part.

w: the writer to emit through
v: the float value
ret: ok, or the writer's failure with the persisted prefix

## fun write_json_string

```mach
pub fun write_json_string(w: *writer.Writer, s: *u8) err[WriteError];
```

write a JSON string literal under ESCAPE_ENSURE_ASCII: the
value quoted, every byte escaped to printable ASCII.

w: the writer to emit through
s: the null-terminated value, or nil for an empty string
ret: ok, or the writer's failure with the persisted prefix

