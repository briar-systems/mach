# mach.lang.session

## rec Overlay

```mach
pub rec Overlay;
```

## rec LinkProvider

```mach
pub rec LinkProvider;
```

## rec LinkPublication

```mach
pub rec LinkPublication;
```

## rec Session

```mach
pub rec Session;
```

## fun init

```mach
pub fun init(alloc: *A.Allocator) res[Session, fail.Fail];
```

## fun dnit

```mach
pub fun dnit(s: *Session);
```

## fun expire_views

```mach
pub fun expire_views(s: *Session);
```

exhaustion permanently refuses new borrowed views while teardown remains infallible

## fun reset_module_registry

```mach
pub fun reset_module_registry(s: *Session);
```

## fun register_module_ast

```mach
pub fun register_module_ast(s: *Session, mid: module.ModuleId, a: *ast.Ast) err[fail.Fail];
```

## fun module_ast

```mach
pub fun module_ast(s: *Session, mid: module.ModuleId) *ast.Ast;
```

## fun module_count

```mach
pub fun module_count(s: *Session) u32;
```

## fun register_module_fqn

```mach
pub fun register_module_fqn(s: *Session, mid: module.ModuleId, fqn: intern.StrId) err[fail.Fail];
```

## fun module_fqn

```mach
pub fun module_fqn(s: *Session, mid: module.ModuleId) intern.StrId;
```

## fun stable_module_id

```mach
pub fun stable_module_id(s: *Session, fqn: intern.StrId) res[module.StableModuleId, fail.Fail];
```

## fun stable_module_id_lookup

```mach
pub fun stable_module_id_lookup(s: *Session, fqn: intern.StrId) module.StableModuleId;
```

## fun register_stable_mid

```mach
pub fun register_stable_mid(s: *Session, stable: module.StableModuleId, mid: module.ModuleId) err[fail.Fail];
```

## fun module_id_for_stable

```mach
pub fun module_id_for_stable(s: *Session, stable: module.StableModuleId) module.ModuleId;
```

## fun register_export_name

```mach
pub fun register_export_name(s: *Session, mid: module.ModuleId, did: u32, name: intern.StrId) err[fail.Fail];
```

## fun export_name

```mach
pub fun export_name(s: *Session, mid: module.ModuleId, did: u32) opt[intern.StrId];
```

## fun register_export_library

```mach
pub fun register_export_library(s: *Session, mid: module.ModuleId, did: u32, lib: intern.StrId) err[fail.Fail];
```

## fun export_library

```mach
pub fun export_library(s: *Session, mid: module.ModuleId, did: u32) opt[intern.StrId];
```

## fun reset_type_projection

```mach
pub fun reset_type_projection(s: *Session) err[fail.Fail];
```

## fun register_type_align

```mach
pub fun register_type_align(s: *Session, tid: u32, align: u32) err[fail.Fail];
```

## fun register_type_packed

```mach
pub fun register_type_packed(s: *Session, tid: u32) err[fail.Fail];
```

## fun type_is_packed

```mach
pub fun type_is_packed(s: *Session, tid: u32) bool;
```

## fun register_type_volatile

```mach
pub fun register_type_volatile(s: *Session, tid: u32) err[fail.Fail];
```

## fun type_is_volatile

```mach
pub fun type_is_volatile(s: *Session, tid: u32) bool;
```

## fun type_align

```mach
pub fun type_align(s: *Session, tid: u32) opt[u32];
```

## fun register_import_library

```mach
pub fun register_import_library(s: *Session, name: intern.StrId, lib: intern.StrId) err[fail.Fail];
```

## fun import_library

```mach
pub fun import_library(s: *Session, name: intern.StrId) opt[intern.StrId];
```

## fun register_link_provider

```mach
pub fun register_link_provider(s: *Session, name: intern.StrId, is_static: bool) err[fail.Fail];
```

## fun link_provider

```mach
pub fun link_provider(s: *Session, name: intern.StrId) opt[LinkProvider];
```

## fun link_providers

```mach
pub fun link_providers(s: *Session) *LinkProvider;
```

## fun link_provider_count

```mach
pub fun link_provider_count(s: *Session) u32;
```

## fun reset_link_providers

```mach
pub fun reset_link_providers(s: *Session);
```

## fun record_link_publication

```mach
pub fun record_link_publication(s: *Session, path: str, revision: query.Revision, digest: *[32]u8) err[fail.Fail];
```

## fun lookup_link_publication

```mach
pub fun lookup_link_publication(s: *Session, path: str, revision: query.Revision, out: *[32]u8) bool;
```

## fun reset_link_publications

```mach
pub fun reset_link_publications(s: *Session);
```

## fun register_module_sema

```mach
pub fun register_module_sema(s: *Session, mid: module.ModuleId, p: ptr) err[fail.Fail];
```

## fun module_sema_ptr

```mach
pub fun module_sema_ptr(s: *Session, mid: module.ModuleId) ptr;
```

## fun register_module_resolve

```mach
pub fun register_module_resolve(s: *Session, mid: module.ModuleId, p: ptr) err[fail.Fail];
```

## fun module_resolve_ptr

```mach
pub fun module_resolve_ptr(s: *Session, mid: module.ModuleId) ptr;
```

## fun register_module_comptime

```mach
pub fun register_module_comptime(s: *Session, mid: module.ModuleId, p: ptr) err[fail.Fail];
```

## fun register_module_phase

```mach
pub fun register_module_phase(s: *Session, mid: module.ModuleId, sema: ptr, resolve: ptr,
comptime_ctx: ptr) err[fail.Fail];
```

## fun register_module_identity

```mach
pub fun register_module_identity(s: *Session, mid: module.ModuleId, a: *ast.Ast,
fqn: intern.StrId, stable: module.StableModuleId) err[fail.Fail];
```

## fun unregister_module_identity

```mach
pub fun unregister_module_identity(s: *Session, mid: module.ModuleId, stable: module.StableModuleId);
```

## fun module_comptime_ptr

```mach
pub fun module_comptime_ptr(s: *Session, mid: module.ModuleId) ptr;
```

## fun next_parse_incarnation

```mach
pub fun next_parse_incarnation(s: *Session) res[u64, fail.Fail];
```

## fun load_source

```mach
pub fun load_source(s: *Session, path: str, text: str) res[source.FileId, fail.Fail];
```

## rec PreparedOverlay

```mach
pub rec PreparedOverlay;
```

## fun prepare_overlay

```mach
pub fun prepare_overlay(s: *Session, path: str, text: str, remove: bool) res[PreparedOverlay, fail.Fail];
```

the caller exclusively borrows overlay state through commit or discard

## fun discard_overlay

```mach
pub fun discard_overlay(prepared: *PreparedOverlay);
```

## fun commit_overlay

```mach
pub fun commit_overlay(prepared: *PreparedOverlay) bool;
```

## fun set_overlay

```mach
pub fun set_overlay(s: *Session, path: str, text: str) err[fail.Fail];
```

## fun clear_overlay

```mach
pub fun clear_overlay(s: *Session, path: str) res[bool, fail.Fail];
```

## fun overlay_get

```mach
pub fun overlay_get(s: *Session, path: str) opt[str];
```

