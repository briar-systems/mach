__declspec(dllimport) unsigned local_add(unsigned value);

__declspec(noinline) unsigned qz_indirect(unsigned value) {
    return local_add(value) + 1;
}
