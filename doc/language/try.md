# `try`: explicit failure handling

The `try` expression provides explicit, visible failure handling for canonical
result, option, and error tags. It evaluates a fallible operand, extracts its
payload on success, and routes failure through a mandatory terminating block.

## Implementation status

The features described on this page represent the accepted Mach v5 contract
specified in [the tagged value design](../design/tagged-values.md). At base
commit `fc5c9e7e`,
parsing of `try` expressions and explicit failure blocks is implemented. Semantic
type checking, payload extraction lowering, and runtime support remain under
active development.

## Grammar

```mach
try operand or block
try operand or (binding: error_type) block
```

The operand is a prefix expression followed by any postfix chain. A binary or
larger expression requires explicit parentheses:

```mach
val sum: i64 = (try parse(a) or (e: ParseError) {
    ret res[i64, ParseError].err{e};
}) + (try parse(b) or (e: ParseError) {
    ret res[i64, ParseError].err{e};
});
```

The failure block is introduced by `or` and is mandatory. Error bindings are
optional for `res` and `err`. When present, the declared binding type must match
the exact error payload type. Options permit no error binding. Mach provides no
shorthand try syntax, no implicit error propagation, and no method constructors.

## Supported operands

The `try` operator accepts only the three canonical tag types:

- `res[T, E]` produces value `T` on `ok`, or binds error `E` on failure
- `opt[T]` produces value `T` on `some`, with no error binding on `none`
- `err[E]` produces no value on `ok`, or binds error `E` on failure

User-declared tags acquire no automatic `try` convention. Non-canonical tags use
explicit case tests (`if (value == MyTag.case)`) instead.

## Mandatory terminating failure blocks

Every reachable code path through a failure block must terminate control flow.
Permitted exits are:

- `ret` returning from the enclosing function
- `brk` or `cnt` targeting a loop that encloses the `try` expression

A `brk` or `cnt` targeting a loop declared inside the failure block does not
satisfy this requirement. Calling a function does not prove non-return, even if
that function panics or loops indefinitely.

A failure block cannot fall through, cannot provide a fallback value, and cannot
initialize the expression destination. Propagation and domain conversions remain
ordinary, visible statements:

```mach
fun increment(input: str) res[i64, ParseError] {
    val number: i64 = try parse(input) or (error: ParseError) {
        ret res[i64, ParseError].err{error};
    };
    ret res[i64, ParseError].ok{number + 1};
}
```

Existing `fin` restrictions apply unchanged. A `ret` inside a failure block
cannot cross an enclosing `fin` boundary outward.

## Option handling

Options carry no failure payload, so the failure block takes no binding:

```mach
fun fetch_item(table: *Table, key: str) res[i64, TableError] {
    val index: usize = try lookup(table, key) or {
        ret res[i64, TableError].err{TableError.missing{}};
    };
    ret res[i64, TableError].ok{get_entry(table, index)};
}
```

A failure block can also break or continue an enclosing loop:

```mach
for (more_records()) {
    val item: i64 = try read_next() or {
        cnt;
    };
    process_item(item);
}
```

## Statement try with `err[E]`

The canonical `err[E]` tag represents either payloadless success (`ok`) or an
error payload (`err: E`).

Because its successful case carries no payload value, `try` on an `err[E]` operand
produces no value. It is legal only as a direct expression statement. It cannot
serve as a variable initializer, call argument, or arithmetic operand.

```mach
pub tag WriteError: u8 {
    denied;
    full;
}

pub fun flush() err[WriteError] {
    # standalone successful outcome
    ret err[WriteError].ok{};
}

pub fun sync_disk() err[WriteError] {
    # statement try consuming flush
    try flush() or (error: WriteError) {
        ret err[WriteError].err{error};
    };
    ret err[WriteError].ok{};
}

pub fun fail_flush() err[WriteError] {
    # standalone failure outcome
    ret err[WriteError].err{WriteError.denied{}};
}
```

## Evaluation order and assignments

Evaluation proceeds under strict left-to-right rules:

- The operand of `try` evaluates once
- Function call arguments evaluate left to right
- In assignments, the right hand side evaluates and captures before the destination place on the left hand side is evaluated

If a `try` extraction fails, remaining evaluation of the enclosing expression
skips entirely. The failed extraction does not initialize or assign its destination. Side effects
from operand evaluation, earlier expression evaluation, or the failure block
remain in effect. `try` does not roll them back.

```mach
consume(try parse(input) or (error: ParseError) {
    ret res[i64, ParseError].err{error};
}, later());
```

In this example, `later()` executes only if `parse(input)` succeeds.

## See also

- [tag.md](tag.md) - declaration and semantics of tagged values
- [statements.md](statements.md) - statement structure, loops, and fin blocks
- [expressions.md](expressions.md) - expression forms and evaluation rules
