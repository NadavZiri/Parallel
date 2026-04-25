/* 211388921 Nadav Ziri */
long decode_c_version(long x, long y, long z) {
    y = y - z;
    x = x * y;
    y = y << 63;
    y = y >> 63;
    y = y ^ x;
    return y;
};