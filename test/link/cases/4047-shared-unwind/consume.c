/* the C consumer of the shared-unwind case (mach #4047): walks its own stack
 * from inside the library with libgcc's unwinder and reports the frame each
 * function was found in, innermost first */
#include <stdio.h>
#include <unwind.h>

extern long long lib_run(void *walker);
extern void lib_addrs(unsigned long long *out);

#define MAX_FRAMES 32

static unsigned long long starts[MAX_FRAMES];
static int count;

static _Unwind_Reason_Code record(struct _Unwind_Context *ctx, void *arg) {
    (void)arg;
    if (count < MAX_FRAMES) {
        starts[count] = _Unwind_GetRegionStart(ctx);
        count = count + 1;
    }
    return _URC_NO_REASON;
}

__attribute__((noinline)) static void walk(void) {
    count = 0;
    _Unwind_Backtrace(record, 0);
    __asm__ volatile("" ::: "memory");
}

static long long position(unsigned long long start) {
    for (int i = 0; i < count; i++) {
        if (starts[i] == start) return i;
    }
    return -1;
}

int main(void) {
    unsigned long long at[6];
    lib_run((void *)walk);
    lib_addrs(at);
    printf("walker %lld, probe_walk %lld, inner %lld, dwarf %lld, middle %lld, outer %lld, run %lld, main %lld\n",
        position((unsigned long long)walk), position(at[0]), position(at[1]), position(at[2]),
        position(at[3]), position(at[4]), position(at[5]), position((unsigned long long)main));
    return 0;
}
