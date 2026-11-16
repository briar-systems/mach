[mach](../README.md) / [base](README.md) / **fingerprint**

# mach.base.fingerprint

domain-separated fingerprint encoding, and the sha-256 digests of bytes and files

a fingerprinted value goes into a wire sink after its domain and schema version, so values of
two domains never share bytes. a name goes by its spelling, so fingerprints agree across
interners, and content goes by its digest. a file's digest is refused when the file changed
while it was read.

Source: [`src/base/fingerprint.mach`](../../../src/base/fingerprint.mach)

Uses types from [`mach.base.fail`](fail.md), [`mach.base.intern`](intern.md), [`mach.base.wire`](wire.md).

| Type | | |
| :--- | :--- | :--- |
| [`Domain`](#domain) | `def` |  |

| On `Domain` | |
| :--- | :--- |
| [`DOMAIN_SUBSYSTEM`](#domain), [`DOMAIN_BUILD_GOAL`](#domain), [`DOMAIN_ARCH`](#domain), [`DOMAIN_ABI`](#domain), [`DOMAIN_OS`](#domain), [`DOMAIN_BUILD_STEP`](#domain), [`DOMAIN_OF`](#domain), [`DOMAIN_DEBUG`](#domain), [`DOMAIN_TARGET_MODEL`](#domain), [`DOMAIN_BACKEND`](#domain), [`DOMAIN_IMAGE`](#domain), [`DOMAIN_FS_POLICY`](#domain), [`put_domain_u8`](#put_domain_u8), [`put_domain_u32`](#put_domain_u32), [`put_domain_u64`](#put_domain_u64), [`put_domain_bytes`](#put_domain_bytes), [`put_domain_str`](#put_domain_str) | |

| Functions | |
| :--- | :--- |
| [`file_sha256`](#file_sha256) | the sha-256 of the file at `path` into `digest`, or what refused it, which the caller names with the path |
| [`held_file_sha256`](#held_file_sha256) | borrows the file and exclusive seek/read access, rewinds first and leaves it at eof failure does not publish a digest, the caller retains close ownership and names the file |
| [`put_text`](#put_text) | a text as its u32 length then its bytes; nil and empty both put the length 0 |
| [`put_name`](#put_name) | an interned name by its spelling, so fingerprints agree across interners: a u8 1 then its text, or a u8 0 for no name |
| [`put_content`](#put_content) | bytes by their sha-256 digest, a u32 32 then the digest, or the u32 0 when there are none |
| [`put_file`](#put_file) | a link input by the digest of its contents, as put_content puts bytes; a refusal names the input, its text interned through `a` into `itn` |

---

## Domain

```mach
pub def Domain: u16
```

| Name | Value | |
| :--- | :--- | :--- |
| `DOMAIN_SUBSYSTEM` | `1` |  |
| `DOMAIN_BUILD_GOAL` | `2` |  |
| `DOMAIN_ARCH` | `3` |  |
| `DOMAIN_ABI` | `4` |  |
| `DOMAIN_OS` | `5` |  |
| `DOMAIN_BUILD_STEP` | `6` |  |
| `DOMAIN_OF` | `7` |  |
| `DOMAIN_DEBUG` | `8` |  |
| `DOMAIN_TARGET_MODEL` | `9` |  |
| `DOMAIN_BACKEND` | `10` |  |
| `DOMAIN_IMAGE` | `11` |  |
| `DOMAIN_FS_POLICY` | `12` |  |

### put_domain_u8

```mach
pub fun put_domain_u8(s: *mach.base.wire.Sink, domain: Domain, schema_version: u8, value: u8)
```

### put_domain_u32

```mach
pub fun put_domain_u32(s: *mach.base.wire.Sink, domain: Domain, schema_version: u8, value: u32)
```

### put_domain_u64

```mach
pub fun put_domain_u64(s: *mach.base.wire.Sink, domain: Domain, schema_version: u8, value: u64)
```

### put_domain_bytes

```mach
pub fun put_domain_bytes(s: *mach.base.wire.Sink, domain: Domain, schema_version: u8, p: *u8, len: usize)
```

### put_domain_str

```mach
pub fun put_domain_str(s: *mach.base.wire.Sink, domain: Domain, schema_version: u8, text: str)
```

## Functions

### file_sha256

```mach
pub fun file_sha256(path: str, digest: *u8) err[mach.base.fail.Cause]
```

the sha-256 of the file at `path` into `digest`, or what refused it, which the
caller names with the path

### held_file_sha256

```mach
pub fun held_file_sha256(f: std.filesystem.File, digest: *u8) err[mach.base.fail.Cause]
```

borrows the file and exclusive seek/read access, rewinds first and leaves it at eof
failure does not publish a digest, the caller retains close ownership and names the file

### put_text

```mach
pub fun put_text(s: *mach.base.wire.Sink, text: str)
```

a text as its u32 length then its bytes; nil and empty both put the length 0

### put_name

```mach
pub fun put_name(s: *mach.base.wire.Sink, itn: *mach.base.intern.Interner, name: mach.base.intern.StrId)
```

an interned name by its spelling, so fingerprints agree across interners:
a u8 1 then its text, or a u8 0 for no name

### put_content

```mach
pub fun put_content(s: *mach.base.wire.Sink, bytes: *u8, len: usize)
```

bytes by their sha-256 digest, a u32 32 then the digest, or the u32 0 when
there are none

### put_file

```mach
pub fun put_file(s: *mach.base.wire.Sink, itn: *mach.base.intern.Interner, a: *std.allocator.Allocator, path: str)
```

a link input by the digest of its contents, as put_content puts bytes; a refusal names the
input, its text interned through `a` into `itn`

