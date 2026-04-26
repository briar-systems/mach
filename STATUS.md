# Status — fat-str migration + /new/ rewrite

Snapshot for moving work between machines. The `tmp` branches in this
repo and in `dep/mach-std` carry the WIP. **The `new/` tree is normally
untracked**; the `tmp` branch in this repo is the only place it lives.

## Where we are

Two parallel threads are in flight:

1. **fat-str migration of the existing toolchain.** The `str` type became
   a compiler-builtin record (`rec { len: u32; data: *u8 }`) instead of
   the old `pub def str: *char`. Backtick literals now produce `zstr`
   (`*u8`). The mach-boot and mach-std landings happened in PRs (#59 /
   #181) — both merged. Subsequent cleanup is on the `tmp` branches.

2. **Compiler rewrite in `new/`.** Untracked, intentionally. We are
   building the new compiler incrementally. The frontend (lexer, parser,
   AST) is in place; semantic analysis (`new/lang/fe/sema/`,
   `new/lang/fe/resolve.mach`) is the next slab of work.

## What changed since the merged PRs

### `dep/mach-std` (`tmp` branch)

- `src/types/string.mach` — str-only operations. Internal iteration
  switched to `var i: u32` to drop sprinkled `s.len::usize` casts.
  `str_index_of` and `str_last_index_of` cast to `usize` once at the
  success return. `str_slice` uses `?s.data[start]` instead of
  `(s.data::usize + start)::*u8`. Public API return types unchanged.
  Absorbs `str_dup`, `str_dup_range`, `str_free` (now own `use std.allocator`).
- `src/types/zstr.mach` — new file. `pub def zstr: *u8`, `zstr_len`,
  `zstr_equals` (new), `zstr_to_str` (renamed from `str_from_zstr`).
- `src/allocator.mach` — drops the dead `use std.types.string` (allocator
  only ever needed the `str` type, which is now a builtin). Removing this
  unblocked moving `str_dup` into `std.types.string`.
- `src/system/os/{linux,darwin,windows}/shared.mach`,
  `src/types/path.mach` — add `use std.types.zstr;` to keep `zstr` /
  `zstr_len` resolving after the split.

### `mach` (this repo, `tmp` branch)

- `STATUS.md` — this file.
- `dep/mach-std` submodule pointer bumped to the `tmp` branch tip.
- `new/` — entire untracked tree, carried on this branch for transfer.

### `new/` migration progress

**Done (audited):**
- `lang/fe/token.mach` — `KIND_LIT_STRING` → `KIND_LIT_STR`,
  `KIND_LIT_CSTR` → `KIND_LIT_ZSTR`.
- `lang/fe/ast/expr.mach` — matching `EXPR_KIND_LIT_STR` /
  `EXPR_KIND_LIT_ZSTR`.
- `lang/fe/parser/expr.mach` — `parse_lit_str` / `parse_lit_zstr`.
- `lang/fe/parser/decl.mach` — `KIND_LIT_STR` reference.
- `lang/fe/lexer.mach` — token-kind references updated;
  `LEX_ERR_UNTERMINATED_STR` / `LEX_ERR_UNTERMINATED_ZSTR`; iteration
  uses `source.data[pos]` (str is now a record); `text()` builds a fat
  str by setting fields directly instead of casting from `usize`.
- `main.mach` — signature now `fun main(argc: i64, argv: **u8) i64`
  matching `std.runtime`.
- `lang/str_util.mach` — **deleted**. The local home is no longer
  needed; `str_dup` etc. live in `std.types.string`.
- `lang/intern.mach`, `lang/source.mach`, `lang/diagnostic.mach` —
  switched from `mach.lang.str_util` to direct `std.types.string`
  helpers; replaced `s::ptr::*u8` / `s != nil` / `str_len(s) + 1`
  open-coded deallocate dance with `str_free`.
- `lang/README.md` — `str_util.mach` entry removed.

**Done but not audited yet:**
- `cli/cmd.mach` — signature switched to `argv: **u8` / returns `i64`.
  Body still uses old `*str = *char` patterns (`val command: str = argv[1];`,
  `str_equals(command, "build")`). Migration plan: command becomes a
  `zstr`; comparisons use `zstr_equals(command, ` `` `build` `` `)` from
  `std.types.zstr`.
- `cli/args.mach` — signature switched to `argv: **u8`. Body still
  references old patterns; same migration plan as `cmd.mach`.

**Not started:**
- `lang/fe/sema/{check,coerce,generics,infer}.mach` — empty placeholders.
- `lang/fe/resolve.mach` — empty placeholder.

## Next up

1. Audit / merge the str-migration cleanup PRs (or fast-forward the `tmp`
   branches into `dev` if happy with them as-is).
2. Finish migrating `cli/cmd.mach` and `cli/args.mach` to use `**u8` +
   `std.types.zstr` backtick comparisons.
3. Begin sema work — `lang/fe/resolve.mach` first (name resolution +
   scope collection), then the four `lang/fe/sema/*.mach` files.

## Key reference points

- Runtime contract: `dep/mach-std/src/runtime.mach` documents
  `fun main(argc: i64, argv: **u8) i64` as the user-supplied entrypoint.
- `str` builtin layout: `mach-boot/src/compiler/type.c::type_get_builtin_str`.
- `usize` / `isize` are *userland* aliases in `std.types.size`, gated by
  `$mach.build.target.pointer_width`. The compiler can't see them; that
  is why `str.len` is `u32` and not `usize`.
- Cast (`::`) rules: numeric primitives convert; non-primitives must be
  the same size (bit reinterpret). `str` (16 bytes) cannot be cast to or
  from `*u8` (8 bytes); construct the record manually.

## Conventions to remember

- `new/` is intentionally untracked on the mainline. Make small,
  incremental changes and stop for audit between files.
- Avoid `# --- section ---` comment banners.
- Small commits, no PR squash bundles when re-landing the `tmp` work
  unless explicitly approved.
