#include "ising.h"
#include "xoshiro256plus.h"


#include <immintrin.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>


static float exp_table[17];

static void init_exp_table(const float temp) {
  // Create vector of input values: [-8, -4, 0, 4, 8] / temp
  __m256 inputs = _mm256_set_ps(
      0.0f, // Padding with zeros in the upper elements
      0.0f, // Padding with zeros in the upper elements
      0.0f, // Padding with zeros in the upper elements
      8.0f / temp, 4.0f / temp, 0.0f / temp, -4.0f / temp, -8.0f / temp);

  // Compute exponentials of all values in parallel
  __m256 results = _mm256_exp_ps(inputs);

  // Store the results back to exp_table
  float temp_results[8];
  _mm256_storeu_ps(temp_results, results);

  // Copy results to exp_table with appropriate indices
  exp_table[(-8) + 8] = temp_results[0];
  exp_table[(-4) + 8] = temp_results[1];
  exp_table[(0) + 8] = temp_results[2];
  exp_table[(4) + 8] = temp_results[3];
  exp_table[(8) + 8] = temp_results[4];
}

static void update_red_black_grid(int (*write_grid)[2+L],
                                  const int (*read_grid)[2+L],
                                  bool invert_aux_mask) {
  for (size_t i = 1; i < 1+L / 2; ++i) {
    for (size_t j = 1; j < 1+L; j += 8) {
      // Load 8 spins
      __m256i spin = _mm256_loadu_si256((__m256i *)&write_grid[i][j]);
      // Neighbor north
      __m256i neigh_n =
          _mm256_loadu_si256((__m256i *)&read_grid[(i - 1)][j]);
      // Neighbor south
      __m256i neigh_s =
          _mm256_loadu_si256((__m256i *)&read_grid[(i + 1)][j]);
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
        spins_e[k] = read_grid[i][(cur + 1)];
        spins_w[k] = read_grid[i][(cur - 1)];
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

      // Load probabilities
      __m256 p_vec = optimized_random_probability();

      // Vectorized lookup: exp_table[-delta_E + 8]
      __m256i index_vec = _mm256_sub_epi32(_mm256_set1_epi32(8), delta_E);
      __m256 exp_vec = _mm256_i32gather_ps((const float *)exp_table, index_vec,
                                           sizeof(float));

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
  // Duplicate the first row to the last row and the last row to the first row
  for (size_t j = 0; j < 2+L; ++j) {
    write_grid[0][j] = write_grid[L / 2][j];
    write_grid[1+L / 2][j] = write_grid[1][j];
  }
  // Duplicate the first column to the last column and the last column to the first column
  for (size_t i = 0; i < 2+L / 2; ++i) {
    write_grid[i][0] = write_grid[i][L];
    write_grid[i][1+L] = write_grid[i][1];
  }

}

static int calculate_red_black_grid(const int (*red_grid)[2+L],
                                    const int (*black_grid)[2+L], int *M_max,
                                    bool invert_aux_mask) {
  int E = 0;
  int M_local = 0;
  __m256i E_vec = _mm256_setzero_si256();
  __m256i M_vec = _mm256_setzero_si256();

  for (size_t i = 1; i < 1+L / 2; ++i) {
    for (size_t j = 1; j < 1+L; j += 8) {
      // Load red spin
      __m256i spin = _mm256_loadu_si256((__m256i *)&red_grid[i][j]);
      // Neighbor north
      __m256i neigh_n =
          _mm256_loadu_si256((__m256i *)&black_grid[(i - 1)][j]);
      // Neighbor south
      __m256i neigh_s =
          _mm256_loadu_si256((__m256i *)&black_grid[(i + 1)][j]);
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
        spins_e[k] = black_grid[i][(cur + 1)];
        spins_w[k] = black_grid[i][(cur - 1)];
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

void update(const float temp, int (*red_grid)[2+L], int (*black_grid)[2+L]) {

  // Only initialized on first call
  static float last_temp = -99.99f;

  if (temp != last_temp) {
    init_exp_table(temp);
    last_temp = temp;
  }

  update_red_black_grid(black_grid, red_grid, true);
  update_red_black_grid(red_grid, black_grid, false);
}


float calculate(int (*red_grid)[2+L], int (*black_grid)[2+L], int *M_max) {
  int E = 0;

  E += calculate_red_black_grid(black_grid, red_grid, M_max, true);
  E += calculate_red_black_grid(red_grid, black_grid, M_max, false);

  return -((float)E / 2.0f);
}