[mach](../README.md) / [front](README.md) / **transition**

# mach.front.transition

the forms of the previous major and the tables that carry a project across them

each form is refused, its diagnostic naming what this major spells. the head, option
and alias tables are what `mach migrate` rewrites with, and the next major deletes them

<sub>source: [`src/front/transition.mach`](../../../src/front/transition.mach) · imported by 5 modules</sub>

<sub>`diagnostic_kind` is [`mach.base.diagnostic.kind`](../base/diagnostic/kind.md) · `base_source` is [`mach.base.source`](../base/source.md)</sub>

| Type | | |
| :--- | :--- | :--- |
| [`Form`](#form) | `def` |  |
| [`Row`](#row-1) | `rec` | one form |
| [`Head`](#head) | `rec` | a head a previous major wrote and the selection of masc's that stands for it: the architecture's first level, which adds nothing to a target of it |
| [`Flag`](#flag) | `rec` | an option of mach's a previous major spelled otherwise |
| [`Alias`](#alias) | `rec` | a `def` a previous major spelled over a width this major has a truth value for |

| On `Form` | |
| :--- | :--- |
| [`CONDITION_U8`](#form) | a condition of type u8 |
| [`U1_AS_U8`](#form) | a u1 under arithmetic, a shift, or a bitwise operator beside a wider operand |
| [`ASM_HEAD`](#form) | an `asm` block headed by a previous major's identifier |
| [`row`](#row) | the row of `form` |

| On `Head` | |
| :--- | :--- |
| [`head_of`](#head_of) | the row of the head `span` of `source` writes, nil when no previous major wrote it |

| On `Flag` | |
| :--- | :--- |
| [`flag_at`](#flag_at) | the row of renamed option `i` |

| On `Alias` | |
| :--- | :--- |
| [`alias_of`](#alias_of) | the row of a `def` of the name at `name` over the type at `over`, nil for any other |

| Constants | |
| :--- | :--- |
| [`FLAG_COUNT`](#constants) | |

---

## Form

```mach
pub def Form: u8
```

| Name | Value | |
| :--- | :--- | :--- |
| `CONDITION_U8` | `0` | a condition of type u8 |
| `U1_AS_U8` | `1` | a u1 under arithmetic, a shift, or a bitwise operator beside a wider operand |
| `ASM_HEAD` | `2` | an `asm` block headed by a previous major's identifier |

### row

```mach
pub fun row(form: Form) *Row
```

the row of `form`

## Row

```mach
pub rec Row {
    refusal: diagnostic_kind.Kind;
    message: str;
    note:    str;
}
```

one form

| Field | Type | |
| :--- | :--- | :--- |
| `refusal` | `diagnostic_kind.Kind` | the kind it is refused with |
| `message` | `str` | what the diagnostic says |
| `note` | `str` | the replacement it names |

## Head

```mach
pub rec Head {
    from: str;
    to:   str;
}
```

a head a previous major wrote and the selection of masc's that stands for it: the
architecture's first level, which adds nothing to a target of it

### head_of

```mach
pub fun head_of(source: str, span: base_source.Span) *Head
```

the row of the head `span` of `source` writes, nil when no previous major wrote it

## Flag

```mach
pub rec Flag {
    from: str;
    to:   str;
}
```

an option of mach's a previous major spelled otherwise

### flag_at

```mach
pub fun flag_at(i: usize) *Flag
```

the row of renamed option `i`

## Alias

```mach
pub rec Alias {
    name: str;
    from: str;
    to:   str;
}
```

a `def` a previous major spelled over a width this major has a truth value for

### alias_of

```mach
pub fun alias_of(source: str, name: base_source.Span, over: base_source.Span) *Alias
```

the row of a `def` of the name at `name` over the type at `over`, nil for any other

## Constants

| Name | Type | Value | |
| :--- | :--- | :--- | :--- |
| `FLAG_COUNT` | `usize` | `1` |  |

