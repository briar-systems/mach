# Comptime

The `$` prefix opens the comptime channel. It reads facts the compiler knows about the build, and selects or expands code by them. It never runs user code.

## Build facts

```mach
use std.types.string.str;

val IS_LINUX: u1  = $mach.build.sys == $mach.sys.linux;
val WORD:     u64 = $mach.build.pointer_width;
val COMPILER: str = $mach.version;
val ARTIFACT: str = $bin.name;
```

A read is a constant. The roots are `$mach` for the compiler and the build, `$project` for the root project and `$bin` for the artifact.

| Read | Gives |
|---|---|
| `$mach.build.sys` | The target system: `$mach.sys.linux`, `darwin`, `windows`, `freestanding`, `uefi`, `wasi`, `vulkan` |
| `$mach.build.arch` | The architecture: `$mach.arch.x86_64`, `aarch64`, `riscv64`, `riscv32`, `wasm32`, `spirv` |
| `$mach.build.abi` | The calling convention: `$mach.abi.sysv64`, `win64`, `aapcs64`, `darwinpcs`, `lp64d` and others |
| `$mach.build.mode` | `$mach.mode.release` when the build optimizes, `$mach.mode.debug` when it does not |
| `$mach.build.pointer_width` | The size of a pointer in bytes |
| `$mach.build.pie` | 1 when the build is position independent |
| `$mach.build.ext.<name>` | 1 when the target selects that instruction-set extension |
| `$mach.version` | The compiler's version string. `.major`, `.minor` and `.patch` are integers |
| `$mach.project.id`, `$mach.project.version` | The project that owns this module |
| `$mach.source.file`, `.line`, `.module` | Where the read is written |
| `$project.id`, `$project.version` | The root project being built |
| `$project.target.sys`, `.arch`, `.abi` | The target as the manifest spells it, as strings |
| `$bin.name`, `$bin.kind` | The artifact and its kind, such as `"bin"` or `"shared"` |

- `sys`, `arch`, `abi` and `mode` compare against their tags with `==`. An unknown path or tag is an error, never a silent 0.
- Inside a dependency, `$project.version` is the consumer's version and `$mach.project.version` is the dependency's own. A library reports its version with the second.

## Conditional compilation

```mach
$if ($mach.build.sys == $mach.sys.linux) {
    use native: std.system.os.linux;
}
$or ($mach.build.sys == $mach.sys.windows) {
    use native: std.system.os.windows;
}
$or {
    $error("this module needs a linux or windows target");
}
```

`$or (cond)` chains and a bare `$or` is the fallback. Only the taken arm is compiled. The others are not resolved or type checked, so an arm may import a module or call a function that exists on its target alone. Per-target imports live here: after this chain the rest of the module uses `native` on either system.

At declaration scope the arms hold declarations. Inside a function body they hold statements:

```mach
fun page_size() u64 {
    $if ($mach.build.sys == $mach.sys.darwin && $mach.build.arch == $mach.arch.aarch64) {
        ret 16384;
    }
    $or {
        ret 4096;
    }
}
```

- A condition is a `u1` built from build facts, module `val` constants, comptime parameters and [intrinsics](intrinsics.md). A run-time value is refused.
- An extension name belongs to one architecture and is an error on another. `&&` and `||` short-circuit, so ask after the architecture test: `$mach.build.arch == $mach.arch.x86_64 && $mach.build.ext.avx2 == 1`.
- A declaration-scope chain with an arm that declares something is decided before types exist, so its condition cannot ask about a type, as `$size_of` does. A chain that declares nothing can, and so can any chain in a body.
- A chain decided by a comptime parameter has every arm resolved and checked, because each call picks its own.

## Build errors

```mach
rec Header {
    magic: u32;
    size:  u32;
}

$if ($size_of(Header) != 8) {
    $error("Header must be 8 bytes");
}
```

`$error("message")` fails the build when the compiler reaches it: in an arm it takes, or outside any `$if`. There is no `$assert`, and the chain above is how one is written.

At declaration scope it refuses the whole module. The build reports the message and nothing else from that module:

```text
error[comptime.user_error] this module needs a linux or windows target
```

## Unrolling

```mach
val PRIMES: [4]i64 = [4]i64{2, 3, 5, 7};

fun sum() i64 {
    var total: i64 = 0;
    $each p in PRIMES {
        total = total + p;
    }
    ret total;
}
```

`$each` copies its block once per element of a list the compiler knows, with the name bound to that element. Nothing iterates at run time, and each copy is checked at its own element's type. It stands only in a function body.

| Form | One copy per |
|---|---|
| `$each x in ARR { }` | Element of a constant array `val` |
| `$each a in va { }` | Argument of a pack |
| `$each f in $fields(T) { }` | Field of a record |
| `$each c in $cases(T) { }` | Case of a tag |

Packs are on the [functions](functions.md) page, `$fields` and `$cases` on the [intrinsics](intrinsics.md) page.

## Limits

- `$` selects and expands. A call to a function is always a run-time call, so a module-level `val` whose initializer calls one is refused.
- A bare `$name` is not a read. A comptime parameter, declared `$name: T`, is named in the body without the `$`, as [functions](functions.md) shows.

The full rules are in the reference for [the channel](../language/comptime.md), [`$mach`](../language/comptime-mach.md) and [`$if`](../language/comptime-control.md).
