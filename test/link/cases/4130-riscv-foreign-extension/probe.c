/* C half of the foreign-extension link (mach #4130). Built with Zfh, so its
 * .riscv.attributes names zfh and its body computes in half-precision
 * instructions, neither of which mach generates. Values cross the boundary as
 * their bits, so the ABI is plain integers. */
typedef unsigned short u16;

static u16 bits(_Float16 x) {
    union { _Float16 f; u16 u; } c;
    c.f = x;
    return c.u;
}

static _Float16 mk(u16 u) {
    union { _Float16 f; u16 u; } c;
    c.u = u;
    return c.f;
}

u16 c_half_mul_add(u16 a, u16 b, u16 c) {
    _Float16 p = mk(a) * mk(b);
    return bits(p + mk(c));
}
