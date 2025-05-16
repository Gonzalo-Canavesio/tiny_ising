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

static void update_red_black_grid(int (*write_grid)[L], const int (*read_grid)[L]) {
  for (size_t i = 0; i < L / 2; ++i) {
    for (size_t j = 0; j < L; ++j) {
      int spin_old = write_grid[i][j];

      int spin_neigh_n = read_grid[i][j];
      int spin_neigh_e = read_grid[i][(j + 1) % L];
      int spin_neigh_w = read_grid[i][(j + L - 1) % L];
      int spin_neigh_s = read_grid[(i + 1) % (L / 2)][j];

      int delta_E = 2 * spin_old *
                    (spin_neigh_n + spin_neigh_e + spin_neigh_w + spin_neigh_s);
      float p = optimized_random_probability();
      if (delta_E <= 0 || p <= exp_table[-delta_E + 8]) {
        write_grid[i][j] = (-1) * spin_old;
      }
    }
  }
}

static int calculate_red_black_grid(const int (*write_grid)[L],
                                    const int (*read_grid)[L], int *M_max) {
  int E = 0;
  for (size_t i = 0; i < L / 2; ++i) {
    for (size_t j = 0; j < L; ++j) {
      int spin = write_grid[i][j];
      int spin_neigh_n = read_grid[i][j];
      int spin_neigh_e = read_grid[i][(j + 1) % L];
      int spin_neigh_w = read_grid[i][(j + L - 1) % L];
      int spin_neigh_s = read_grid[(i + 1) % (L / 2)][j];

      E += spin * (spin_neigh_n + spin_neigh_e + spin_neigh_w + spin_neigh_s);
      *M_max += spin;
    }
  }
  return E;
}

void update(const float temp, int (*red_grid)[L], int (*black_grid)[L]) {

  // Only initialized on first call
  static float last_temp = -99.99f;

  if (temp != last_temp) {
    init_exp_table(temp);
    last_temp = temp;
  }

  update_red_black_grid(black_grid, red_grid);
  update_red_black_grid(red_grid, black_grid);
}


float calculate(int (*red_grid)[L], int (*black_grid)[L], int *M_max) {
  int E = 0;

  E += calculate_red_black_grid(black_grid, red_grid, M_max);
  E += calculate_red_black_grid(red_grid, black_grid, M_max);

  return -((float)E / 2.0f);
}