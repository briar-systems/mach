[mach](../README.md) / [package](README.md) / **pin**

# mach.package.pin

the pin contract: where a project records the exact revision each dependency is held at,
and how it reads one back. git is the member today: a pin is the gitlink the enclosing
repository commits at dep/\<id>, registered in .gitmodules. a member recording pins
elsewhere, beside a source served over http, fills the same table

Source: [`src/package/pin.mach`](../../../src/package/pin.mach)

| Type | | |
| :--- | :--- | :--- |
| [`Pin`](#pin) | `rec` | a member of the pin contract: its context and the operations it fills |

| Constants | |
| :--- | :--- |
| [`REPOSITORY_ROOT`](#constants) | how a project records pins: a repository root records its own at dep/\<id>; a project in a subdirectory of a repository records them under its prefix when the enclosing repository commits one, and a project in no repository records none |
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

a member of the pin contract: its context and the operations it fills

| Field | Type | |
| :--- | :--- | :--- |
| `ctx` | `ptr` | the member's state, passed to every operation |
| `begin` | `BeginFn` | starts recording pins at a root |
| `mode` | `ModeFn` | how the project at a root records pins |
| `read` | `ReadFn` | the pin a root records for an id |
| `read_at` | `ReadAtFn` | the pin a dependency's project records for an id at a revision |
| `tracked` | `TrackedFn` | whether a directory is in a place that records pins |
| `recorded` | `RecordedFn` | whether a root records a pin for an id |
| `record` | `RecordFn` | records what dep/\<id> holds as its pin |
| `drop` | `DropFn` | stops recording a pin for an id |

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `REPOSITORY_ROOT` | `u8` | `0` | how a project records pins: a repository root records its own at dep/\<id>; a project in a subdirectory of a repository records them under its prefix when the enclosing repository commits one, and a project in no repository records none |
| `SUBPROJECT` | `u8` | `1` |  |

