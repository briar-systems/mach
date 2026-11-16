# mach.base.textbuild

## tag Error

```mach
pub tag Error: u8 {
    alloc: A.Error;
    overflow;
}
```

the builder's failure: the allocator refused, or a length or capacity left usize

## fun text

```mach
pub fun text(e: Error) str;
```

## rec TextBuilder

```mach
pub rec TextBuilder;
```

text built one append at a time. the first failure is kept: every later append
and the finish refuse with it, so a writer may append a run of pieces and take
the failure once at `tb_finish` or `tb_fault`

## fun tb_init

```mach
pub fun tb_init(a: *A.Allocator) TextBuilder;
```

## fun tb_append

```mach
pub fun tb_append(b: *TextBuilder, text: str) err[Error];
```

## fun tb_fault

```mach
pub fun tb_fault(b: *TextBuilder) err[Error];
```

the builder's first failure, or ok when every append so far held

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

