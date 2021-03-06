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
    pthread_mutex_init(&bsound->mymutex, NULL);
    bsound->bufsize               = BUF;
    bsound->requested_bufsize     = bsound->bufsize;
    bsound->sample_rate           = SR;
    bsound->wnd                   = NULL;
    bsound->num_ops               = 0;
    bsound->quit_flag             = 0;
    bsound->hi_damp               = 2-cos(20.0*MY_2_PI/bsound->sample_rate);
    bsound->hi_damp               = (bsound->hi_damp  - sqrt(bsound->hi_damp *bsound->hi_damp - 1.0));
    bsound->mono_input            = 0;
    bsound->in_out_chanmatch      = true;
    bsound->bypass_flag           = 0;
    bsound->record_flag           = 0;
    bsound->playback_flag         = 0;
    bsound->crossfade_looping     = true;
    bsound->errors                = (BERROR*) calloc(sizeof(int32_t)*BERROR_NUM_ERRORS, 1);
    //not currently in use
    //legacy
    bsound->pause_flag            = 0;
    //deprecated
    bsound->default_buffer_length = SR;
    bsound->out_of_range          = 0;
    //planned
    bsound->restart_requested     = 0;
    bsound->filter_bank_active    = 0;
    bsound->filter_bank_exists    = 0;
    return bsound;
}
OP_STACK* init_head (){
    OP_STACK* head    = (OP_STACK*) malloc (sizeof(OP_STACK));
    head->func        = NULL;
    head->next_op     = NULL;
    head->previous_op = NULL;
    return head;
}
