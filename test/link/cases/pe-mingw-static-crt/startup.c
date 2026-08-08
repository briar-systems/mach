extern void __imp_GetStartupInfoA(void);

void WinMainCRTStartup(void) {
    __imp_GetStartupInfoA();
}
