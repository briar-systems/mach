---
trigger: always_on
---

This workspace contains 2 internal projects: `cmach` and `mach`. Both are compilers for the Mach programming language.

# `cmach`
Contained in `boot/`. Uses `C23` principals.

The bootstrap compiler can be built with `make cmach`. This will produce a `cmach` executable binary in the `out/...` directory. This is already in the system path and can be called directly.

# `mach`
Contained in the `src` directory as a Mach project.

Can be built with `cmach build .` in the workspace root. Produces an executable according to the rules in `mach.toml`. Can be executed after build with `cmach run .` (note that the run subcommand does not invoke a rebuild of the mach code).

# Testing
The self-hosted mach project has not been implemented yet and the contents of `src` are actively set up as a valid project and are currently used as a testing scratchpad. Please use the above cmach executable on the contents of `src/main.mach` and related files for testing as these files can be completely overwritten with no impact.
