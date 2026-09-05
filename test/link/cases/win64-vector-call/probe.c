typedef int v4si __attribute__((vector_size(16)));
typedef short v4hi __attribute__((vector_size(8)));
typedef long long (*vec_op)(v4si);
typedef long long (*stack_op)(long long, long long, long long, long long, v4si);
typedef long long (*narrow_op)(v4hi);
typedef long long (*narrow_stack_op)(long long, long long, long long, long long, v4hi);
typedef struct { long long a; long long b; } pair;
typedef long long (*pair_op)(pair);

long long foreign_fold(v4si v) {
    return v[0] + v[1] * 3 + v[2] * 5 + v[3] * 7;
}

long long foreign_stack(long long a, long long b, long long c, long long d, v4si v) {
    return a + b + c + d + foreign_fold(v);
}

long long foreign_narrow(v4hi v) {
    return v[0] + v[1] * 3 + v[2] * 5 + v[3] * 7;
}

long long foreign_narrow_stack(long long a, long long b, long long c, long long d, v4hi v) {
    return a + b + c + d + foreign_narrow(v);
}

__declspec(noinline) vec_op select_vec(vec_op f) { return f; }
__declspec(noinline) stack_op select_stack(stack_op f) { return f; }
__declspec(noinline) narrow_op select_narrow(narrow_op f) { return f; }
__declspec(noinline) narrow_stack_op select_narrow_stack(narrow_stack_op f) { return f; }
__declspec(noinline) pair_op select_pair(pair_op f) { return f; }
