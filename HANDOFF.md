# Session Handoff — Mach Compiler

**TEMPORARY. Delete after picking up on the other machine.**

In-flight state from the conversation. Most durable context already lives in `STATUS.md` and `~/.claude/projects/-home-octalide-dev-src-github-com-octalide-mach/memory/`; this file captures what *isn't* persisted yet — the live language-syntax design discussion.

---

## What's happening right now

Mid-walkthrough of 8 syntax-design clusters using `examples/full/` as the seed of the eventual "biblical spec." The user paused the M1 implementation push to lock down the language syntax first.

On the other machine, after reading this file: **react to Idea δ (cluster C, below)** — that was my last message; the user has not yet responded.

## Cluster status

| # | Cluster | Status |
|---|---------|--------|
| A | Missing forms in the fixture | **DONE** (decisions below) |
| B | Overloaded sigils | **DONE** — all "leave as-is" |
| C1 | `use` form | **IN PROGRESS** — Idea δ awaiting reply |
| C2 | `fwd` replacement | **IN PROGRESS** — Idea δ proposes `pub PATH;` |
| C3 | Self-prefix `this.` | **DONE** — keep full project ID form |
| C4 | Path separator `.` vs `::` | **DONE** — keep `.` |
| D | Inline asm | not started (proposal pre-loaded) |
| E | Comptime redesign | not started (proposal pre-loaded) |
| F | AI-friendliness | not started (mostly "leave") |
| G | Clamp suggestions | not started (5 candidates) |
| H | `uni` consumption pattern | not started (big question) |

## Cluster A — LOCKED decisions

- **`ext`**: as a modifier (like `pub`): `ext fun foo(...) R;`. C ABI only. NO extern constants. Linker symbol rename via comptime attr: `$foo.symbol = "real_name";`.
- **No body-less functions outside `ext`** — forward decls don't exist.
- **`def X: uni { ... };`** — parity with `def X: rec { … };`. Include.
- **Char escapes**: `\n \t \r \\ \' \0 \xHH`.
- **String escapes**: same set plus `\"`.
- **Numeric literals**: hex (already in fixture), plus `0b…`, `0o…`, underscores `1_000_000`, scientific `1.5e10`.
- **Strings**: backtick = `zstr` (`*u8`), double-quote = `str` (fat pointer). Multi-line backtick **deferred** to a larger strings conversation later — user "not extremely happy" with this design overall.

## Cluster B — LOCKED

