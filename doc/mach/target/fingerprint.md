[mach](../README.md) / [target](README.md) / **fingerprint**

# mach.target.fingerprint

the bytes that identify a resolved target to the build cache: the owners' rows
it names, by their spelling, the selection it stands on and the image keys
its manifest states

<sub>source: [`src/target/fingerprint.mach`](../../../src/target/fingerprint.mach) · imported by 1 module</sub>

<sub>`resolved` is [`mach.target.resolved`](resolved.md) · `wire` is [`mach.base.wire`](../base/wire.md)</sub>

| Functions | |
| :--- | :--- |
| [`put`](#put) | |

---

## Functions

### put

```mach
pub fun put(t: *resolved.Target, s: *wire.Sink)
```

