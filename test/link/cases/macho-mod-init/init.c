/* A genuine dyld initializer: __DATA,__mod_init_func holds one pointer to a real
 * function, and dyld runs it before the program entry point.
 *
 * Nothing here is synthetic. The section that #2606's original probe carried
 * alongside this one - a fabricated __objc_classlist - could never be valid,
 * because a class list entry has to point at a real Objective-C class object and
 * the probe held recognizable marker integers instead. libobjc reads that list for
 * real on darwin, so an image carrying it faults before main (#2637). This fixture
 * carries only content a runtime can act on correctly.
 *
 * Rebuild (checked in so the case needs no macOS SDK or clang on the runner):
 *   clang -target x86_64-apple-macos11 -fno-common -fno-unwind-tables \
 *         -fno-asynchronous-unwind-tables -c init.c -o init.o
 */
int init_state = -1;

static void run_init(void) { init_state = 1; }

__attribute__((used, section("__DATA,__mod_init_func")))
static void (*const init_entry)(void) = run_init;

int init_probe(void) { return init_state; }
