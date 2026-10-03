# mach.lang.target.catalog.arch

the instruction set catalog: every architecture id, its spelling and its
fingerprint tag, and the riscv selection strings that name an architecture

## val UNKNOWN

```mach
pub val UNKNOWN: u32 = 0
```

## val X86_64

```mach
pub val X86_64:  u32 = 1
```

## val AARCH64

```mach
pub val AARCH64: u32 = 2
```

## val RISCV64

```mach
pub val RISCV64: u32 = 3
```

## val SPIRV

```mach
pub val SPIRV:   u32 = 5
```

4 was the withdrawn MOS 6502 target; no catalog row carries it

## val RISCV32

```mach
pub val RISCV32: u32 = 6
```

## def DwarfRegFn

```mach
pub def DwarfRegFn: fun(i32) i32
```

## def CvRegFn

```mach
pub def CvRegFn: fun(i32) i32
```

## val VERSION

```mach
pub val VERSION: u8 = 2
```

version 2: catalog id 4 is reserved with no row and the tags after it moved up

## fun id_for

```mach
pub fun id_for(name: str) u32;
```

## fun float_absence_note

```mach
pub fun float_absence_note(name: str) str;
```

why a selection has no floating-point unit, for a diagnostic that already names
the selection: the letters a RISC-V string would need, or empty when the name
carries no extension vocabulary the front end could ask the user to add

## fun name_for

```mach
pub fun name_for(id: u32) str;
```

## fun count

```mach
pub fun count() usize;
```

## fun name_at

```mach
pub fun name_at(index: usize) str;
```

## fun fingerprint_tag

```mach
pub fun fingerprint_tag(id: u32) u8;
```

