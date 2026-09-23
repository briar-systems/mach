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

## fun fp_ct_value

```mach
pub fun fp_ct_value(fb: *FpBuf, value: *comptime.CTValue, origin: session.StableModuleId,
definition_revision: query.Revision) err[fail.Fail];
```

a CONST_ELEM value names an element of its origin's definition, so it folds that definition's revision

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

## rec GateRecord

```mach
pub rec GateRecord;
```

one module's gate pass product under one target: what it decided, and what its importers read

status: the pass's standing; never internal, which fails the product instead
consts: how many imported public constants a union tuple round bound; 0 for the build target
states: the gate state of every expression, indexed by ExprId, borrowed from the bytes
state_count: how many
view: the importer view, as gate_view_encode writes it, borrowed
view_len: its length

## rec GateView

```mach
pub rec GateView;
```

what an importer's gate pass reads of a module: its gate surface and, under a union tuple,
the public symbols that tuple's resolution exported

surface: the encoded gate surface, as typed_surface_encode writes it, borrowed
surface_len: its length
exports: the tuple's public symbols, as fp_public_surface writes them; empty for the build target
exports_len: their length

## fun gate_view_encode

```mach
pub fun gate_view_encode(fb: *FpBuf, surface: *u8, surface_len: u32, exports: *u8, exports_len: u32) err[fail.Fail];
```

## fun gate_view_decode

```mach
pub fun gate_view_decode(bytes: *u8, len: u32) res[GateView, fail.Fail];
```

## fun gate_record_encode

```mach
pub fun gate_record_encode(fb: *FpBuf, status: fail.PhaseStatus, consts: u32, states: *comptime.GateState, state_count: u32,
view: *u8, view_len: u32) err[fail.Fail];
```

## fun gate_record_decode

```mach
pub fun gate_record_decode(bytes: *u8, len: u32) res[GateRecord, fail.Fail];
```

