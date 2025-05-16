#include "ising.h"
#include "params.h"

#include <stdlib.h>

unsigned char *create_spin_grid() {
  // + 7 se añade para asegurar el redondeo correcto hacia arriba
  int bytes = (L * L + 7) / 8;
  return calloc(bytes, sizeof(unsigned char));
}

int get_spin(unsigned char *grid, size_t x, size_t y) {
  int index = x * L + y;
  return (grid[index / 8] & (1 << (index % 8))) ? 1 : -1;
}

void set_spin(unsigned char *grid, size_t x, size_t y, int new_value) {
  int index = x * L + y;
  if (new_value > 0) {
    grid[index / 8] |= (1 << (index % 8));
  } else {
    grid[index / 8] &= ~(1 << (index % 8));
  }
}

void free_spin_grid(unsigned char *grid) { free(grid); }