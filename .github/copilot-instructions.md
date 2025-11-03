# Style
- Maintain existing coding style and conventions.
- Keep comments prefferably single-line and `// lower case`.
- Avoid overcommenting trivial code or changes. Focus comments on complex logic.
- If the bootstrap compiler is modified, once all changes are complete, format the C code using `clang-format` if available on the host system.

# Quality
- Maintain clarity and simplicity in all code changes.
- Avoid introducing compatibility shims. Prefer full implementations.
- Fully remove any dead code or lost features if affected by a change. This includes replaced functionality.

# Documentation
- If a change requires documentation, update any existing documentation files in `doc` that pertain to the change.
- Maintain parity with the existing documentation style.
