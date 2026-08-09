/* Two objects each contribute to the same named sections, so the linked image
 * must CONCATENATE them in order rather than emit two sections sharing a name:
 * libobjc walks __objc_classlist as one contiguous pointer array (#2606).
 *
 * Rebuild (checked in so the case needs no macOS SDK or clang on the runner):
 *   clang -target x86_64-apple-macos11 -fno-common -fno-unwind-tables -fno-asynchronous-unwind-tables -c probe-a.c -o probe-a.o
 */
__attribute__((used, section("__DATA,__objc_classlist")))
static const void *const classlist_a = (const void *)0x2606;

__attribute__((used, section("__DATA,__objc_imageinfo")))
static const unsigned imageinfo[2] = {0, 64};

/* non-zero so it lands in __DATA,__data: this case is about named sections, not
 * about how a zero-initialized global is spelled. */
int probe_initialized = -1;

static void probe_init(void) { probe_initialized = 1; }

/* dyld runs every pointer in __DATA,__mod_init_func before main. the same defect
 * loses this section, and losing it fails SILENTLY - the program runs with the
 * initializer never called. */
__attribute__((used, section("__DATA,__mod_init_func")))
static void (*const init_entry)(void) = probe_init;

int probe_state(void) { return probe_initialized; }
