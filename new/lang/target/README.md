# lang/target

Target descriptors and implementations. A `Target` is a composition of four orthogonal dimensions: instruction set, ABI, operating system, and object format. Each dimension defines an interface; implementations live in the corresponding subdirectory.

The top-level `Target` record and known valid compositions (e.g. `x64_linux_sysv_elf`) are defined in `lang/target.mach`. Invalid combinations are rejected at construction.

## Dimensions

- `isa.mach` — instruction set interface: register classes, opcodes, pointer width, endianness. Implementations in `isa/`.
- `abi.mach` — ABI interface: argument classification, return conventions, stack alignment, aggregate passing rules. Implementations in `abi/`.
- `os.mach` — operating system interface: entry convention, syscall layer, default library paths. Implementations in `os/`.
- `of.mach` — object format interface: section model, relocation kinds, symbol tables, and file writer. Implementations in `of/`.

## Design

The target subsystem is independent of the backend. It defines *what* a target is, not *how* to compile for it. The backend receives a fully composed `Target` record and calls through the dimension interfaces; it never reaches for target-specific constants directly.

This separation means the `Target` record is equally usable from the CLI (for parsing `--target` flags), from `cli/config.mach` (for reading `mach.toml`), and from tools external to the compiler that need to reason about targets as data.
