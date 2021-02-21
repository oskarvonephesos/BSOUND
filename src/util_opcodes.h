#ifndef util_opcodes_h
#define util_opcodes_h

#include <math.h>
#include <stdlib.h>
#include "programm_state.h"
///utilities
void copylefttoright(float* input, BSOUND* bsound, int inchannels);
MYFLT dB(MYFLT x); ///conversion utilities
MYFLT amp(MYFLT x_dB);
double dc_offset(float* input, BSOUND* bsound); ///measurement utilities
float rms(float* input, BSOUND* bsound );
///random utilities
int rand_int(int min, int max);
MYFLT rand_float(MYFLT min, MYFLT max);
#endif /* util_opcodes_h */
