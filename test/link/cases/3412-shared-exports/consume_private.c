/* the negative arm: the same library, a name that exists inside it but is not
   `pub`. the link must fail. */
extern long case_private(long n);

int main(void) { return (int)case_private(1); }
