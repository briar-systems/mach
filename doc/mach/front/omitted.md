[mach](../README.md) / [front](README.md) / **omitted**

# mach.front.omitted

the names a build leaves out: each name a module-scope `$if` branch the build does not select
declares, with the condition that left it out

name resolution records them as it discards a branch, from the syntax the parser already
built, and reads them only once a lookup has failed, to say that the name exists under a
condition this build does not meet.

Source: [`src/front/omitted.mach`](../../../src/front/omitted.mach)

Uses types from [`mach.base.diagnostic`](../base/diagnostic.md), [`mach.base.fail`](../base/fail.md), [`mach.base.intern`](../base/intern.md), [`mach.base.source`](../base/source.md), [`mach.front.ast`](ast.md), [`mach.front.ast.decl`](ast/decl.md).

| Type | | |
| :--- | :--- | :--- |
| [`Omitted`](#omitted) | `rec` | one name a branch the build does not select declares |

| On `Omitted` | |
| :--- | :--- |
| [`record`](#record) | record every name branch `br` of a module-scope chain declares, at any depth, as left out by condition `cond`; a nested chain is left out by the same condition, since it was never decided |
| [`find`](#find) | the first record of `name` among `count` records, nil when the build leaves no such name out |
| [`attach`](#attach) | point a report of a name no lookup found at where the build left it out: the declaration, and the condition that excluded it |
| [`same`](#same) | whether two records say the same, so an importer reading one reports what it would read in the other |

| Functions | |
| :--- | :--- |
| [`condition`](#condition) | the span of a branch's condition, empty for a bare `$or` |

---

## Omitted

```mach
pub rec Omitted {
    name:     mach.base.intern.StrId;
    file:     mach.base.source.FileId;
    at:       mach.base.source.Span;
    cond:     mach.base.source.Span;
    selected: bool;
}
```

one name a branch the build does not select declares

| Field | Type | |
| :--- | :--- | :--- |
| `name` | `mach.base.intern.StrId` | the name |
| `file` | `mach.base.source.FileId` | the file that declares it |
| `at` | `mach.base.source.Span` | the name where it is declared |
| `cond` | `mach.base.source.Span` | the condition that left it out: the branch's own, which is false, or, for a branch after the one the build selects, the selected branch's |
| `selected` | `bool` | `cond` is the selected branch's |

### record

```mach
pub fun record(out: *std.collections.vector.Vector[Omitted], a: *mach.front.ast.Ast, source: str, itn: *mach.base.intern.Interner,
    br: *mach.front.ast.decl.ComptimeBranch, cond: mach.base.source.Span, selected: bool) err[mach.base.fail.Fail]
```

record every name branch `br` of a module-scope chain declares, at any depth, as left out by
condition `cond`; a nested chain is left out by the same condition, since it was never decided

| Parameter | |
| :--- | :--- |
| `out` | where the names go, in declaration order |
| `a` | the module's syntax |
| `source` | its text |
| `itn` | the interner names are interned in |
| `br` | the branch the build does not select |
| `cond` | the condition that left it out |
| `selected` | `cond` is an earlier branch's, which the build selects |
| **returns** | the allocator's refusal, or a nesting refusal for chains deeper than the stack |

### find

```mach
pub fun find(list: *Omitted, count: u32, name: mach.base.intern.StrId) *Omitted
```

the first record of `name` among `count` records, nil when the build leaves no such name out

### attach

```mach
pub fun attach(b: *mach.base.diagnostic.DiagnosticBuilder, o: *Omitted) err[mach.base.fail.Fail]
```

point a report of a name no lookup found at where the build left it out: the declaration, and
the condition that excluded it

### same

```mach
pub fun same(l: *Omitted, r: *Omitted) bool
```

whether two records say the same, so an importer reading one reports what it would read in the other

## Functions

### condition

```mach
pub fun condition(a: *mach.front.ast.Ast, br: *mach.front.ast.decl.ComptimeBranch) mach.base.source.Span
```

the span of a branch's condition, empty for a bare `$or`

