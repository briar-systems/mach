extern int qz_helper(int value);

__declspec(noinline) int qz_answer(void) {
    return qz_helper(40) + 2;
}
