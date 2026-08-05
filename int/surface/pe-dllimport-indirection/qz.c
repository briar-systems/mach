__declspec(dllimport) void Sleep(unsigned long milliseconds);

__declspec(noinline) void qz_indirect(void) {
    Sleep(7);
}
