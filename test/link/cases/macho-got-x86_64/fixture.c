/* Real clang Mach-O GOT_LOAD inputs for a defined data symbol and an imported
   libSystem data symbol. Keep PIC enabled: direct PC-relative lowering would no
   longer exercise the relocation contract this fixture guards.

   Regenerate from this directory with:
     clang -target x86_64-apple-macos11 -c fixture.c -o fixture.o -O1 -fPIC \
           -fno-stack-protector -fno-asynchronous-unwind-tables */

extern void *__stderrp;
extern int mach_got_local;

int got_load_local(void) {
    return mach_got_local;
}

int got_load_import(void) {
    return __stderrp != 0;
}
