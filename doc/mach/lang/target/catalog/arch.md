# mach.lang.target.catalog.arch

the instruction set catalog: every architecture id, its spelling, its
fingerprint tag and the selection syntax its names may carry

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

## fun syntax_for

```mach
pub fun syntax_for(name: str) *extension.SelectionSyntax;
```

the selection syntax that claims a name, nil when none does and only a
spelling names an instruction set

## fun syntax_of

```mach
pub fun syntax_of(id: u32) *extension.SelectionSyntax;
```

the selection syntax an instruction set's names carry, nil when it has none

## fun float_absence_note

```mach
pub fun float_absence_note(name: str) res[opt[str], fail.Fail];
```

why a selection has no floating-point unit, for a diagnostic that already names
the selection: what the selection syntax says it lacks, or none when the name
carries no extension vocabulary the front end could ask the user to add. the
name is the selection the target was resolved from, so one that does not
parse is the failure that refused it

## fun host_id

```mach
pub fun host_id() u32;
```

the instruction set the compiler itself was built for, UNKNOWN when the
catalog has no row for it

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

