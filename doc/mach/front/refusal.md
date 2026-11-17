[mach](../README.md) / [front](README.md) / **refusal**

# mach.front.refusal

a module refused by a module-scope `$error`. a module in which one fires is refused and
reports each that fired and nothing else, and a module that imports a refused module,
directly or through others, is not analysed and reports nothing

Source: [`src/front/refusal.mach`](../../../src/front/refusal.mach)

Uses types from [`mach.base.fail`](../base/fail.md).

| Type | | |
| :--- | :--- | :--- |
| [`State`](#state) | `def` |  |
| [`Refusal`](#refusal) | `rec` | why a module is refused, if it is |

| On `State` | |
| :--- | :--- |
| [`NONE`](#state) | |
| [`OWN`](#state) | a module-scope `$error` fired in the module |
| [`IMPORTED`](#state) | a module it imports is refused |

| On `Refusal` | |
| :--- | :--- |
| [`init`](#init), [`dnit`](#dnit), [`is_refused`](#is_refused) | |
| [`fired`](#fired) | a module-scope `$error` that fired at `offset` |
| [`copy`](#copy) | an owned copy of `from`, from `a` |

---

## State

```mach
pub def State: u8
```

| Name | Value | |
| :--- | :--- | :--- |
| `NONE` | `0` |  |
| `OWN` | `1` | a module-scope `$error` fired in the module |
| `IMPORTED` | `2` | a module it imports is refused |

## Refusal

```mach
pub rec Refusal {
    state: State;
    sites: std.collections.vector.Vector[u32];
}
```

why a module is refused, if it is

| Field | Type | |
| :--- | :--- | :--- |
| `state` | `State` | NONE, OWN or IMPORTED |
| `sites` | `std.collections.vector.Vector[u32]` | where each module-scope `$error` that fired begins in the module's file, for OWN |

### init

```mach
pub fun init(a: *std.allocator.Allocator) Refusal
```

### dnit

```mach
pub fun dnit(r: *Refusal)
```

### is_refused

```mach
pub fun is_refused(r: *Refusal) bool
```

### fired

```mach
pub fun fired(r: *Refusal, offset: usize) err[mach.base.fail.Fail]
```

a module-scope `$error` that fired at `offset`

### copy

```mach
pub fun copy(from: *Refusal, a: *std.allocator.Allocator) res[Refusal, mach.base.fail.Fail]
```

an owned copy of `from`, from `a`

