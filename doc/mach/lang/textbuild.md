# mach.lang.textbuild

## tag Error

```mach
pub tag Error: u8 {
    alloc: A.Error;
    overflow;
    failed;
}
```

the builder's failure: the allocator refused, a length or capacity left
usize, or the builder was used after an earlier failure

## fun text

```mach
pub fun text(e: Error) str;
```

## rec TextBuilder

```mach
pub rec TextBuilder;
```

## fun tb_init

```mach
pub fun tb_init(a: *A.Allocator) TextBuilder;
```

## fun tb_append

```mach
pub fun tb_append(b: *TextBuilder, text: str) err[Error];
```

## fun tb_append_all

```mach
pub fun tb_append_all(b: *TextBuilder, parts: *str, count: usize) err[Error];
```

## fun tb_failed

```mach
pub fun tb_failed(b: *TextBuilder) bool;
```

## fun tb_len

```mach
pub fun tb_len(b: *TextBuilder) usize;
```

## fun tb_finish

```mach
pub fun tb_finish(b: *TextBuilder) res[str, Error];
```

## fun tb_dnit

```mach
pub fun tb_dnit(b: *TextBuilder);
```

