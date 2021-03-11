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
#ifndef types_h
#define types_h
#include <pthread.h>
#include <curses.h>
#include <stdbool.h>
#include "../portaudio/portaudio.h"
#ifndef MYFLT
typedef double MYFLT;
#endif
#ifndef _USR_IN
#define _USR_IN
typedef enum {
    DELAY, // delay opcodes
    PPDEL,
    TAPE,
    MULTITAP,
    CLOUD, // partikkel opcodes
    SHIMMER,
    TRANSPOSE,
    REVERB, // resonator opcodes
    PEDAL,
    RINGMOD, //moddemod opcodes
    MODDEMOD,
    CRUSH,
    BBD,
    RESEQ,
    DELETE,
    MANUAL,
    CLEAR,
    PREFERENCES_MENU,
    NUM_OPTIONS,
    //manipulate stack
    EMPTY = -1,
    ERROR = -2
}USR_IN;
#endif
typedef struct _BSOUND{
    int32_t num_ops;
    int32_t sample_rate;
    int32_t default_buffer_length;
    int16_t num_chans;
    int32_t bufsize;
    int32_t requested_bufsize;
    ///@deprecated?
    int32_t out_of_range;
    MYFLT hi_damp;
    struct _OP_STACK* head;
    bool quit_flag;
    bool modify_flag;
    const char* programm_loc;
    bool mono_input;
    int16_t in_chans;
    int16_t out_chans;
    bool in_out_chanmatch;
    bool pause_flag;
    bool bypass_flag;
    bool record_flag;
    bool playback_flag;
    bool crossfade_looping;
    WINDOW* wnd;
    bool filter_bank_active;
    bool filter_bank_exists;
    PaStream* stream_handle;
    } BSOUND;
typedef struct _OP_STACK{
    //operator
    void (*func)(float* input, float* output, void* data, const int16_t* attr, const BSOUND* bsound);
    //dealloc function
    void (*dealloc)(BSOUND* bsound, void* data);
    //function data
    void * func_st;
    struct _OP_STACK *next_op;
    struct _OP_STACK *previous_op;
    //quick access to original type
    USR_IN type;
    int32_t y;
    int32_t x;
    int16_t* attr;
}OP_STACK;
typedef struct {
    OP_STACK** head;
    void (*operator)(float* input, float* output, void* data, const int16_t* attr, const BSOUND* bsound);
    void* (*init)(BSOUND* bsound,  USR_IN type);
    void (*dealloc)(BSOUND* bsound, void* data);
    OP_STACK* cursor;
    BSOUND* bsound;
    USR_IN type;
    bool push_flag;
}COMMAND;
#endif /* types_h */
