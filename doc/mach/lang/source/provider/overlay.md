# mach.lang.source.provider.overlay

## rec Overlay

```mach
pub rec Overlay;
```

the provider an editor reads through: the session's open buffers stand in front of the
provider under them, so a buffer is read for its file and a buffer whose file is not on
disk yet still names a module

base: the provider the buffers stand in front of
s: the session holding the buffers, keyed by the path the base locates

## fun init

```mach
pub fun init(base: provider.Provider, s: *session.Session) Overlay;
```

## fun provider_of

```mach
pub fun provider_of(o: *Overlay) provider.Provider;
```

