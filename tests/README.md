# tests

The Mach compiler's test corpus — and the project's definition of done.

Each `NNN_name.mach` is a standalone program. Line 1 declares the
expected process exit code:

    # expect: exit 42

`run.sh [path-to-compiler]` compiles every test with the compiler,
runs the result, and checks the exit code.

The corpus only grows. Every language feature lands together with a
test that exercises it. Self-hosting is reached when the compiler's
own source compiles and the whole corpus stays green.
