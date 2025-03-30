#include "ising.h"
#include "xoshiro256plus.h"
#include <math.h>
#include <stdint.h>

// Tabla de valores exponenciales precalculados
static float exp_table[32];

// Inicializa la tabla de exponenciales para una temperatura dada
static void init_exp_table(float temp) {
  exp_table[(-8) + 8] = expf(-8 / temp);
  exp_table[(-4) + 8] = expf(-4 / temp);
  exp_table[(-2) + 8] = expf(-2 / temp);
  exp_table[(0) + 8] = expf(0 / temp);
  exp_table[(2) + 8] = expf(2 / temp);
  exp_table[(4) + 8] = expf(4 / temp);
  exp_table[(8) + 8] = expf(8 / temp);
}

// Actualiza el grid utilizando el algoritmo de Metropolis-Hastings
void update(const float temp, Grid *grid) {
  // Solo se inicializa en la primera llamada o cuando cambia la temperatura
  static float last_temp = -999.0;
  if (temp != last_temp) {
    init_exp_table(temp);
    last_temp = temp;
  }

  // Recorre el grid en patrón de máquina de escribir
  for (unsigned int i = 0; i < L; ++i) {
    for (unsigned int j = 0; j < L; ++j) {
      int8_t spin_old = grid->rows[i][j];
      int8_t spin_new = (-1) * spin_old;

      // Obtiene los spines vecinos (con condiciones de borde periódicas)
      int8_t spin_neigh_n = grid->rows[(i + L - 1) % L][j];
      int8_t spin_neigh_e = grid->rows[i][(j + 1) % L];
      int8_t spin_neigh_w = grid->rows[i][(j + L - 1) % L];
      int8_t spin_neigh_s = grid->rows[(i + 1) % L][j];

      // Calcula la energía antes del cambio
      int8_t h_before = -(spin_old * spin_neigh_n) - (spin_old * spin_neigh_e) -
                        (spin_old * spin_neigh_w) - (spin_old * spin_neigh_s);

      // Calcula la energía después del cambio
      int8_t h_after = -(spin_new * spin_neigh_n) - (spin_new * spin_neigh_e) -
                       (spin_new * spin_neigh_w) - (spin_new * spin_neigh_s);

      // Cambio de energía
      int8_t delta_E = h_after - h_before;

      // Criterio de Metropolis-Hastings
      float p = optimized_random_probability();
      if (delta_E <= 0 || p <= exp_table[-delta_E + 8]) {
        grid->rows[i][j] = spin_new;
      }
    }
  }
}

// Calcula la energía total y la magnetización del sistema
double calculate(Grid *grid, int *M_max) {
  int E = 0;
  *M_max = 0;

  for (unsigned int i = 0; i < L; ++i) {
    for (unsigned int j = 0; j < L; ++j) {
      int8_t spin = grid->rows[i][j];

      // Obtiene los spines vecinos (con condiciones de borde periódicas)
      int8_t spin_neigh_n = grid->rows[(i + L - 1) % L][j];
      int8_t spin_neigh_e = grid->rows[i][(j + 1) % L];
      int8_t spin_neigh_w = grid->rows[i][(j + L - 1) % L];
      int8_t spin_neigh_s = grid->rows[(i + 1) % L][j];

      // Suma las contribuciones a la energía
      E += (spin * spin_neigh_n) + (spin * spin_neigh_e) +
           (spin * spin_neigh_w) + (spin * spin_neigh_s);

      // Suma el spin a la magnetización total
      *M_max += spin;
    }
  }

  // Devuelve la energía (dividida por 2 para evitar contar interacciones dos
  // veces)
  return -((double)E / 2.0);
}