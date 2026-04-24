long decode_c_version(long x, long y, long z) {
    y = y - z;
    x = x * y;
    x = -(x & 1);
    return x;
};