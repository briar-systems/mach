/* the definitions probe.c reaches through the GOT, in their own TU so the
   compiler cannot resolve the references directly */
int shared_counter = 7;

int shared_bump(void) { return ++shared_counter; }
