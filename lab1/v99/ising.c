#include "ising.h"
#include "spins.h"

#include <math.h>
#include <stdlib.h>


void update(const float temp, unsigned char *grid) {
  // typewriter update
  for (unsigned int i = 0; i < L; ++i) {
    for (unsigned int j = 0; j < L; ++j) {
      int spin_old = get_spin(grid, i, j);
      int spin_new = -spin_old;

      // computing h_before
      int spin_neigh_n = get_spin(grid, (i + L - 1) % L, j);
      int spin_neigh_e = get_spin(grid, i, (j + 1) % L);
      int spin_neigh_w = get_spin(grid, i, (j + L - 1) % L);
      int spin_neigh_s = get_spin(grid, (i + 1) % L, j);
      int h_before = -(spin_old * spin_neigh_n) - (spin_old * spin_neigh_e) -
                     (spin_old * spin_neigh_w) - (spin_old * spin_neigh_s);

      // h after taking new spin
      int h_after = -(spin_new * spin_neigh_n) - (spin_new * spin_neigh_e) -
                    (spin_new * spin_neigh_w) - (spin_new * spin_neigh_s);

      int delta_E = h_after - h_before;
      float p = rand() / (float)RAND_MAX;
      if (delta_E <= 0 || p <= expf(-delta_E / temp)) {
        set_spin(grid, i, j, spin_new);
      }
    }
  }
}


double calculate(unsigned char *grid, int *M_max) {
  int E = 0;
  for (unsigned int i = 0; i < L; ++i) {
    for (unsigned int j = 0; j < L; ++j) {
      int spin = get_spin(grid, i, j);
      int spin_neigh_n = get_spin(grid, (i + 1) % L, j);
      int spin_neigh_e = get_spin(grid, i, (j + 1) % L);
      int spin_neigh_w = get_spin(grid, i, (j + L - 1) % L);
      int spin_neigh_s = get_spin(grid, (i + L - 1) % L, j);

      E += (spin * spin_neigh_n) + (spin * spin_neigh_e) +
           (spin * spin_neigh_w) + (spin * spin_neigh_s);
      *M_max += spin;
    }
  }
  return -((double)E / 2.0);
}
