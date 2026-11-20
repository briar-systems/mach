[mach](../README.md) / [base](README.md) / **role**

# mach.base.role

which side of a build the calling thread works on, the owner or a worker, and a check that a
worker leaves the build's shared stores alone. the check is compiled in only when CHECKED is set:
each store's write path calls touched, which counts a touch made on the worker side, and report
prints the counts when a build ends. a normal build compiles neither the role nor the counts

Source: [`src/base/role.mach`](../../../src/base/role.mach)

| Functions | |
| :--- | :--- |
| [`enter`](#enter) | the calling thread starts working on the worker side |
| [`leave`](#leave) | the calling thread stops working on the worker side |
| [`worker`](#worker) | whether the calling thread works on the worker side |
| [`permit`](#permit) | the calling worker owns the stores at `first` and `second` until it names others, or nil |
| [`touched`](#touched) | the store `store` at `at` was touched; counted when the calling thread works on the worker side and does not own it |
| [`report`](#report) | every store a worker touched and how often, on stderr |

| Constants | |
| :--- | :--- |
| [`CHECKED`](#constants) | whether the shared-store check is compiled in |
| [`INTERN_ADD`](#constants) | the stores the check watches, each an index into the counts |
| [`INTERN_FIND`](#constants), [`TYPE_ADD`](#constants), [`TYPE_MEMO`](#constants), [`TYPE_SCAN`](#constants), [`TYPE_FACTS`](#constants), [`QUERY`](#constants), [`DIAGNOSTIC`](#constants), [`DEMAND`](#constants), [`IDENTITY`](#constants), [`LINKAGE`](#constants), [`EMBED`](#constants), [`SOURCES`](#constants), [`MODULES`](#constants) | |

---

## Functions

### enter

```mach
pub fun enter()
```

the calling thread starts working on the worker side

### leave

```mach
pub fun leave()
```

the calling thread stops working on the worker side

### worker

```mach
pub fun worker() bool
```

whether the calling thread works on the worker side

### permit

```mach
pub fun permit(first: ptr, second: ptr)
```

the calling worker owns the stores at `first` and `second` until it names others, or nil

### touched

```mach
pub fun touched(store: usize, at: ptr)
```

the store `store` at `at` was touched; counted when the calling thread works on the worker side
and does not own it. `at` is nil for a store only ever shared

### report

```mach
pub fun report()
```

every store a worker touched and how often, on stderr

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `CHECKED` | `bool` | `false` | whether the shared-store check is compiled in |
| `INTERN_ADD` | `usize` | `0` | the stores the check watches, each an index into the counts |
| `INTERN_FIND` | `usize` | `1` |  |
| `TYPE_ADD` | `usize` | `2` |  |
| `TYPE_MEMO` | `usize` | `3` |  |
| `TYPE_SCAN` | `usize` | `4` |  |
| `TYPE_FACTS` | `usize` | `5` |  |
| `QUERY` | `usize` | `6` |  |
| `DIAGNOSTIC` | `usize` | `7` |  |
| `DEMAND` | `usize` | `8` |  |
| `IDENTITY` | `usize` | `9` |  |
| `LINKAGE` | `usize` | `10` |  |
| `EMBED` | `usize` | `11` |  |
| `SOURCES` | `usize` | `12` |  |
| `MODULES` | `usize` | `13` |  |

