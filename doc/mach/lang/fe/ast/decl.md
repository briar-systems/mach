# mach.lang.fe.ast.decl

## def DeclKind

```mach
pub def DeclKind: u8
```

## val DECL_KIND_USE

```mach
pub val DECL_KIND_USE:                DeclKind = 0
```

## val DECL_KIND_FWD

```mach
pub val DECL_KIND_FWD:                DeclKind = 1
```

## val DECL_KIND_FUN

```mach
pub val DECL_KIND_FUN:                DeclKind = 2
```

## val DECL_KIND_REC

```mach
pub val DECL_KIND_REC:                DeclKind = 3
```

## val DECL_KIND_VAL

```mach
pub val DECL_KIND_VAL:                DeclKind = 4
```

## val DECL_KIND_VAR

```mach
pub val DECL_KIND_VAR:                DeclKind = 5
```

## val DECL_KIND_DEF

```mach
pub val DECL_KIND_DEF:                DeclKind = 6
```

## val DECL_KIND_TEST

```mach
pub val DECL_KIND_TEST:               DeclKind = 7
```

## val DECL_KIND_COMPTIME_IF

```mach
pub val DECL_KIND_COMPTIME_IF:        DeclKind = 8
```

## val DECL_KIND_COMPTIME_DIRECTIVE

```mach
pub val DECL_KIND_COMPTIME_DIRECTIVE: DeclKind = 9
```

## val DECL_KIND_UNI

```mach
pub val DECL_KIND_UNI:                DeclKind = 10
```

## val DECL_KIND_TAG

```mach
pub val DECL_KIND_TAG:                DeclKind = 11
```

## val DECL_KIND_ERROR

```mach
pub val DECL_KIND_ERROR:              DeclKind = 255
```

## val DECL_FLAG_PUB

```mach
pub val DECL_FLAG_PUB: u8 = 1
```

## val DECL_FLAG_EXT

```mach
pub val DECL_FLAG_EXT: u8 = 2
```

## val DECL_FLAG_GATED

```mach
pub val DECL_FLAG_GATED: u8 = 4
```

a module declaration in the arm of a declaration-level `$if`, at any depth of nesting: the parser
sets it where it first sees the declaration, and every later phase reads it

## rec TypedName

```mach
pub rec TypedName;
```

## rec DeclUse

```mach
pub rec DeclUse;
```

## rec DeclFwd

```mach
pub rec DeclFwd;
```

## rec DeclFun

```mach
pub rec DeclFun;
```

## rec DeclRec

```mach
pub rec DeclRec;
```

## rec DeclUni

```mach
pub rec DeclUni;
```

## rec TagCase

```mach
pub rec TagCase;
```

## rec DeclTag

```mach
pub rec DeclTag;
```

## rec DeclBind

```mach
pub rec DeclBind;
```

## rec DeclDef

```mach
pub rec DeclDef;
```

## rec DeclTest

```mach
pub rec DeclTest;
```

## rec ComptimeBranch

```mach
pub rec ComptimeBranch;
```

## rec DeclComptimeIf

```mach
pub rec DeclComptimeIf;
```

## rec DeclComptimeDirective

```mach
pub rec DeclComptimeDirective;
```

## rec Decorator

```mach
pub rec Decorator;
```

`#[name(args)]`; `id` is the decorator registry's id the name spells, decoded
once by the parser, and 0 when it spells none

## rec Decl

```mach
pub rec Decl;
```

