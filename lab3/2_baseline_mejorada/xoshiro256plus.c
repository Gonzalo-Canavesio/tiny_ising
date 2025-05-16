/*  Written in 2019 by Sebastiano Vigna (vigna@acm.org)

To the extent possible under law, the author has dedicated all copyright
and related and neighboring rights to this software to the public domain
worldwide.

Permission to use, copy, modify, and/or distribute this software for any
purpose with or without fee is hereby granted.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES

WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE. */

#include <stdint.h>
#include <immintrin.h>

// You can play with this value on your architecture
#define XOSHIRO256_UNROLL (8)

/* The current state of the generators. */
static uint64_t s[4][XOSHIRO256_UNROLL];

static __inline uint64_t rotl(const uint64_t x, int k) {
	return (x << k) | (x >> (64 - k));
}

static inline uint64_t next(uint64_t * const restrict array, int len) {
 	uint64_t t[XOSHIRO256_UNROLL];

	for(int b = 0; b < len; b += XOSHIRO256_UNROLL) {
		for(int i = 0; i < XOSHIRO256_UNROLL; i++) array[b + i] = s[0][i] + s[3][i];

		for(int i = 0; i < XOSHIRO256_UNROLL; i++) t[i] = s[1][i] << 17;

		for(int i = 0; i < XOSHIRO256_UNROLL; i++) s[2][i] ^= s[0][i];
		for(int i = 0; i < XOSHIRO256_UNROLL; i++) s[3][i] ^= s[1][i];
		for(int i = 0; i < XOSHIRO256_UNROLL; i++) s[1][i] ^= s[2][i];
		for(int i = 0; i < XOSHIRO256_UNROLL; i++) s[0][i] ^= s[3][i];

		for(int i = 0; i < XOSHIRO256_UNROLL; i++) s[2][i] ^= t[i];

		for(int i = 0; i < XOSHIRO256_UNROLL; i++) s[3][i] = rotl(s[3][i], 45);
	}

	// This is just to avoid dead-code elimination
	return array[0] ^ array[len - 1];
}

__m256 optimized_random_probability(void) {
  uint64_t random_numbers[8];
  next(random_numbers, 8);
  
  // Load 64-bit integers into 256-bit vectors (4 values per vector)
  __m256i r_low = _mm256_loadu_si256((__m256i*)random_numbers);
  __m256i r_high = _mm256_loadu_si256((__m256i*)(random_numbers + 4));
  
  // Shift right by 40 bits on each 64-bit integer
  r_low = _mm256_srli_epi64(r_low, 40);
  r_high = _mm256_srli_epi64(r_high, 40);
  
  // Pack the 64-bit integers into 32-bit integers
  __m256i combined = _mm256_castps_si256(
      _mm256_shuffle_ps(
          _mm256_castsi256_ps(r_low),
          _mm256_castsi256_ps(r_high),
          _MM_SHUFFLE(2, 0, 2, 0)
      )
  );
  
  // Convert to float and multiply by the scaling factor
  __m256 result = _mm256_cvtepi32_ps(combined);
  return _mm256_mul_ps(result, _mm256_set1_ps(0x1.0p-24f));
}

void seed(uint64_t seed) {
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < XOSHIRO256_UNROLL; j++) {
      seed += 0x9e3779b97f4a7c15;
      uint64_t z = seed;
      z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9;
      z = (z ^ (z >> 27)) * 0x94d049bb133111eb;
      s[i][j] = z ^ (z >> 31);
    }
  }
}
