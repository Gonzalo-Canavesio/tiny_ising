#include "ising.h"
#include "stdbool.h"
#include "xoshiro256plus.h"
#include <immintrin.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>

#define L_mask (L - 1)

static float exp_table[32];
__m256 exp_vec;

static void init_exp_table(const float temp) {
  exp_table[(-8) + 8] = expf(-8.0f / temp);
  exp_table[(-4) + 8] = expf(-4.0f / temp);
  exp_table[(0) + 8] = expf(0.0f / temp);
  exp_table[(4) + 8] = expf(4.0f / temp);
  exp_table[(8) + 8] = expf(8.0f / temp);
  exp_vec = _mm256_loadu_ps(exp_table);
}

int32_t sum_avx2_fast(__m256i vec) {
  __m128i low = _mm256_castsi256_si128(vec);
  __m128i high = _mm256_extracti128_si256(vec, 1);
  __m128i sum128 = _mm_add_epi32(low, high);
  sum128 = _mm_hadd_epi32(sum128, sum128);
  return _mm_extract_epi32(sum128, 0) + _mm_extract_epi32(sum128, 1);
}

void update(const float temp, int (*red)[L], int (*black)[L]) {
  static float last_temp = -99.99f;
  if (temp != last_temp) {
    init_exp_table(temp);
    last_temp = temp;
  }

  __m256i zero = _mm256_setzero_si256();
  __m256i two = _mm256_set1_epi32(2); //[2, 2, 2, 2, 2, 2, 2 ,2] = [2] * 8
  __m256i eight = _mm256_set1_epi32(8);

  const unsigned int Ldiv = L / 2;

  for (size_t i = 0; i < Ldiv; ++i) {

    // SPIN_OLD = BLACK
    for (size_t j = 0; j < L; j += 8) {
      //[spin_old[1...8]]
      __m256i spin_old = _mm256_loadu_si256((__m256i *)&red[i][j]);
      __m256i spin_new = _mm256_sub_epi32(zero, spin_old);

      int idx_e[8];
      int idx_w[8];
      float p_array[8];
      for (size_t k = 0; k < 8; k++) {
        int cur = j + k;
        // TODO: this could be L_mask
        idx_e[k] = (cur + 1) % L;
        idx_w[k] = (cur - 1 + L) % L;
        p_array[k] = optimized_random_probability();
      }
      __m256 p_vec = _mm256_loadu_ps(p_array);
      // Load these index arrays into SIMD registers.
      __m256i idx_e_vec = _mm256_loadu_si256((__m256i *)idx_e);
      __m256i idx_w_vec = _mm256_loadu_si256((__m256i *)idx_w);

      int *ptr = &black[i][0];
      int *ptr_s = &black[(i + 1) % Ldiv][0];
      int *ptr_n = &black[(i + Ldiv - 1) % Ldiv][0];

      // Estos los puedo levantar directamente porque estan en la misma fila
      // Es decir la operacion modulo ya me deja en la fila correcta para
      // poder levantarlos consecutivamente
      __m256i N = _mm256_loadu_si256((__m256i *)&ptr_n[j]);
      __m256i M = _mm256_loadu_si256((__m256i *)&ptr[j]);
      __m256i S = _mm256_loadu_si256((__m256i *)&ptr_s[j]);

      /* __m256i N = _mm256_i32gather_epi32(ptr_n, idx_vec, sizeof(int)); */
      /* __m256i M = _mm256_i32gather_epi32(ptr, idx_vec, sizeof(int)); */
      /* __m256i S = _mm256_i32gather_epi32(ptr_s, idx_vec, sizeof(int)); */

      // Replace even-indexed elements of N with M's even elements
      __m256i n_vals = _mm256_blend_epi32(N, M, 0x55); // Mask: 01010101
      // Replace odd-indexed elements of S with M's odd elements
      __m256i s_vals = _mm256_blend_epi32(S, M, 0xAA); // Mask: 10101010

      __m256i w_vals = _mm256_i32gather_epi32(ptr, idx_w_vec, sizeof(int));
      __m256i e_vals = _mm256_i32gather_epi32(ptr, idx_e_vec, sizeof(int));

      // Sum neighbor values: sum = n + e + w + s.
      __m256i sum = _mm256_add_epi32(n_vals, e_vals);
      sum = _mm256_add_epi32(sum, w_vals);
      sum = _mm256_add_epi32(sum, s_vals);

      // Compute energy change: delta_E = 2 * spin_old * sum
      __m256i prod = _mm256_mullo_epi32(spin_old, sum);
      __m256i delta_E = _mm256_mullo_epi32(two, prod);

      //[0xF, 0, ...] 0xF si delta_E <= 0
      __m256i mask_GT = _mm256_cmpgt_epi32(zero, delta_E); // zero > delta_E
      __m256i mask_EQ = _mm256_cmpeq_epi32(zero, delta_E); // zero == delta_E
      __m256i mask_delta_LE_neg = _mm256_or_si256(mask_GT, mask_EQ);

      __m256i neg_delta_E = _mm256_sub_epi32(zero, delta_E);
      __m256i neg_delta_E_plus_8 = _mm256_add_epi32(neg_delta_E, eight);
      // exp_table[-delta_E + 8])  */
      __m256 exp_vec =
          _mm256_i32gather_ps(exp_table, neg_delta_E_plus_8, sizeof(float));
      // p <= exp_table[-delta_E + 8]
      __m256i mask_p_LE_exp =
          _mm256_castps_si256(_mm256_cmp_ps(p_vec, exp_vec, _CMP_LE_OS));

      __m256i mask = _mm256_or_si256(mask_p_LE_exp, mask_delta_LE_neg);
      // If mask has a 0xF on the i-element then it changes it to the new sping
      __m256i new_values = _mm256_blendv_epi8(spin_old, spin_new, mask);

      /*   grid[i][j] = spin_new; */
      _mm256_storeu_si256((__m256i *)&red[i][j], new_values);
    }

    // SPIN_OLD = BLACK
    for (size_t j = 0; j < L; j += 8) {
      __m256i spin_old = _mm256_loadu_si256((__m256i *)&black[i][j]);
      __m256i spin_new = _mm256_sub_epi32(zero, spin_old);

      int idx_e[8];
      int idx_w[8];
      float p_array[8];
      for (size_t k = 0; k < 8; k++) {
        int cur = j + k;
        idx_e[k] = (cur + 1) % L;
        idx_w[k] = (cur - 1 + L) % L;
        p_array[k] = optimized_random_probability();
      }
      __m256i idx_e_vec = _mm256_loadu_si256((__m256i *)idx_e);
      __m256i idx_w_vec = _mm256_loadu_si256((__m256i *)idx_w);
      __m256 p_vec = _mm256_loadu_ps(p_array);

      int *ptr = &red[i][0];
      int *ptr_s = &red[(i + 1) % Ldiv][0];
      int *ptr_n = &red[(i + Ldiv - 1) % Ldiv][0];

      __m256i N = _mm256_loadu_si256((__m256i *)&ptr_n[j]);
      __m256i M = _mm256_loadu_si256((__m256i *)&ptr[j]);
      __m256i S = _mm256_loadu_si256((__m256i *)&ptr_s[j]);

      // Aca la mascara es al revez que en el otro loop
      __m256i n_vals = _mm256_blend_epi32(N, M, 0xAA);
      __m256i s_vals = _mm256_blend_epi32(S, M, 0x55);

      __m256i w_vals = _mm256_i32gather_epi32(ptr, idx_w_vec, sizeof(int));
      __m256i e_vals = _mm256_i32gather_epi32(ptr, idx_e_vec, sizeof(int));

      __m256i sum = _mm256_add_epi32(n_vals, e_vals);
      sum = _mm256_add_epi32(sum, w_vals);
      sum = _mm256_add_epi32(sum, s_vals);
      __m256i prod = _mm256_mullo_epi32(spin_old, sum);
      __m256i delta_E = _mm256_mullo_epi32(two, prod);

      __m256i mask_GT = _mm256_cmpgt_epi32(zero, delta_E); // zero > delta_E
      __m256i mask_EQ = _mm256_cmpeq_epi32(zero, delta_E); // zero == delta_E
      __m256i mask_delta_LE_neg = _mm256_or_si256(mask_GT, mask_EQ);

      __m256i neg_delta_E = _mm256_sub_epi32(zero, delta_E);
      __m256i neg_delta_E_plus_8 = _mm256_add_epi32(neg_delta_E, eight);
      __m256 exp_vec =
          _mm256_i32gather_ps(exp_table, neg_delta_E_plus_8, sizeof(float));
      __m256i mask_p_LE_exp =
          _mm256_castps_si256(_mm256_cmp_ps(p_vec, exp_vec, _CMP_LE_OS));

      __m256i mask = _mm256_or_si256(mask_p_LE_exp, mask_delta_LE_neg);
      __m256i new_values = _mm256_blendv_epi8(spin_old, spin_new, mask);

      _mm256_storeu_si256((__m256i *)&black[i][j], new_values);
    }
  }
}

