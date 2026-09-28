/* the foreign C object the case links, compiled by clang for Mach-O with debug
   info and unwind tables left on - the shape of every normally-compiled clang
   object, whose __compact_unwind and __debug_* references into __text are
   section-based relocations (briar-systems/mach#2521). The three immediate
   stores make clang emit X86_64_RELOC_SIGNED_1, _2, and _4 respectively; the
   initialized values give the structural test independent target bytes for
   each resolved displacement (briar-systems/mach#2546).

   regenerate (from this directory, any host clang):
     clang -target x86_64-apple-macos11 -c shim.c -o shim.o -g -O1 \
           -fdebug-compilation-dir=. */

static volatile unsigned char signed1_target = 0xa5;
static volatile unsigned short signed2_target = 0xbeef;
static volatile unsigned int signed4_target = 0xcafebabe;

int mad_device_open(int a) {
    signed1_target = 1;
    signed2_target = 0x1234;
    signed4_target = 0x12345678;
    return a + signed1_target + signed2_target + signed4_target;
}

/* nothing calls it: the object is built with subsections via symbols, so the
   link collects this function, its unwind entry and what its debug info says
   of it (briar-systems/mach#3410). its multiply by 0x3410 is the marker the
   producer looks for */
int mad_device_unused(int a) {
    return a * 0x3410 + 0x77;
}
