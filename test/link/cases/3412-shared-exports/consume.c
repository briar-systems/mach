/* a real consumer of the built library: it names the one exported entry point by
   its `#[symbol]` name and prints what it returns, so the case proves the export
   is callable and not merely listed. */
#include <stdio.h>

extern long case_add(long a, long b);

int main(void) {
    printf("case_add=%ld\n", case_add(4, 5));
    return 0;
}
