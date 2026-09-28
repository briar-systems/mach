extern int qz_helper(int value);

__declspec(noinline) int qz_answer(void) {
    return qz_helper(40) + 2;
}

__declspec(noinline) int qz_unused(int value) {
    return qz_helper(value * 3) + 17;
}
