[mach](../README.md) / [base](README.md) / **growth**

# mach.base.growth

the compiler's one growth routine for arrays it keeps as a pointer and a
capacity beside its own count. std's Vector owns an array whole; these are
for the tables whose count and capacity are fields of a larger record,
usually a compact u32.

reserve doubles the capacity, starting at MINIMUM, until it holds what is
needed, and never past what the capacity's type can count. resize sets the
capacity exactly, growing or shrinking. a refusal leaves the array as it
was: the old storage and capacity stay valid and nothing moved. a capacity
that cannot count what is needed, or a byte size that overflows, refuses
with overflow; otherwise the refusal is the allocator's.

Source: [`src/base/growth.mach`](../../../src/base/growth.mach)

| Functions | |
| :--- | :--- |
| [`reserve`](#reserve) | grow the array at `data`, of `cap` elements, so it holds at least `needed` |
| [`reserve_usize`](#reserve_usize) | reserve for an array whose capacity is a usize |
| [`resize`](#resize) | give the array at `data`, of `cap` elements, exactly `count`; zero releases it |
| [`resize_usize`](#resize_usize) | resize for an array whose capacity is a usize |

| Constants | |
| :--- | :--- |
| [`MINIMUM`](#constants) | the first capacity reserve gives an empty array |

---

## Functions

### reserve

```mach
pub fun reserve[T](a: *std.allocator.Allocator, data: **T, cap: *u32, needed: usize) err[std.allocator.Error]
```

grow the array at `data`, of `cap` elements, so it holds at least `needed`

### reserve_usize

```mach
pub fun reserve_usize[T](a: *std.allocator.Allocator, data: **T, cap: *usize, needed: usize) err[std.allocator.Error]
```

reserve for an array whose capacity is a usize

### resize

```mach
pub fun resize[T](a: *std.allocator.Allocator, data: **T, cap: *u32, count: usize) err[std.allocator.Error]
```

give the array at `data`, of `cap` elements, exactly `count`; zero releases it

### resize_usize

```mach
pub fun resize_usize[T](a: *std.allocator.Allocator, data: **T, cap: *usize, count: usize) err[std.allocator.Error]
```

resize for an array whose capacity is a usize

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `MINIMUM` | `usize` | `8` | the first capacity reserve gives an empty array |

