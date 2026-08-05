extern int codeview_helper(int value);

__declspec(noinline) int codeview_answer(void) {
    return codeview_helper(40) + 2;
}
