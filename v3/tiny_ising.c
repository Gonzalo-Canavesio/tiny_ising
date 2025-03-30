/*
 * Tiny Ising model.
 * Loosely based on  "q-state Potts model metastability
 * study using optimized GPU-based Monte Carlo algorithms",
 * Ezequiel E. Ferrero, Juan Pablo De Francesco, Nicolás Wolovick,
 * Sergio A. Cannas
 * http://arxiv.org/abs/1101.0876
 *
 * Debugging: Ezequiel Ferrero
 */

 #include "ising.h"
 #include "params.h"
 #include "xoshiro256plus.h"
 
 #include <assert.h>
 #include <limits.h> // UINT_MAX
 #include <omp.h>
 #include <stdint.h>
 #include <stdio.h>  // printf()
 #include <stdlib.h> // abs(), malloc(), free()
 #include <string.h> // memset()
 #include <time.h>   // time()
 
 
 // Internal definitions and functions
 // out vector size, it is +1 since we reach TEMP_
 #define NPOINTS (1 + (int)((TEMP_FINAL - TEMP_INITIAL) / TEMP_DELTA))
 #define N (L * L)   // system size
 #define SEED 0xC4FE //(time(NULL)) // random seed
 
 // Macro para acceder a elementos del grid
 #define GRID(i, j) (grid[(i) * L + (j)])
 
 // temperature, E, E^2, E^4, M, M^2, M^4
 struct statpoint {
   double t;
   double e;
   double e2;
   double e4;
   double m;
   double m2;
   double m4;
 };
 
 static void cycle(int8_t *grid, const double min, const double max,
                   const double step, const unsigned int calc_step,
                   struct statpoint stats[]) {
 
   assert((0 < step && min <= max) || (step < 0 && max <= min));
   int modifier = (0 < step) ? 1 : -1;
 
   unsigned int index = 0;
   for (double temp = min; modifier * temp <= modifier * max; temp += step) {
 
     // equilibrium phase
     for (unsigned int j = 0; j < TRAN; ++j) {
       update(temp, grid);
     }
 
     // measurement phase
     unsigned int measurements = 0;
     double e = 0.0, e2 = 0.0, e4 = 0.0, m = 0.0, m2 = 0.0, m4 = 0.0;
     for (unsigned int j = 0; j < TMAX; ++j) {
       update(temp, grid);
       if (j % calc_step == 0) {
         double energy = 0.0, mag = 0.0;
         int M_max = 0;
         energy = calculate(grid, &M_max);
         mag = abs(M_max) / (float)N;
         e += energy;
         e2 += energy * energy;
         e4 += energy * energy * energy * energy;
         m += mag;
         m2 += mag * mag;
         m4 += mag * mag * mag * mag;
         ++measurements;
       }
     }
     assert(index < NPOINTS);
     stats[index].t = temp;
     stats[index].e += e / measurements;
     stats[index].e2 += e2 / measurements;
     stats[index].e4 += e4 / measurements;
     stats[index].m += m / measurements;
     stats[index].m2 += m2 / measurements;
     stats[index].m4 += m4 / measurements;
     ++index;
   }
 }
 
 // Inicializa la matriz grid con valores 1
 static void init(int8_t *grid) {
   memset(grid, 1, L * L * sizeof(int8_t));
 }
 
 int main(void) {
   // parameter checking
   static_assert(TEMP_DELTA != 0, "Invalid temperature step");
   static_assert(((TEMP_DELTA > 0) && (TEMP_INITIAL <= TEMP_FINAL)) ||
                     ((TEMP_DELTA < 0) && (TEMP_INITIAL >= TEMP_FINAL)),
                 "Invalid temperature range+step");
   static_assert(
       TMAX % DELTA_T == 0,
       "Measurements must be equidistant"); // take equidistant calculate()
   static_assert(
       (L * L / 2) * 4ULL < UINT_MAX,
       "L too large for uint indices"); // max energy, that is all spins are the
                                        // same, fits into a ulong
 
   // the stats
   struct statpoint stat[NPOINTS];
   for (unsigned int i = 0; i < NPOINTS; ++i) {
     stat[i].t = 0.0;
     stat[i].e = stat[i].e2 = stat[i].e4 = 0.0;
     stat[i].m = stat[i].m2 = stat[i].m4 = 0.0;
   }
 
   // print header
   printf("# L: %i\n", L);
   printf("# Minimum Temperature: %f\n", TEMP_INITIAL);
   printf("# Maximum Temperature: %f\n", TEMP_FINAL);
   printf("# Temperature Step: %.12f\n", TEMP_DELTA);
   printf("# Equilibration Time: %i\n", TRAN);
   printf("# Measurement Time: %i\n", TMAX);
   printf("# Data Acquiring Step: %i\n", DELTA_T);
   printf("# Number of Points: %i\n", NPOINTS);
 
   // configure RNG
   seed(SEED);
 
   // start timer
   double start = omp_get_wtime();
 
   // Alocar e inicializar el grid
   int8_t *grid = (int8_t*)malloc(L * L * sizeof(int8_t));
   if (grid == NULL) {
     fprintf(stderr, "Error: No se pudo alocar memoria para el grid\n");
     exit(EXIT_FAILURE);
   }
   
   init(grid);
 
   // temperature increasing cycle
   cycle(grid, TEMP_INITIAL, TEMP_FINAL, TEMP_DELTA, DELTA_T, stat);
 
   // Liberar la memoria del grid
   free(grid);
 
   // stop timer
   double elapsed = omp_get_wtime() - start;
   printf("# Total Simulation Time (sec): %lf\n", elapsed);
   printf("# Spins/ms: %lf\n", N / (elapsed * 1000));
 
   printf("# Temp\tE\tE^2\tE^4\tM\tM^2\tM^4\n");
   for (unsigned int i = 0; i < NPOINTS; ++i) {
     printf("%lf\t%.10lf\t%.10lf\t%.10lf\t%.10lf\t%.10lf\t%.10lf\n", stat[i].t,
            stat[i].e / ((double)N), stat[i].e2 / ((double)N * N),
            stat[i].e4 / ((double)N * N * N * N), stat[i].m, stat[i].m2,
            stat[i].m4);
   }
 
   return 0;
 }