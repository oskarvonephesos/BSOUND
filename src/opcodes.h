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
#include <stdlib.h>
#include <math.h>
#include <time.h>
#ifndef MY_2_PI
#define MY_2_PI (6.28318530718)
#endif

#ifndef opcodes_h
#define opcodes_h
#include "globaltypes.h"
#include "util_opcodes.h"
#include "data_types.h"
typedef struct {
    RNGBUF* aux;
    int32_t* delay_length; //left and right
    MYFLT* read_index;
    int32_t write_index;
    MYFLT** feedback; //feedback matrix
    MYFLT** current_feedback;
    MYFLT lp_freq;
    MYFLT prv_lp_freq;
    MYFLT lp_damp;
    MYFLT* prv_y0;
    MYFLT output_scaling;
    MYFLT** weighting;
    MYFLT** current_weighting;
    MYFLT* spacing;
    MYFLT* current_spacing;
    DELAY_LINE** lines;
    int16_t num_taps;
} DELAY_OPS;
typedef struct {
    RNGBUF* in;
    MYFLT** err_term;
    RNGBUF* out;
    int32_t* disttab; //vars for displacement tab
    int32_t disttab_index;
    int32_t disttab_length;
    int32_t curr_dist;
    int32_t max_dist;
    int32_t grain_length; //in samps; 10 000 currently max
    MYFLT* envtab;
    int32_t envtab_length; //10 000 currently max.
    int32_t frequency; //every frequency number of grains new grain is sown
    int32_t phs; //counter to keep track of grains over k-cycles
    MYFLT pitch_factor; //repitching factor: 1 preserves original pitch
    DELAY_LINE** transposed;
    MYFLT transposed_index;
    MYFLT lp_freq;
    MYFLT lp_damp;
    MYFLT* prv_y0;
    int32_t transpose_frequency; //x number of grains pass before 1 is repitched
    int32_t transpose_phs; //phase counter for transposing
    MYFLT max_val; //output scaling
} PARTIKKEL_OPS;


typedef struct{
    RNGBUF* aux;
} ALLPASS_OPS;

typedef struct {
    RNGBUF* in;
    RNGBUF* out;
    int16_t num_lines;
    DELAY_LINE ** resonator;
    MYFLT lp_freq;
    MYFLT prv_lp_freq;
    MYFLT lp_damp;
    MYFLT fdbk;
    MYFLT output_scaling;
    int32_t random_speed;
    ALLPASS_OPS* allpass1;
    ALLPASS_OPS* allpass2;
    ALLPASS_OPS* allpass3;
    ALLPASS_OPS* allpass4;
} RESON_OPS;

typedef struct {
    MYFLT* modulator;
    MYFLT freq;
    MYFLT index;
    MYFLT mod_freq;
    MYFLT mod_index;
    MYFLT* prv_y0;
    int32_t tab_length;
    int16_t randomize;
    int16_t prv_fold;
    int16_t prv_offset;
    int16_t square;
} MODDEMOD_OPS;
typedef struct {
    MYFLT** samp_reduced;
    MYFLT samp_index;
    MYFLT samp_read;
    MYFLT** in_buffer;
    MYFLT in_index;
    MYFLT in_read;
    int32_t tab_length;
    MYFLT read_factor; // 1 is unchanged
    int16_t interpolate;
    MYFLT* prv_y0;
    MYFLT lp_freq;
    MYFLT prv_lp_freq;
    MYFLT* prv_y0_hipass;
    MYFLT damp_factor;
} CRUSH_OPS;
typedef struct {
    RNGBUF* aux;
    MYFLT* modulator;
    MYFLT* current_readfactors;
    MYFLT mod_freq;
    int mod_index;
    int mod_tab_length;
    MYFLT** samp_reduced;
    MYFLT samp_index;
    MYFLT samp_read;
    MYFLT** in_buffer;
    MYFLT in_index;
    MYFLT in_read;
    int32_t tab_length;
    MYFLT read_factor;
    int16_t interpolate;
    MYFLT* prv_y0;
    MYFLT lp_freq;
    MYFLT prv_lp_freq;
    MYFLT* prv_y0_hipass;
    MYFLT damp_factor;
} BBD_OPS;
typedef struct {
    MYFLT* buffer;
    MYFLT** xmem1;
    MYFLT** xmem2;
    MYFLT** ymem1;
    MYFLT** ymem2;
    int32_t num_bands;
    MYFLT** peak; //in hz
    MYFLT** bandwidth; //in octaves
    MYFLT** gain; // in db
    int32_t prv_tilt;
    MYFLT** b0;
    MYFLT** b1;
    MYFLT** b2;
    MYFLT** a0;
    MYFLT** a1;
    MYFLT** a2;
} RESEQ_OPS;
void* init_delay(BSOUND* bsound, USR_IN type);
void dealloc_delay(BSOUND* bsound, void* data);
void delay(float *input, float * output, void* data, const int16_t* attr, const BSOUND* bsound);

void* init_partikkel(BSOUND* bsound, USR_IN type);
void dealloc_partikkel(BSOUND* bsound, void* data);
void partikkel(float*input, float*output, void* data_st, const int16_t* attr, const BSOUND* bsound);

void* init_reson(BSOUND* bsound, USR_IN type);
void dealloc_reson(BSOUND* bsound, void* data);
void randomize_delay_line(DELAY_LINE* line, const BSOUND* bsound, RESON_OPS* data);
void reson(float *input, float* output, void* data_st, const int16_t* attr, const BSOUND* bsound);
void* init_allpass(BSOUND* bsound, int32_t buf_length);
void dealloc_allpass(BSOUND* bsound, void* data);
void allpass(RNGBUF* buf, const MYFLT feedback, ALLPASS_OPS* data, const BSOUND* bsound);

void* init_moddemod(BSOUND* bsound, USR_IN type);
void dealloc_moddemod(BSOUND* bsound, void* data);
void moddemod(float* input, float* output, void* data_st, const int16_t* attr, const BSOUND* bsound);

void* init_crush(BSOUND* bsound, USR_IN type);
void dealloc_crush(BSOUND* bsound, void* data);
void crush(float* input, float* output, void* data_st, const int16_t* attr, const BSOUND* bsound);

void* init_bbd(BSOUND* bsound, USR_IN type);
void dealloc_bbd(BSOUND*, void* data);
void bbd(float* input, float* output, void* data_st, const int16_t* attr, const BSOUND* bsound);

void* init_reseq(BSOUND* bsound, USR_IN type);
void dealloc_reseq(BSOUND* bsound, void* data);
void reseq(float* input, float* output, void* data_st, const int16_t* attr, const BSOUND* bsound);
#endif /* opcodes_h */
