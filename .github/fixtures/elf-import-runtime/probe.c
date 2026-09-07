extern int got_object;
extern int got_value(void);
int got_probe(void) {
    int (*volatile pointer)(void) = got_value;
    int direct = got_value();
    int indirect = pointer();
    return direct == 73 && indirect == 73 && got_object == 19;
}
