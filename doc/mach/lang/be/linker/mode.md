# mach.lang.be.linker.mode

## def LinkMode

```mach
pub def LinkMode: u8
```

## val LINK_EXE

```mach
pub val LINK_EXE:         LinkMode = 0
```

## val LINK_RELOCATABLE

```mach
pub val LINK_RELOCATABLE: LinkMode = 1
```

## val LINK_SHARED

```mach
pub val LINK_SHARED:      LinkMode = 2
```

## fun link_mode_valid

```mach
pub fun link_mode_valid(mode: LinkMode) bool;
```

## fun mode_allocates_commons

```mach
pub fun mode_allocates_commons(mode: LinkMode) opt[bool];
```

whether a link allocates storage for common symbols: a relocatable link
leaves them for the final link; absent for a mode outside the catalog

## fun unknown_link_mode

```mach
pub fun unknown_link_mode(s: *session.Session, mode: LinkMode) str;
```

## fun mode_needs_loader

```mach
pub fun mode_needs_loader(mode: LinkMode) opt[bool];
```

whether a link mode's product is mapped by a loader on its own account: a
shared library always is; absent for a mode outside the catalog

