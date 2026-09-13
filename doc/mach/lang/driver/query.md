# mach.lang.driver.query

## rec FpBuf

```mach
pub rec FpBuf;
```

## fun fp_u8

```mach
pub fun fp_u8(b: *FpBuf, v: u8) err[fail.Fail];
```

## fun fp_u32

```mach
pub fun fp_u32(b: *FpBuf, v: u32) err[fail.Fail];
```

## fun fp_u16

```mach
pub fun fp_u16(b: *FpBuf, v: u16) err[fail.Fail];
```

## fun fp_domain_u8

```mach
pub fun fp_domain_u8(b: *FpBuf, domain: fingerprint.Domain, schema_version: u8, value: u8) err[fail.Fail];
```

## fun fp_bytes

```mach
pub fun fp_bytes(b: *FpBuf, p: *u8, len: u32) err[fail.Fail];
```

## fun resolve_handle_encode

```mach
pub fun resolve_handle_encode(alloc: *A.Allocator, r: *resolve.ResolveResult) res[query.QueryOutput, fail.Fail];
```

## fun resolve_handle_decode

```mach
pub fun resolve_handle_decode(e: query.QueryView) *resolve.ResolveResult;
```

## fun fp_file_content

```mach
pub fun fp_file_content(fb: *FpBuf, path: *u8) err[fail.Fail];
```

## fun fp_content

```mach
pub fun fp_content(fb: *FpBuf, bytes: *u8, len: usize) err[fail.Fail];
```

## fun fp_u64

```mach
pub fun fp_u64(b: *FpBuf, v: u64) err[fail.Fail];
```

## fun fp_cstr

```mach
pub fun fp_cstr(b: *FpBuf, s: *u8) err[fail.Fail];
```

## fun fp_name

```mach
pub fun fp_name(b: *FpBuf, itn: *intern.Interner, name: intern.StrId) err[fail.Fail];
```

## fun fp_free

```mach
pub fun fp_free(fb: *FpBuf);
```

## fun fp_input_len

```mach
pub fun fp_input_len(fb: *FpBuf) res[u32, fail.Fail];
```

## fun fp_take

```mach
pub fun fp_take(fb: *FpBuf) res[query.QueryOutput, fail.Fail];
```

## fun fp_constant

```mach
pub fun fp_constant(fb: *FpBuf, item: *resolve.ExportConstant, origin: session.StableModuleId,
definition_revision: query.Revision) err[fail.Fail];
```

## fun fp_public_surface

```mach
pub fun fp_public_surface(fb: *FpBuf, rr: *resolve.ResolveResult, origin: session.StableModuleId,
definition_revision: query.Revision) err[fail.Fail];
```

## fun public_symbols_decode

```mach
pub fun public_symbols_decode(a: *A.Allocator, s: *session.Session, bytes: *u8, len: u32,
path: intern.StrId, module: session.ModuleId) res[resolve.ModuleExports, fail.Fail];
```

## fun typed_surface_encode

```mach
pub fun typed_surface_encode(fb: *FpBuf, surface: *scx.ModuleSema) err[fail.Fail];
```

## fun typed_surface_decode

```mach
pub fun typed_surface_decode(itn: *intern.Interner, a: *A.Allocator, bytes: *u8, len: u32,
path: intern.StrId, mid: session.ModuleId) res[scx.ModuleSema, fail.Fail];
```

