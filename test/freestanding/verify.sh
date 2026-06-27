#!/usr/bin/env bash
# build the freestanding x86_64 fixture as a raw flat image, verify the build
# bypassed the per-module .o round-trip, confirm the output is a container-free
# flat binary (not an ELF), then load and run it to assert its exit code. proves
# an `os=freestanding, of=raw` program builds end to end to a working flat image
# (mach#1616): codegen -> in-memory image link -> raw emit_exec.
#
# usage: verify.sh [path-to-mach]   (defaults to `mach` on PATH)
#
# requires: a C compiler (cc) for the loader harness. the harness mmaps the flat
# image into an executable page and calls it; the image is position-independent
# and exits via a raw `exit` syscall, so it runs at any load address and the
# harness process exits with the image's computed code.
set -euo pipefail

# the exit code the fixture computes (sum 0..9) and exits with.
expect_code=45

mach="${1:-mach}"
here="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$here"

fail() { echo "FAIL: $1" >&2; exit 1; }

target="freestanding-x86_64"
img="out/$target/rawprobe"

echo "building flat image for $target with $mach"
rm -rf out
"$mach" build . --target "$target" --profile debug

[ -f "$img" ] || fail "flat image was not produced at $img"

# a flat-image build links straight from the in-memory codegen images, so no
# per-module relocatable object is ever written.
if find out -name '*.o' | grep -q .; then
    fail "a .o was written - the per-module round-trip was not bypassed"
fi

# the image is a container-free flat binary: it must not carry an ELF header.
magic="$(head -c4 "$img" | od -An -tx1 | tr -d ' \n')"
[ "$magic" != "7f454c46" ] || fail "output is an ELF, not a flat image"

# the first byte is the entry function's code (the image is entered at its base).
[ -s "$img" ] || fail "flat image is empty"

echo "loading and running the flat image"
work="$(mktemp -d)"
trap 'rm -rf "$work"' EXIT
cat > "$work/harness.c" <<'EOF'
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
int main(int argc, char** argv) {
    FILE* f = fopen(argv[1], "rb");
    if (!f) { perror("open"); return 2; }
    fseek(f, 0, SEEK_END); long n = ftell(f); fseek(f, 0, SEEK_SET);
    void* p = mmap(NULL, n, PROT_READ | PROT_WRITE | PROT_EXEC,
                   MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (p == MAP_FAILED) { perror("mmap"); return 2; }
    if (fread(p, 1, n, f) != (size_t)n) { return 2; }
    fclose(f);
    ((void (*)(void))p)();
    return 0; /* unreachable: the flat image exits via syscall */
}
EOF
cc -O2 -o "$work/harness" "$work/harness.c" || fail "could not build the loader harness"

set +e
"$work/harness" "$img"
code=$?
set -e
[ "$code" -eq "$expect_code" ] || fail "exit code $code, expected $expect_code"

echo "OK: os=freestanding, of=raw builds a working flat image that runs to exit code $expect_code"
