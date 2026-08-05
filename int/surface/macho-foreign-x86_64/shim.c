/* the foreign C object the case links, compiled by clang for Mach-O with debug
   info and unwind tables left on - the shape of every normally-compiled clang
   object, whose __compact_unwind and __debug_* references into __text are
   section-based relocations (briar-systems/mach#2521).

   regenerate (from this directory, any host clang):
     clang -target x86_64-apple-macos11 -c shim.c -o shim.o -g -O1 \
           -fdebug-compilation-dir=. */

static const char msg[] = "hello";
static int counter;

static int helper(int a) { return a * 3; }

int mad_device_open(int a) {
    counter = counter + 1;
    return helper(a) + (int)msg[0] + counter;
}

const char *mad_msg(void) { return msg; }
