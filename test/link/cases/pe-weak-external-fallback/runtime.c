/* a runtime member in the shape zig's mingw compiler_rt.lib ships: each
 * function in its own section, public only as a weak external aliasing its
 * `.weak.<name>.default` body */
__attribute__((weak)) int hook_add(int a, int b) { return a + b; }

__attribute__((weak)) int hook_unused(int x) { return x * 3; }
