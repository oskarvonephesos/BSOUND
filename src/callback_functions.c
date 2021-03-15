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
#include "callback_functions.h"
CALLBACK_DATA* init_callbackdata(BSOUND* bsound){
CALLBACK_DATA* c = (CALLBACK_DATA*) malloc(sizeof(CALLBACK_DATA));
c->temp1     = (float *)calloc(sizeof(float)*2048*bsound->num_chans, 1);
c->temp2     = (float *)calloc(sizeof(float)*2048*bsound->num_chans, 1);
RECORD_INFO* r = init_recordinfo(bsound);
float* recordbuf = (float*) calloc(sizeof(float)*r->recordbuflength, 1);
c->r = r;
c->bsound = bsound;
return c;
}
RECORD_INFO* init_recordinfo(BSOUND* bsound){
RECORD_INFO* r = (RECORD_INFO*) malloc(sizeof(RECORD_INFO));
    r->bypass_active = 0;
    r->record_active = 0;
    r->recordbuflength =bsound->sample_rate*20*bsound->num_chans;
    r->readhead = 0;
    r->crosses_zero = false;
    r->is_crossfaded = true; // this prevents function being called on init
    r->recordzero = r->recordend = r->recordstart = 0;
    r->recordbuf = (float*) calloc(sizeof(float)*r->recordbuflength, 1);
    return r;
}
void cross_fade_buffer(BSOUND* bsound, RECORD_INFO* r){
      int32_t crossfade_length = 1000; int32_t crossfade_start, i, j, k;
      int32_t recordbuflength = r->recordbuflength, rzero = r->recordzero;
      MYFLT incr = 1.0/crossfade_length, amount = 0.0;
      float* recordbuf = r->recordbuf;
      crossfade_start = r->recordend - crossfade_length;
      if (crossfade_start < 0)
            crossfade_start += r->recordbuflength;
      j = crossfade_start;
      k = r->recordstart;
      for (i=0; i<crossfade_length; i++){
            recordbuf[j] = amount * recordbuf[k] + (1.0-amount)* recordbuf[j];
            amount += incr;
            if (++j>=recordbuflength){j = rzero;}
            if (++k>=recordbuflength){k = rzero;}
      }
      r->recordstart += crossfade_length ;
      if (r->recordstart >= recordbuflength){
            r->recordstart -= recordbuflength;
            r->crosses_zero = false;
      }
      if (r->recordstart > r->recordend)
          r->recordzero = 0;
      else //recordstart != recordend, so this is recordstart<recordend
          r->recordzero = r->recordstart;
          r->is_crossfaded = true;
}

int test_callback( const void *input,
                                      void *output,
                                      unsigned long frameCount,
                                      const PaStreamCallbackTimeInfo* timeInfo,
                                      PaStreamCallbackFlags statusFlags,
                                      void *userData ){
      CALLBACK_DATA* data = (CALLBACK_DATA*) userData;
      float* in = (float*) input;
      float* out = (float*) output;
      BSOUND* bsound = data->bsound;
      bsound->statusFlags = statusFlags;
      write_input(in, bsound, data->r );
      apply_fx(in, out, bsound,data->temp1, data->temp2);
      int i;
      for (i=0; i<bsound->bufsize*bsound->num_chans; i++){
          if (out[i]>1.0f){
              out[i]=0.0f;
          }
          if (out[i]< -1.0f){
              out[i]=0.0f;
          }
      }
      if (bsound->pause_flag){
          for (i =0; i<bsound->bufsize*bsound->num_chans; i++)
          out[i]= 0.0f;
      }
      if (bsound->quit_flag)
            return paComplete;
      else
            return 0;
      }
