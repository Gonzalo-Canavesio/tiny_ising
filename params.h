#pragma once

#ifndef L
#define L 512U // linear system size
#endif

#ifndef TEMP_INITIAL
#define TEMP_INITIAL 1.73 // initial temperature
#endif

#ifndef TEMP_FINAL
#define TEMP_FINAL 3.0 // final temperature
#endif

#ifndef TEMP_DELTA
#define TEMP_DELTA 0.01 // temperature step
#endif

#ifndef TRAN
#define TRAN 8 // equilibration time
#endif

#ifndef TMAX
#define TMAX 32 // measurement time
#endif

#ifndef DELTA_T
#define DELTA_T 8 // sampling period for energy and magnetization
#endif
