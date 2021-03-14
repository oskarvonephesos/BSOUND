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
#include "globaltypes.h"
#include "input_handling.h"
#include "../portaudio/portaudio.h"
#include "util_opcodes.h"
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <pthread.h>
#ifndef callback_functions_h
#define callback_functions_h
typedef struct {
    bool bypass_active;
    bool record_active;
    bool crosses_zero;
    bool is_crossfaded;
    int32_t recordbuflength;
    int32_t recordstart;
    int32_t recordend;
    int32_t recordzero;
    int32_t readhead;
    float* recordbuf;
}RECORD_INFO;
typedef struct {
  BSOUND* bsound;
  RECORD_INFO* r;
  float* temp1;
  float* temp2;
} CALLBACK_DATA;
void write_input(float* input, BSOUND* bsound, RECORD_INFO* r);
void cross_fade_buffer(BSOUND* bsound, RECORD_INFO* r);
RECORD_INFO* init_recordinfo(BSOUND* bsound);
CALLBACK_DATA* init_callbackdata(BSOUND* bsound);
int test_callback( const void *input, void *output, unsigned long frameCount, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void *userData );
void apply_fx(const float* input, float* output, BSOUND* bsound, float* temp1, float* temp2);
#endif
