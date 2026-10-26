# mach.lang.me.lower.oblivious

mach's oblivious rule: a function that computes on a secret declares `#[oblivious]`. one that
only moves, stores or converts a secret may omit it. which opcodes move a value rather than
compute on it is mach's reading of mirl's rows

## fun moves

```mach
pub fun moves(code: Opcode) bool;
```

whether `code` moves or converts its operands rather than computing on them

## fun class_name

```mach
pub fun class_name(code: Opcode) str;
```

what a diagnostic calls the computation `code` performs

