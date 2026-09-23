# mach.lang.be.linker.resolve

## rec SymbolLoc

```mach
pub rec SymbolLoc;
```

## fun synthesize_common_storage

```mach
pub fun synthesize_common_storage(s: *session.Session, modules: *of.ObjectImage,
module_count: u32, mode: LinkMode,
out: *of.ObjectImage) res[bool, fail.Fail];
```

## fun weak_fallback_name

```mach
pub fun weak_fallback_name(img: *of.ObjectImage,
sym: *of.Symbol) res[intern.StrId, fail.Fail];
```

## fun resolved_symbol_name

```mach
pub fun resolved_symbol_name(sym_locs: *map.Map[intern.StrId, SymbolLoc],
name: intern.StrId) res[intern.StrId, fail.Fail];
```

## fun collect_symbols

```mach
pub fun collect_symbols(s: *session.Session, modules: *of.ObjectImage, module_count: u32,
placements: *Placement, sec_base: *u32,
sym_locs: *map.Map[intern.StrId, SymbolLoc], have_vaddrs: bool,
atoms: *AtomPlan) err[fail.Fail];
```

## fun is_undefined_extern

```mach
pub fun is_undefined_extern(sym: *of.Symbol) bool;
```

