# mach.lang.fe.asm

the lexis of an inline-asm body that the language owns: its `#` comments, and the
`{name}` references that bind a local. everything else in the body is the instruction
set's, and reaches its assembler as written

## def RefKind

```mach
pub def RefKind: u8
```

## val REF_END

```mach
pub val REF_END: RefKind = 0
```

the body has no more braces

## val REF_NAME

```mach
pub val REF_NAME: RefKind = 1
```

`{name}`, a local the block reads or writes

## val REF_OTHER

```mach
pub val REF_OTHER: RefKind = 2
```

a brace group that is not one identifier, such as an aarch64 `{v0.16b}` register list,
which belongs to the instruction set

## val REF_UNTERMINATED

```mach
pub val REF_UNTERMINATED: RefKind = 3
```

a `{` the body never closes

## val REF_EMPTY

```mach
pub val REF_EMPTY: RefKind = 4
```

`{}`

## rec Ref

```mach
pub rec Ref;
```

one brace group of a body: its kind and the text between its braces

## fun ref_next

```mach
pub fun ref_next(source: str, body: lang_source.Span, at: *usize) Ref;
```

the next brace group of `body` at or after `at`, outside any comment; `at` moves past it

source: the text the body indexes
body: the body's span in `source`
at: the scan's offset into `source`, which starts at body.offset

## fun comments_strip

```mach
pub fun comments_strip(a: *A.Allocator, source: str, body: lang_source.Span) res[str, fail.Fail];
```

the body with its comments removed, the text an assembler reads; owned by `a`, body.len + 1
bytes

