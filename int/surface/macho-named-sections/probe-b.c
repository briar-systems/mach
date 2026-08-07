/* the second contributor to __DATA,__objc_classlist; see probe-a.c.
 *   clang -target x86_64-apple-macos11 -fno-common -fno-unwind-tables -fno-asynchronous-unwind-tables -c probe-b.c -o probe-b.o
 */
__attribute__((used, section("__DATA,__objc_classlist")))
static const void *const classlist_b = (const void *)0x260b;
