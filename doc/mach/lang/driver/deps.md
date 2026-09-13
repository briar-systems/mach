# mach.lang.driver.deps

## fun set_git_config_env

```mach
pub fun set_git_config_env(env: **u8);
```

## fun resolve_deps

```mach
pub fun resolve_deps(p: *project.Project, m: *manifest.Manifest, project_root: str) err[outcome.Fail];
```

## fun resolve_cascade_libs

```mach
pub fun resolve_cascade_libs(p: *project.Project, isa: str, os: str, abi: str,
own_libs: *manifest.LinkRequirement, own_count: u32) err[outcome.Fail];
```

## fun parse_toml_file

```mach
pub fun parse_toml_file(alloc: *A.Allocator, path: str) res[toml.Table, outcome.Fail];
```

## fun cell_tmpl_vars

```mach
pub fun cell_tmpl_vars(p: *project.Project) manifest.TmplVars;
```

## fun initialize_repository

```mach
pub fun initialize_repository(s: *session.Session, root: str) err[outcome.Fail];
```

## fun check_dep_identity

```mach
pub fun check_dep_identity(s: *session.Session, key: str, declared: str) err[outcome.Fail];
```

the manifest key, the directory under dep/ and the project id are one name

## fun refuse_nested_realization

```mach
pub fun refuse_nested_realization(s: *session.Session, alias: str, dep_dir: str) err[outcome.Fail];
```

a realized nested dependency (`dep/<id>/dep/<x>/mach.toml`) is refused: the
root owns the flat closure and a dependency's own dep/ is never realized.
an empty directory git materializes for a consumed dependency's gitlink is not
a realization and passes

## fun declared_dep_id

```mach
pub fun declared_dep_id(s: *session.Session, dep_full: str) res[str, outcome.Fail];
```

## fun verify_dep_manifest_id

```mach
pub fun verify_dep_manifest_id(s: *session.Session, dep_full: str, id: str) err[outcome.Fail];
```

## fun realize_git_submodule

```mach
pub fun realize_git_submodule(s: *session.Session, root: str, id: str, url: str, ref: str) err[outcome.Fail];
```

## val REALIZE_REPO_ROOT

```mach
pub val REALIZE_REPO_ROOT: u8 = 0
```

## val REALIZE_NESTED

```mach
pub val REALIZE_NESTED:    u8 = 1
```

## fun realization_mode

```mach
pub fun realization_mode(s: *session.Session, root: str) res[u8, outcome.Fail];
```

## fun dep_rel_of

```mach
pub fun dep_rel_of(alloc: *A.Allocator, id: str) res[str, outcome.Fail];
```

## fun dep_full_of

```mach
pub fun dep_full_of(alloc: *A.Allocator, root: str, id: str) res[str, outcome.Fail];
```

## fun staged_gitlink

```mach
pub fun staged_gitlink(s: *session.Session, root: str, id: str) res[opt[str], outcome.Fail];
```

## fun staged_paths

```mach
pub fun staged_paths(s: *session.Session, root: str, rel: str) res[str, outcome.Fail];
```

## fun own_work_tree

```mach
pub fun own_work_tree(alloc: *A.Allocator, dep_full: str) bool;
```

## fun initialize_git_submodule

```mach
pub fun initialize_git_submodule(s: *session.Session, root: str, id: str) err[outcome.Fail];
```

## fun checkout_commit

```mach
pub fun checkout_commit(s: *session.Session, dep_full: str, commit: str) err[outcome.Fail];
```

## fun checkout_head

```mach
pub fun checkout_head(s: *session.Session, dep_full: str) res[str, outcome.Fail];
```

## fun realize_git_clone

```mach
pub fun realize_git_clone(s: *session.Session, root: str, id: str, url: str, ref: str) err[outcome.Fail];
```

## fun realize_git_dependency

```mach
pub fun realize_git_dependency(s: *session.Session, root: str, id: str, url: str, ref: str,
mode: u8) err[outcome.Fail];
```

## fun remove_dependency_index

```mach
pub fun remove_dependency_index(s: *session.Session, root: str, id: str, mode: u8) err[outcome.Fail];
```

## fun realize_path_dependency

```mach
pub fun realize_path_dependency(s: *session.Session, root: str, id: str, src_dir: str,
mode: u8) err[outcome.Fail];
```

## fun realized_ids

```mach
pub fun realized_ids(alloc: *A.Allocator, root: str) res[Vector[str], outcome.Fail];
```

## fun fetch_and_checkout

```mach
pub fun fetch_and_checkout(s: *session.Session, dep_full: str, ref: str) err[outcome.Fail];
```

## fun stage_dependency

```mach
pub fun stage_dependency(s: *session.Session, root: str, id: str) err[outcome.Fail];
```