All overloaded forms left as-is: `[]`, `.`, `{}`, `()`, `:`. The `$` overload (control / namespace / attribute) is handled by cluster E (tighten the namespace, don't change the sigil).

## Cluster C — IN PROGRESS (most important pickup point)

User constraints expressed across iterations:
- **No splat ever.**
- **One form preferred** — asymmetric module-vs-symbol forms annoy the user.
- **Keep `:`** — rejected `=` for `use`.
- **`fwd` keyword should go**, functionality preserved.
- **`pub use` has potential** but the user worried about info loss / ambiguation (re-exports hidden inside import lines).
- **Multiplatform** (`$if`-guarded import then re-export) is the primary `fwd` use case.

### Earlier iterations the user rejected
- **Option 1** (`use ALIAS: PATH;` always required) — verbose.
- **Option 2** (alias for modules, bare for symbols) — picked but "not perfect."
- **`=` rework** (`use sz = std.types.size;`) — rejected.
- **Ideas α / β / γ** (variant module/symbol schemes) — rejected.
- **`pub use` as combined re-export** — info-loss concern.

### Idea δ — MY LATEST PROPOSAL (awaiting user reply)

**Single `use` form**:
```
# use: ALIAS optional, defaults to the leaf component
use std.types.size;                  # alias = 'size'  (module)
use sz: std.types.size;              # explicit module alias
use std.types.size.usize;            # alias = 'usize' (symbol)
use my_usize: std.types.size.usize;  # explicit symbol rename
```

One form `use [ALIAS:] PATH;`. Resolver decides module vs symbol. **Splat is killed** — bare form now means "bind under leaf name," not splat. The original rationale for requiring alias (avoid splat) is met by simply removing splat behavior, so optional alias is safe.

**Re-export via standalone `pub PATH;`**:
```
use impl: this.os.linux;
pub impl.spawn;
pub impl.kill;
pub impl;          # whole module (or ban for max explicitness)
```

- `pub` extended: today modifies a decl; now also standalone with a path to publish a name.
- Per-symbol re-exports each on their own line — preserves visibility (addresses the info-loss concern).
- Two separate acts (`use` then `pub`) deliberately — NOT combined into `pub use`.

**Multiplatform pattern**:
```
$if (os == linux)        { use impl: this.os.linux; }
$or $if (os == windows)  { use impl: this.os.windows; }

pub impl.spawn;
pub impl.kill;
```

Zero new keywords. (Fallback if `pub` standalone feels too overloaded: dedicated `expose PATH;` keyword — but I lean keep `pub`.)

## Clusters D–H — pre-loaded proposals (not yet walked through)

### D. Inline asm
```
asm x86_64 {
    in  a   = src,
    in  b   = other,
    out res,
    clobber "rax", "memory"

    "mov %[res], %[a]"
    "add %[res], %[b]"
}
```
- Operand declarations inside block (no separate paren list).
- Body = sequence of string literals.
- Substitution: `%[name]`.
- Clobbers as quoted strings.
- ISA from a closed compiler-known set.
- All asm implicitly volatile.
- Multi-arch dispatch via dedicated `asm { arch x86_64 { … } arch aarch64 { … } }`.

### E. Comptime — tighten `$` namespace
- `$mach.*` → read-only compiler-provided values.
- `$<sym>.<attr> = value;` → top-level only, write-once, sym must be declared, RHS comptime-evaluable.
- `$if`, `$or` → comptime control keywords.
- Optional sugar: **decl-attached attributes** `$inline pub fun foo() {}` and stacking `$inline\n$noreturn\npub fun panic(...) {}`.

### F. AI-friendliness
- Keep short keywords — Mach character.
- Keep `?`/`@` (address-of / deref) — frees `&`/`*` cleanly.
- Biggest AI-clarity win is killing wildcard `use` — handled in C.

### G. Clamps
- Wildcard `use` splat — REMOVE (handled in C).
- `fin`: allow only `fin stmt;` or `fin { }`, no bare expression form.
- ISA names from closed set.
- Clobber operands quoted only.
- `$<sym>.<attr>` RHS restricted to comptime-evaluable values.

### H. `uni` consumption
Current: raw aliasing (no built-in tag).
- **Option A** — leave raw, document loudly. Use the `rec { kind: K; data: uni { … } }` pattern (which `new/`'s own Stmt/Decl/Type already do). KISS. **My recommendation.**
- **Option B** — built-in tag + `match`. Big addition.

If A: add an explicit example to `examples/full` of the safe-use pattern.

---

## `examples/full/` state

Located at `examples/full/` — created and once-corrected this session:
- `mach.toml` — minimal project config (id=`full`, deps mach-std, linux/x86_64 target).
- `lib.mach` — every public declaration form (`rec`, generic `rec`, `uni`, generic `uni`, `fun`, void `fun`, generic `fun`, variadic `fun`, `def` alias, `def` of fn-pointer type, `pub val`, `var`).
- `main.mach` — entry: imports, `fwd`, `$main.symbol`, `fun main`, `test`. Has one example of every statement and expression form.

**Eyeball items, user feedback received**:
1. `uni Name[T] {}` generic union — fine.
2. Generic union literal `lib.Maybe[i64]{ some: 7 }` — fine.
3. ~~Inferred binding `val inferred = dec + hex;`~~ — WRONG (Mach has NO inference). Already fixed to `val combined: i64 = dec + hex;`. Memory updated (`project_no_inference.md`).
4. Inline nested array literal — fine.

**Pending updates** (batch at end after all clusters lock):
- `ext fun foo(...) R;` example.
- `def X: uni { ... };` example.
- `use` and re-export forms per cluster C resolution (currently Idea δ: kill `fwd`, use `pub PATH;`).
- Numeric forms (`0b`, `0o`, `_`, `e`).
- D/E/G/H — once locked.

## `new/` cleanup plan (PENDING approval — paused until syntax locks)

| Action | What |
|---|---|
| Keep | `token`, `lexer`, `parser`, `ast` — but add the missing `uni` decl kind (AST currently has `rec` decls only) |
| Delete | `sema`, `me/`, `be/`, `target/`, `query`, 0-byte `cli/cmd/` stubs |
| Delete | `doc/` spec tree (docs-first dead weight; drifted from code) |

After approval: M1 begins (sema → IR → MIR → encode → ELF → link).

## Tasks (in repo task list)

- #23 M0 — build harness + cmach proof — **completed**
- #24 M1 — ret 42 vertical slice — **in_progress** (paused for syntax lock)
- #25 M-final — self-host — pending

## Memory state (already persisted)

- `project_pivot` — the rebuild direction
- `feedback_process_discipline` — build-is-spec, vertical slices, no specs ahead
- `feedback_stop_building` — reason before building, don't loop
- `feedback_no_half_solutions`
- `feedback_no_defensive_fallbacks`
- `feedback_engineering_mindset`
- `project_use_semantics` — **stale** under cluster-C redesign; rewrite once C locks
- `project_no_inference` — Mach has no type inference

## Resume prompt for the new machine

> Read `HANDOFF.md` — we're mid-walkthrough of language-syntax design clusters. The last thing I sent the user was Idea δ for cluster C (single `use [ALIAS:] PATH;` form with optional alias, splat killed, standalone `pub PATH;` for re-export, multiplatform pattern shown). The user hasn't replied yet; await their reaction. Don't restart the walkthrough.

Delete `HANDOFF.md` once absorbed.
