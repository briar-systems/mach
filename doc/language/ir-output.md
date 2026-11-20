# IR output

`mach build --emit ir` writes mirl's text of each module in an `ir/` tree beside
the `obj/` tree of its objects, as `<work>/ir/<module path>.ir`, the module's dotted
name written as directories (`out/linux-x86_64/debug/ir/demo/main.ir` for `demo.main`,
beside its object `out/linux-x86_64/debug/obj/demo/main.o`). The text is the module mach hands mirl,
written before mirl compiles it, in mirl's text form: its grammar, its version
statement and what each instruction means are mirl's to state, in
[the mirl IR specification](https://github.com/briar-systems/mirl/blob/dev/doc/ir.md)
and [its text form](https://github.com/briar-systems/mirl/blob/dev/doc/ir/text.md).

`--emit ir` or `--emit asm` without `--emit exe` writes the text and the objects
and links nothing. `--emit ir --emit exe` also links, and `-o` is refused with
`cli.flag_inapplicable` when the `--emit` set names no `exe`, since nothing is
linked for it to name.

The text reads back. `mirl` reads a file mach wrote, so a module can be
compiled, verified or optimised on its own with `mirl compile` (`--level` sets
the optimisation level, and the output goes to standard output unless `-o`
names a file) and `mirl verify`:

```
mirl compile out/linux-x86_64/debug/ir/demo/main.ir --emit obj -o main.o
```

An emitted build is never restored from the object cache, so every module's
text is written each time. A build that fails removes the `ir/` tree it wrote,
as a module's text is written before mirl compiles it and can stand beside a
refusal (see [When one cell fails](build.md#when-one-cell-fails)).

`--emit asm` writes masc's listing of each module the same way, as `<work>/asm/<module path>.s`. A target whose objects mirl writes without masc's builder, wasm and SPIR-V, has no listing, and the flag is refused for it with `cli.flag_inapplicable` before anything compiles.

## See also

- [mirl's IR specification](https://github.com/briar-systems/mirl/blob/dev/doc/ir.md): what the text means
- [manifest.md](manifest.md): the `mach.toml` manifest reference
- `mach help build`: the full command-line reference
