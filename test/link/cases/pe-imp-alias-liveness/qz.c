__declspec(dllimport) unsigned long AliasProbe(void);

__declspec(noinline) unsigned long alias_probe_indirect(void) {
    return AliasProbe();
}
