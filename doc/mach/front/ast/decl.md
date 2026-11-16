[mach](../../README.md) / [front](../README.md) / [ast](README.md) / **decl**

# mach.front.ast.decl

the declaration nodes of the syntax tree, with typed names, tag cases and decorators

<sub>source: [`src/front/ast/decl.mach`](../../../../src/front/ast/decl.mach) · imported by 38 modules</sub>

<sub>`base_source` is [`mach.base.source`](../../base/source.md) · `ast_id` is [`mach.front.ast.id`](id.md)</sub>

| Type | | |
| :--- | :--- | :--- |
| [`DeclKind`](#declkind) | `def` |  |
| [`TypedName`](#typedname) | `rec` |  |
| [`DeclUse`](#decluse) | `rec` |  |
| [`DeclFwd`](#declfwd) | `rec` |  |
| [`DeclFun`](#declfun) | `rec` |  |
| [`DeclRec`](#declrec) | `rec` |  |
| [`DeclUni`](#decluni) | `rec` |  |
| [`TagCase`](#tagcase) | `rec` |  |
| [`DeclTag`](#decltag) | `rec` |  |
| [`DeclBind`](#declbind) | `rec` |  |
| [`DeclDef`](#decldef) | `rec` |  |
| [`DeclTest`](#decltest) | `rec` |  |
| [`ComptimeBranch`](#comptimebranch) | `rec` |  |
| [`DeclComptimeIf`](#declcomptimeif) | `rec` |  |
| [`DeclComptimeDirective`](#declcomptimedirective) | `rec` |  |
| [`Decorator`](#decorator) | `rec` | `#[name(args)]`, all of which `span` covers; `id` is the decorator registry's id the name spells, decoded once by the parser, and 0 when it spells none |
| [`Decl`](#decl) | `rec` |  |

| On `DeclKind` | |
| :--- | :--- |
| [`DECL_KIND_USE`](#declkind) · [`DECL_KIND_FWD`](#declkind) · [`DECL_KIND_FUN`](#declkind) · [`DECL_KIND_REC`](#declkind) · [`DECL_KIND_VAL`](#declkind) · [`DECL_KIND_VAR`](#declkind) · [`DECL_KIND_DEF`](#declkind) · [`DECL_KIND_TEST`](#declkind) · [`DECL_KIND_COMPTIME_IF`](#declkind) · [`DECL_KIND_COMPTIME_DIRECTIVE`](#declkind) · [`DECL_KIND_UNI`](#declkind) · [`DECL_KIND_TAG`](#declkind) · [`DECL_KIND_ERROR`](#declkind) | |

| Constants | |
| :--- | :--- |
| [`DECL_FLAG_PUB`](#constants) · [`DECL_FLAG_EXT`](#constants) | |

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
    name:     base_source.Span;
    ty:       ast_id.TypeId;
    comptime: bool;
}
```

## DeclUse

```mach
pub rec DeclUse {
    alias: base_source.Span;
    path:  base_source.Span;
}
```

## DeclFwd

```mach
pub rec DeclFwd {
    alias: base_source.Span;
    path:  base_source.Span;
}
```

## DeclFun

```mach
pub rec DeclFun {
    name:           base_source.Span;
    generics_start: u32;
    generics_len:   u32;
    params_start:   u32;
    params_len:     u32;
    return_type:    ast_id.TypeId;
    body:           ast_id.StmtId;
    c_variadic:     bool;
}
```

## DeclRec

```mach
pub rec DeclRec {
    name:           base_source.Span;
    generics_start: u32;
    generics_len:   u32;
    fields_start:   u32;
    fields_len:     u32;
}
```

## DeclUni

```mach
pub rec DeclUni {
    name:           base_source.Span;
    generics_start: u32;
    generics_len:   u32;
    fields_start:   u32;
    fields_len:     u32;
}
```

## TagCase

```mach
pub rec TagCase {
    name:             base_source.Span;
    payload:          ast_id.TypeId;
    decorators_start: u32;
    decorators_len:   u32;
}
```

## DeclTag

```mach
pub rec DeclTag {
    name:           base_source.Span;
    generics_start: u32;
    generics_len:   u32;
    cases_start:    u32;
    cases_len:      u32;
    disc:           ast_id.TypeId;
}
```

## DeclBind

```mach
pub rec DeclBind {
    name: base_source.Span;
    ty:   ast_id.TypeId;
    init: ast_id.ExprId;
}
```

## DeclDef

```mach
pub rec DeclDef {
    name: base_source.Span;
    ty:   ast_id.TypeId;
}
```

## DeclTest

```mach
pub rec DeclTest {
    name: base_source.Span;
    body: ast_id.StmtId;
}
```

## ComptimeBranch

```mach
pub rec ComptimeBranch {
    cond:       ast_id.ExprId;
    body_start: u32;
    body_len:   u32;
}
```

## DeclComptimeIf

```mach
pub rec DeclComptimeIf {
    branches_start: u32;
    branches_len:   u32;
}
```

## DeclComptimeDirective

```mach
pub rec DeclComptimeDirective {
    target: ast_id.ExprId;
}
```

## Decorator

```mach
pub rec Decorator {
    span:       base_source.Span;
    name:       base_source.Span;
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
    span:             base_source.Span;
    kind:             DeclKind;
    flags:            u8;
    doc:              base_source.Span;
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

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `DECL_FLAG_PUB` | `u8` | `1` |  |
| `DECL_FLAG_EXT` | `u8` | `2` |  |

