/* Every reference here is to a symbol this TU does not define, so -fPIC routes it
   through the GOT: the data loads become R_X86_64_REX_GOTPCRELX and, under
   -fno-plt, the call becomes R_X86_64_GOTPCRELX. The values are deliberately
   order-dependent so a GOT slot holding the wrong address cannot produce the
   expected transcript by accident. */
extern int shared_counter;
extern int shared_bump(void);

int read_counter(void) { return shared_counter; }

int add_counter(int n) { shared_counter += n; return shared_counter; }

int call_bump(void) { return shared_bump(); }
