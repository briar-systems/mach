[mach](../../README.md) / [tool](../README.md) / [doc](README.md) / **model**

# mach.tool.doc.model

the document model `mach doc` renders: one project, its directories, its modules
and the entries each module exports

the model is the contract between the builder (`mach.tool.doc.build`) and a
renderer (`mach.tool.doc.markdown`). it holds what the source files declare and how
the declarations relate, read from their syntax trees alone, and nothing of how a
page shows it: headings, anchors, links and escaping are a renderer's. modules are ordered by path and directories by path, the project's
root directory first; entries are in source order. every string and vector lives
in the arena the model was built in

Source: [`src/tool/doc/model.mach`](../../../../src/tool/doc/model.mach)

| Type | | |
| :--- | :--- | :--- |
| [`Kind`](#kind) | `def` | what an entry declares |
| [`Component`](#component) | `rec` | one component line of a doc comment: the name as written (`x`, `$order`, `[T]`, `ret`) and its description |
| [`Doc`](#doc) | `rec` | a doc comment without its comment marks: its summary, the first sentence of its first paragraph as `mach.front.doc.summary_len` reads it, every paragraph of its prose, the first included, and its components; all empty when there is none |
| [`Clause`](#clause) | `rec` | one `$if` or `$or` arm a declaration sits in: its condition as written, "" for a final bare `$or`, and the conditions of the arms before it in its chain, none of which holds where it applies |
| [`Piece`](#piece) | `rec` | one run of source text: as written, or, where `path` is not "", an import alias the source spells as `text` that names the module at `path` |
| [`Text`](#text) | `rec` | source text in pieces, every import alias in it marked with what it names; the layout (line breaks, alignment) is the source's |
| [`Arm`](#arm) | `rec` | one declaration of an entry |
| [`Element`](#element) | `rec` | an element a component line can document: a parameter (`$name` when it is comptime), a generic (`[T]`), a field or a case, with its type, empty for a generic or a case with no payload; a binding's one element is itself, with its declared type |
| [`Provider`](#provider) | `rec` | one module a `fwd` takes its declaration from: the arms that select it (the forward's own and those of the `use` that binds its alias, outermost first), the dotted path it names, the module of this project that path lies in or NONE, and the entry there it names, NONE for a whole module or a name that module does not document |
| [`Entry`](#entry) | `rec` | one public name of a module |
| [`Group`](#group) | `rec` | the entries shown together: a type with the functions and constants that belong to it (owner is the type's entry), or the entries of one kind that belong to no type (owner NONE, kind theirs); members in source order |
| [`Module`](#module) | `rec` | one module of the project, one source file under its source root |
| [`Dir`](#dir) | `rec` | one directory of the project's source tree |
| [`Artifact`](#artifact) | `rec` | one artifact of the manifest: its kind as the manifest spells it, the path of its entry module and that module's index, NONE when the project has no such file |
| [`Project`](#project) | `rec` | the whole project: its id and version, its artifacts in manifest order, the ids of its dependencies, every module its source root holds, and every directory holding one |

| On `Kind` | |
| :--- | :--- |
| [`KIND_FUN`](#kind), [`KIND_REC`](#kind), [`KIND_UNI`](#kind), [`KIND_TAG`](#kind), [`KIND_DEF`](#kind), [`KIND_VAL`](#kind), [`KIND_VAR`](#kind), [`KIND_FWD`](#kind) | |
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

## Piece

```mach
pub rec Piece {
    text: str;
    path: str;
}
```

one run of source text: as written, or, where `path` is not "", an import alias the
source spells as `text` that names the module at `path`

## Text

```mach
pub rec Text {
    pieces: Vector[Piece];
}
```

source text in pieces, every import alias in it marked with what it names; the
layout (line breaks, alignment) is the source's

## Arm

```mach
pub rec Arm {
    clauses:    Vector[Clause];
    decorators: Vector[str];
    signature:  Text;
    value:      str;
}
```

one declaration of an entry

| Field | Type | |
| :--- | :--- | :--- |
| `clauses` | `Vector[Clause]` | the arms it sits in, outermost first; none at the top level |
| `decorators` | `Vector[str]` | each `#[...]` as written but `#[deprecated]`, which is the entry's deprecation |
| `signature` | `Text` | the declaration without its decorators: a function without its body, a record, union or tag with its fields or cases, a `def` with its base, a binding without its value, a `fwd` with its path |
| `value` | `str` | a binding's initializer as written, "" for every other kind |

## Element

```mach
pub rec Element {
    name: str;
    ty:   Text;
}
```

an element a component line can document: a parameter (`$name` when it is comptime), a
generic (`[T]`), a field or a case, with its type, empty for a generic or a case with no
payload; a binding's one element is itself, with its declared type

## Provider

```mach
pub rec Provider {
    clauses: Vector[Clause];
    target:  str;
    module:  u32;
    entry:   u32;
}
```

one module a `fwd` takes its declaration from: the arms that select it (the forward's
own and those of the `use` that binds its alias, outermost first), the dotted path it
names, the module of this project that path lies in or NONE, and the entry there it
names, NONE for a whole module or a name that module does not document

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
    providers:   Vector[Provider];
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
| `providers` | `Vector[Provider]` | for a `fwd`, every module it may take its declaration from, one per module its alias is bound to in some arm |
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
    path:     str;
    rel:      str;
    leaf:     str;
    file:     str;
    dir:      u32;
    declares: bool;
    doc:      Doc;
    entries:  Vector[Entry];
    groups:   Vector[Group];
    mentions: Vector[u32];
}
```

one module of the project, one source file under its source root

| Field | Type | |
| :--- | :--- | :--- |
| `path` | `str` | its dotted path, `mach.base.intern` |
| `rel` | `str` | its path below the project, `base/intern` |
| `leaf` | `str` | its last segment |
| `file` | `str` | its source file relative to the project root |
| `dir` | `u32` | the directory holding it |
| `declares` | `bool` | it declares anything but `use` lines; a module that does not has no page, and its docstring describes the directory of its name |
| `groups` | `Vector[Group]` | every type first, in declaration order, then what belongs to no type: functions, constants, variables, forwards |
| `mentions` | `Vector[u32]` | the modules of this project its entries' signatures name, ascending |

## Dir

```mach
pub rec Dir {
    rel:     str;
    leaf:    str;
    parent:  u32;
    root:    u32;
    modules: Vector[u32];
    dirs:    Vector[u32];
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
module and that module's index, NONE when the project has no such file

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
dependencies, every module its source root holds, and every directory holding one

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `NONE` | `u32` | `0xFFFFFFFF` | an absent index |

