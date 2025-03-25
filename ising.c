#include "ising.h"
#include "xoshiro256plus.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>


void update(const float temp, int grid[L][L]) {
  float exp_table[32];

  exp_table[(-8) + 8] = expf(-8 / temp);
  exp_table[(-4) + 8] = expf(-4 / temp);
  exp_table[(-2) + 8] = expf(-2 / temp);
  exp_table[(0) + 8] = expf(0 / temp);
  exp_table[(2) + 8] = expf(2 / temp);
  exp_table[(4) + 8] = expf(4 / temp);
  exp_table[(8) + 8] = expf(8 / temp);

  // typewriter update
  for (unsigned int i = 0; i < L; ++i) {
    for (unsigned int j = 0; j < L; ++j) {
      int spin_old = grid[i][j];
      int spin_new = (-1) * spin_old;

      // computing h_before
      int spin_neigh_n = grid[(i + L - 1) % L][j];
      int spin_neigh_e = grid[i][(j + 1) % L];
      int spin_neigh_w = grid[i][(j + L - 1) % L];
      int spin_neigh_s = grid[(i + 1) % L][j];
      int h_before = -(spin_old * spin_neigh_n) - (spin_old * spin_neigh_e) -
                     (spin_old * spin_neigh_w) - (spin_old * spin_neigh_s);

      // h after taking new spin
      int h_after = -(spin_new * spin_neigh_n) - (spin_new * spin_neigh_e) -
                    (spin_new * spin_neigh_w) - (spin_new * spin_neigh_s);

      int delta_E = h_after - h_before;
      float p = random();
      if (delta_E <= 0 || p <= exp_table[-delta_E + 8]) {
        grid[i][j] = spin_new;
      }
    }
  }
}


double calculate(int grid[L][L], int *M_max) {
  int E = 0;
  for (unsigned int i = 0; i < L; ++i) {
    for (unsigned int j = 0; j < L; ++j) {
      int spin = grid[i][j];
      int spin_neigh_n = grid[(i + 1) % L][j];
      int spin_neigh_e = grid[i][(j + 1) % L];
      int spin_neigh_w = grid[i][(j + L - 1) % L];
      int spin_neigh_s = grid[(i + L - 1) % L][j];

      E += (spin * spin_neigh_n) + (spin * spin_neigh_e) +
           (spin * spin_neigh_w) + (spin * spin_neigh_s);
      *M_max += spin;
    }
  }
  return -((double)E / 2.0);
}
