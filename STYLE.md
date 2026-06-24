# How Mach Is Written

The canonical style, naming, and idiom spec for the Mach compiler. This is a
strict contract. Code that does not follow it is wrong, not just unfashionable.
`src/lang/type.mach` is kept as the reference exemplar.

The compiler is written in Mach. Reread a representative file before a large
change. Consistency across the codebase beats any one local preference.

## 1. Philosophy

- Caller-owned, callee-allocated. A component never owns the caller's memory.
  If an object needs internal owned storage it takes an `Allocator` in `init`,
  stores it as `a`, uses it for every internal allocation, and frees all of it
  in `dnit`. The caller owns the object and is responsible for its lifetime.
- Components are swappable. Build every component behind an API surface so
  another implementation can be dropped in mechanically. The unit here is a
  capability (an object-file format, an ISA, an optimization pass), not a file
  or a record. Swapping linux for COFF, or skipping register allocation, should
  be a substitution, never a workaround or a patch. If the architecture forces a
  redesign instead of a substitution (a target that cannot fit the pipeline, for
  instance), that is a design defect to flag, not to paper over. This is an
  active goal and not yet fully realized, so flag violations as you find them.
- No hidden behavior. Generics are never inferred. Error handling is explicit
  and written by hand. Nothing happens that the reader cannot see in the source.
- Symmetry. The syntax is deliberately symmetric so related statements line up.
  Write code that uses that. Keep it aligned, scannable, and regular.

## 2. Naming

- Case. `PascalCase` for records and long-standing types, including `def`
  aliases like `TypeId`. `CAPITAL_SNAKE` for long-lived constants (module-top,
  comptime, enum tags). `snake_case` for functions, variables, fields,
  parameters, and module aliases. Never `camelCase`.
- Object first, action last. `field_table_get`, not `get_field_table`.
  `type_append`, not `append_type`. The object in question leads. Two
  exceptions: predicates (below) and the `intern_*` family, which stays
  verb-first because it reads as one cohesive family.
- CRUD verbs. Accessors and mutators use a fixed suffix set: `_get`, `_set`,
  `_add`, `_del`, `_has`, `_put`. Use whatever subset applies
  (`field_table_get`, `field_table_add`). An operation that is not really CRUD
  gets a specific, self-describing name (`field_epoch_bump`) instead of being
  forced into the set.
- Predicates. Booleans and tests use an `is_` / `has_` / `can_` prefix
  (`is_integer`, `function_signature_matches`). Predicates are exempt from
  object-first ordering.
- Abbreviation. Clarity beats brevity, and consistency beats both. A short
  acronym is fine when it is unambiguous in context (`TypeInterner` to `ti`,
  `FieldTable` to `ft`, `tid`, `idx`, `len`, `cap`, `ptr`). If a good short name
  is hard to find, just fully qualify it (`type_interner` is fine). Never
  obscure meaning to save characters (`typin`, `t_intern` are bad). Never
  `self` or `this`.
- `len` and `cap`. Prefer them over `count` and `size`. They are the same width
  and read well where they are declared next to each other.
- Result and Option locals. Self-describing, prefixed by the wrapper: `res_...`
  for `Result`, `opt_...` for `Option`. Short `r_` and `o_` are acceptable when
  context is tight. The rest of the name describes the value so the local stands
  on its own. `res_alloc_types` reads as "a Result from allocating a types
  array." Use plurality to disambiguate (`fields_len_get` when reading the
  length of a `fields` array). The length of the name scales with how much
  context it needs.
- Loop counters. `i`, `ii`, `iii`, and so on by nesting depth. Not `i`, `j`, `k`.
- Receiver. First parameter, named by the type's acronym or full name (`ti`,
  `type_interner`). An allocator is stored and passed as `a`.

## 3. Lifecycle and memory

- `init` / `dnit` is the default pair for anything that owns memory or needs
  setup and teardown. `init(*T, deps...) Option[str]` initializes in place and
  assumes nothing about the value it is handed. `dnit(*T)` deallocates and zeros
  or nils every field back to a clean slate so a stale value cannot cause a
  use-after-free. Both guard `if (t == nil) { ret; }` (or return an error) at the
  top. An object that owns internal memory takes an `Allocator` in `init`, keeps
  it as `a`, and uses it for all internal allocation and for `dnit`.
- `make` is only for small, self-contained values that allocate nothing and own
  no memory. `free` should basically never exist. Caller-owned memory means
  there is nothing for a callee to free.
- Errors. `Option[str]` when a function can fail but returns no value.
  `Result[T, str]` when it returns a value and can fail. String errors are the
  norm for now. A richer error system may come later, in userland, as its own
  effort.
- Growth. Capacity doubling is the default (`if (new_cap == 0) { new_cap =
  <seed>; }`). A different strategy is fine when it is locally justified. Seed
  capacities are named constants.
- Ownership docs. Soft-enforced by `init` / `dnit` semantics, not required. Add
  a short comment only when ownership of a specific pointer is genuinely
  non-standard, like a borrowed view into a larger structure.

## 4. Error handling

- Error handling is written by hand. `Result` and `Option` are not compiler
  built-ins. There is no `?` or `try` propagation, so write the guard out. If a
  userland propagation helper ever exists, moving to it is a separate effort.
- Generic args are always explicit at call sites. The language does not infer
  them: `R.is_err[bool, str](res)`. Never elide.
- Guard density. Break a guard across lines when the line would run long (soft
  target around 80 columns) or when the body does something past a trivial
  return, especially a multi-call expression. A standalone error guard
  (`if (R.is_err...) { ret R.err...; }`) goes on its own lines. A trivial guard
  (`if (idx >= n) { ret O.none[T](); }`) can stay inline. A run of
  syntactically parallel, switch-like guards may stay compact for readability.

