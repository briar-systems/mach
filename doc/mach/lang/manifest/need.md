# mach.lang.manifest.need

the need graph. every step and artifact is a node, and each `need` entry is
an edge from its declaration to every other declaration of a category it may
need whose name the entry selects. one walker follows the edges: it refuses a
cycle wherever it meets one, and orders what it visits after what each needs,
for validation and planning alike

## def Category

```mach
pub def Category: u8
```

a kind of declaration the graph holds

## val CATEGORY_STEP

```mach
pub val CATEGORY_STEP: Category = 0
```

a `[step.<name>]`

## val CATEGORY_ARTIFACT

```mach
pub val CATEGORY_ARTIFACT: Category = 1
```

an `[artifact.<name>]`

## rec Node

```mach
pub rec Node;
```

one declaration of the graph: entry `index` of its category's tables

## fun step_node

```mach
pub fun step_node(index: u32) Node;
```

## fun artifact_node

```mach
pub fun artifact_node(index: u32) Node;
```

## fun is_glob

```mach
pub fun is_glob(s: str) bool;
```

whether a pattern holds a wildcard: `*` matches any run of bytes, `?` any one byte

## fun glob_matches

```mach
pub fun glob_matches(pat: str, name: str) bool;
```

whether `pat` matches all of `name`, `*` matching any run of bytes and `?` any one

## fun selects

```mach
pub fun selects(itn: *intern.Interner, entry: intern.StrId, c: Category, name: intern.StrId) bool;
```

whether the `need` entry `entry` selects the declaration of category `c` named `name`

## rec Walk

```mach
pub rec Walk;
```

a walk of the need graph of one manifest

state: per node, 0 unvisited, 1 open on the walk, 2 done
trail: the edges from the walk's start to the node it is at
depth: how many of `trail` are followed
order: the nodes done, each after every node it needs

## fun walk_init

```mach
pub fun walk_init(alloc: *A.Allocator, itn: *intern.Interner, m: *Manifest) res[Walk, fail.Fail];
```

## fun walk_dnit

```mach
pub fun walk_dnit(w: *Walk);
```

## fun visit

```mach
pub fun visit(w: *Walk, n: Node) err[fail.Fail];
```

visit `n` and every declaration its `need` reaches, adding each to `order`
after what it needs; reaching a declaration again while its own visit is open
is a cycle, refused at the first edge of the cycle with the others related

## fun entry_visit

```mach
pub fun entry_visit(w: *Walk, from: Node, i: u32) err[fail.Fail];
```

visit every declaration entry `i` of `from`'s `need` selects

## fun validate

```mach
pub fun validate(alloc: *A.Allocator, itn: *intern.Interner, m: *Manifest) err[fail.Fail];
```

the rules every `need` entry holds to: it selects a category its declaration
may need, by name or pattern; it selects at least one other declaration; it
does not name its own declaration exactly; and no chain of entries cycles.
artifacts are checked before steps, and each refusal points at its entry

