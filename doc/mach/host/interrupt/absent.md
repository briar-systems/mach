[mach](../../README.md) / [host](../README.md) / [interrupt](README.md) / **absent**

# mach.host.interrupt.absent

the interrupt of a host that delivers no ending event: there is nothing to catch, so a hold
claims nothing and no interrupt is ever taken

Source: [`src/host/interrupt/absent.mach`](../../../../src/host/interrupt/absent.mach)

| Functions | |
| :--- | :--- |
| [`hold`](#hold) | ok: there is no event source to claim |
| [`release`](#release) | ok: there are no handlers to restore |
| [`taken`](#taken) | false: no ending event reaches this process |
| [`name`](#name) | none: no event is ever taken |
| [`status`](#status) | none: no event is ever taken |

---

## Functions

### hold

```mach
pub fun hold() err[std.io.error.Error]
```

ok: there is no event source to claim

### release

```mach
pub fun release() err[std.io.error.Error]
```

ok: there are no handlers to restore

### taken

```mach
pub fun taken() bool
```

false: no ending event reaches this process

### name

```mach
pub fun name() opt[str]
```

none: no event is ever taken

### status

```mach
pub fun status() opt[i64]
```

none: no event is ever taken

