# Dependencies

A dependency is another Mach project, checked out under `dep/<id>`. `mach.toml` declares it, `mach dep` fetches it, and a build reads only what is already there.

## Declaring a dependency

```toml
[dep.std]
git     = "https://github.com/briar-systems/mach-std"
version = "^10.1.1"

[dep.helper]
path = "../helper"
```

The table key is the dependency's project id, the `id` its own `mach.toml` declares. The same name is its directory under `dep/` and the first segment of its module paths.

| Key | Meaning |
|---|---|
| `git` | A Git URL. It needs a `version` or a `ref`, never both |
| `version` | A range over the repository's releases. A release is a tag `vX.Y.Z` |
| `ref` | One selector: `tag/<name>`, `commit/<full id>`, or `branch/<name>` to follow a branch |
| `path` | A local project directory, relative to the manifest. Its files are copied into `dep/<id>` |

An exact ref takes the place of the range:

```toml
[dep.std]
git = "https://github.com/briar-systems/mach-std"
ref = "tag/v10.1.0"
```

## Version ranges

| Range | Accepts |
|---|---|
| `^1.2.3` | `>=1.2.3, <2.0.0` |
| `^0.4.2` | `>=0.4.2, <0.5.0`. Below 1.0 a minor release is breaking |
| `~1.2.3` | `>=1.2.3, <1.3.0` |
| `>=1.2`, `>1.2`, `<=1.2`, `<2` | That bound. A missing component is 0 |
| `=1.2.3` | Exactly that release |
| `>=1.2, <1.5` | Releases that satisfy every clause |

Every clause needs an operator, so a bare `1.2` is refused. `[project].mach` uses the same ranges for the compiler's version. Of the releases every range accepts, `mach dep` picks the highest.

## Importing a dependency

```mach fragment
use helper.lib.helper;
use std.print;
use std.runtime;

#[symbol("main")]
fun main(argc: i64, argv: **u8) i64 {
    print.printlnf("{}", helper.twice(21));
    ret 0;
}
```

An import is `use <id>.<path>`: the dependency's id, then the file's path under its `src`. Here `helper.lib.helper` is `src/lib/helper.mach` of the project `helper`. [Modules](modules.md) covers `use`.

## Pins

There is no lock file. In a Git repository a git dependency is a submodule, and its pin is the gitlink the repository commits under `dep/<id>`. Commit `mach.toml`, `.gitmodules` and the gitlink together.

- Only `mach dep update` moves a pin.
- A path dependency has no pin. `pull` and `update` copy it from its source again.
- `dep/` is flat. It holds every project the dependencies themselves depend on, one directory each, and one id is at one commit.

## The `mach dep` commands

| Command | Effect |
|---|---|
| `mach dep add <path> <id> --git <url>` | Declare a git dependency and check it out. It writes the caret range of the newest compatible release, or what `--range <range>` or `--ref <ref>` names |
| `mach dep add <path> <id> --path <dir>` | Declare a path dependency and copy it |
| `mach dep remove <path> <id>` | Remove the declaration. `--purge` also deletes the directory |
| `mach dep pull <path>` | Check out every dependency at its pin |
| `mach dep update <path> <id>` | Move the pin to the newest commit the declaration selects. `--all` updates every dependency |
| `mach dep outdated <path>` | Show the pinned, the newest compatible and the latest release of each |
| `mach dep list <path>` | Show each declared dependency, its pin and its state |
| `mach dep verify <path>` | Check what `dep/` holds against the manifests and pins, changing nothing |

```sh
mach dep add . std --git https://github.com/briar-systems/mach-std
mach dep add . helper --path ../helper
mach dep pull .
```

A build never fetches, never resolves a range and never writes under `dep/`. A dependency that is not checked out, as in a fresh clone, is refused with the command that fixes it:

```text
error[dep.unrealized] dependency 'std' is not checked out at './dep/std'. run `mach dep pull .`
```

The full rules are in the reference for [dependencies](../language/dependencies.md).
