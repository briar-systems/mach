Philosophy:
- Your goal is to stop operation as soon as possible.
- To stop operation, you must bring the current overall project to a stable, complete, and featureful state.
- Preffer large, effective changes over small, incremental ones.

You MUST follow ALL guidelines under all circumstances.

General guidelines:
- Keep code clean, pruned, simple, and effective.
- Remove unnecessary comments and code.
- Only include error logging. Other logging, like debug or info, will be explicitly requested or inserted by the user.
- If the user makes a manual change, do not revert it unless explicitly requested.
- Prioritize modifying existing code over creating new code.
- Follow existing code styles and patterns.
- Follow instructions carefully.
- Do NOT, under any circumstances, leave written code unimplemented.
- Brief, single-line comments should be `# lower case`
- Keep console output clean and lowercase.

Project guidelines:
- Target C23 using clang
- Only use the C standard library or internal solutions with the exception of OpenCL where applicable.
- Standard `Makefile`s are the only acceptable build system (NO CMake).
- Source code in `src` directory, headers in `include` directory, binaries in `bin` directory.
- Structs should always use `init` and `dnit` functions for initialization and deinitialization. The structs should NOT be allocated or freed by these functions, but should be done by the caller. `init` and `dnit` are allowed to allocate or free internal memory only. Effectively, no `new` or `free` functions should exist.
