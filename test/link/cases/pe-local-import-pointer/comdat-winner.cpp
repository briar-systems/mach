extern "C" __declspec(noinline) inline unsigned qz_select_any(unsigned value) {
    return value + 9;
}

extern "C" unsigned qz_keep_winner(unsigned value) {
    return qz_select_any(value);
}
