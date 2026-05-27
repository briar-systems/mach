# mach.cli.args

Minimal argv inspection helpers. Provides flag-presence and
flag-value lookups over the raw `(argc, argv)` pair the process
received. No argument-grammar parsing, no allocation, no
side effects — every subcommand consumes argv directly via these
helpers and decides for itself which flags it accepts.

## Functions

### `has_flag`

```mach
pub fun has_flag(argc: usize, argv: *zstr, flag: zstr) bool
```

Linear scan of `argv[0..argc]` for an exact `zstr_equals` match
against `flag`. Returns `true` on first hit.

| Param | Type    | Description                                              |
|-------|---------|----------------------------------------------------------|
| argc  | `usize` | Number of entries in `argv`.                             |
| argv  | `*zstr` | Argument vector.                                         |
| flag  | `zstr`  | Exact flag to match (e.g. `` `--verbose` ``).            |

Returns `true` if any entry equals `flag`, `false` otherwise.

### `get_value`

```mach
pub fun get_value(argc: usize, argv: *zstr, flag: zstr) Option[zstr]
```

Linear scan of `argv[0..argc]` for `flag`; on hit, returns the next
arg (`argv[i+1]`) as `some`. Returns `none` when the flag is not
present or when it is the last token.

| Param | Type    | Description                                              |
|-------|---------|----------------------------------------------------------|
| argc  | `usize` | Number of entries in `argv`.                             |
| argv  | `*zstr` | Argument vector.                                         |
| flag  | `zstr`  | Flag to match (e.g. `` `--target` ``).                   |

Returns `some(argv[i+1])` on a successful match with a following arg;
`none` otherwise.

## Notes

The module deliberately does **not** implement a generic
argument-parsing framework. Subcommands have very different flag
shapes (`build --target linux`, `dep add std`, `test --filter X`)
and the simplest stable surface is direct scanning — anything richer
becomes a contract the rest of the compiler has to honour.
`--flag=value` and short flags (`-v`) are not recognised; subcommands
that want them parse the entries themselves.

## Dependencies

`std.types.bool`, `std.types.size`, `std.types.zstr`,
`std.types.option`.
