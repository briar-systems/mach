# mach.lang.target.fingerprint

the bytes that identify a resolved target to the build cache: the owners' rows
it names, by their spelling, the selection it stands on and the image keys
its manifest states

## fun put

```mach
pub fun put(t: *resolved.Target, s: *wire.Sink);
```

