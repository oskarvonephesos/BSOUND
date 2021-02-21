//
//  programm state.c
//  BSOUND
//
//  Created by Richard Schwennicke on 1/15/21.
//
#include "programm_state.h"

BSOUND* init_bsound(){
    BSOUND* bsound = (BSOUND*) malloc(sizeof(BSOUND));
    bsound->bufsize=BUF;
    bsound->sample_rate=SR;
    bsound->default_buffer_length=SR;
    bsound->num_ops = 0;
    bsound->quit_flag = 0;
    bsound->input_mode = 0; //mode caesar off
    bsound->verbose = 0;
    bsound->hi_damp = 2-cos(20.0*MY_2_PI/bsound->sample_rate);
    bsound->hi_damp = (bsound->hi_damp  - sqrt(bsound->hi_damp *bsound->hi_damp - 1.0));
    bsound->modify_flag = 0;
    bsound->out_of_range = 0;
    bsound->mono_input = 0;
    bsound->pause_flag = 0;
    bsound->bypass_flag = 0;
    return bsound;
}
struct _op_stack* init_head (){
    op_stack* head = (op_stack*) malloc (sizeof(op_stack));
    head->func = NULL;
    head->next_op = NULL;
    head->previous_op = NULL;
    return head;
}
