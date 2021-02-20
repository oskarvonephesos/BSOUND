//
//  util_opcodes.c
//  BSOUND
//
//  Created by Richard Schwennicke on 1/15/21.
//

#include "util_opcodes.h"
float rms(float* input, BSOUND* bsound ){
    long bufsize = bsound->bufsize * bsound->num_chans;
    int i;
    double running_count = 0.0;
    for (i=0; i< bufsize; i++){
        running_count += (double) (input[i]*input[i]);
    }
    running_count /= bufsize;
    running_count = sqrt(running_count);
    return (float) running_count;
}
double dc_offset(float* input, BSOUND* bsound){
    long bufsize = bsound->bufsize * bsound->num_chans;
    int i;
    double running_count = 0.0;
    for (i=0; i<bufsize; i++){
        running_count += (double) input[i];
    }
    running_count /= bufsize;
    return running_count;
}
int rand_int(int min, int max){
    int return_val = rand()%(abs(max-min));
    return_val += min;
    return return_val;
}
MYFLT rand_float(MYFLT min, MYFLT max){
    int large_int= 4096<<5;
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
void copylefttoright(float* input, BSOUND* bsound, int inchannels){
    int i, frameCount = bsound->bufsize*bsound->num_chans, j, numchans = bsound->num_chans, k;
    for (j=0; j<numchans; j++){
        k = (j + inchannels)%numchans;
        for (i=j; i<frameCount;){
            input[k]=input[i];
            k += numchans;
            i += numchans;
        }
    }
    
}
