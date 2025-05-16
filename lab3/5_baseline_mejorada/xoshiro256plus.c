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

#include <immintrin.h>
#include <stdint.h>

/* The current state of the generators. */
static __m256i s[4][2];

static inline __m256i rotl(const __m256i x, int k) {
  __m256i low = _mm256_slli_epi64(x, k);
  __m256i high = _mm256_srli_epi64(x, 64 - k);
  return _mm256_or_si256(low, high);
}

static inline void next(__m256i out[2]) {

  out[0] = _mm256_add_epi64(s[0][0], s[3][0]);
  out[1] = _mm256_add_epi64(s[0][1], s[3][1]);

  __m256i t[2];
  t[0] = _mm256_slli_epi64(s[1][0], 17);
  t[1] = _mm256_slli_epi64(s[1][1], 17);

  s[2][0] = _mm256_xor_si256(s[2][0], s[0][0]);
  s[2][1] = _mm256_xor_si256(s[2][1], s[0][1]);
  s[3][0] = _mm256_xor_si256(s[3][0], s[1][0]);
  s[3][1] = _mm256_xor_si256(s[3][1], s[1][1]);
  s[1][0] = _mm256_xor_si256(s[1][0], s[2][0]);
  s[1][1] = _mm256_xor_si256(s[1][1], s[2][1]);
  s[0][0] = _mm256_xor_si256(s[0][0], s[3][0]);
  s[0][1] = _mm256_xor_si256(s[0][1], s[3][1]);
  s[2][0] = _mm256_xor_si256(s[2][0], t[0]);
  s[2][1] = _mm256_xor_si256(s[2][1], t[1]);

  s[3][0] = rotl(s[3][0], 45);
  s[3][1] = rotl(s[3][1], 45);
}

__m256 optimized_random_probability(void) {
  __m256i random_numbers[2];
  next(random_numbers);

  // Shift right by 40 bits on each 64-bit integer
  __m256i r1 = _mm256_srli_epi64(random_numbers[1], 40);
  __m256i r2 = _mm256_srli_epi64(random_numbers[0], 40);

  // Pack the 64-bit integers into 32-bit integers
  __m256i combined = _mm256_castps_si256(
      _mm256_shuffle_ps(_mm256_castsi256_ps(r1), _mm256_castsi256_ps(r2),
                        _MM_SHUFFLE(2, 0, 2, 0)));

  // Convert to float and multiply by the scaling factor
  __m256 result = _mm256_cvtepi32_ps(combined);
  return _mm256_mul_ps(result, _mm256_set1_ps(0x1.0p-24f));
}

void seed(uint64_t seed) {
  // Initialize the state with the seed
  uint64_t state[4][8];
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 8; j++) {
      seed += 0x9e3779b97f4a7c15;
      uint64_t z = seed;
      z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9;
      z = (z ^ (z >> 27)) * 0x94d049bb133111eb;
      state[i][j] = z ^ (z >> 31);
    }
  }
  // Load the state into the s array
  for (int i = 0; i < 4; i++) {
    s[i][0] = _mm256_loadu_si256((__m256i *)&state[i][0]);
    s[i][1] = _mm256_loadu_si256((__m256i *)&state[i][4]);
  }
}
