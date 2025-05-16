#include "ising.h"
#include "xoshiro256plus.h"


#include <immintrin.h>
#include <math.h>
#include <omp.h>
#include <stdbool.h>
#include <stddef.h>


static float exp_table[32];

static void init_exp_table(const float temp) {
  exp_table[(-8) + 8] = expf(-8.0f / temp);
  exp_table[(-4) + 8] = expf(-4.0f / temp);
  exp_table[(0) + 8] = expf(0.0f / temp);
  exp_table[(4) + 8] = expf(4.0f / temp);
  exp_table[(8) + 8] = expf(8.0f / temp);
}

static void update_red_black_grid(int (*write_grid)[L],
                                  const int (*read_grid)[L],
                                  bool invert_aux_mask) {
#pragma omp parallel for
  for (size_t i = 0; i < L / 2; ++i) {
    size_t tid = omp_get_thread_num();
    for (size_t j = 0; j < L; j += 8) {
      // Load 8 spins
      __m256i spin = _mm256_loadu_si256((__m256i *)&write_grid[i][j]);
      // Neighbor north
      __m256i neigh_n =
          _mm256_loadu_si256((__m256i *)&read_grid[(i - 1) % (L / 2)][j]);
      // Neighbor south
      __m256i neigh_s =
          _mm256_loadu_si256((__m256i *)&read_grid[(i + 1) % (L / 2)][j]);
      // Neighbor medium (to avoid moving the elements in the animation)
      __m256i neigh_medium = _mm256_loadu_si256((__m256i *)&read_grid[i][j]);
      if (invert_aux_mask) {
        neigh_n = _mm256_blend_epi32(neigh_n, neigh_medium, 0xAA);
        neigh_s = _mm256_blend_epi32(neigh_s, neigh_medium, 0x55);
      } else {
        neigh_n = _mm256_blend_epi32(neigh_n, neigh_medium, 0x55);
        neigh_s = _mm256_blend_epi32(neigh_s, neigh_medium, 0xAA);
      }
      // Neighbor east and west
      int spins_e[8];
      int spins_w[8];
      for (int k = 0; k < 8; k++) {
        int cur = j + k;
        spins_e[k] = read_grid[i][(cur + 1) % L];
        spins_w[k] = read_grid[i][(cur - 1 + L) % L];
      }
      __m256i neigh_e = _mm256_loadu_si256((__m256i *)spins_e);
      __m256i neigh_w = _mm256_loadu_si256((__m256i *)spins_w);

      // Sum of neighbors
      __m256i neigh_sum = _mm256_add_epi32(neigh_n, neigh_e);
      neigh_sum = _mm256_add_epi32(neigh_sum, neigh_w);
      neigh_sum = _mm256_add_epi32(neigh_sum, neigh_s);

      // delta_E = 2 * spin_old * sum_neighbors
      __m256i delta_E = _mm256_mullo_epi32(spin, neigh_sum);
      delta_E = _mm256_slli_epi32(delta_E, 1); // Multiply by 2

      float probs[8];
      for (int k = 0; k < 8; k++) {
        probs[k] = optimized_random_probability(tid);
      }
      // Load probabilities
      __m256 p_vec = _mm256_loadu_ps(probs);

      // Extract delta_E values and load corresponding exp_table values
      int delta_values[8];
      _mm256_storeu_si256((__m256i *)delta_values, delta_E);

      float exp_values[8];
      for (int k = 0; k < 8; k++) {
        exp_values[k] = exp_table[-delta_values[k] + 8];
      }
      __m256 exp_vec = _mm256_loadu_ps(exp_values);

      // Compare probabilities with exp_table values
      // p > exp_table[-delta_E + 8]
      __m256i mask_2 = _mm256_cmpgt_epi32(_mm256_castps_si256(p_vec),
                                          _mm256_castps_si256(exp_vec));

      // delta_E > 0
      __m256i mask_1 = _mm256_cmpgt_epi32(delta_E, _mm256_setzero_si256());

      // Combine masks (delta_E > 0 AND p > exp_table[-delta_E + 8])
      // This will be true for spins that should not be flipped
      __m256i mask = _mm256_and_si256(mask_1, mask_2);
      // Invert mask to get the indices of spins to flip
      mask = _mm256_xor_si256(mask, _mm256_set1_epi32(-1));

      // Blend the spins based on the mask
      __m256i neg_spin = _mm256_mullo_epi32(spin, _mm256_set1_epi32(-1));
      __m256i new_values = _mm256_blendv_epi8(spin, neg_spin, mask);
      // Store the new values back to the grid
      _mm256_storeu_si256((__m256i *)&write_grid[i][j], new_values);
    }
  }
}

