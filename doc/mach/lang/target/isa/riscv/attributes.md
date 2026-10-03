# mach.lang.target.isa.riscv.attributes

## rec Ext

```mach
pub rec Ext;
```

## rec Arch

```mach
pub rec Arch;
```

## rec Attrs

```mach
pub rec Attrs;
```

## fun parse_arch

```mach
pub fun parse_arch(s: *u8, len: u32, out: *Arch) bool;
```

## fun parse_body

```mach
pub fun parse_body(body: *u8, len: u32, out: *Attrs) err[fail.Fail];
```

## fun serialize

```mach
pub fun serialize(alloc: *A.Allocator, at: *Attrs, out_len: *u32) res[*u8, fail.Fail];
```

## fun riscv64_build_attributes

```mach
pub fun riscv64_build_attributes(alloc: *A.Allocator, xlen_bits: u32, extension_bits: u64, float_arg_bits: u32,
has_compressed: bool, out_len: *u32) res[*u8, fail.Fail];
```

## fun validate_input

```mach
pub fun validate_input(bytes: *u8, len: u32, xlen_bits: u32, flags: u32) err[fail.Fail];
```

refuses only what cannot link into the target: an attribute section that does not parse,
a Tag_RISCV_arch of another XLEN, or the RV32E/RV64E calling convention, which no target
uses. the extensions an object names are merged into the output and never refused, since
whether mach generates code for one says nothing about linking an object that uses it.
the float ABI in the flags is checked where every input's flags are merged

## fun riscv64_merge_attributes

```mach
pub fun riscv64_merge_attributes(alloc: *A.Allocator, a: *u8, a_len: u32,
b: *u8, b_len: u32, out_len: *u32) res[*u8, fail.Fail];
```

