//
//  opcodes.h
//  BSOUND
//
//  Created by Richard Schwennicke on 1/3/21.
//
#include <stdlib.h>
#include <math.h>
#include <time.h>
#ifndef MY_2_PI
#define MY_2_PI (6.28318530718)
#endif

#ifndef opcodes_h
#define opcodes_h
#include "globaltypes.h"
#include "input_handling.h"
#include "util_opcodes.h"
#include "data_types.h"
typedef struct {
    void* (*init_opcode)(BSOUND* bsound, USR_IN type);
    void (*dealloc_opcode)(BSOUND* bsound, void* data);
    void (*opcode)(float* input, float* output, void* data, BSOUND* bsound);
    char* name;
}OP_LIST;
typedef struct {
    RNGBUF* aux;
    int* delay_length; //left and right
    MYFLT* read_index;
    int write_index;
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
    short num_taps;
} DELAY_OPS;
typedef struct {
    RNGBUF* in;
    RNGBUF* out;
    int* disttab; //vars for displacement tab
    int disttab_index;
    int disttab_length;
    int curr_dist;
    int max_dist;
    int grain_length; //in samps; 10 000 currently max
    MYFLT* envtab;
    int envtab_length; //10 000 currently max.
    int frequency; //every frequency number of grains new grain is sown
    int phs; //counter to keep track of grains over k-cycles
    MYFLT pitch_factor; //repitching factor: 1 preserves original pitch
    DELAY_LINE** transposed;
    MYFLT transposed_index;
    MYFLT lp_freq;
    MYFLT lp_damp;
    MYFLT* prv_y0;
    int transpose_frequency; //x number of grains pass before 1 is repitched
    int transpose_phs; //phase counter for transposing
    MYFLT max_val; //output scaling
} PARTIKKEL_OPS;

typedef struct {
    RNGBUF* in;
    RNGBUF* out;
    short num_lines;
    DELAY_LINE ** resonator;
    MYFLT lp_freq;
    MYFLT prv_lp_freq;
    MYFLT lp_damp;
    MYFLT fdbk;
    MYFLT output_scaling;
    int random_speed;
    ///prv lp_frq and lp_frq might be a way to do k-rate changes of lp-freq
} RESON_OPS;
typedef struct {
    MYFLT* modulator;
    MYFLT freq;
    MYFLT index;
    MYFLT mod_freq;
    MYFLT mod_index;
    MYFLT* prv_y0;
    int tab_length;
    short randomize;
    short prv_fold;
    short prv_offset;
    short square;
} MODDEMOD_OPS;
typedef struct {
    MYFLT** samp_reduced;
    MYFLT samp_index;
    MYFLT samp_read;
    MYFLT** in_buffer;
    MYFLT in_index;
    MYFLT in_read;
    int tab_length;
    MYFLT read_factor; // 1 is unchanged
    short interpolate;
    MYFLT* prv_y0;
    MYFLT lp_freq;
    MYFLT prv_lp_freq;
    MYFLT* prv_y0_hipass;
    MYFLT damp_factor;
} CRUSH_OPS;
typedef struct {
    MYFLT** samp_reduced;
    MYFLT samp_index;
    MYFLT samp_read;
    MYFLT** in_buffer;
    MYFLT in_index;
    MYFLT in_read;
    int tab_length;
    MYFLT read_factor;
    short interpolate;
    MYFLT* prv_y0;
    MYFLT lp_freq;
    MYFLT prv_lp_freq;
    MYFLT* prv_y0_hipass;
    MYFLT damp_factor;
} BBD_OPS;
void* init_delay(BSOUND* bsound, USR_IN type);
void dealloc_delay(BSOUND* bsound, void* data);
void delay(float *input, float * output, void* data, short* attr, BSOUND* bsound);

void* init_partikkel(BSOUND* bsound, USR_IN type);
void dealloc_partikkel(BSOUND* bsound, void* data);
void partikkel(float*input, float*output, void* data_st, short* attr, BSOUND* bsound);

void* init_reson(BSOUND* bsound, USR_IN type);
void dealloc_reson(BSOUND* bsound, void* data);
void randomize_delay_line(DELAY_LINE* line, BSOUND* bsound, RESON_OPS* data);
void reson(float *input, float* output, void* data_st, short* attr,BSOUND* bsound);

void* init_moddemod(BSOUND* bsound, USR_IN type);
void dealloc_moddemod(BSOUND* bsound, void* data);
void moddemod(float* input, float* output, void* data_st,short* attr, BSOUND* bsound);

void* init_crush(BSOUND* bsound, USR_IN type);
void dealloc_crush(BSOUND* bsound, void* data);
void crush(float* input, float* output, void* data_st,short* attr, BSOUND* bsound);

void* init_bbd(BSOUND* bsound, USR_IN type);
void dealloc_bbd(BSOUND*, void* data);
void bbd(float* input, float* output, void* data_st, short* attr, BSOUND* bsound);

#endif /* opcodes_h */
