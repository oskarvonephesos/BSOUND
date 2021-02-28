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
#include "programm_state.h"

BSOUND* init_bsound(){
    BSOUND* bsound = (BSOUND*) malloc(sizeof(BSOUND));
    bsound->bufsize               = BUF;
    bsound->sample_rate           = SR;
    bsound->default_buffer_length = SR;
    bsound->wnd                   = NULL;
    bsound->num_ops               = 0;
    bsound->quit_flag             = 0;
    bsound->hi_damp               = 2-cos(20.0*MY_2_PI/bsound->sample_rate);
    bsound->hi_damp               = (bsound->hi_damp  - sqrt(bsound->hi_damp *bsound->hi_damp - 1.0));
    bsound->out_of_range          = 0;
    bsound->mono_input            = 0;
    bsound->in_out_chanmatch      = true;
    bsound->pause_flag            = 0;
    bsound->bypass_flag           = 0;
    bsound->record_flag           = 0;
    bsound->playback_flag         = 0;
    bsound->filter_bank_active    = 0;
    bsound->filter_bank_exists    = 0;
    return bsound;
}
struct _op_stack* init_head (){
    op_stack* head    = (op_stack*) malloc (sizeof(op_stack));
    head->func        = NULL;
    head->next_op     = NULL;
    head->previous_op = NULL;
    return head;
}
