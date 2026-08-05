__declspec(noinline) unsigned local_add(unsigned value) {
    return value + 35;
}

__declspec(noinline) unsigned qz_direct(unsigned value) {
    return local_add(value) + 2;
}
