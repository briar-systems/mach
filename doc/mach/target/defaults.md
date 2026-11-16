[mach](../README.md) / [target](README.md) / **defaults**

# mach.target.defaults

what mach asks of a target's images where its manifest states nothing

these are mach's own requirements of the programs it builds, not facts of a
platform, so they live here and not in mink's host rows or a manifest. each
row is keyed by mink's system and states a value only where mach requires
one. a target whose manifest states the key keeps its own value, and a
system no row names states none, so mink writes its format's default

<sub>source: [`src/target/defaults.mach`](../../../src/target/defaults.mach) · imported by 1 module</sub>

<sub>`mink_os` is `mink.catalog.os`</sub>

| Functions | |
| :--- | :--- |
| [`stack_reserve`](#stack_reserve) | the main stack reserve mach states for an image of `sys`, none where it states none |

---

## Functions

### stack_reserve

```mach
pub fun stack_reserve(sys: *mink_os.System) opt[u64]
```

the main stack reserve mach states for an image of `sys`, none where it states none