void write_input(float* input, BSOUND* bsound, RECORD_INFO* r){
    PaError  err = paNoError;
    int32_t i, recordhead = r->readhead;
    float* record_buf = r->recordbuf;
    if (bsound->mono_input)
    copylefttoright(input, bsound, 1);
    if (bsound->record_flag){
        //record_start case
        if (!r->record_active){
            r->recordstart   = r->readhead;
            r->record_active = true;
            r->crosses_zero  = false;
            r->is_crossfaded = false;
        }
        int32_t sampCount = bsound->bufsize * bsound->num_chans;
        int32_t rbuflength = r->recordbuflength;
        for (i=0; i<sampCount;){
        record_buf[recordhead++] = input[i++];
            if (recordhead >= rbuflength){
                recordhead = 0;
                r->crosses_zero = true;
            }
        }
    }
    else {
    //record_end case: set appropriate points on tape
    if (r->record_active){
            if (--recordhead<0){
                recordhead += r->recordbuflength;
                r->crosses_zero = false;
          }
            r->recordend = recordhead;
            if (r->crosses_zero)
                r->recordzero = 0;
            else //recordstart != recordend, so this is recordstart<recordend
                r->recordzero = r->recordstart;
            //make sure not to call again || playbackflag is set in input_handling
            r->record_active = 0;
            if (bsound->crossfade_looping)
            cross_fade_buffer(bsound, r);
        recordhead = r->recordstart;
        }
    if (bsound->bypass_flag){
        if (!r->bypass_active){
            int32_t j; MYFLT incr = 0.99;
            int32_t loopend = bsound->bufsize*bsound->num_chans > 512 ? 512: bsound->bufsize *bsound->num_chans;
            for (j=0; j<loopend; j++){
            input[j]=input[j]*incr;//samplein[j]*(1.0/(j+1));
                incr = incr*incr;
            }
            if (loopend<bsound->bufsize*bsound->num_chans){
            for (j=loopend; j<bsound->bufsize*bsound->num_chans; j++)
            input[j]=0.0f;
      }
            r->bypass_active = 1;
        }
           else {
        int32_t j; //this is necessary because of in/out swapping
        for (j=0; j<bsound->bufsize*bsound->num_chans; j++)
        input[j]=0.0f;
            }
    }
    else{
        if (r->bypass_active){
            int32_t j; MYFLT factor = pow(pow(10, 20), 1.0/1024);
            MYFLT incr = pow(10, -20);
            for (j=0; j<1024; j++){
            input[j]=input[j]*incr;
            incr *= factor;}
            r->bypass_active = 0;
        }

    }

    if (bsound->playback_flag){
        int32_t sampCount = bsound->bufsize * bsound->num_chans;
        int32_t rend       = r->recordend,

             rstart     = r->recordstart,
             rbuflength = r->recordbuflength,
             rzero      = r->recordzero;
        for (i=0; i<sampCount;){
            input[i++]+=record_buf[recordhead++];
        if (recordhead>=rbuflength)
            recordhead = rzero;
        if (r->crosses_zero){
            if (recordhead>=rend && recordhead < rstart)
                recordhead = rstart;
            }
            else {
            if (recordhead>=rend )
                recordhead = rstart;
            }
        }
    }
    }
    if(bsound->crossfade_looping && !r->is_crossfaded)
    cross_fade_buffer(bsound, r);
    r->readhead = recordhead;
}
void apply_fx(const float* input, float* output, BSOUND* bsound, float* temp1, float* temp2){
    int32_t i, j;
    float* temp;
    pthread_mutex_lock(&bsound->mymutex);
    OP_STACK* current_op = bsound->head;
    if (bsound->num_ops == 0){
        for (i=0; i<bsound->bufsize*bsound->num_chans; i++){
            output[i]=0.0f;
        }
    }
    else{
        int16_t attr_num;
        int64_t sampcount = bsound->bufsize * bsound->num_chans;
        for (i=0; i<sampcount; i++){
            temp1[i]=input[i];
            output[i]=0.0f;
        }
        current_op = bsound->head;
    for (i= 0; i<bsound->num_ops;){
            //this used to happen all the time and caused a segmentation fault
             if (current_op->func == NULL){
                   bsound->num_ops = i;
                   bsound->errors[BERROR_FUNCST_NULL]++;
                   break;
             }
        current_op->func(temp1, temp2, current_op->func_st, current_op->attr , bsound);
        attr_num = which_attr_is(SHARED_WET_ATTR, current_op->type, 0);
        if (current_op->attr[attr_num]!=100){
             float wet_amount = current_op->attr[attr_num]/100.0;
             for (j=0; j< sampcount; j++)
             temp2[j]= temp2[j]*wet_amount + (1.0-wet_amount)*temp1[j];
       }
        i++;
        if (i<bsound->num_ops){
            attr_num = which_attr_is(SHARED_SKIP_ATTR, current_op->type, 0);
            if (current_op->attr[attr_num]){
                MYFLT skip_amount = current_op->attr[attr_num]/100.0;
                for (j=0; j<sampcount; j++){
                output[j]+=temp2[j]*skip_amount;
                temp2[j]=temp2[j]*(1.0-skip_amount);
                }
            }

            temp = temp1;
            temp1 = temp2;
            temp2 = temp;
        }
        if (current_op->next_op == NULL && i < bsound->num_ops){
             bsound->errors[BERROR_NEXT_OP_NULL]++;
       }
        if (current_op->next_op != NULL)
            current_op = current_op->next_op;
    }
        for (i=0; i<sampcount; i++)
        output[i]+=temp2[i];
    }
    pthread_mutex_unlock(&bsound->mymutex);
}
