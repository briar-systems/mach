/* flat_loader — load and run a freestanding raw flat image (the int/ flat-loader
 * producer's runner).
 *
 * usage: flat_loader <image>
 *
 * the image is an `os=freestanding, of=raw` build: a container-free binary entered
 * at its base, position-independent, that exits through a raw `exit` syscall. mmap
 * it into an executable page and call it; the call never returns (the image exits
 * the process with its computed code), so this process's exit status is the image's.
 * cc is the only dependency; the producer compiles this once and runs it per case. */

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

int main(int argc, char** argv) {
    if (argc != 2) { fprintf(stderr, "usage: flat_loader <image>\n"); return 2; }

    FILE* f = fopen(argv[1], "rb");
    if (!f) { perror("flat_loader: open"); return 2; }
    if (fseek(f, 0, SEEK_END) != 0) { perror("flat_loader: seek"); return 2; }
    long n = ftell(f);
    if (n <= 0) { fprintf(stderr, "flat_loader: empty image\n"); return 2; }
    rewind(f);

    void* p = mmap(NULL, (size_t)n, PROT_READ | PROT_WRITE | PROT_EXEC,
                   MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (p == MAP_FAILED) { perror("flat_loader: mmap"); return 2; }

    if (fread(p, 1, (size_t)n, f) != (size_t)n) {
        fprintf(stderr, "flat_loader: short read\n");
        return 2;
    }
    fclose(f);

    ((void (*)(void))p)();
    return 0; /* unreachable: the image exits via syscall */
}
