/* the C half of the backtrace case (mach #4013): the caller that enters mach and
 * the callback libSystem's unwinder reports each frame to. compiled by the
 * runner's own toolchain, and -O0 so every frame is the one the source says. */

#include <stdint.h>
#include <unwind.h>

/* a mach #[symbol] name is the literal object symbol, and darwin's C compiler
 * prefixes an underscore to every C name, so the label makes C ask for the literal one */
#ifdef __APPLE__
#define MACH_SYM(name) __asm__(#name)
#else
#define MACH_SYM(name)
#endif

extern long long unwind_outer(long long depth) MACH_SYM(unwind_outer);
extern long long unwind_inner(long long depth) MACH_SYM(unwind_inner);

#define MAX_FRAMES 16

static uintptr_t starts[MAX_FRAMES];
static int count;

static _Unwind_Reason_Code record(struct _Unwind_Context *ctx, void *arg) {
    (void)arg;
    if (count < MAX_FRAMES) {
        starts[count] = _Unwind_GetRegionStart(ctx);
        count = count + 1;
    }
    return _URC_NO_REASON;
}

void *probe_callback(void) {
    return (void *)record;
}

/* the frame the walk reached each mach function in, innermost first, or -1 */
static long long position(uintptr_t start) {
    for (int i = 0; i < count; i++) {
        if (starts[i] == start) return i;
    }
    return -1;
}

void probe_run(long long *out) {
    count = 0;
    unwind_outer(0);
    out[0] = position((uintptr_t)unwind_inner);
    out[1] = position((uintptr_t)unwind_outer);
}
