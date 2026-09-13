# mach.lang.target.isa.riscv.attributes

## val TAG_STACK_ALIGN

```mach
pub val TAG_STACK_ALIGN:        u32 = 4
```

## val TAG_ARCH

```mach
pub val TAG_ARCH:               u32 = 5
```

## val TAG_UNALIGNED_ACCESS

```mach
pub val TAG_UNALIGNED_ACCESS:   u32 = 6
```

## val TAG_PRIV_SPEC

```mach
pub val TAG_PRIV_SPEC:          u32 = 8
```

## val TAG_PRIV_SPEC_MINOR

```mach
pub val TAG_PRIV_SPEC_MINOR:    u32 = 10
```

## val TAG_PRIV_SPEC_REVISION

```mach
pub val TAG_PRIV_SPEC_REVISION: u32 = 12
```

## val RISCV64_STACK_ALIGN

```mach
pub val RISCV64_STACK_ALIGN: u32 = 16
```

## rec Ext

```mach
pub rec Ext;
```

## rec Arch

```mach
pub rec Arch;
```

## rec Unknown

```mach
pub rec Unknown;
```

## rec Attrs

```mach
pub rec Attrs;
```

## fun attrs_blank

```mach
pub fun attrs_blank(at: *Attrs);
```

## fun parse_arch

```mach
pub fun parse_arch(s: *u8, len: u32, out: *Arch) bool;
```

## fun merge_arch

```mach
pub fun merge_arch(a: *Arch, b: *Arch) err[fail.Fail];
```

## fun parse_body

```mach
pub fun parse_body(body: *u8, len: u32, out: *Attrs) err[fail.Fail];
```

## fun merge_attrs

```mach
pub fun merge_attrs(a: *Attrs, b: *Attrs) err[fail.Fail];
```

## fun serialize

```mach
pub fun serialize(alloc: *A.Allocator, at: *Attrs, out_len: *u32) res[*u8, fail.Fail];
```

## fun riscv64_build_attributes

```mach
pub fun riscv64_build_attributes(alloc: *A.Allocator, xlen_bits: u32, extension_bits: u32, float_arg_bits: u32,
has_compressed: bool, out_len: *u32) res[*u8, fail.Fail];
```

## fun validate_selected

```mach
pub fun validate_selected(bytes: *u8, len: u32, xlen_bits: u32, selected: u32, flags: u32) err[fail.Fail];
```

refuses an object whose Tag_RISCV_arch or header flags need something the selected
target lacks: an unknown extension, an unavailable one, another revision or XLEN

## fun riscv64_merge_attributes

```mach
pub fun riscv64_merge_attributes(alloc: *A.Allocator, a: *u8, a_len: u32,
b: *u8, b_len: u32, out_len: *u32) res[*u8, fail.Fail];
```

