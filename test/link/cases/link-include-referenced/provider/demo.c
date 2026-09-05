/* a fixture-owned shared library (REM-141): a real cube root, computed by
 * Newton's method so this .so calls no libc routine of its own, giving the
 * consumer a genuine value to check rather than a bare constant. */
double demo_cbrt(double x) {
    double g = x > 1.0 ? x : 1.0;
    int i = 0;
    while (i < 60) {
        g = g - (g * g * g - x) / (3.0 * g * g);
        i = i + 1;
    }
    return g;
}
