# IR output

`mach build --emit-ir` writes mirl's text of each module beside its object, as
`<out>/ir/<artifact>/<module>.ir`. The text is the module mach hands mirl,
written before mirl compiles it, in mirl's text form: its grammar, its version
statement and what each instruction means are mirl's to state, in
[the mirl IR specification](https://github.com/briar-systems/mirl/blob/dev/doc/ir.md)
and [its text form](https://github.com/briar-systems/mirl/blob/dev/doc/ir/text.md).

The text reads back. `mirl` reads a file mach wrote, so a module can be
compiled, verified or optimised on its own:

```
mirl emit out/ir/demo/demo.main.ir --kind object
```

An emitted build is never restored from the object cache, so every module's
text is written each time.

`--emit-asm` would write each module's assembly beside its object. It is
refused until masc lists the objects mirl compiles (masc#484).

## See also

- [mirl's IR specification](https://github.com/briar-systems/mirl/blob/dev/doc/ir.md) — what the text means
- [manifest.md](manifest.md) — the `mach.toml` manifest reference
- `mach help build` — the full command-line reference
