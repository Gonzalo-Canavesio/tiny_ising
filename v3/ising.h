#include "params.h"
#include <stdint.h>

typedef struct {
  int8_t *data;
  int8_t **rows;
} Grid;

void update(const float temp, Grid *grid);

double calculate(Grid *grid, int *M_max);
