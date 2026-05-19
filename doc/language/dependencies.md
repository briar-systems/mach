# Dependencies

Dependencies are declared in `mach.toml` under `[deps.<alias>]` sections. The alias is a local handle for tooling -- import paths use the dependency's actual project ID.


## Vendoring Layout

Dependencies live under `<dir_dep>/<alias>/`:

```
project/
  mach.toml
  src/
  dep/
    mach-std/         # [deps.mach-std]
      mach.toml       # has id = "std"
      src/
```

The compiler reads each dependency's `mach.toml` to discover its project ID and source directory. Import paths use the dependency's ID, not the alias:

```mach
use std.print;        # resolves to dep/mach-std/src/print.mach
```


## Version Selectors

The `version` field in `[deps.<alias>]` specifies which version of a remote dependency to check out.

| Selector | Meaning |
|----------|---------|
| `branch/<name>` | Track a branch. `mach dep pull` checks it out and records the resulting commit hash. |
| `<40-hex>` | Exact commit hash. |
| `^MAJOR.MINOR.PATCH` | Caret constraint: highest version with the same major. |
| `~MAJOR.MINOR.PATCH` | Tilde constraint: highest version within the same minor. |
| `MAJOR.MINOR.PATCH` or `vMAJOR.MINOR.PATCH` | Exact tag. |

When no version is provided, `mach dep add` pins to the current HEAD commit after cloning. Local dependencies ignore the version field.


## The `mach dep` Command

### `mach dep list`

Prints every dependency entry with its alias, type, source path, version, and vendor directory.

### `mach dep info <alias>`

Shows detailed information about a single dependency, including whether its vendor directory exists.

### `mach dep add [--local] <path> [alias] [--version <selector>]`

Registers a new dependency:

- Without `--local`: assumes a remote git repository, runs `git submodule add`
- With `--local`: writes `type = "local"`, no git operations
- Alias is inferred from the last path segment if omitted
- Remote deps default to the current HEAD commit when no version is specified

### `mach dep del <alias>`

Removes a dependency. For remote dependencies, also runs `git submodule deinit` and cleans up the submodule directory. Local dependencies are removed from config only.

### `mach dep pull [alias]`

Fetches or refreshes vendored dependencies:

1. Initializes the submodule if needed
2. Fetches tags and branches
3. Resolves the version selector and checks out the appropriate commit
4. Updates `mach.toml` with the resolved commit hash

Without an alias, iterates all remote dependencies. Local dependencies are skipped.
