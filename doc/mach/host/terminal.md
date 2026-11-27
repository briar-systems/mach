[mach](../README.md) / [host](README.md) / **terminal**

# mach.host.terminal

whether a descriptor is a terminal, where the target has terminal control to ask it of

Source: [`src/host/terminal.mach`](../../../src/host/terminal.mach)

| Functions | |
| :--- | :--- |
| [`is_terminal`](#is_terminal) | whether descriptor `fd` is a terminal |

---

## Functions

### is_terminal

```mach
pub fun is_terminal(fd: usize) bool
```

whether descriptor `fd` is a terminal. a target with no terminal control has none to draw on,
so every descriptor there is not one

