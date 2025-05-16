#include "params.h"
#include <stddef.h>
void init_exp_table();
void update(const size_t index, int (*red_grid)[2 + L],
            int (*black_grid)[2 + L]);
float calculate(int (*red_grid)[2 + L], int (*black_grid)[2 + L], int *M_max);
