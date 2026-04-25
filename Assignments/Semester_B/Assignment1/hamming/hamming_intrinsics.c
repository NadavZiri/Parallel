#include <emmintrin.h> // SSE2
#include <smmintrin.h> // SSE4.1
#include <string.h>

#define MAX_STR 256

int max(int a, int b)
{
    return (a > b) ? a : b;
}

int min(int a, int b)
{
    return (a > b) ? b : a;
}

int hamming_dist(char str1[MAX_STR], char str2[MAX_STR])
{
    size_t len1 = 0, len2 = 0;
    len1 = strlen(str1);
    len2 = strlen(str2);
    __m128i vector1;
    __m128i vector2;
    __m128i result;
    size_t distance = 0;
    for (size_t i = 0; i < min(len1, len2); i += 16)
    {
        int remaining = min(len1, len2) - i;
        vector1 = _mm_loadu_si128((const __m128i *)(str1 + i));
        vector2 = _mm_loadu_si128((const __m128i *)(str2 + i));
        result = _mm_cmpeq_epi8(vector1, vector2);
        int f = _mm_movemask_epi8(result);
        if (remaining < 16)
        {
            f = f & ((1 << remaining) - 1);
        }
        distance += __builtin_popcount(f);
    }
    size_t final_dist = max(len1, len2) - distance;
    return final_dist;
}
