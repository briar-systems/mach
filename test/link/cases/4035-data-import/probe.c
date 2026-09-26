/* a shared library whose data the case imports. no libc, so it links the same on
   every leg without a target sysroot */
long probe_value = 7;
double probe_scale = 1.5;

long probe_get(void) { return probe_value; }
