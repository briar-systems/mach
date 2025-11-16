## Dependency Management

Mach projects declare their external packages in `mach.toml` under `[deps.<alias>]`. The compiler treats this table as the canonical source of truth for module aliases, vendored directories, and version locks. This document explains how dependencies are structured, how they are checked out on disk, and how to use the `mach dep` command family to manage them.

- [Dependency Management](#dependency-management)
- [Declaring dependencies in `mach.toml`](#declaring-dependencies-in-machtoml)
- [Vendoring layout and lifecycle](#vendoring-layout-and-lifecycle)
- [Version selectors](#version-selectors)
- [The `mach dep` command](#the-mach-dep-command)
	- [`mach dep list`](#mach-dep-list)
	- [`mach dep info <alias>`](#mach-dep-info-alias)
	- [`mach dep add [--local] <path> [alias] [--version <selector>]`](#mach-dep-add---local-path-alias---version-selector)
	- [`mach dep del <alias>`](#mach-dep-del-alias)
	- [`mach dep pull [alias]`](#mach-dep-pull-alias)


## Declaring dependencies in `mach.toml`

Every dependency lives under a TOML table named `[deps.<alias>]`.
The alias is a short identifier you control -- it is not necessarily the dependency’s project ID.

Each table must specify fields according to the specification in [config.md](config.md#depsalias-sections).


## Vendoring layout and lifecycle

The `[project].dep` key defines the dependency root (default `dep`).
Remote dependencies are cloned into `<dep>/<alias>` as git submodules, while local dependencies are copied from the specified path.
For example:

```
mach.toml
dep/
	mach-std/
		mach.toml
		src/
```

Key points:

- **Canonical structure.** The compiler loads each dependency’s `mach.toml` to learn its `[project].id` and `src` directory. If either file is missing or out of date, module resolution will fail.
- **Commit locking.** Remote dependencies will only update to a version matching the specified selector when you run `mach dep pull`. This may be a specific commit hash, branch, or semver tag.
- **Local development.** For `type = "local"`, no submodule is created. The compiler will, however, copy the source files from the specified path into the vendor directory when "update"-style commands like `mach dep pull` are run. This ensures that the dependency is always available in a consistent location, even if the original path changes or is deleted.


## Version selectors

The `version` field in `[deps.<alias>]` specifies which version of a remote dependency to check out.
It is not applicable to local dependencies and does not involve a project's CONFIGURED version (in `mach.toml`).

Mach project repositories that intend to use version selectors should tag releases using git tags that follow [semantic versioning](https://semver.org/) conventions (e.g., `v1.2.3`).

The loader understands several selector formats:

| Selector                                    | Meaning                                                                                                         |
| ------------------------------------------- | --------------------------------------------------------------------------------------------------------------- |
| `branch/<name>`                             | Track the named branch. `mach dep pull` checks it out and writes the resulting commit hash back to `mach.toml`. |
| `<40-hex>`                                  | Exact commit hash. `mach dep pull` simply ensures the submodule matches the recorded hash.                      |
| `^MAJOR.MINOR.PATCH`                        | Semver caret constraint. Matches the highest version with the same major number (and >= the specified version). |
| `~MAJOR.MINOR.PATCH`                        | Semver tilde constraint. Matches the highest version within the same minor version (>= specified patch).        |
| `MAJOR.MINOR.PATCH` or `vMAJOR.MINOR.PATCH` | Exact tag.                                                                                                      |

When no `version` is provided for a remote dependency, `mach dep add` pins it to the current HEAD commit after cloning.


## The `mach dep` command

The bootstrap CLI exposes `mach dep` with several subcommands.
Each command operates on the project rooted at the current working directory (or its nearest ancestor containing `mach.toml`).


### `mach dep list`

Prints every `[deps.*]` entry along with metadata:

- Alias (`deps.<alias>`)
- Type (`remote` or `local`)
- Source path
- Recorded `version` (if any)
- Vendor directory (`<dep>/<alias>`)

Use this to verify that your config matches what the compiler expects.


### `mach dep info <alias>`

Shows detailed information about a single dependency, including whether its vendor directory exists.
This is useful when diagnosing module resolution errors or confirming that `mach dep pull` succeeded.


### `mach dep add [--local] <path> [alias] [--version <selector>]`

Registers a new dependency and updates `mach.toml` accordingly.

- Without `--local`, the command assumes a remote git repository and runs `git submodule add <path> dep/<alias>`.
- `--local` skips git operations and writes `type = "local"`.
- If you omit `alias`, the tool infers one from the last path segment (dropping `.git`).
- `--version` accepts any selector supported by the loader. Remote dependencies default to the current commit hash when not provided.

After modifying `mach.toml`, the command saves the file so it stays in sync with the new dependency list.


### `mach dep del <alias>`

Removes a dependency entry from `mach.toml`. For remote dependencies it also:

1. Runs `git submodule deinit -f dep/<alias>`.
2. Executes `git rm -f dep/<alias>`.
3. Deletes the orphaned `.git/modules/dep/<alias>` directory.

Local dependencies simply disappear from the configuration; their source directories are untouched.

### `mach dep pull [alias]`

Fetches or refreshes vendored dependencies. With no arguments it iterates every remote dependency; providing an alias limits the operation to that entry. The command:

1. Ensures the submodule is initialized (`git submodule update --init --recursive`).
2. Fetches all tags and branches.
3. Resolves the requested `version` selector and checks out the appropriate commit.
4. Updates `mach.toml` with the resolved commit hash (for floating selectors or missing versions).

Local dependencies are skipped because there is nothing to pull.

