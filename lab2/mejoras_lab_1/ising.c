#include "ising.h"
#include "xoshiro256plus.h"

#include <math.h>
#include <stddef.h>

static float exp_table[32];

static void init_exp_table(const float temp) {
  exp_table[(-8) + 8] = expf(-8.0f / temp);
  exp_table[(-4) + 8] = expf(-4.0f / temp);
  exp_table[(0) + 8] = expf(0.0f / temp);
  exp_table[(4) + 8] = expf(4.0f / temp);
  exp_table[(8) + 8] = expf(8.0f / temp);
}

void update(const float temp, int grid[L][L]) {

  // Only initialized on first call
  static float last_temp = -99.99f;

  if (temp != last_temp) {
    init_exp_table(temp);
    last_temp = temp;
  }

  // typewriter update
  for (size_t i = 0; i < L; ++i) {
    for (size_t j = 0; j < L; ++j) {
      int spin_old = grid[i][j];

      // computing h_before
      int spin_neigh_n = grid[(i + L - 1) % L][j];
      int spin_neigh_e = grid[i][(j + 1) % L];
      int spin_neigh_w = grid[i][(j + L - 1) % L];
      int spin_neigh_s = grid[(i + 1) % L][j];

      int delta_E = 2 * spin_old *
                    (spin_neigh_n + spin_neigh_e + spin_neigh_w + spin_neigh_s);
      float p = optimized_random_probability();
      if (delta_E <= 0 || p <= exp_table[-delta_E + 8]) {
        grid[i][j] = (-1) * spin_old;
      }
    }
  }
}


float calculate(const int grid[L][L], int *M_max) {
  int E = 0;
  for (size_t i = 0; i < L; ++i) {
    for (size_t j = 0; j < L; ++j) {
      int spin = grid[i][j];
      int spin_neigh_n = grid[(i + 1) % L][j];
      int spin_neigh_e = grid[i][(j + 1) % L];
      int spin_neigh_w = grid[i][(j + L - 1) % L];
      int spin_neigh_s = grid[(i + L - 1) % L][j];

      E += spin * (spin_neigh_n + spin_neigh_e + spin_neigh_w + spin_neigh_s);
      *M_max += spin;
    }
  }
  return -((float)E / 2.0f);
}
