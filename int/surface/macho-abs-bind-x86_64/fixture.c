/* Absolute 64-bit references to a libSystem data import, the shape ObjC class
   metadata uses for runtime symbols (#2563): no call, no GOT load - dyld must
   bind each pointer cell in place. Two slots at adjacent offsets prove each
   site gets its own bind row.

   Regenerate from this directory with:
     clang -target x86_64-apple-macos11 -c fixture.c -o fixture.o -O1 -fPIC \
           -fno-stack-protector -fno-asynchronous-unwind-tables */

extern void *__stdoutp;
extern void *__stderrp;

void **mach_abs_bind_out = &__stdoutp;
void **mach_abs_bind_err = &__stderrp;

int mach_abs_bind_check(void) {
    return mach_abs_bind_out == &__stdoutp && *mach_abs_bind_out == __stdoutp
        && mach_abs_bind_err == &__stderrp && *mach_abs_bind_err == __stderrp;
}
