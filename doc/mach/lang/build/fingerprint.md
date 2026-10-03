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
pub fun file_sha256(path: str, digest: *u8) err[fail.Fail];
```

## fun held_file_sha256

```mach
pub fun held_file_sha256(f: fs.File, digest: *u8) err[fail.Fail];
```

borrows the file and exclusive seek/read access, rewinds first and leaves it at eof
failure does not publish a digest, the caller retains close ownership

## fun put_domain_u8

```mach
pub fun put_domain_u8(s: *wire.Sink, domain: Domain, schema_version: u8, value: u8);
```

## fun put_domain_u32

```mach
pub fun put_domain_u32(s: *wire.Sink, domain: Domain, schema_version: u8, value: u32);
```

## fun put_domain_u64

```mach
pub fun put_domain_u64(s: *wire.Sink, domain: Domain, schema_version: u8, value: u64);
```

## fun put_domain_bytes

```mach
pub fun put_domain_bytes(s: *wire.Sink, domain: Domain, schema_version: u8, p: *u8, len: usize);
```

## fun put_domain_str

```mach
pub fun put_domain_str(s: *wire.Sink, domain: Domain, schema_version: u8, text: str);
```

## fun put_text

```mach
pub fun put_text(s: *wire.Sink, text: str);
```

a text as its u32 length then its bytes; nil and empty both put the length 0

## fun put_name

```mach
pub fun put_name(s: *wire.Sink, itn: *intern.Interner, name: intern.StrId);
```

an interned name by its spelling, so fingerprints agree across interners:
a u8 1 then its text, or a u8 0 for no name

## fun put_content

```mach
pub fun put_content(s: *wire.Sink, bytes: *u8, len: usize);
```

bytes by their sha-256 digest, a u32 32 then the digest, or the u32 0 when
there are none

## fun put_file

```mach
pub fun put_file(s: *wire.Sink, path: str);
```

a file by the digest of its contents, as put_content puts bytes

