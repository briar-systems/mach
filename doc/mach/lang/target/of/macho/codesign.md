# mach.lang.target.of.macho.codesign

## fun codesign_ident_len

```mach
pub fun codesign_ident_len(name: *u8) usize;
```

## fun code_signature_size

```mach
pub fun code_signature_size(ident_len: usize, code_limit: usize) usize;
```

## fun write_code_signature

```mach
pub fun write_code_signature(buf: *u8, sig_off: usize, name: *u8, code_limit: usize,
exec_base: u64, exec_limit: u64) usize;
```

## fun seal_build_id

```mach
pub fun seal_build_id(buf: *u8, code_limit: usize, uuid_off: usize) err[fail.Fail];
```

the build id is the sha-256 of everything the code signature covers (header,
load commands, segments, __DWARF and the __LINKEDIT structures before the
signature) with the uuid bytes as zero; the signature written afterwards
then covers the final uuid, in ld64's order

