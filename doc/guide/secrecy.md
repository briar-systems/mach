# Secrecy

`^T` marks constant-time data, not confidential data. The compiler keeps a `^` value from deciding anything an attacker can time, and it does not hide, encrypt or wipe that value.

## Secret types

```mach
rec Session {
    id:  u64;
    key: ^[32]u8;
}

fun mix(key: ^u32, salt: u32) ^u32 {
    val k: ^u32 = salt;
    ret (key ^ k) + 1;
}
```

| Type | Meaning |
|---|---|
| `^u32` | A secret integer |
| `^[32]u8` | A secret array, whose elements read as `^u8` |
| `*^u8` | A public address of secret bytes |
| `^*u8` | A secret address |

Secrets combine with public values by three rules:

- A public value becomes secret with no syntax, wherever a secret is expected. A literal does too.
- An operation with a secret operand gives a secret result, comparisons included.
- Nothing becomes public implicitly. The casts `::` and `:~` cannot drop `^`, and a `*^u8` converts to neither `*u8` nor `ptr`.

## What a secret may not reach

In every function, a secret is refused wherever its value would show in timing:

- the condition of an `if` or a `for`
- the left operand of `&&` or `||`, which decides whether the right one runs
- an index, as in `table[s]`
- an address, as in `@p` or `p[i]` through a secret pointer
- an operand of `/` or `%`
- an operand of float arithmetic
- an argument of a C-variadic call

```mach error secret value used as a branch condition
fun same(given: ^u64, key: ^u64) u1 {
    if (given == key) { ret 1; }
    ret 0;
}
```

```text
error[secret.branch] secret value used as a branch condition
  note compute without branching on it, or declassify with `:>T` when disclosure is intended
```

Everything else is allowed. A secret can be stored, copied, passed, returned, shifted and combined with `+`, `^`, `&` and the other integer operators.

## Declassifying

```mach
fun same(given: ^u64, key: ^u64) u1 {
    ret ((given ^ key):>u64) == 0;
}
```

`x:>T` is the only way to remove `^`. It gives a public copy, and `T` must be the type of `x` without its `^`. Each one is the author's claim that the value is safe to reveal, so the compiler lists them for review:

```sh
mach check . --declassified
```

```text
src/bin/main.mach:15:9: same: ^u64
```

Each line is a position, a function and the type declassified there.

## Constant-time functions

```mach
#[constant_time("hard")]
fun differ(a: *^[32]u8, b: *^[32]u8) ^u8 {
    var d: ^u8 = 0;
    var i: u64 = 0;
    for (i < 32) {
        d = d | (a[0][i] ^ b[0][i]);
        i = i + 1;
    }
    ret d;
}
```

The rules above check the source. `#[constant_time]` makes the build check the machine code as well. It walks the function's final instructions, after optimization and register allocation, and refuses the build unless it proves all of these:

- No branch depends on a secret.
- No memory address depends on a secret.
- No instruction whose timing depends on its operands receives a secret.
- No secret reaches public storage or a public result except through `:>T`.
- A secret is passed only to functions that are `#[constant_time]` themselves.

Some processors promise an instruction's timing only under a condition: the DIT mode on aarch64, the Zkt extension on riscv. The decorator's argument is required, and it says what to do about that.

| Mode | What the build proves |
|---|---|
| `"soft"` | The list above. The function turns the timing mode on around itself where the processor has it, and runs without it where it does not. |
| `"hard"` | The list above, and that the target guarantees the timing of every instruction used. A target that does not is refused. |

On linux x86-64 both modes build. On linux aarch64 a hard function builds only when the target selects `dit`, as in `ext = ["dit"]`.

A function without the decorator may still compute on secrets. It is held to the source rules, with no promise about its machine code.

## Wiping

```mach fragment
fun forget(s: *Session) {
    $zeroize(?s.key, 1);
}
```

`$zeroize(p, count)` writes zero over `count` values at `p`, and no optimization removes it. Nothing else wipes a secret. Stack slots and registers that held one are not cleared when a function returns.

## Assurance

**Guaranteed.** In a `#[constant_time]` function on a covered target, the machine code's branches and addresses never depend on a secret, and no instruction with operand-dependent timing receives one. A build that cannot prove this is refused.

**Covered targets.** Linux x86-64 and linux aarch64, where independent analyzers have cross-checked the output on a small corpus. Every other target is experimental: the same checks run without that cross-check. On wasm the decorator compiles with a warning, because the runtime chooses the instructions. On SPIR-V it is refused.

**Not guaranteed.**

- Confidentiality. Secret bytes can stay in memory after use and nothing encrypts them.
- Cache and power side channels.
- Accesses outside the object a pointer addresses.
- Anything after `:>T`. That is the author's claim, not the compiler's.
- The machine code of functions without the decorator, and inline `asm` outside a constant-time function.
- The processor itself. The x86-64 timing table is Intel's published list, and AMD publishes none.

The full rules are in the reference for [`^`](../language/secrecy.md).
