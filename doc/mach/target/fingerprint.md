[mach](../README.md) / [target](README.md) / **fingerprint**

# mach.target.fingerprint

the bytes that identify a resolved target to the build cache: the owners' rows
it names, by their spelling, the selection it stands on and the image keys
its manifest states

Source: [`src/target/fingerprint.mach`](../../../src/target/fingerprint.mach)

Uses types from [`mach.base.wire`](../base/wire.md), [`mach.target.resolved`](resolved.md).

| Functions | |
| :--- | :--- |
| [`put`](#put) | |

---

## Functions

### put

```mach
pub fun put(t: *mach.target.resolved.Target, s: *mach.base.wire.Sink)
```

