//
//  types.h
//  BSOUND
//
//  Created by Richard Schwennicke on 1/17/21.
//


#ifndef types_h
#define types_h
#include "programm_state.h"
#include "input_handling.h"
#include <pthread.h>
#include <curses.h>

typedef struct _BSOUND{
    int num_ops;
    int sample_rate;
    long default_buffer_length;
    short num_chans;
    long bufsize;
    short num_caesar;
    long out_of_range;
    float hi_damp;
    struct _op_stack* head;
    short input_mode;
    short verbose;
    short quit_flag;
    short modify_flag;
    char* programm_loc;
    short mono_input;
    short pause_flag;
    short bypass_flag;
    WINDOW* wnd;
    } BSOUND;
typedef struct _op_stack{
    //operator
    void (*func)(float* input, float* output, void* data,short* attr, BSOUND* bsound);
    //dealloc function
    void (*dealloc)(BSOUND* bsound, void* data);
    //function data
    void * func_st;
    struct _op_stack *next_op;
    //currently not in use
    struct _op_stack *previous_op;
    //continuous ids
    int func_id;
    //quick access to original type
    USR_IN type;
    int y;
    int x;
    short* attr;
    short change_flag;
    short delete_flag;
    pthread_mutex_t lock;
}op_stack;
typedef struct {
    op_stack** head;
    void (*operator)(float* input, float* output, void* data, short* attr, BSOUND* bsound);
    void* (*init)(BSOUND* bsound,  USR_IN type);
    void (*dealloc)(BSOUND* bsound, void* data);
    ///this is for delete case
    op_stack* to_delete;
    op_stack* cursor;
    BSOUND* bsound;
    USR_IN type;
    short push_flag;
}COMMAND;
#endif /* types_h */
