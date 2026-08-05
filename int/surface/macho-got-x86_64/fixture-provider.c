/* Keep the local definition in a separate translation unit so fixture.c's PIC
   reference remains interposable and clang emits X86_64_RELOC_GOT_LOAD.

   Regenerate from this directory with:
     clang -target x86_64-apple-macos11 -c fixture-provider.c \
           -o fixture-provider.o -O1 -fPIC -fno-stack-protector \
           -fno-asynchronous-unwind-tables */

int mach_got_local = 40;
