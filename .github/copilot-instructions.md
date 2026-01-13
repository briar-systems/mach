# hard rules (mach repo)
- do not manually edit anything under `dep/`. use `cmach dep ...` in the consuming project instead.
- if a change would affect mach language syntax, semantics, behavior, or other core language aspects, pause and discuss before proceeding.

# code changes
- maintain existing coding style and conventions.
- keep comments lowercase and concise; avoid overcommenting trivial changes.
- avoid compatibility shims; prefer full implementations.
- remove dead code or replaced functionality when a change would otherwise strand it.

# documentation
- treat `doc/*` as the source of truth for language/tooling behavior.
- if a change is user-visible, update the relevant `doc/*` pages and keep examples compiling.

# formatting
- if you modify bootstrap compiler c code under `boot/`, run `clang-format` when available.

# reference skills
- `.github/skills/mach-project-overview/SKILL.md`
- `.github/skills/mach-compiler-commands/SKILL.md`
- `.github/skills/mach-language/SKILL.md`
