[mach](../README.md) / [package](README.md) / **pin**

# mach.package.pin

the pin contract: where a project records the exact revision each dependency is held at,
and how it reads one back. git is the member today: a pin is the gitlink the enclosing
repository commits at dep/&lt;id>, registered in .gitmodules. a member recording pins
elsewhere, beside a source served over http, fills the same table

<sub>source: [`src/package/pin.mach`](../../../src/package/pin.mach) · imported by 5 modules</sub>

<sub>`BeginFn` is private · `ModeFn` is private · `ReadFn` is private · `ReadAtFn` is private · `TrackedFn` is private · `RecordedFn` is private · `RecordFn` is private · `DropFn` is private</sub>

| Type | | |
| :--- | :--- | :--- |
| [`Pin`](#pin) | `rec` |  |

| Constants | |
| :--- | :--- |
| [`REPOSITORY_ROOT`](#constants) | how a project records pins: a repository root records its own at dep/&lt;id>; a project in a subdirectory of a repository records them under its prefix when the enclosing repository commits one, and a project in no repository records none |
| [`SUBPROJECT`](#constants) | |

---

## Pin

```mach
pub rec Pin {
    ctx:      ptr;
    begin:    BeginFn;
    mode:     ModeFn;
    read:     ReadFn;
    read_at:  ReadAtFn;
    tracked:  TrackedFn;
    recorded: RecordedFn;
    record:   RecordFn;
    drop:     DropFn;
}
```

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `REPOSITORY_ROOT` | `u8` | `0` | how a project records pins: a repository root records its own at dep/&lt;id>; a project in a subdirectory of a repository records them under its prefix when the enclosing repository commits one, and a project in no repository records none |
| `SUBPROJECT` | `u8` | `1` |  |