float calculate(int (*red)[L], int (*black)[L], int *M_max) {
  int E = 0;
  __m256i E_vec = _mm256_setzero_si256();
  __m256i M_max_vec = _mm256_setzero_si256();
  int Ldiv = L / 2;
  for (size_t i = 0; i < L; ++i) {
    for (size_t j = 0; j < L / 2; j += 8) {
      __m256i spin = _mm256_loadu_si256((__m256i *)&red[i][j]);

      int idx_e[8];
      int idx_w[8];
      for (size_t k = 0; k < 8; k++) {
        int cur = j + k;
        idx_e[k] = (cur + 1) % L;
        idx_w[k] = (cur - 1 + L) % L;
      }
      __m256i idx_e_vec = _mm256_loadu_si256((__m256i *)idx_e);
      __m256i idx_w_vec = _mm256_loadu_si256((__m256i *)idx_w);

      int *ptr = &black[i][0];
      int *ptr_s = &red[(i + 1) % Ldiv][0];
      int *ptr_n = &red[(i + Ldiv - 1) % Ldiv][0];

      __m256i N = _mm256_loadu_si256((__m256i *)&ptr_n[j]);
      __m256i M = _mm256_loadu_si256((__m256i *)&ptr[j]);
      __m256i S = _mm256_loadu_si256((__m256i *)&ptr_s[j]);

      // Aca la mascara es al revez que en el otro loop
      __m256i n_vals = _mm256_blend_epi32(N, M, 0xAA);
      __m256i s_vals = _mm256_blend_epi32(S, M, 0x55);

      __m256i w_vals = _mm256_i32gather_epi32(ptr, idx_w_vec, sizeof(int));
      __m256i e_vals = _mm256_i32gather_epi32(ptr, idx_e_vec, sizeof(int));

      __m256i sum = _mm256_add_epi32(n_vals, e_vals);
      sum = _mm256_add_epi32(sum, w_vals);
      sum = _mm256_add_epi32(sum, s_vals);
      __m256i prod = _mm256_mullo_epi32(spin, sum);
      E_vec = _mm256_add_epi32(E_vec, prod);

      M_max_vec = _mm256_add_epi32(M_max_vec, spin);
    }
    for (unsigned int j = 0; j < L / 2; j += 8) {
      __m256i spin = _mm256_loadu_si256((__m256i *)&black[i][j]);

      int idx_e[8];
      int idx_w[8];
      for (size_t k = 0; k < 8; k++) {
        int cur = j + k;
        idx_e[k] = (cur + 1) % L;
        idx_w[k] = (cur - 1 + L) % L;
      }
      __m256i idx_e_vec = _mm256_loadu_si256((__m256i *)idx_e);
      __m256i idx_w_vec = _mm256_loadu_si256((__m256i *)idx_w);

      int *ptr = &red[i][0];
      int *ptr_s = &red[(i + 1) % Ldiv][0];
      int *ptr_n = &red[(i + Ldiv - 1) % Ldiv][0];

      __m256i N = _mm256_loadu_si256((__m256i *)&ptr_n[j]);
      __m256i M = _mm256_loadu_si256((__m256i *)&ptr_s[j]);
      __m256i S = _mm256_loadu_si256((__m256i *)&ptr[j]);

      // Aca la mascara es al revez que en el otro loop
      __m256i n_vals = _mm256_blend_epi32(N, M, 0xAA);
      __m256i s_vals = _mm256_blend_epi32(S, M, 0x55);

      __m256i w_vals = _mm256_i32gather_epi32(ptr, idx_w_vec, sizeof(int));
      __m256i e_vals = _mm256_i32gather_epi32(ptr, idx_e_vec, sizeof(int));

      __m256i sum = _mm256_add_epi32(n_vals, e_vals);
      sum = _mm256_add_epi32(sum, w_vals);
      sum = _mm256_add_epi32(sum, s_vals);
      __m256i prod = _mm256_mullo_epi32(spin, sum);
      E_vec = _mm256_add_epi32(E_vec, prod);

      M_max_vec = _mm256_add_epi32(M_max_vec, spin);
    }
  }

  E = sum_avx2_fast(E_vec);
  *M_max += sum_avx2_fast(M_max_vec);

  return -((float)E / 2.0f);
}