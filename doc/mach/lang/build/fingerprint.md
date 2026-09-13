# mach.lang.build.fingerprint

## def Domain

```mach
pub def Domain: u16
```

## val DOMAIN_SUBSYSTEM

```mach
pub val DOMAIN_SUBSYSTEM:  Domain = 1
```

## val DOMAIN_BUILD_GOAL

```mach
pub val DOMAIN_BUILD_GOAL: Domain = 2
```

## val DOMAIN_ARCH

```mach
pub val DOMAIN_ARCH:       Domain = 3
```

## val DOMAIN_ABI

```mach
pub val DOMAIN_ABI:        Domain = 4
```

## val DOMAIN_OS

```mach
pub val DOMAIN_OS:         Domain = 5
```

## val DOMAIN_BUILD_STEP

```mach
pub val DOMAIN_BUILD_STEP: Domain = 6
```

## val DOMAIN_OF

```mach
pub val DOMAIN_OF:           Domain = 7
```

## val DOMAIN_DEBUG

```mach
pub val DOMAIN_DEBUG:        Domain = 8
```

## val DOMAIN_TARGET_MODEL

```mach
pub val DOMAIN_TARGET_MODEL: Domain = 9
```

## val DOMAIN_BACKEND

```mach
pub val DOMAIN_BACKEND:      Domain = 10
```

## val DOMAIN_IMAGE

```mach
pub val DOMAIN_IMAGE:        Domain = 11
```

## val DOMAIN_FS_POLICY

```mach
pub val DOMAIN_FS_POLICY:    Domain = 12
```

## val FILE_DIGEST_CHUNK

```mach
pub val FILE_DIGEST_CHUNK: usize = 8192
```

## fun file_sha256

```mach
pub fun file_sha256(path: str, digest: *u8) err[outcome.Fail];
```

## fun held_file_sha256

```mach
pub fun held_file_sha256(f: fs.File, digest: *u8) err[outcome.Fail];
```

borrows the file and exclusive seek/read access, rewinds first and leaves it at eof
failure does not publish a digest, the caller retains close ownership

## fun write_domain_u8

```mach
pub fun write_domain_u8(e: *binary.Encoder, domain: Domain, schema_version: u8, value: u8) bool;
```

## fun write_domain_u32

```mach
pub fun write_domain_u32(e: *binary.Encoder, domain: Domain, schema_version: u8, value: u32) bool;
```

## fun write_domain_u64

```mach
pub fun write_domain_u64(e: *binary.Encoder, domain: Domain, schema_version: u8, value: u64) bool;
```

## fun write_domain_bytes

```mach
pub fun write_domain_bytes(e: *binary.Encoder, domain: Domain, schema_version: u8, p: *u8, len: usize) bool;
```

## fun write_domain_str

```mach
pub fun write_domain_str(e: *binary.Encoder, domain: Domain, schema_version: u8, s: str) bool;
```

