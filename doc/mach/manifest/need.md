[mach](../README.md) / [manifest](README.md) / **need**

# mach.manifest.need

the need graph. every step and artifact is a node, and each `need` entry is
an edge from its declaration to every other declaration of a category it may
need whose name the entry selects. one walker follows the edges: it refuses a
cycle wherever it meets one, and orders what it visits after what each needs,
for validation and planning alike

<sub>source: [`src/manifest/need.mach`](../../../src/manifest/need.mach) · imported by 4 modules</sub>

<sub>`Category` is private · `A` is `std.allocator` · `intern` is [`mach.base.intern`](../base/intern.md) · `Edge` is private · `fail` is [`mach.base.fail`](../base/fail.md)</sub>

| Type | | |
| :--- | :--- | :--- |
| [`Node`](#node) | `rec` | one declaration of the graph: entry `index` of its category's tables |
| [`Walk`](#walk) | `rec` | a walk of the need graph of one manifest |

| On `Node` | |
| :--- | :--- |
| [`step_node`](#step_node) | |

| On `Walk` | |
| :--- | :--- |
| [`walk_init`](#walk_init) · [`walk_dnit`](#walk_dnit) | |
| [`visit`](#visit) | visit `n` and every declaration its `need` reaches, adding each to `order` after what it needs; reaching a declaration again while its own visit is open is a cycle, refused at the first edge of the cycle with the others related |

| Functions | |
| :--- | :--- |
| [`is_glob`](#is_glob) | whether a pattern holds a wildcard: `*` matches any run of bytes, `?` any one byte |
| [`glob_matches`](#glob_matches) | whether `pat` matches all of `name`, `*` matching any run of bytes and `?` any one |
| [`selects`](#selects) | whether the `need` entry `entry` selects the declaration of category `c` named `name` |
| [`validate`](#validate) | the rules every `need` entry holds to: it selects a category its declaration may need, by name or pattern; it selects at least one other declaration; it does not name its own declaration exactly; and no chain of entries cycles |

| Constants | |
| :--- | :--- |
| [`CATEGORY_STEP`](#constants) | a `[step.<name>]` |
| [`CATEGORY_ARTIFACT`](#constants) | an `[artifact.<name>]` |

---

## Node

```mach
pub rec Node {
    category: Category;
    index:    u32;
}
```

one declaration of the graph: entry `index` of its category's tables

### step_node

```mach
pub fun step_node(index: u32) Node
```

## Walk

```mach
pub rec Walk {
    alloc: *A.Allocator;
    itn:   *intern.Interner;
    m:     *Manifest;
    state: *u8;
    trail: *Edge;
    total: usize;
    depth: u32;
    order: Vector[Node];
}
```

a walk of the need graph of one manifest

| Field | Type | |
| :--- | :--- | :--- |
| `state` | `*u8` | per node, 0 unvisited, 1 open on the walk, 2 done |
| `trail` | `*Edge` | the edges from the walk's start to the node it is at |
| `depth` | `u32` | how many of `trail` are followed |
| `order` | `Vector[Node]` | the nodes done, each after every node it needs |

### walk_init

```mach
pub fun walk_init(alloc: *A.Allocator, itn: *intern.Interner, m: *Manifest) res[Walk, fail.Fail]
```

### walk_dnit

```mach
pub fun walk_dnit(w: *Walk)
```

### visit

```mach
pub fun visit(w: *Walk, n: Node) err[fail.Fail]
```

visit `n` and every declaration its `need` reaches, adding each to `order`
after what it needs; reaching a declaration again while its own visit is open
is a cycle, refused at the first edge of the cycle with the others related

## Functions

### is_glob

```mach
pub fun is_glob(s: str) bool
```

whether a pattern holds a wildcard: `*` matches any run of bytes, `?` any one byte

### glob_matches

```mach
pub fun glob_matches(pat: str, name: str) bool
```

whether `pat` matches all of `name`, `*` matching any run of bytes and `?` any one

### selects

```mach
pub fun selects(itn: *intern.Interner, entry: intern.StrId, c: Category, name: intern.StrId) bool
```

whether the `need` entry `entry` selects the declaration of category `c` named `name`

### validate

```mach
pub fun validate(alloc: *A.Allocator, itn: *intern.Interner, m: *Manifest) err[fail.Fail]
```

the rules every `need` entry holds to: it selects a category its declaration
may need, by name or pattern; it selects at least one other declaration; it
does not name its own declaration exactly; and no chain of entries cycles.
artifacts are checked before steps, and each refusal points at its entry

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `CATEGORY_STEP` | `Category` | `0` | a `[step.<name>]` |
| `CATEGORY_ARTIFACT` | `Category` | `1` | an `[artifact.<name>]` |

