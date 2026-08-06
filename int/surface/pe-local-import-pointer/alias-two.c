__declspec(dllimport) unsigned local_add(unsigned value);

__declspec(noinline) unsigned qz_indirect_two(unsigned value) {
    return local_add(value) + 0x12345678;
}
