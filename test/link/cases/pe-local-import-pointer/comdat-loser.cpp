extern "C" __declspec(dllimport) unsigned dead_local(unsigned value);

extern "C" __declspec(noinline) inline unsigned qz_select_any(unsigned value) {
    return dead_local(value);
}

extern "C" unsigned qz_keep_loser(unsigned value) {
    return qz_select_any(value);
}