## 5. Types and data modeling

- Tagged unions. `rec X { kind: XKind; data: uni { ... }; }` with a
  `def XKind: u8` and `CAPITAL_SNAKE` `val` tags. This is the canonical shape.
- Handles versus pointers is a performance and ergonomics call, not dogma. Use
  integer-id handles plus a central store when they are faster or easier to work
  with. Use a raw pointer when it is faster or simpler. Do not fear either, and
  do not fear changing one to the other when the use case shifts.
- Sentinels as top-of-range handle values (`0xFFFFFFFF`, `0xFFFFFFFE`) are fine.
  They read clearly as sentinels.
- Storage style (parallel arrays plus side pools plus a dedup map, the interner
  shape) is performance and situation dependent. Reach for it when it fits.

## 6. Control flow

- `for (<cond>)` is the only loop. No other construct exists and we will never
  grow one. Manage the index by hand: `var i: u32 = 0; for (i < n) { ...;
  i = i + 1; }`.
- No compound assignment operators. Write `i = i + 1`.
- `brk` and `con` are valid and used. Flag variables are equally fine. Fit the
  situation.
- Casts use the `::` suffix operator (`x::Type`), with no surrounding spaces. It
  applies to any expression, parenthesized as needed.
- An early-exit `ret;` is fine. Do not put a bare `ret;` at the end of a void
  function.

## 7. Formatting and layout

- 4-space indent, no tabs. Opening brace on the same line.
- Braces and parens are mandatory. Any statement that can take a block gets
  braces. Any statement that can take a condition (`if`, `for`) gets parens. The
  compiler enforces both.
- `or` blocks. Never put a logical statement on the same line as a closing
  brace. Write `}`, then `or {` (or `or (cond) {`) on the next line. Align the
  opening braces of a run of single-line `if` / `or` arms so they read cleanly.
- Alignment. Align by `:`, and by `=` for consecutive statements of similar
  shape. This goes for fields, declarations, and assignments. `i = i + 1;` is
  the one case not worth aligning.
- Blank lines inside a function separate logical phases. Use them.
- Spaces around binary operators. No spaces around `::`, `:~`, or `.`.
- Soft target around 80 columns. Not a hard cap.

## 8. Documentation and comments

- Every entity is documented: records, unions, fields, and all functions
  regardless of visibility.
- Module header. Free-form. By convention a one-line summary, a blank `#` line,
  then longer paragraphs if they are warranted. No module-path or
  identifier-name prefix.
- Doc bodies. The `# ---` divider is mandatory whenever you document parameters
  or a return value, which here you almost always should. Above the divider is
  free-form. Below it is `# name: description` per parameter and
  `# ret: description`. Document the receiver and every parameter.
- Voice. A one-line summary is lowercase, with no trailing period, and reads as
  an imperative ("construct an empty interner"). Longer prose is full sentences
  with normal capitalization and periods.
- Records. Prose, then `# ---`, then `# field: description`. Per-field inline doc
  comments are also fine, especially for a longer explanation.
- Inline comments. Lowercase, terse, and explaining why rather than what, on
  their own line above the code. No section or separator banners
  (`# --- helpers ---`, `Phase 1:`). The doc-internal `# ---` divider is exempt.
  It is not a banner.
- Issue references (`(#1583)`) only when a piece of logic exists because of that
  issue. A routine bug fix in a long-lived function does not get cited. These
  should be rare.

## 9. Modularity and file organization

- A module earns its own file when it can be cleanly separated, whether by
  capability, by function, or just to stay maintainable. The CLI handlers are
  similar in shape but distinct enough in purpose to split. More files is not
  worse, so split when it helps and keep your head about it.
- Sub-records live with their owner until they are used by more than one module,
  or grow complex enough to warrant their own file.
- Constants live at the top of the module for easy scanning, unless a particular
  constant reads better next to the code that uses it. That is rare.
- Information lives where its relevance applies. Do not document a function at
  its call sites. Do not explain a specific bug at module scope.
- Canonical module order: module doc, then `use`s, then `def` and `val`
  type-ids and enum tags, then `rec`s and their config constants, then
  functions, then `test`s.
- Function order: public functions grouped by relation (lifecycle, accessors,
  predicates, core operations, equality, per-subsystem groups), then a single
  private-helper block, then tests. A helper with one caller may sit under it.
  Shared helpers go in the block.
- Visibility is about API surface, not security. `pub` is earned when the entity
  belongs on the module's surface. Prefer minimal when it costs no ergonomics.

## 10. Imports

- Group `use`s: std imports, then aliased std, then mach imports. Separate the
  groups with a blank line and alphabetize within each group.
- Aliasing. Alias `Allocator`, `Option`, and `Result` as `A`, `O`, and `R`,
  since they are used almost everywhere. Otherwise import bare. `use std.foo;`
  already binds `foo`. Reserve other aliases for a real name clash or for
  clarity. Never write the redundant `use foo: std.foo;`.

## 11. Tests

- `test` blocks live in a flat block at the bottom of the module they test,
  always co-located. Tests are discovered through import, like modules.
- Names follow `<full.module.path>[.<symbol>][:<instance>]`, for example
  `"mach.lang.type.intern_function:identical_signatures"`. They are built to
  filter on. A comment above the test carries any longer explanation.
- Skeleton: an allocator via `page.make`, failures guarded with
  `if (...) { dnit(?ti); ret 1; }`, and `ret 0` on success. The backing
  allocator changes only when the situation demands it. A page allocator is the
  default.
