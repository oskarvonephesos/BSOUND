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
#include "gui_utilities.h"
#include <stdlib.h>
#include <stdio.h>
#include "../portaudio/portaudio.h"

#ifndef input_handling_h
#define input_handling_h

typedef enum {
SHARED_DECAY_TIME_ATTR,
SHARED_FILTER_ATTR,
SHARED_DISTORT_ATTR,
SHARED_SKIP_ATTR,
SHARED_WET_ATTR,
SHARED_VOLUME_ATTR,
SHARED_NUM_ATTR
} SHARED_ATTR_NUM;

typedef enum {
D_TIME_ATTR,
D_FEEDBACK_ATTR,
D_FILTER_ATTR,
D_SPREAD_ATTR,
D_VOLUME_ATTR,
D_SKIP_ATTR,
D_WET_ATTR,
D_NUM_ATTR
} DELAY_ATTR_NUM;
typedef enum {
P_SPREAD_ATTR,
P_DENSITY_ATTR,
P_GRAINLENGTH_ATTR,
P_TRANSPOSE_ATTR,
P_VOLUME_ATTR,
P_SKIP_ATTR,
P_WET_ATTR,
P_REVERSE_ATTR,
P_FEEDBACK_ATTR,
P_NUM_ATTR
} PARTIKKEL_ATTR_NUM;
typedef enum {
R_FEEDBACK_ATTR,
R_FILTER_ATTR,
R_MODULATION_ATTR,
R_VOLUME_ATTR,
R_SKIP_ATTR,
R_WET_ATTR,
R_NUM_ATTR
} RESON_ATTR_NUM;
typedef enum {
M_FREQ_ATTR,
M_FMFREQ_ATTR,
M_FMINDEX_ATTR,
M_FOLD_ATTR,
M_OFFSET_ATTR,
M_VOLUME_ATTR,
M_SKIP_ATTR,
M_WET_ATTR,
M_NUM_ATTR
} MODDEMOD_ATTR_NUM;
typedef enum {
C_NYQUIST_ATTR,
C_INTERPOLATE_ATTR,
C_FILTER_ATTR,
C_MU_ATTR,
C_VOLUME_ATTR,
C_SKIP_ATTR,
C_WET_ATTR,
C_NUM_ATTR
} CRUSH_ATTR_NUM;
typedef enum {
B_QUALITY_ATTR,
B_FEEDBACK_ATTR,
B_FILTER_ATTR,
B_MODAMT_ATTR,
B_MODSPEED_ATTR,
B_VOLUME_ATTR,
B_SKIP_ATTR,
B_WET_ATTR,
B_NUM_ATTR
} BBD_ATTR_NUM;
typedef enum {
Q_GAIN_ATTR,
Q_BANDWIDTH_ATTR,
Q_TILT_ATTR,
Q_CRUNCH_ATTR,
Q_VOLUME_ATTR,
Q_SKIP_ATTR,
Q_WET_ATTR,
Q_NUM_ATTR
} RESEQ_ATTR_NUM;

void* input_handler(void* id );
int16_t which_attr_is(SHARED_ATTR_NUM attr, USR_IN type, int16_t attr_num);
//deprecated
int16_t which_attr_is_skip(int16_t attr_num);
void error_message(const char* message, BSOUND* bsound);

COMMAND* parse(char* line, int32_t length);
int32_t insert_op(BSOUND* bsound, COMMAND* command);
#endif /* input_handling_h */
