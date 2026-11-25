[mach](../README.md) / [compile](README.md) / **view**

# mach.compile.view

the texts of a unit a build writes beside its object when `--emit` names them: mirl's text of
the module mach lowered and masc's listing of the object. each is a row of VIEWS, which every
layer reads, so a new view is one row, and a new case of Source where mirl writes it from a
stream of its own

Source: [`src/compile/view.mach`](../../../src/compile/view.mach)

| Type | | |
| :--- | :--- | :--- |
| [`Source`](#source) | `tag` | where a view's text is taken from |
| [`View`](#view) | `rec` | one view of a unit |
| [`Set`](#set) | `def` | a set of views, bit `i` for the view at VIEWS[i] |
| [`Streams`](#streams) | `rec` | the texts mirl writes as it compiles a unit, each where a view of it asks: masc's listing of the object |

| On `Set` | |
| :--- | :--- |
| [`NONE`](#set) | the empty set |
| [`only`](#only) | the set holding the view at VIEWS[i] alone |
| [`has`](#has) | whether `s` holds the view at VIEWS[i] |

| On `Streams` | |
| :--- | :--- |
| [`UNSTREAMED`](#streams) | no text written as mirl compiles |
| [`streams`](#streams-1) | the texts `writers` name, one a view in the order of VIEWS, as mirl's options take them |

| Functions | |
| :--- | :--- |
| [`unwritten`](#unwritten) | a path for each view, none of them written |
| [`named`](#named) | the view `name` names, none for none |
| [`streamed`](#streamed) | whether mirl writes the view at VIEWS[i] as it compiles, rather than mach before it |

| Constants | |
| :--- | :--- |
| [`COUNT`](#constants) | the views a build can write |
| [`VIEWS`](#constants) | every view, in the order `--emit` lists them |

---

## Source

```mach
pub tag Source: u8 {
    lowered;
    listing;
}
```

where a view's text is taken from

| Case | Payload | |
| :--- | :--- | :--- |
| `lowered` |  | the module mach hands mirl, which mach prints before mirl compiles it |
| `listing` |  | masc's listing of the object, which mirl writes as its back end makes it |

## View

```mach
pub rec View {
    name:   str;
    tree:   str;
    ext:    str;
    source: Source;
}
```

one view of a unit

| Field | Type | |
| :--- | :--- | :--- |
| `name` | `str` | the value `--emit` takes |
| `tree` | `str` | the tree of the work directory its files are written in, one a module, by the module's path |
| `ext` | `str` | the extension of each file |
| `source` | `Source` | where its text is taken from |

## Set

```mach
pub def Set: u8
```

a set of views, bit `i` for the view at VIEWS[i]

| Name | Value | |
| :--- | :--- | :--- |
| `NONE` | `0` | the empty set |

### only

```mach
pub fun only(i: usize) Set
```

the set holding the view at VIEWS[i] alone

### has

```mach
pub fun has(s: Set, i: usize) bool
```

whether `s` holds the view at VIEWS[i]

## Streams

```mach
pub rec Streams {
    listing: opt[*std.io.writer.Writer];
}
```

the texts mirl writes as it compiles a unit, each where a view of it asks: masc's listing of the
object

| Name | Value | |
| :--- | :--- | :--- |
| `UNSTREAMED` | `Streams{listing: opt[*io_writer.Writer].none{}}` | no text written as mirl compiles |

### streams

```mach
pub fun streams(writers: [COUNT]opt[*std.io.writer.Writer]) Streams
```

the texts `writers` name, one a view in the order of VIEWS, as mirl's options take them

## Functions

### unwritten

```mach
pub fun unwritten() [COUNT]str
```

a path for each view, none of them written

### named

```mach
pub fun named(name: str) opt[usize]
```

the view `name` names, none for none

### streamed

```mach
pub fun streamed(i: usize) bool
```

whether mirl writes the view at VIEWS[i] as it compiles, rather than mach before it

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `COUNT` | `usize` | `2` | the views a build can write |
| `VIEWS` | `[COUNT]View` | `[COUNT]View{ View{name: "ir", tree: layout.IR, ext: "ir", source: Source.lowered{}}, View{name: "asm", tree: layout.ASM, ext: "s", source: Source.listing{}}, }` | every view, in the order `--emit` lists them |

