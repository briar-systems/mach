/* C side of the SysV float-aggregate C-FFI test. compiled with the system `cc`
 * to a loose object, then linked into the Mach consumer. these functions exchange
 * by-value structs containing floats, which the System V AMD64 psABI passes and
 * returns in SSE registers (XMM0/XMM1) per eightbyte. before the Mach compiler
 * classified float-bearing aggregate eightbytes as SSE, it passed/returned them in
 * GP registers, so every call below saw garbage and the consumer exited non-zero. */

typedef struct { double x; double y; } Vec2;   /* SSE,SSE  -> XMM0:XMM1        */
typedef struct { float  a; float  b; } F2;      /* one SSE eightbyte -> XMM0    */
typedef struct { long   i; double d; } Mixed;   /* INTEGER,SSE -> RDI:XMM0      */

double vec2_sum(Vec2 v)             { return v.x + v.y; }
Vec2   vec2_scale(Vec2 v, double s) { Vec2 r; r.x = v.x * s; r.y = v.y * s; return r; }
float  f2_sum(F2 f)                 { return f.a + f.b; }
double mixed_sum(Mixed m)           { return (double)m.i + m.d; }
