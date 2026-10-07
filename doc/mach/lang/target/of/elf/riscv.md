# mach.lang.target.of.elf.riscv

## val EF_RVC

```mach
pub val EF_RVC:              u32 = 0x1
```

the e_flags bits riscv defines: compressed instructions, the float abi, and
the RV32E/RV64E calling convention

## val EF_FLOAT_ABI_SOFT

```mach
pub val EF_FLOAT_ABI_SOFT:   u32 = 0x0
```

## val EF_FLOAT_ABI_SINGLE

```mach
pub val EF_FLOAT_ABI_SINGLE: u32 = 0x2
```

## val EF_FLOAT_ABI_DOUBLE

```mach
pub val EF_FLOAT_ABI_DOUBLE: u32 = 0x4
```

## val EF_RVE

```mach
pub val EF_RVE:              u32 = 0x8
```

## val SHT_ATTRIBUTES

```mach
pub val SHT_ATTRIBUTES: u32 = 0x70000003
```

the type of the `.riscv.attributes` section

## fun flags

```mach
pub fun flags(float_arg_bits: u32, has_compressed: bool) u32;
```

the e_flags word of an object whose abi passes floats in `float_arg_bits`-wide
registers

## val FLAGS_ABI

```mach
pub val FLAGS_ABI: u32 = EF_FLOAT_ABI_SINGLE | EF_FLOAT_ABI_DOUBLE
```

the e_flags bits that name the float abi, which every linked object shares

## fun flags_abi_name

```mach
pub fun flags_abi_name(abi_bits: u32) str;
```

the float abi the bits `abi_bits` of an e_flags word name, for a refusal

## fun target_flags

```mach
pub fun target_flags(target: *target_of.ObjectTarget) u32;
```

the e_flags word of an object built for `target`, which holds no compressed instructions

## fun target_attributes

```mach
pub fun target_attributes(alloc: *A.Allocator, target: *target_of.ObjectTarget, out_len: *u32) res[*u8, fail.Fail];
```

the attribute section of an object built for `target`

## fun target_attributes_validate

```mach
pub fun target_attributes_validate(bytes: *u8, len: u32, target: *target_of.ObjectTarget, flags: u32) err[fail.Fail];
```

an input's attribute section and e_flags against what `target` can link

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

## fun attributes_build

```mach
pub fun attributes_build(alloc: *A.Allocator, xlen_bits: u32, extension_bits: u64, float_arg_bits: u32,
has_compressed: bool, out_len: *u32) res[*u8, fail.Fail];
```

## fun attributes_validate

```mach
pub fun attributes_validate(bytes: *u8, len: u32, xlen_bits: u32, flags: u32) err[fail.Fail];
```

refuses only what cannot link into the target: an attribute section that does not parse,
a Tag_RISCV_arch of another XLEN, or the RV32E/RV64E calling convention, which no target
uses. the extensions an object names are merged into the output and never refused, since
whether mach generates code for one says nothing about linking an object that uses it.
the float ABI in the flags is checked where every input's flags are merged

## fun attributes_merge

```mach
pub fun attributes_merge(alloc: *A.Allocator, a: *u8, a_len: u32,
b: *u8, b_len: u32, out_len: *u32) res[*u8, fail.Fail];
```

