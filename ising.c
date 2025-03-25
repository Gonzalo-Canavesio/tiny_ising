#include "ising.h"
#include "xoshiro256plus.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>


void update(const float temp, int_fast8_t grid[L][L]) {
  float exp_table[32];
  exp_table[(-8) + 8] = expf(-8 / temp);
  exp_table[(-4) + 8] = expf(-4 / temp);
  exp_table[(-2) + 8] = expf(-2 / temp);
  exp_table[(0) + 8] = expf(0 / temp);
  exp_table[(2) + 8] = expf(2 / temp);
  exp_table[(4) + 8] = expf(4 / temp);
  exp_table[(8) + 8] = expf(8 / temp);

  // Block size (typically chosen to fit in cache)
  const int BLOCK_SIZE = 8;

  // Iterate over blocks
  for (unsigned int block_i = 0; block_i < L; block_i += BLOCK_SIZE) {
    for (unsigned int block_j = 0; block_j < L; block_j += BLOCK_SIZE) {
      // Inner loop over each block
      for (unsigned int i = block_i; i < (block_i + BLOCK_SIZE < L ? block_i + BLOCK_SIZE : L); ++i) {
        for (unsigned int j = block_j; j < (block_j + BLOCK_SIZE < L ? block_j + BLOCK_SIZE : L); ++j) {
          int_fast8_t spin_old = grid[i][j];
          int_fast8_t spin_new = (-1) * spin_old;

          // Computing neighboring spins with periodic boundary conditions
          int_fast8_t spin_neigh_n = grid[(i + L - 1) % L][j];
          int_fast8_t spin_neigh_e = grid[i][(j + 1) % L];
          int_fast8_t spin_neigh_w = grid[i][(j + L - 1) % L];
          int_fast8_t spin_neigh_s = grid[(i + 1) % L][j];

          // Calculate energy before and after spin flip
          int_fast8_t h_before =
              -(spin_old * spin_neigh_n) - (spin_old * spin_neigh_e) -
              (spin_old * spin_neigh_w) - (spin_old * spin_neigh_s);
          int_fast8_t h_after =
              -(spin_new * spin_neigh_n) - (spin_new * spin_neigh_e) -
              (spin_new * spin_neigh_w) - (spin_new * spin_neigh_s);

          int_fast8_t delta_E = h_after - h_before;
          float p = random();

          // Metropolis criterion
          if (delta_E <= 0 || p <= exp_table[-delta_E + 8]) {
            grid[i][j] = spin_new;
          }
        }
      }
    }
  }
}


double calculate(int_fast8_t grid[L][L], int *M_max) {
  int E = 0;
  for (unsigned int i = 0; i < L; ++i) {
    for (unsigned int j = 0; j < L; ++j) {
      int_fast8_t spin = grid[i][j];
      int_fast8_t spin_neigh_n = grid[(i + 1) % L][j];
      int_fast8_t spin_neigh_e = grid[i][(j + 1) % L];
      int_fast8_t spin_neigh_w = grid[i][(j + L - 1) % L];
      int_fast8_t spin_neigh_s = grid[(i + L - 1) % L][j];

      E += (spin * spin_neigh_n) + (spin * spin_neigh_e) +
           (spin * spin_neigh_w) + (spin * spin_neigh_s);
      *M_max += spin;
    }
  }
  return -((double)E / 2.0);
}
