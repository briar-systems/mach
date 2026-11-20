[mach](../README.md) / [base](README.md) / **checked**

# mach.base.checked

checked arithmetic refusing overflow with a cause

Source: [`src/base/checked.mach`](../../../src/base/checked.mach)

| Type | | |
| :--- | :--- | :--- |
| [`Cause`](#cause) | `def` |  |

| On `Cause` | |
| :--- | :--- |
| [`ADD_OVERFLOW`](#cause) | |
| [`usize_add`](#usize_add) | the sum of two sizes, refused when it does not fit a usize |

---

## Cause

```mach
pub def Cause: u8
```

| Name | Value | |
| :--- | :--- | :--- |
| `ADD_OVERFLOW` | `1` |  |

### usize_add

```mach
pub fun usize_add(left: usize, right: usize) res[usize, Cause]
```

the sum of two sizes, refused when it does not fit a usize

| Parameter | |
| :--- | :--- |
| `left` | the first operand |
| `right` | the second operand |
| **returns** | the sum, or ADD_OVERFLOW when it would wrap |

