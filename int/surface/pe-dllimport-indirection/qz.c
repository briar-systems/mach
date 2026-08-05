__declspec(dllimport) void Sleep(unsigned long milliseconds);
__declspec(dllimport) unsigned long GetTickCount(void);

__declspec(noinline) unsigned long qz_indirect(void) {
    Sleep(7);
    return GetTickCount();
}
