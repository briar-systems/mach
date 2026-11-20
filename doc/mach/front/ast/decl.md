[mach](../../README.md) / [front](../README.md) / [ast](README.md) / **decl**

# mach.front.ast.decl

the declaration nodes of the syntax tree, with typed names, tag cases and decorators

Source: [`src/front/ast/decl.mach`](../../../../src/front/ast/decl.mach)

Uses types from [`mach.base.source`](../../base/source.md), [`mach.front.ast.id`](id.md).

| Type | | |
| :--- | :--- | :--- |
| [`DeclKind`](#declkind) | `def` |  |
| [`TypedName`](#typedname) | `rec` | a name with its type syntax: one parameter or one field |
| [`DeclUse`](#decluse) | `rec` | a `use` import |
| [`DeclFwd`](#declfwd) | `rec` | a `fwd` re-export |
| [`DeclFun`](#declfun) | `rec` | a `fun` or `ext fun` declaration |
| [`DeclRec`](#declrec) | `rec` | a `rec` declaration |
| [`DeclUni`](#decluni) | `rec` | a `uni` declaration |
| [`TagCase`](#tagcase) | `rec` | one case of a `tag` |
| [`DeclTag`](#decltag) | `rec` | a `tag` declaration |
| [`DeclBind`](#declbind) | `rec` | a `val` or `var` binding, top-level or local |
| [`DeclDef`](#decldef) | `rec` | a `def` type alias |
| [`DeclTest`](#decltest) | `rec` | a `test` declaration |
| [`ComptimeBranch`](#comptimebranch) | `rec` | one arm of a `$if` chain, shared by declaration and statement chains |
| [`DeclComptimeIf`](#declcomptimeif) | `rec` | a top-level `$if` chain |
| [`DeclComptimeDirective`](#declcomptimedirective) | `rec` | a top-level comptime directive statement such as `$error(...)` |
| [`Decorator`](#decorator) | `rec` | `#[name(args)]`, all of which `span` covers; `id` is the decorator registry's id the name spells, decoded once by the parser, and 0 when it spells none |
| [`Decl`](#decl) | `rec` | one declaration node |

| On `DeclKind` | |
| :--- | :--- |
| [`DECL_KIND_USE`](#declkind), [`DECL_KIND_FWD`](#declkind), [`DECL_KIND_FUN`](#declkind), [`DECL_KIND_REC`](#declkind), [`DECL_KIND_VAL`](#declkind), [`DECL_KIND_VAR`](#declkind), [`DECL_KIND_DEF`](#declkind), [`DECL_KIND_TEST`](#declkind), [`DECL_KIND_COMPTIME_IF`](#declkind), [`DECL_KIND_COMPTIME_DIRECTIVE`](#declkind), [`DECL_KIND_UNI`](#declkind), [`DECL_KIND_TAG`](#declkind), [`DECL_KIND_ERROR`](#declkind) | |

| Constants | |
| :--- | :--- |
| [`DECL_FLAG_PUB`](#constants), [`DECL_FLAG_EXT`](#constants) | |

---

## DeclKind

```mach
pub def DeclKind: u8
```

| Name | Value | |
| :--- | :--- | :--- |
| `DECL_KIND_USE` | `0` |  |
| `DECL_KIND_FWD` | `1` |  |
| `DECL_KIND_FUN` | `2` |  |
| `DECL_KIND_REC` | `3` |  |
| `DECL_KIND_VAL` | `4` |  |
| `DECL_KIND_VAR` | `5` |  |
| `DECL_KIND_DEF` | `6` |  |
| `DECL_KIND_TEST` | `7` |  |
| `DECL_KIND_COMPTIME_IF` | `8` |  |
| `DECL_KIND_COMPTIME_DIRECTIVE` | `9` |  |
| `DECL_KIND_UNI` | `10` |  |
| `DECL_KIND_TAG` | `11` |  |
| `DECL_KIND_ERROR` | `255` |  |

## TypedName

```mach
pub rec TypedName {
    name:     mach.base.source.Span;
    ty:       mach.front.ast.id.TypeId;
    comptime: bool;
}
```

a name with its type syntax: one parameter or one field

| Field | Type | |
| :--- | :--- | :--- |
| `name` | `mach.base.source.Span` | the span of the name |
| `ty` | `mach.front.ast.id.TypeId` | its type syntax |
| `comptime` | `bool` | whether a parameter is written `$name`, so its argument is a comptime value |

## DeclUse

```mach
pub rec DeclUse {
    alias: mach.base.source.Span;
    path:  mach.base.source.Span;
}
```

a `use` import

| Field | Type | |
| :--- | :--- | :--- |
| `alias` | `mach.base.source.Span` | the span of the written alias, empty when none is written |
| `path` | `mach.base.source.Span` | the span of the imported path |

## DeclFwd

```mach
pub rec DeclFwd {
    alias: mach.base.source.Span;
    path:  mach.base.source.Span;
}
```

a `fwd` re-export

| Field | Type | |
| :--- | :--- | :--- |
| `alias` | `mach.base.source.Span` | the span of the written alias, empty when none is written |
| `path` | `mach.base.source.Span` | the span of the re-exported path |

## DeclFun

```mach
pub rec DeclFun {
    name:           mach.base.source.Span;
    generics_start: u32;
    generics_len:   u32;
    params_start:   u32;
    params_len:     u32;
    return_type:    mach.front.ast.id.TypeId;
    body:           mach.front.ast.id.StmtId;
    c_variadic:     bool;
}
```

a `fun` or `ext fun` declaration

| Field | Type | |
| :--- | :--- | :--- |
| `name` | `mach.base.source.Span` | the span of the function name |
| `generics_start` | `u32` | first generic parameter in the generic name table |
| `generics_len` | `u32` | how many generic parameters |
| `params_start` | `u32` | first parameter in the typed name table |
| `params_len` | `u32` | how many parameters |
| `return_type` | `mach.front.ast.id.TypeId` | the return type syntax, TYPE_NIL when none is written |
| `body` | `mach.front.ast.id.StmtId` | the body block, STMT_NIL for a declaration with no body |
| `c_variadic` | `bool` | whether an `ext fun` ends in a C `...` |

## DeclRec

```mach
pub rec DeclRec {
    name:           mach.base.source.Span;
    generics_start: u32;
    generics_len:   u32;
    fields_start:   u32;
    fields_len:     u32;
}
```

a `rec` declaration

| Field | Type | |
| :--- | :--- | :--- |
| `name` | `mach.base.source.Span` | the span of the record name |
| `generics_start` | `u32` | first generic parameter in the generic name table |
| `generics_len` | `u32` | how many generic parameters |
| `fields_start` | `u32` | first field in the typed name table |
| `fields_len` | `u32` | how many fields |

## DeclUni

```mach
pub rec DeclUni {
    name:           mach.base.source.Span;
    generics_start: u32;
    generics_len:   u32;
    fields_start:   u32;
    fields_len:     u32;
}
```

a `uni` declaration

| Field | Type | |
| :--- | :--- | :--- |
| `name` | `mach.base.source.Span` | the span of the union name |
| `generics_start` | `u32` | first generic parameter in the generic name table |
| `generics_len` | `u32` | how many generic parameters |
| `fields_start` | `u32` | first field in the typed name table |
| `fields_len` | `u32` | how many fields |

## TagCase

```mach
pub rec TagCase {
    name:             mach.base.source.Span;
    payload:          mach.front.ast.id.TypeId;
    decorators_start: u32;
    decorators_len:   u32;
}
```

one case of a `tag`

| Field | Type | |
| :--- | :--- | :--- |
| `name` | `mach.base.source.Span` | the span of the case name |
| `payload` | `mach.front.ast.id.TypeId` | the payload type syntax, TYPE_NIL for a case with none |
| `decorators_start` | `u32` | first decorator on the case in the decorator table |
| `decorators_len` | `u32` | how many decorators |

## DeclTag

```mach
pub rec DeclTag {
    name:           mach.base.source.Span;
    generics_start: u32;
    generics_len:   u32;
    cases_start:    u32;
    cases_len:      u32;
    disc:           mach.front.ast.id.TypeId;
}
```

a `tag` declaration

| Field | Type | |
| :--- | :--- | :--- |
| `name` | `mach.base.source.Span` | the span of the tag name |
| `generics_start` | `u32` | first generic parameter in the generic name table |
| `generics_len` | `u32` | how many generic parameters |
| `cases_start` | `u32` | first case in the tag case table |
| `cases_len` | `u32` | how many cases |
| `disc` | `mach.front.ast.id.TypeId` | the discriminator type syntax |

## DeclBind

```mach
pub rec DeclBind {
    name: mach.base.source.Span;
    ty:   mach.front.ast.id.TypeId;
    init: mach.front.ast.id.ExprId;
}
```

a `val` or `var` binding, top-level or local

| Field | Type | |
| :--- | :--- | :--- |
| `name` | `mach.base.source.Span` | the span of the bound name |
| `ty` | `mach.front.ast.id.TypeId` | its type syntax |
| `init` | `mach.front.ast.id.ExprId` | the initializer, EXPR_NIL when none is written |

## DeclDef

```mach
pub rec DeclDef {
    name: mach.base.source.Span;
    ty:   mach.front.ast.id.TypeId;
}
```

a `def` type alias

| Field | Type | |
| :--- | :--- | :--- |
| `name` | `mach.base.source.Span` | the span of the alias name |
| `ty` | `mach.front.ast.id.TypeId` | the aliased type syntax |

## DeclTest

```mach
pub rec DeclTest {
    name: mach.base.source.Span;
    body: mach.front.ast.id.StmtId;
}
```

a `test` declaration

| Field | Type | |
| :--- | :--- | :--- |
| `name` | `mach.base.source.Span` | the span of the test name |
| `body` | `mach.front.ast.id.StmtId` | the body block |

## ComptimeBranch

```mach
pub rec ComptimeBranch {
    cond:       mach.front.ast.id.ExprId;
    body_start: u32;
    body_len:   u32;
}
```

one arm of a `$if` chain, shared by declaration and statement chains

| Field | Type | |
| :--- | :--- | :--- |
| `cond` | `mach.front.ast.id.ExprId` | the condition, EXPR_NIL for a bare `$or` |
| `body_start` | `u32` | first body item, in the declaration id table for a declaration chain and the statement id table for a statement chain |
| `body_len` | `u32` | how many body items |

## DeclComptimeIf

```mach
pub rec DeclComptimeIf {
    branches_start: u32;
    branches_len:   u32;
}
```

a top-level `$if` chain

| Field | Type | |
| :--- | :--- | :--- |
| `branches_start` | `u32` | first arm in the comptime branch table |
| `branches_len` | `u32` | how many arms |

## DeclComptimeDirective

```mach
pub rec DeclComptimeDirective {
    target: mach.front.ast.id.ExprId;
}
```

a top-level comptime directive statement such as `$error(...)`

| Field | Type | |
| :--- | :--- | :--- |
| `target` | `mach.front.ast.id.ExprId` | the directive expression |

## Decorator

```mach
pub rec Decorator {
    span:       mach.base.source.Span;
    name:       mach.base.source.Span;
    id:         u8;
    args_start: u32;
    args_len:   u32;
}
```

`#[name(args)]`, all of which `span` covers; `id` is the decorator registry's id the
name spells, decoded once by the parser, and 0 when it spells none

## Decl

```mach
pub rec Decl {
    span:             mach.base.source.Span;
    kind:             DeclKind;
    flags:            u8;
    doc:              mach.base.source.Span;
    decorators_start: u32;
    decorators_len:   u32;
    data:             uni {
        use_: DeclUse;
        fwd_: DeclFwd;
        fun_: DeclFun;
        rec_: DeclRec;
        uni_: DeclUni;
        tag_: DeclTag;
        bind: DeclBind;
        def_: DeclDef;
        test_: DeclTest;
        comptime_if: DeclComptimeIf;
        comptime_directive: DeclComptimeDirective;
    };
}
```

one declaration node

| Field | Type | |
| :--- | :--- | :--- |
| `span` | `mach.base.source.Span` | the whole declaration |
| `kind` | `DeclKind` | which DECL_KIND_ the declaration is, which names the active `data` field |
| `flags` | `u8` | DECL_FLAG_PUB and DECL_FLAG_EXT bits |
| `doc` | `mach.base.source.Span` | the docstring above the declaration, empty when there is none |
| `decorators_start` | `u32` | first decorator in the decorator table |
| `decorators_len` | `u32` | how many decorators |
| `data` | `uni { use_: DeclUse; fwd_: DeclFwd; fun_: DeclFun; rec_: DeclRec; uni_: DeclUni; tag_: DeclTag; bind: DeclBind; def_: DeclDef; test_: DeclTest; comptime_if: DeclComptimeIf; comptime_directive: DeclComptimeDirective; }` | the payload of the declaration's kind |

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `DECL_FLAG_PUB` | `u8` | `1` |  |
| `DECL_FLAG_EXT` | `u8` | `2` |  |

