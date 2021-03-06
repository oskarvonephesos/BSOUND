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
#include "util_opcodes.h"
float rms(float* input, BSOUND* bsound ){
    int64_t bufsize = bsound->bufsize * bsound->num_chans;
    int32_t i;
    double running_count = 0.0;
    for (i=0; i< bufsize; i++){
        running_count += (double) (input[i]*input[i]);
    }
    running_count /= bufsize;
    running_count = sqrt(running_count);
    return (float) running_count;
}
double dc_offset(float* input, BSOUND* bsound){
    int64_t bufsize = bsound->bufsize * bsound->num_chans;
    int32_t i;
    double running_count = 0.0;
    for (i=0; i<bufsize; i++){
        running_count += (double) input[i];
    }
    running_count /= bufsize;
    return running_count;
}
int32_t rand_int(int32_t min, int32_t max){
    int32_t return_val = rand()%(abs(max-min));
    return_val += min;
    return return_val;
}
MYFLT rand_float(MYFLT min, MYFLT max){
    int32_t large_int= 4096<<5;
    double rand_db = (double) (rand()%large_int);
    rand_db = rand_db / (double) large_int;
    rand_db *=(max-min);
    rand_db += min;
    return (MYFLT) rand_db;
}
MYFLT dB(MYFLT x){
    return log(x)*20.0;
}
MYFLT amp(MYFLT x_dB){
    return pow(10.0, x_dB/20.0);
}
//this utility handles mono_input
void copylefttoright(float* input, BSOUND* bsound, int32_t inchannels){
    int32_t i, frameCount = bsound->bufsize*bsound->num_chans, j, numchans = bsound->num_chans, k;
    if (bsound->in_out_chanmatch){
    for (j=0; j<numchans-inchannels; j++){
        k = (j + inchannels)%numchans;
        for (i=j; i<frameCount;){
            input[k]=input[i];
            k += numchans;
            i += numchans;
        }
    }
    }
    else {
        int16_t inchans = bsound->in_chans;
        frameCount = bsound->bufsize*inchans;
        k=bsound->bufsize*bsound->num_chans;
        for (i=0; i<frameCount;){
            input[k++]=input[i];
            i+= inchans;
        }
        for (j=0; j<numchans; j++){
            k = j;
            frameCount = bsound->num_chans*bsound->bufsize;
            i = frameCount;
            for (k=j; k<frameCount; ){
                input[k]=input[i];
                k +=numchans;
                i++;
            }
        }
    }
}
void match_outputchannels(float* output, BSOUND* bsound){
      int16_t numChans = bsound->num_chans;
      int32_t i, frameCount = bsound->bufsize * bsound->num_chans;
      int32_t k = frameCount, j, outChans = bsound->out_chans;
      for (j=0; j<outChans; j++){
            k = frameCount + j;
            for (i=j; i<frameCount;){
            output[k] = output[i];
            i += numChans;
            k += outChans;
            }
      }
      frameCount = bsound->bufsize * bsound->out_chans;
      for (j=0; j<outChans; j++){
            k = bsound->bufsize * bsound->num_chans + j;
            for (i=j; i<frameCount;){
                  output[i]=output[k];
                  i += outChans;
                  k += outChans;
            }
      }
}
