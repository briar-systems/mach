# Getting started

Follow the steps below to set up the Mach toolchain and run this sample project locally. The commands assume the three repositories (`mach-c`, `mach-std`, `mach`) are checked out as siblings in the same parent directory.

```
~/dev/src/github.com/octalide/
  ├── mach-c
  ├── mach-std
  └── mach
```

## 1. Build the compiler

```bash
cd mach-c
make
```

This produces `bin/cmach`, the bootstrap compiler. The build requires Clang/LLVM 16 or newer and uses the system linker (`cc`) during the final link step.

## 2. Build the standard library

```bash
cd ../mach-std
make
```

The `mach-std` `Makefile` packages the library into `out/lib/libmachstd.a`. The archive is linked into any executable that uses standard modules such as `std.io.console` or `std.types.array`.

## 3. Build the sample application

```bash
cd ../mach
make
```

The `mach` `Makefile` invokes `../mach-c/bin/cmach` to compile `src/main.mach` to `out/obj/main.o`, then links the resulting object with `../mach-std/out/lib/libmachstd.a`. The final binary is written to `out/bin/mach`.

## 4. Run the CLI

```bash
make run
# or run the binary directly
./out/bin/mach help
```

At the moment the CLI understands two commands:

- `help` – prints usage information.
- `build mach.toml` – parses project options; this stub will grow into the real builder.

## 5. Next steps

- Explore the language spec in [`mach-c/doc`](https://github.com/octalide/mach-c/tree/main/doc).
- Review `src/commands.mach` to see how slices, strings, and the standard library are used in practice.
- Try adding a new subcommand and wiring it into `commands_dispatch`.