static int calculate_red_black_grid(const int (*red_grid)[L],
                                    const int (*black_grid)[L], int *M_max,
                                    bool invert_aux_mask) {
  int E = 0;
  int M_local = 0;
  __m256i E_vec = _mm256_setzero_si256();
  __m256i M_vec = _mm256_setzero_si256();

  for (size_t i = 0; i < L / 2; ++i) {
    for (size_t j = 0; j < L; j += 8) {
      // Load red spin
      __m256i spin = _mm256_loadu_si256((__m256i *)&red_grid[i][j]);
      // Neighbor north
      __m256i neigh_n =
          _mm256_loadu_si256((__m256i *)&black_grid[(i - 1) % (L / 2)][j]);
      // Neighbor south
      __m256i neigh_s =
          _mm256_loadu_si256((__m256i *)&black_grid[(i + 1) % (L / 2)][j]);
      // Neighbor medium (to avoid moving the elements in the animation)
      __m256i neigh_medium = _mm256_loadu_si256((__m256i *)&black_grid[i][j]);
      if (invert_aux_mask) {
        neigh_n = _mm256_blend_epi32(neigh_n, neigh_medium, 0xAA);
        neigh_s = _mm256_blend_epi32(neigh_s, neigh_medium, 0x55);
      } else {
        neigh_n = _mm256_blend_epi32(neigh_n, neigh_medium, 0x55);
        neigh_s = _mm256_blend_epi32(neigh_s, neigh_medium, 0xAA);
      }
      // Neighbor east and west
      int spins_e[8];
      int spins_w[8];
      for (int k = 0; k < 8; k++) {
        int cur = j + k;
        spins_e[k] = black_grid[i][(cur + 1) % L];
        spins_w[k] = black_grid[i][(cur - 1 + L) % L];
      }
      __m256i neigh_e = _mm256_loadu_si256((__m256i *)spins_e);
      __m256i neigh_w = _mm256_loadu_si256((__m256i *)spins_w);

      // Sum of neighbors
      __m256i neigh_sum = _mm256_add_epi32(neigh_n, neigh_e);
      neigh_sum = _mm256_add_epi32(neigh_sum, neigh_w);
      neigh_sum = _mm256_add_epi32(neigh_sum, neigh_s);

      // spin * sum of neighbors
      __m256i energy_contrib = _mm256_mullo_epi32(spin, neigh_sum);
      E_vec = _mm256_add_epi32(E_vec, energy_contrib);

      // Accumulate magnetization
      M_vec = _mm256_add_epi32(M_vec, spin);
    }
  }

  // Horizontal sum of vector E_vec and M_vec
  int energy_array[8], magnet_array[8];
  _mm256_storeu_si256((__m256i *)energy_array, E_vec);
  _mm256_storeu_si256((__m256i *)magnet_array, M_vec);

  for (int k = 0; k < 8; ++k) {
    E += energy_array[k];
    M_local += magnet_array[k];
  }

  *M_max += M_local;
  return E;
}

void update(const float temp, int (*red_grid)[L], int (*black_grid)[L]) {

  // Only initialized on first call
  static float last_temp = -99.99f;

  if (temp != last_temp) {
    init_exp_table(temp);
    last_temp = temp;
  }

  update_red_black_grid(black_grid, red_grid, true);
  update_red_black_grid(red_grid, black_grid, false);
}


float calculate(int (*red_grid)[L], int (*black_grid)[L], int *M_max) {
  int E = 0;

  E += calculate_red_black_grid(black_grid, red_grid, M_max, true);
  E += calculate_red_black_grid(red_grid, black_grid, M_max, false);

  return -((float)E / 2.0f);
}