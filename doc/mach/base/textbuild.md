[mach](../README.md) / [base](README.md) / **textbuild**

# mach.base.textbuild

a text builder that keeps its first failure

every later append and the finish refuse with that failure, so a writer appends a run of
pieces and checks once.

<sub>source: [`src/base/textbuild.mach`](../../../src/base/textbuild.mach) · imported by 12 modules</sub>

<sub>`A` is `std.allocator`</sub>

| Type | | |
| :--- | :--- | :--- |
| [`Error`](#error) | `tag` | the builder's failure: the allocator refused, or a length or capacity left usize |
| [`TextBuilder`](#textbuilder) | `rec` | text built one append at a time |

| On `Error` | |
| :--- | :--- |
| [`text`](#text) | |

| On `TextBuilder` | |
| :--- | :--- |
| [`tb_init`](#tb_init) · [`tb_append`](#tb_append) | |
| [`tb_fault`](#tb_fault) | the builder's first failure, or ok when every append so far held |
| [`tb_len`](#tb_len) · [`tb_finish`](#tb_finish) · [`tb_dnit`](#tb_dnit) | |

---

## Error

```mach
pub tag Error: u8 {
    alloc: A.Error;
    overflow;
}
```

the builder's failure: the allocator refused, or a length or capacity left usize

### text

```mach
pub fun text(e: Error) str
```

## TextBuilder

```mach
pub rec TextBuilder {
    alloc: *A.Allocator;
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
pub fun tb_init(a: *A.Allocator) TextBuilder
```

### tb_append

```mach
pub fun tb_append(b: *TextBuilder, text: str) err[Error]
```

### tb_fault

```mach
pub fun tb_fault(b: *TextBuilder) err[Error]
```

the builder's first failure, or ok when every append so far held

### tb_len

```mach
pub fun tb_len(b: *TextBuilder) usize
```

### tb_finish

```mach
pub fun tb_finish(b: *TextBuilder) res[str, Error]
```

### tb_dnit

```mach
pub fun tb_dnit(b: *TextBuilder)
```

