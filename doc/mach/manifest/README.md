[mach](../README.md) / **manifest**

# mach.manifest

`mach.toml`: its schema and model, loading, the selection of targets, profiles and
artifacts, templates, steps, and edits

| Module | |
| :--- | :--- |
| [`artifact`](artifact.md) | the `[artifact.<name>]` tables: their decoding, `--set` over them, and lookups |
| [`dep`](dep.md) | the decoding of the `[dep.<name>]` tables |
| [`edit`](edit.md) | editing a manifest's text in place: adding and removing a `[dep.<name>]` table, leaving every other byte as it was written |
| [`link`](link.md) | the `[link.<name>]` tables: their decoding, and their resolution for one target |
| [`load`](load.md) | reading a manifest: its TOML held to the schema, decoded into the model and checked |
| [`model`](model.md) | the manifest's model: a parsed `mach.toml` as records of interned strings and arrays |
| [`need`](need.md) | the need graph |
| [`plan`](plan.md) | a build cell resolved into a build unit: target, profile, artifact, paths and links |
| [`profile`](profile.md) | the decoding of the `[profile.<name>]` tables, and the rule of at most one default |
| [`project`](project.md) | the decoding of the `[project]` table, and the expansion of its `work` directory |
| [`refusal`](refusal.md) | how the manifest refuses: the text of a refusal, made through the caller's allocator, where in the file it points, and the other places it names |
| [`runner`](runner.md) | the command a target's programs are launched through, and the executable `mach run` launches |
| [`schema`](schema.md) | the manifest's schema: every key's row with its shape, rule and rewrite, and the check |
| [`select`](select.md) | the one selection model: the selection a command's `-a`, `-t` and `-p` options and `--all` describe, resolved against a manifest into the (artifact, target, profile) cells it covers, which the build plan takes |
| [`setting`](setting.md) | `--set`: a manifest key written over every table of its section for one invocation |
| [`step`](step.md) | the decoding of the `[step.<name>]` tables |
| [`target`](target.md) | the `[target.<name>]` tables: their decoding, `--set`, migration and resolution |
| [`template`](template.md) | the one template engine: every path and value template of a manifest, and every `#[embed]` path, expands through it |
| [`version`](version.md) | semver versions, and the version ranges a manifest states |

