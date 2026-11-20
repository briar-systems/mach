[mach](../README.md) / [base](README.md) / **textbuild**

# mach.base.textbuild

a text builder that keeps its first failure

every later append and the finish refuse with that failure, so a writer appends a run of
pieces and checks once.

Source: [`src/base/textbuild.mach`](../../../src/base/textbuild.mach)

| Type | | |
| :--- | :--- | :--- |
| [`Error`](#error) | `tag` | the builder's failure: the allocator refused, or a length or capacity left usize |
| [`TextBuilder`](#textbuilder) | `rec` | text built one append at a time |

| On `Error` | |
| :--- | :--- |
| [`text`](#text) | the text of the builder's failure |

| On `TextBuilder` | |
| :--- | :--- |
| [`tb_init`](#tb_init) | an empty builder growing in `a`, released with tb_dnit |
| [`tb_append`](#tb_append) | append `text` up to its first NUL |
| [`tb_fault`](#tb_fault) | the builder's first failure, or ok when every append so far held |
| [`tb_len`](#tb_len) | how many bytes the builder has appended |
| [`tb_finish`](#tb_finish) | hand off the built text, terminated and fitted to its length, owned through the builder's allocator |
| [`tb_dnit`](#tb_dnit) | release what the builder holds and leave it empty |

---

## Error

```mach
pub tag Error: u8 {
    alloc: std.allocator.Error;
    overflow;
}
```

the builder's failure: the allocator refused, or a length or capacity left usize

### text

```mach
pub fun text(e: Error) str
```

the text of the builder's failure

## TextBuilder

```mach
pub rec TextBuilder {
    alloc: *std.allocator.Allocator;
    data:  *u8;
    len:   usize;
    cap:   usize;
    fault: opt[Error];
}
```

text built one append at a time. the first failure is kept: every later append
and the finish refuse with it, so a writer may append a run of pieces and take
the failure once at `tb_finish` or `tb_fault`

### tb_init

```mach
pub fun tb_init(a: *std.allocator.Allocator) TextBuilder
```

an empty builder growing in `a`, released with tb_dnit

### tb_append

```mach
pub fun tb_append(b: *TextBuilder, text: str) err[Error]
```

append `text` up to its first NUL

| Parameter | |
| :--- | :--- |
| `b` | the builder |
| `text` | the text |
| **returns** | the builder's first failure, which this append may be |

### tb_fault

```mach
pub fun tb_fault(b: *TextBuilder) err[Error]
```

the builder's first failure, or ok when every append so far held

### tb_len

```mach
pub fun tb_len(b: *TextBuilder) usize
```

how many bytes the builder has appended

### tb_finish

```mach
pub fun tb_finish(b: *TextBuilder) res[str, Error]
```

hand off the built text, terminated and fitted to its length, owned through the builder's
allocator. the builder keeps no storage after a success

| Parameter | |
| :--- | :--- |
| `b` | the builder |
| **returns** | the text, or the builder's first failure, which the final fit may be |

### tb_dnit

```mach
pub fun tb_dnit(b: *TextBuilder)
```

release what the builder holds and leave it empty

