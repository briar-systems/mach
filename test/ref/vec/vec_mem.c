#include "corpus.h"

/* f32x3 is 12 bytes aligned to 4 in mach, and a plain C struct of three floats has
 * exactly that size and alignment, so buf, Vertex, Odd and Mesh all match mach's
 * layout element for element and field for field.
 *
 * f32x5 is 20 bytes aligned to 16 in mach, which no C struct reproduces at once
 * because 20 is not a multiple of 16. the reference takes the size, so wbuf strides
 * by 20 as the lane-derived size says. Wide is value-faithful rather than
 * layout-faithful for the same reason: mach places w at 16 and C places it at 4.
 * the case only reads fields it wrote, so agreement here is agreement about values
 * and about mach being self-consistent. */
typedef struct { float l[3]; } f32x3;
typedef struct { float l[5]; } f32x5;

typedef struct { f32x3 pos; uint32_t id; } Vertex;
typedef struct { uint8_t tag; f32x3 v; uint8_t tail; } Odd;
typedef struct { uint32_t head; f32x3 tri[3]; uint32_t foot; } Mesh;
typedef struct { uint32_t lead; f32x5 w; uint32_t trail; } Wide;

static f32x3 v3add(f32x3 x, f32x3 y) {
    f32x3 r;
    for (unsigned i = 0; i < 3u; i++) { r.l[i] = x.l[i] + y.l[i]; }
    return r;
}

static f32x3 v3mul(f32x3 x, f32x3 y) {
    f32x3 r;
    for (unsigned i = 0; i < 3u; i++) { r.l[i] = x.l[i] * y.l[i]; }
    return r;
}

static f32x5 v5add(f32x5 x, f32x5 y) {
    f32x5 r;
    for (unsigned i = 0; i < 5u; i++) { r.l[i] = x.l[i] + y.l[i]; }
    return r;
}

static uint64_t fold3(uint64_t h, f32x3 v) {
    for (unsigned i = 0; i < 3u; i++) { h = mix_f32(h, v.l[i]); }
    return h;
}

static uint64_t fold5(uint64_t h, f32x5 v) {
    for (unsigned i = 0; i < 5u; i++) { h = mix_f32(h, v.l[i]); }
    return h;
}

uint64_t checksum(uint64_t seed) {
    uint64_t h = fold_init();
    const float z = (float)seed;

    f32x3 buf[7];
    for (uint64_t i = 0; i < UINT64_C(7); i = (uint64_t)(i + UINT64_C(1))) {
        const float fi = (float)i;
        f32x3 v;
        v.l[0] = fi + z;
        v.l[1] = fi - 1.25f;
        v.l[2] = 100.0f - fi;
        buf[i] = v;
    }
    for (uint64_t k = UINT64_C(7); k > UINT64_C(0); ) {
        k = (uint64_t)(k - UINT64_C(1));
        h = fold3(h, buf[k]);
    }

    Vertex verts[4];
    for (uint64_t j = 0; j < UINT64_C(4); j = (uint64_t)(j + UINT64_C(1))) {
        verts[j].pos = buf[j];
        verts[j].id = (uint32_t)(UINT32_C(2863311530) - (uint32_t)j);
    }
    for (uint64_t j = 0; j < UINT64_C(4); j = (uint64_t)(j + UINT64_C(1))) {
        h = fold3(h, verts[j].pos);
        h = mix_u32(h, verts[j].id);
    }

    Odd o;
    o.tag = 219;
    o.v = buf[5];
    o.tail = 173;
    h = mix_u8(h, o.tag);
    h = fold3(h, o.v);
    h = mix_u8(h, o.tail);

    Odd o2 = o;
    o2.v = v3add(o2.v, buf[1]);
    h = mix_u8(h, o2.tag);
    h = fold3(h, o2.v);
    h = mix_u8(h, o2.tail);
    h = mix_u8(h, o.tag);
    h = fold3(h, o.v);
    h = mix_u8(h, o.tail);

    Mesh mesh;
    mesh.head = UINT32_C(4294967295);
    mesh.tri[0] = buf[0];
    mesh.tri[1] = buf[3];
    mesh.tri[2] = buf[6];
    mesh.foot = UINT32_C(305419896);
    h = mix_u32(h, mesh.head);
    for (uint64_t t = 0; t < UINT64_C(3); t = (uint64_t)(t + UINT64_C(1))) {
        h = fold3(h, mesh.tri[t]);
    }
    h = mix_u32(h, mesh.foot);

    f32x3 pv = buf[2];
    f32x3 *p = &pv;
    *p = v3add(*p, buf[4]);
    h = fold3(h, pv);
    const f32x3 readback = *p;
    h = fold3(h, readback);

    f32x3 *q = &buf[3];
    const f32x3 scale = { { 2.0f, 4.0f, 8.0f } };
    *q = v3mul(*q, scale);
    h = fold3(h, buf[2]);
    h = fold3(h, buf[3]);
    h = fold3(h, buf[4]);

    f32x5 wbuf[5];
    for (uint64_t m = 0; m < UINT64_C(5); m = (uint64_t)(m + UINT64_C(1))) {
        const float fm = (float)m;
        f32x5 w;
        w.l[0] = fm + z;
        w.l[1] = fm - 1.25f;
        w.l[2] = 100.0f - fm;
        w.l[3] = fm + 7.5f;
        w.l[4] = 0.0f - fm;
        wbuf[m] = w;
    }
    for (uint64_t m = UINT64_C(5); m > UINT64_C(0); ) {
        m = (uint64_t)(m - UINT64_C(1));
        h = fold5(h, wbuf[m]);
    }

    Wide wide;
    wide.lead = UINT32_C(4294967295);
    wide.w = wbuf[3];
    wide.trail = UINT32_C(305419896);
    h = mix_u32(h, wide.lead);
    h = fold5(h, wide.w);
    h = mix_u32(h, wide.trail);

    f32x5 *wp = &wbuf[1];
    *wp = v5add(*wp, wbuf[4]);
    h = fold5(h, wbuf[0]);
    h = fold5(h, wbuf[1]);
    h = fold5(h, wbuf[2]);
    return h;
}
