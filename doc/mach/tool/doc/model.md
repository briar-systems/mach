[mach](../../README.md) / [tool](../README.md) / [doc](README.md) / **model**

# mach.tool.doc.model

the document model `mach doc` renders: one project, its directories, its modules
and the entries each module exports

the model is the contract between the builder (`mach.tool.doc.build`) and a
renderer (`mach.tool.doc.markdown`). it holds what the source declares and how
the declarations relate, as text taken verbatim from the source's syntax tree, and
nothing of how a page shows it: headings, anchors, links and escaping are a
renderer's. modules are ordered by path and directories by path, the project's
root directory first; entries are in source order. every string and vector lives
in the arena the model was built in

<sub>source: [`src/tool/doc/model.mach`](../../../../src/tool/doc/model.mach) · imported by 3 modules</sub>

| Type | | |
| :--- | :--- | :--- |
| [`Kind`](#kind) | `def` | what an entry declares |
| [`Component`](#component) | `rec` | one component line of a doc comment: the name as written (`x`, `$order`, `[T]`, `ret`) and its description |
| [`Doc`](#doc) | `rec` | a doc comment without its comment marks: its summary, the first sentence of its first paragraph as `mach.front.doc.summary_len` reads it, every paragraph of its prose, the first included, and its components; all empty when there is none |
| [`Clause`](#clause) | `rec` | one `$if` or `$or` arm a declaration sits in: its condition as written, "" for a final bare `$or`, and the conditions of the arms before it in its chain, none of which holds where it applies |
| [`Arm`](#arm) | `rec` | one declaration of an entry |
| [`Element`](#element) | `rec` | an element a component line can document: a parameter (`$name` when it is comptime), a generic (`[T]`), a field or a case, with its type as written, "" for a generic or a case with no payload; a binding's one element is itself, with its declared type |
| [`Alias`](#alias) | `rec` | a name an entry's types use that the reader cannot look up on the page: an import alias with the module path it names and that module's index when the project has it, or a type this module keeps private (target "", module NONE) |
| [`Forward`](#forward) | `rec` | what a `fwd` forwards, as name resolution decided it for the target analyzed |
| [`Entry`](#entry) | `rec` | one public name of a module |
| [`Group`](#group) | `rec` | the entries shown together: a type with the functions and constants that belong to it (owner is the type's entry), or the entries of one kind that belong to no type (owner NONE, kind theirs); members in source order |
| [`Module`](#module) | `rec` | one module of the project |
| [`Dir`](#dir) | `rec` | one directory of the project's source tree |
| [`Artifact`](#artifact) | `rec` | one artifact of the manifest: its kind as the manifest spells it, the path of its entry module and that module's index, NONE when it is not documented |
| [`Project`](#project) | `rec` | the whole project: its id and version, its artifacts in manifest order, the ids of its dependencies, every module some artifact reaches, and every directory holding one |

| On `Kind` | |
| :--- | :--- |
| [`KIND_FUN`](#kind) · [`KIND_REC`](#kind) · [`KIND_UNI`](#kind) · [`KIND_TAG`](#kind) · [`KIND_DEF`](#kind) · [`KIND_VAL`](#kind) · [`KIND_VAR`](#kind) · [`KIND_FWD`](#kind) | |
| [`is_type`](#is_type) | whether an entry of kind `k` is a type |

| Constants | |
| :--- | :--- |
| [`NONE`](#constants) | an absent index |

---

## Kind

```mach
pub def Kind: u8
```

what an entry declares

| Name | Value | |
| :--- | :--- | :--- |
| `KIND_FUN` | `0` |  |
| `KIND_REC` | `1` |  |
| `KIND_UNI` | `2` |  |
| `KIND_TAG` | `3` |  |
| `KIND_DEF` | `4` |  |
| `KIND_VAL` | `5` |  |
| `KIND_VAR` | `6` |  |
| `KIND_FWD` | `7` |  |

### is_type

```mach
pub fun is_type(k: Kind) bool
```

whether an entry of kind `k` is a type

## Component

```mach
pub rec Component {
    name: str;
    text: str;
}
```

one component line of a doc comment: the name as written (`x`, `$order`, `[T]`, `ret`)
and its description

## Doc

```mach
pub rec Doc {
    summary:    str;
    paragraphs: Vector[str];
    components: Vector[Component];
}
```

a doc comment without its comment marks: its summary, the first sentence of its first
paragraph as `mach.front.doc.summary_len` reads it, every paragraph of its prose, the
first included, and its components; all empty when there is none

## Clause

```mach
pub rec Clause {
    text:  str;
    prior: Vector[str];
}
```

one `$if` or `$or` arm a declaration sits in: its condition as written, "" for a
final bare `$or`, and the conditions of the arms before it in its chain, none of
which holds where it applies

## Arm

```mach
pub rec Arm {
    clauses:    Vector[Clause];
    decorators: Vector[str];
    signature:  str;
    value:      str;
}
```

one declaration of an entry

| Field | Type | |
| :--- | :--- | :--- |
| `clauses` | `Vector[Clause]` | the arms it sits in, outermost first; none at the top level |
| `decorators` | `Vector[str]` | each `#[...]` as written but `#[deprecated]`, which is the entry's deprecation |
| `signature` | `str` | the declaration as written without its decorators: a function without its body, a record, union or tag with its fields or cases, a `def` with its base, a binding without its value, a `fwd` with its path |
| `value` | `str` | a binding's initializer as written, "" for every other kind |

## Element

```mach
pub rec Element {
    name: str;
    ty:   str;
}
```

an element a component line can document: a parameter (`$name` when it is comptime), a
generic (`[T]`), a field or a case, with its type as written, "" for a generic or a
case with no payload; a binding's one element is itself, with its declared type

## Alias

```mach
pub rec Alias {
    name:    str;
    target:  str;
    module:  u32;
    private: bool;
}
```

a name an entry's types use that the reader cannot look up on the page: an import
alias with the module path it names and that module's index when the project has it,
or a type this module keeps private (target "", module NONE)

## Forward

```mach
pub rec Forward {
    resolved: bool;
    whole:    bool;
    target:   str;
    module:   u32;
    entry:    u32;
}
```

what a `fwd` forwards, as name resolution decided it for the target analyzed

| Field | Type | |
| :--- | :--- | :--- |
| `resolved` | `bool` | resolution decided the target; false when the arms that declare it were not selected, and every other field is then empty |
| `whole` | `bool` | it forwards a whole module |
| `target` | `str` | the target's module path, then `.name` unless it is a whole module |
| `module` | `u32` | the target's module when the project has it, else NONE |
| `entry` | `u32` | the target's entry in that module, NONE for a whole module or one not there |

## Entry

```mach
pub rec Entry {
    name:        str;
    kind:        Kind;
    external:    bool;
    doc:         Doc;
    deprecated:  bool;
    deprecation: str;
    arms:        Vector[Arm];
    elements:    Vector[Element];
    aliases:     Vector[Alias];
    forward:     Forward;
    owner:       u32;
}
```

one public name of a module

| Field | Type | |
| :--- | :--- | :--- |
| `external` | `bool` | an `ext fun` |
| `doc` | `Doc` | its own doc comment, or else that of an arm declaring it, or else that of the `$if` around it |
| `deprecation` | `str` | the message of `#[deprecated]`, "" without one; deprecated says it is there |
| `arms` | `Vector[Arm]` | one per declaration, in source order: a name several `$if` arms declare is one entry |
| `elements` | `Vector[Element]` | what its first declaration's component lines can name |
| `aliases` | `Vector[Alias]` | what its declarations' types name through an import or privately, in order of first use |
| `owner` | `u32` | the type entry it is grouped under, NONE for none |

## Group

```mach
pub rec Group {
    owner:   u32;
    kind:    Kind;
    members: Vector[u32];
}
```

the entries shown together: a type with the functions and constants that belong to it
(owner is the type's entry), or the entries of one kind that belong to no type (owner
NONE, kind theirs); members in source order

## Module

```mach
pub rec Module {
    path:      str;
    rel:       str;
    leaf:      str;
    file:      str;
    dir:       u32;
    doc:       Doc;
    entries:   Vector[Entry];
    groups:    Vector[Group];
    imports:   Vector[u32];
    importers: Vector[u32];
}
```

one module of the project

| Field | Type | |
| :--- | :--- | :--- |
| `path` | `str` | its dotted path, `mach.base.intern` |
| `rel` | `str` | its path below the project, `base/intern` |
| `leaf` | `str` | its last segment |
| `file` | `str` | its source file relative to the project root |
| `dir` | `u32` | the directory holding it |
| `groups` | `Vector[Group]` | every type first, in declaration order, then what belongs to no type: functions, constants, variables, forwards |
| `imports` | `Vector[u32]` | the project's modules it imports, by index, ascending |
| `importers` | `Vector[u32]` | the project's modules importing it, ascending |

## Dir

```mach
pub rec Dir {
    rel:       str;
    leaf:      str;
    parent:    u32;
    root:      u32;
    modules:   Vector[u32];
    dirs:      Vector[u32];
    total:     u32;
    imports:   Vector[u32];
    importers: Vector[u32];
}
```

one directory of the project's source tree

| Field | Type | |
| :--- | :--- | :--- |
| `rel` | `str` | its path below the project, "" for the root |
| `leaf` | `str` | its last segment |
| `parent` | `u32` | the directory holding it, NONE for the root |
| `root` | `u32` | its root module, the file beside it of the same name, NONE when it has none |
| `modules` | `Vector[u32]` | the modules directly in it |
| `dirs` | `Vector[u32]` | the directories directly in it |
| `total` | `u32` | the modules in it and below it |
| `imports` | `Vector[u32]` | the other top-level directories its modules import, by index |
| `importers` | `Vector[u32]` | the other top-level directories whose modules import its modules |

## Artifact

```mach
pub rec Artifact {
    name:   str;
    kind:   str;
    entry:  str;
    module: u32;
}
```

one artifact of the manifest: its kind as the manifest spells it, the path of its entry
module and that module's index, NONE when it is not documented

## Project

```mach
pub rec Project {
    id:        str;
    version:   str;
    artifacts: Vector[Artifact];
    deps:      Vector[str];
    modules:   Vector[Module];
    dirs:      Vector[Dir];
}
```

the whole project: its id and version, its artifacts in manifest order, the ids of its
dependencies, every module some artifact reaches, and every directory holding one

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `NONE` | `u32` | `0xFFFFFFFF` | an absent index |

