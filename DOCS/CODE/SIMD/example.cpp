#include <immintrin.h>
#include <iostream>
// Time related headers
#include <chrono>

float *add(const float *a, const float *b, size_t size)
{
    float *result = new float[size];
    const auto numof_vectorizable_elements = size - (size % 4);
    unsigned i = 0;
    for (; i < numof_vectorizable_elements; i += 4)
    {
        __m128 a_reg = _mm_loadu_ps(a + i);
        __m128 b_reg = _mm_loadu_ps(b + i);
        __m128 sum = _mm_add_ps(a_reg, b_reg);
        _mm_storeu_ps(result + i, sum);
    }
    for (; i < size; ++i)
        result[i] = a[i] + b[i];
    return result;
}

float *normal_add(const float *a, const float *b, size_t size)
{
    float *result = new float[size];
    for (unsigned i = 0; i < size; ++i)
        result[i] = a[i] + b[i];
    return result;
}

int main()
{
    const size_t size = 8;
    float a[size] = {1, 2, 3, 4, 5, 6, 7, 8};
    float b[size] = {1, 2, 3, 4, 5, 6, 7, 8};
    auto start = std::chrono::high_resolution_clock::now();
    float *result = add(a, b, size);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Elapsed time: " << elapsed.count() << " s\n";
    std::cout << std::endl;
    auto start2 = std::chrono::high_resolution_clock::now();
    float *result2 = normal_add(a, b, size);
    auto end2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed2 = end2 - start2;
    std::cout << "Elapsed time: " << elapsed2.count() << " s\n";
    delete[] result;
    return 0;
}