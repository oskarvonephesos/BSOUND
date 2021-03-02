/**
      BSOUND
      a live audio processing tool
      designed to interact with acoustic instruments
    Copyright (C) <2021> <Richard Schwennicke>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
    **/
#ifndef util_opcodes_h
#define util_opcodes_h

#include <math.h>
#include <stdlib.h>
#include "programm_state.h"
///utilities
void copylefttoright(float* input, BSOUND* bsound, int inchannels);
void match_outputchannels(float* output, BSOUND* bsound);
MYFLT dB(MYFLT x); ///conversion utilities
MYFLT amp(MYFLT x_dB);
double dc_offset(float* input, BSOUND* bsound); ///measurement utilities
float rms(float* input, BSOUND* bsound );
///random utilities
int rand_int(int min, int max);
MYFLT rand_float(MYFLT min, MYFLT max);
#endif /* util_opcodes_h */
