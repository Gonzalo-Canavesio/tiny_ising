#include <stdint.h>
#include <immintrin.h>

__m256 optimized_random_probability(size_t tid);

void seed(uint64_t seed);
