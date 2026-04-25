#include <xmmintrin.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pmmintrin.h>

#define MAX_STR 255

float formula1(float *x, unsigned int length)
{
    __m128 float1, float2;
    __m128 sum_vec = _mm_set1_ps(0.0f), prod_vec = _mm_set1_ps(1.0f);
    for (size_t i = 0; i < length; i += 4)
    {
        float1 = _mm_loadu_ps(x + i);
        float1 = _mm_sqrt_ps(float1);
        float2 = _mm_loadu_ps(x + i);
        float2 = _mm_mul_ps(float2, float2);
        __m128 ones = _mm_set1_ps(1.0f);
        float2 = _mm_add_ps(float2, ones);
        sum_vec = _mm_add_ps(sum_vec, float1);
        prod_vec = _mm_mul_ps(prod_vec, float2);
    }
    sum_vec = _mm_hadd_ps(sum_vec, sum_vec);
    sum_vec = _mm_hadd_ps(sum_vec, sum_vec);
    float sum = _mm_cvtss_f32(sum_vec);
    float prod_arr[4];
    _mm_storeu_ps(prod_arr, prod_vec);
    float product = prod_arr[0] * prod_arr[1] * prod_arr[2] * prod_arr[3];
    sum = cbrtf(sum);
    return sqrt(1 + (sum/product));
}
