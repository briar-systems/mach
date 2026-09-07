# Approved v5 tooling work

The owner approved these changes during the language and tooling design review.
They follow the published 4.30 transition seed and join the existing pre-v5 work.
They do not replace the release correctness gates or authorize a backlog reset.

## Manifest policy

Require an explicit profile instead of synthesizing profiles when none are
declared. A declared profile states opt, debug, simd, vectorize and float_reassoc.
Retain default selection markers and documented optional platform overrides.
Distinguish explicit compilation policy from values derived from target facts
and absent optional features. Keep mach init scaffolds complete and update the
manifest reference to describe the actual rules.

Acceptance includes missing-policy diagnostics, complete generated scaffolds,
explicit profile selection and migrations of compiler and std manifests.

## Artifact identity and filenames

Add an explicit {artifact.suffix} output template so one logical artifact can
name its conventional output across targets. Define the mapping for each
supported artifact kind and target combination. Literal output paths remain
literal. Do not add implicit suffixes or a conditional template language.

Acceptance includes Windows and Unix filenames, stable artifact identity,
collision checks after expansion and build/run agreement on output paths.

## Qualified requirements

Make need references identify their category, for example step.generate and
artifact.support. Keep link references unchanged because they have one category.
Define qualified glob behavior and reject ambiguous or invalid references.

Acceptance includes same-name step/artifact declarations, missing references,
cycles, qualified globs and migration of existing manifests.

## Build plan inspection

Add mach build <path> --plan using the normal planner and selection rules.
Display the selected project, target, profile, artifact, entry, output, required
steps and link inputs. Do not execute steps, fetch dependencies or publish build
outputs. Identify facts unavailable until generation instead of guessing them.

Acceptance includes agreement with actual build selection and paths, generator
boundaries, invalid manifests and absence of build/dependency mutations.

## Dependency project selection

Use mach dep <action> <path> followed by action-specific operands, including
mach dep update <path> <name>. Do not add a competing --project option.
Pull realizes recorded dependencies and retains existing path copies. Update
refreshes copied path dependencies and applies the declared update policy.

Acceptance includes every action invoked outside its project, unambiguous path
and dependency-name parsing, accurate help and preservation of Git boundaries.

## Separate language discussion

try, res, opt, err and tag are permitted design candidates, not approved language
features. Preserve explicit types, visible failure paths and manual ownership
while evaluating them. No inference, propagation or tagged-union proposal has
been approved for implementation.
