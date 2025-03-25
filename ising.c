#include "ising.h"
#include "xoshiro256plus.h"

#include <math.h>
#include <stdlib.h>


void update(const float temp, int8_t grid[L][L])
{
    // typewriter update
    for (unsigned int i = 0; i < L; ++i) {
        for (unsigned int j = 0; j < L; ++j) {
            int8_t spin_old = grid[i][j];
            int8_t spin_new = (-1) * spin_old;

            // computing h_before
            int8_t spin_neigh_n = grid[(i + L - 1) % L][j];
            int8_t spin_neigh_e = grid[i][(j + 1) % L];
            int8_t spin_neigh_w = grid[i][(j + L - 1) % L];
            int8_t spin_neigh_s = grid[(i + 1) % L][j];
            int8_t h_before = -(spin_old * spin_neigh_n) - (spin_old * spin_neigh_e) - (spin_old * spin_neigh_w) - (spin_old * spin_neigh_s);

            // h after taking new spin
            int8_t h_after = -(spin_new * spin_neigh_n) - (spin_new * spin_neigh_e) - (spin_new * spin_neigh_w) - (spin_new * spin_neigh_s);

            int8_t delta_E = h_after - h_before;
            float p = random();
            if (delta_E <= 0 || p <= expf(-delta_E / temp)) {
                grid[i][j] = spin_new;
            }
        }
    }
}


double calculate(int8_t grid[L][L], int* M_max)
{
    int8_t E = 0;
    for (unsigned int i = 0; i < L; ++i) {
        for (unsigned int j = 0; j < L; ++j) {
            int8_t spin = grid[i][j];
            int8_t spin_neigh_n = grid[(i + 1) % L][j];
            int8_t spin_neigh_e = grid[i][(j + 1) % L];
            int8_t spin_neigh_w = grid[i][(j + L - 1) % L];
            int8_t spin_neigh_s = grid[(i + L - 1) % L][j];

            E += (spin * spin_neigh_n) + (spin * spin_neigh_e) + (spin * spin_neigh_w) + (spin * spin_neigh_s);
            *M_max += spin;
        }
    }
    return -((double)E / 2.0);
}
