#include <stdlib.h>

unsigned char *create_spin_grid();

int get_spin(unsigned char *grid, size_t x, size_t y);

void set_spin(unsigned char *grid, size_t x, size_t y, int new_value);

void free_spin_grid(unsigned char *grid);
