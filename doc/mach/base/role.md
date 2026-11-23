[mach](../README.md) / [base](README.md) / **role**

# mach.base.role

which side of a build the calling thread works on, the owner or a worker, and a check that a
worker leaves the build's shared stores alone. the check is compiled in only when CHECKED is set:
each store's write path calls touched, which counts a touch made on the worker side, and report
prints the counts when a build ends. a store may also be frozen for a while, when no thread may
write it, the owner's included, and a touch of it is counted apart. a normal build compiles
neither the role nor the counts

Source: [`src/base/role.mach`](../../../src/base/role.mach)

| Functions | |
| :--- | :--- |
| [`enter`](#enter) | the calling thread starts working on the worker side |
| [`leave`](#leave) | the calling thread stops working on the worker side |
| [`worker`](#worker) | whether the calling thread works on the worker side |
| [`permit`](#permit) | the calling worker owns the `count` stores at `stores` until it names others, none for nil |
| [`freeze`](#freeze) | the `count` stores at `stores` frozen until others are named, none for nil |
| [`touched`](#touched) | the store `store` at `at` was touched; counted when the calling thread works on the worker side and does not own it, and counted apart when it is frozen |
| [`report`](#report) | every store a worker touched and how often, on stderr |

| Constants | |
| :--- | :--- |
| [`CHECKED`](#constants) | whether the shared-store check is compiled in |
| [`INTERN_ADD`](#constants) | the stores the check watches, each an index into the counts |
| [`INTERN_FIND`](#constants), [`TYPE_ADD`](#constants), [`TYPE_MEMO`](#constants), [`TYPE_SCAN`](#constants), [`TYPE_FACTS`](#constants), [`QUERY`](#constants), [`DIAGNOSTIC`](#constants), [`DEMAND`](#constants), [`IDENTITY`](#constants), [`LINKAGE`](#constants), [`EMBED`](#constants), [`SOURCES`](#constants), [`MODULES`](#constants) | |
| [`OWNED`](#constants) | the most stores a worker may own, or the owner may freeze, at once |

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
pub fun permit(stores: *ptr, count: usize)
```

the calling worker owns the `count` stores at `stores` until it names others, none for nil

### freeze

```mach
pub fun freeze(stores: *ptr, count: usize)
```

the `count` stores at `stores` frozen until others are named, none for nil. called by the owner
while no worker runs

### touched

```mach
pub fun touched(store: usize, at: ptr)
```

the store `store` at `at` was touched; counted when the calling thread works on the worker side
and does not own it, and counted apart when it is frozen. `at` is nil for a store only ever shared

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
| `OWNED` | `usize` | `8` | the most stores a worker may own, or the owner may freeze, at once |

