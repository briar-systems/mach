# IR output

`mach build --emit ir` writes mirl's text of each module in an `ir/` tree beside
the `obj/` tree of its objects, as `<work>/ir/<module path>.ir`, the module's dotted
name written as directories (`out/linux-x86_64/debug/ir/demo/main.ir` for `demo.main`,
beside its object `out/linux-x86_64/debug/obj/demo/main.o`). The text is the module mach hands mirl,
written before mirl compiles it, in mirl's text form: its grammar, its version
statement and what each instruction means are mirl's to state, in
[the mirl IR specification](https://github.com/briar-systems/mirl/blob/dev/doc/ir.md)
and [its text form](https://github.com/briar-systems/mirl/blob/dev/doc/ir/text.md).

`--emit ir`, `--emit ir-opt` or `--emit asm` without `--emit exe` writes the text
and the objects and links nothing. `--emit ir --emit exe` also links, and `-o` is refused with
`cli.flag_inapplicable` when the `--emit` set names no `exe`, since nothing is
linked for it to name.

The text reads back. `mirl` reads a file mach wrote, so a module can be
compiled, verified or optimised on its own with `mirl compile` (`-O` runs the
optimisation passes, `--pass <name>=off` turns one off, and the output goes to
standard output unless `-o` names a file) and `mirl verify`:

```
mirl compile out/linux-x86_64/debug/ir/demo/main.ir --emit obj -o main.o
```

An emitted build is never restored from the object cache, so every module's
text is written each time. A build that fails removes the `ir/`, `ir-opt/` and
`asm/` trees it wrote, as a module's text is written before mirl's back end
compiles it and can stand beside a refusal (see
[When one cell fails](build.md#when-one-cell-fails)).

## Optimised IR

`--emit ir-opt` writes mirl's text of each module after mirl's pass schedule has
run on it and before its back end compiles it, as `<work>/ir-opt/<module path>.ir`
beside the `ir/` tree (`out/linux-x86_64/release/ir-opt/demo/main.ir` for
`demo.main`). It is the text `mirl compile -O --emit ir` prints for the module
`--emit ir` wrote under the same settings, so `--emit ir --emit ir-opt` gives two
trees to compare, before and after the passes:

```
mach build . -p release --emit ir --emit ir-opt
```

It shows what inlining, promotion and the other passes made of the module. Under
a profile that does not optimise, it is the module after only the passes mirl
always runs, the ones that legalise it for the target. Unlike `ir/`, the text
makes no promise to read back: it already holds the target's legalised form. It
is written for every target, a wasm or SPIR-V one included, since mirl prints it
before any back end runs.

`--emit asm` writes masc's listing of each module the same way, as `<work>/asm/<module path>.s`. A target whose objects mirl writes without masc's builder, wasm and SPIR-V, has no listing, and the flag is refused for it with `cli.flag_inapplicable` before anything compiles.

## See also

- [mirl's IR specification](https://github.com/briar-systems/mirl/blob/dev/doc/ir.md): what the text means
- [manifest.md](manifest.md): the `mach.toml` manifest reference
- `mach help build`: the full command-line reference
