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

//TODO: time stretching using a read offset in partikkel

#include "opcodes.h"
#include "input_handling.h"
#include <stdio.h>
///this is a mess, but it is explained in input_handling.c
//maybe belongs in util_opcodes
MYFLT interval_to_float_conv(int16_t attr){
    MYFLT interval_convert_tab[32];
    int32_t display_index = attr/10, i;
    interval_convert_tab[0]=0.25f; interval_convert_tab[1]=0.375f;
    for (i=2; i<11; i++)
        interval_convert_tab[i] = (8.0+i-2.0)/16.0;
    for (i= 11; i<19; i++)
        interval_convert_tab[i] = (i-2.0)/8.0;
    interval_convert_tab[19]=2.5f;
    if (attr%10){
        return interval_convert_tab[display_index]*pow((interval_convert_tab[display_index+1]/interval_convert_tab[display_index]), attr%10/10.0);
    }
    else {
        return interval_convert_tab[display_index];
    }
}
MYFLT attr_to_freq_conv(int16_t attr){
    return 1.0 + pow(20000.0, attr/100.0);
}
void* init_delay(BSOUND* bsound, USR_IN type){
    DELAY_OPS* delay_struct = (DELAY_OPS*) calloc(sizeof(DELAY_OPS), 1);
    int16_t num_chans = bsound->num_chans;
    int32_t i, buf_length=0, padding = 100;
    MYFLT lp_damp;

    delay_struct->delay_length = (int32_t*)calloc(sizeof(int32_t)*num_chans, 1);
    delay_struct->feedback = calloc(sizeof(MYFLT*)*num_chans, 1);
    delay_struct->current_feedback = calloc(sizeof(MYFLT*)*num_chans, 1);
    delay_struct->weighting = calloc(sizeof(MYFLT*)*num_chans, 1);
    delay_struct->current_weighting = calloc(sizeof(MYFLT*)*num_chans, 1);
    delay_struct->current_spacing = (MYFLT*)malloc(sizeof(MYFLT)*num_chans);
    for (i = 0; i<num_chans; i++){
        delay_struct->feedback[i]= (MYFLT*) calloc(sizeof(MYFLT)*num_chans, 1);
        delay_struct->current_feedback[i]= (MYFLT*)calloc(sizeof(MYFLT)*num_chans, 1);
    }
    switch (type) {
        case DELAY:
            for (i=0; i<num_chans; i++){
                delay_struct->delay_length[i]=bsound->sample_rate*3/2;
                delay_struct->feedback[i][i]=1.0;//left to left and right to right
                delay_struct->feedback[i][(i+1)%num_chans]=0.0;//cross-over
            }
            delay_struct->lp_freq = 20000.0f;
            delay_struct->output_scaling = 0.9;
            delay_struct->num_taps = 4;
            delay_struct->spacing =(MYFLT*) malloc(sizeof(MYFLT)*num_chans);
            for (i=0; i<num_chans; i++){
                delay_struct->spacing[i] = 1.0/ 20;
            }
            for (i= 0; i<num_chans; i++){
                int32_t j;
                delay_struct->weighting[i] = (MYFLT*) calloc(sizeof(MYFLT)*delay_struct->num_taps, 1);
                delay_struct->weighting[i][0]= 1.0 * delay_struct->output_scaling;
                for (j=1; j<delay_struct->num_taps; j++){
                    delay_struct->weighting[i][j] = 0.1 * delay_struct->output_scaling;
                }
            }
            break;
        case TAPE:
            for (i=0; i<num_chans; i++){
                delay_struct->delay_length[i]=(bsound->sample_rate)/2;
                delay_struct->feedback[i][i]=0.5;//left to left and right to right
                delay_struct->feedback[i][(i+1)%num_chans]=0.5;//cross-over
            }
            delay_struct->lp_freq = 2500.0;
            delay_struct->output_scaling = 0.7;
            delay_struct->num_taps = 4;
            delay_struct->spacing =(MYFLT*) malloc(sizeof(MYFLT)*num_chans);
            for (i=0; i<num_chans; i++){
                delay_struct->spacing[i] = 1.5 / delay_struct->num_taps;
            }
            for (i= 0; i<num_chans; i++){
                delay_struct->weighting[i] = (MYFLT*) calloc(sizeof(MYFLT)*delay_struct->num_taps, 1);
                delay_struct->weighting[i][0]= 1.0 * delay_struct->output_scaling;
                int32_t j;
                for (j=1; j<delay_struct->num_taps; j++)
                delay_struct->weighting[i][j]=1.0*delay_struct->output_scaling/(j+1);
            }
            //printf("visting vintage shops and trying to look cool\n\n");
            break;
        case PPDEL:
            for (i=0; i<num_chans; i++){
                delay_struct->delay_length[i]=(bsound->sample_rate)/(1.5*i+2);
                delay_struct->feedback[i][i]=0.0f;//left to left and right to right
                delay_struct->feedback[i][(i+1)%num_chans]=1.0f;//cross-over
            }
            delay_struct->lp_freq = 20000.0;
            delay_struct->output_scaling = 0.56;
            delay_struct->num_taps = 4;
            delay_struct->spacing =(MYFLT*) malloc(sizeof(MYFLT)*num_chans);
            for (i=0; i<num_chans; i++){
                delay_struct->spacing[i] = 1.0 / (delay_struct->num_taps + 1);
            }
        for (i=0; i<num_chans; i++){
            delay_struct->weighting[i]=(MYFLT*)malloc(sizeof(MYFLT)*delay_struct->num_taps);
        }
        for (i=0; i<num_chans; i++){
            int32_t j;
            for (j=0; j<delay_struct->num_taps; j++){
                delay_struct->weighting[i][j]=delay_struct->output_scaling/((delay_struct->num_taps)-j) ;//* delay_struct->output_scaling;
            }
        }
            //printf("performing black magic on the feedback matrix\n\n");
            break;
        case MULTITAP:
            for (i=0; i<num_chans; i++){
                delay_struct->delay_length[i]              =(bsound->sample_rate)*3/2;
                delay_struct->feedback[i][i]               =0.0f;//left to left and right to right
                delay_struct->feedback[i][(i+1)%num_chans] =1.0f;//cross-over
            }
                delay_struct->lp_freq        = 2500.0;
                delay_struct->output_scaling = 0.74f;
                delay_struct->num_taps       = 4;
            delay_struct->spacing =(MYFLT*) malloc(sizeof(MYFLT)*num_chans);
            for (i=0; i<num_chans; i++){
                delay_struct->spacing[i] = 1.0 / (delay_struct->num_taps + 1);
            }
            for (i=0; i<num_chans; i++){
                delay_struct->weighting[i]=(MYFLT*)malloc(sizeof(MYFLT)*delay_struct->num_taps);
            }
            for (i=0; i<num_chans; i++){
                int32_t j;
                for (j=0; j<delay_struct->num_taps; j++){
                    delay_struct->weighting[i][j]=0.8f/((delay_struct->num_taps)-j) * delay_struct->output_scaling;
                }
            }
            //printf("accessing long forgotten quintuplet magic\n\n");
            break;
        default:
            //printf("error");
            break;
    }
    //calculate filter coefficient
    for (i=0; i<delay_struct->num_taps; i++)
        delay_struct->current_weighting[i]=(MYFLT*)malloc(sizeof(MYFLT)*delay_struct->num_taps);
    lp_damp                   = 2.0-cos(delay_struct->lp_freq*MY_2_PI/bsound->sample_rate);
    lp_damp                   = lp_damp - sqrt(lp_damp*lp_damp - 1.0);
    delay_struct->prv_lp_freq = delay_struct->lp_freq;
    delay_struct->lp_damp     = lp_damp;
    delay_struct->prv_y0      = (MYFLT *)malloc(sizeof(MYFLT)*num_chans);
for (i=0; i<num_chans; i++){
    buf_length = buf_length > delay_struct->delay_length[i]?buf_length:delay_struct->delay_length[i];
    delay_struct->prv_y0[i]=0.0;
}
delay_struct->aux = alloc_rngbuf(bsound, buf_length);
if (delay_struct->aux == NULL){
    puts("error allocating delay struct\n");
    exit (0);
}
    delay_struct->lines = malloc(sizeof(DELAY_LINE*)*bsound->num_chans);
    for (i= 0; i<bsound->num_chans; i++){
        delay_struct->lines[i] = alloc_delay_line(bsound, delay_struct->delay_length[i] + padding);
        delay_struct->lines[i]->read_index = (MYFLT)padding;
    }
    return (void*) delay_struct;
}
void dealloc_delay(BSOUND* bsound, void* data_st){
    DELAY_OPS* data = (DELAY_OPS *) data_st;
    int32_t i;
    for (i=0; i<bsound->num_chans; i++){
        free(data->feedback[i]);

    }
    free (data->feedback);
    dealloc_rngbuf(data->aux, bsound);
    free(data->delay_length);
}
void delay(float *input, float * output, void* data,const int16_t* attr, const BSOUND* bsound){
    DELAY_OPS* delay = (DELAY_OPS*) data;
    int32_t i, i2, ii, j, j1, k, num_chans;
    int64_t auxlength;
    int64_t frameCount    = bsound->bufsize;
    DELAY_LINE ** line = delay->lines;
    MYFLT** feedback   = delay->current_feedback;
    MYFLT dampfactor   = delay->lp_damp, y0;
    MYFLT volume       = pow(10.0, attr[4]/20.0);
    MYFLT interpolated, interpolated2, f0, f1, f2, f3, x, read_index, read_index2, incr, incr2;
    MYFLT* spacing = delay->current_spacing, **weighting = delay->current_weighting;
    int32_t x0, x1, x2, x3, line_length, line_length2;
    num_chans=bsound->num_chans;
    RNGBUF* aux = delay->aux;
    auxlength=aux->length;
    k=aux->index;
    //update DELAY_OPS from stack
    delay->lp_freq = attr_to_freq_conv(attr[2]);
    for (j= 0; j<num_chans; j++){
        for (i=0; i<num_chans; i++)
        feedback[j][i] = delay->feedback[j][i] * ((MYFLT)attr[1]/100.0);
    }
    //find largest weight
    int32_t largest_weight_x = 0; MYFLT my_largest_weight = 0.0;
    for (j=0; j<num_chans; j++){
        for (i=0; i<delay->num_taps; i++){
            if (delay->weighting[j][i] > my_largest_weight){
                my_largest_weight = delay->weighting[j][i];
                largest_weight_x=i;
        }
    }
    //as spread increases, increase small weights, decrease largest weight
        for (i=0; i<delay->num_taps; i++){
            weighting[j][i]= volume * delay->weighting[j][i] * ((MYFLT)attr[3]/100.0);
        }
        weighting[j][largest_weight_x] =volume * ( 1 - (1 - my_largest_weight)*((MYFLT)attr[3]/100.0));
    }
    for (i=0; i<num_chans; i++){
        MYFLT delay_time;
       if (line[i]->read_index < line[i]->index)
           delay_time =  line[i]->index - line[i]->read_index ;
        else
            delay_time =  line[i]->length - line[i]->read_index + line[i]->index;
        delay->current_spacing[i] = delay->spacing[i]*delay_time;
        if (((MYFLT)attr[0]/100.0)*line[i]->length != delay_time){
            delay_time -= ((MYFLT)attr[0]/100.0)*line[i]->length;
            if (delay_time<(-bsound->bufsize/32))
                delay_time =  - bsound->bufsize / 32;
            if (delay_time>bsound->bufsize/32)
                delay_time = bsound->bufsize / 32;
            line[i]->read_incr = (bsound->bufsize + delay_time) / bsound->bufsize;
        }
        else
            line[i]->read_incr = 1.0;
    }
    if (delay->prv_lp_freq != delay->lp_freq){
        dampfactor = 2.0-cos(delay->lp_freq*MY_2_PI/bsound->sample_rate);
        dampfactor = dampfactor-sqrt(dampfactor*dampfactor - 1.0);
        delay->lp_damp = dampfactor;
    }
    delay->prv_lp_freq = delay->lp_freq;
    for (j= 0; j<num_chans; j++){
        ii= j;
        j1 = (j+1)%num_chans;
        i= line[j]->index;
        i2 = line[j1]->index;
        read_index = line[j]->read_index;
        read_index2 = line[(j1)%num_chans]->read_index;
        incr = line[j]->read_incr;
        incr2 = line[j1]->read_incr;
        line_length = line[j]->length;
        line_length2 = line[j1]->length;
        y0 = line[j]->prv_y0;
        for (k = 0; k<frameCount; k++){
            read_index += incr;
            read_index2 += incr2;
            if (((int32_t)read_index)>=line_length ){read_index-=(MYFLT)line_length;}
            x0 = (int32_t)(read_index - 1.0);
            x1 = (int32_t) read_index;
            x2 = (int32_t)(read_index + 1.0);
            x3 = (int32_t)(read_index + 2.0);
            if (x0<0){x0+= line_length;}
            if (x2>=line_length){x2-= line_length;}
            if (x3>=line_length){x3-= line_length;}
            x = read_index - ((MYFLT) x1);
            f0 = line[j]->value[x0];
            f1 = line[j]->value[x1];
            f2 = line[j]->value[x2];
            f3 = line[j]->value[x3];
            interpolated= f1 + (((f3 - f0 - 3 * f2 + 3 * f1)* x + 3 * (f2 + f0 - 2*f1))* x - (f3 + 2*f0 - 6*f2 + 3* f1))*x/6.0;
            if (((int32_t)read_index2)>=line_length2 ){read_index2-=(MYFLT)line_length2;}
            x0 = (int32_t)(read_index2 - 1.0);
            x1 = (int32_t) read_index2;
            x2 = (int32_t)(read_index2 + 1.0);
            x3 = (int32_t)(read_index2 + 2.0);
            if (x0<0){x0+= line_length2;}
            if (x2>=line_length2){x2-= line_length2;}
            if (x3>=line_length2){x3-= line_length2;}
            x = read_index2 - ((MYFLT) x1);
            f0 = line[j1]->value[x0];
            f1 = line[j1]->value[x1];
            f2 = line[j1]->value[x2];
            f3 = line[j1]->value[x3];
            interpolated2= f1 + (((f3 - f0 - 3 * f2 + 3 * f1)* x + 3 * (f2 + f0 - 2*f1))* x - (f3 + 2*f0 - 6*f2 + 3* f1))*x/6.0;
            //so LP filter on input instead of output. let's see, i guess
            input[ii] = feedback[j][j]*interpolated + feedback[j1][j]*interpolated2 + input[ii] ;
            input[ii] = (y0 - input[ii])*dampfactor + input[ii];
            y0 = input[ii];
            line[j]->value[i++] = input[ii];
            output[ii] = weighting[j][0] * interpolated;
            ii += num_chans;
            if (i>=line_length){i-=line_length;}
        }
        line[j]->prv_y0 = y0;
        line[j]->index = i;
        line[j]->read_index = read_index;
    }
    for (j = 0; j<num_chans; j++){
        for (i=1; i<delay->num_taps; i++){
            ii = j;
            incr = line[j]->read_incr;
            line_length = line[j]->length;
            read_index = line[j]->read_index - i*spacing[j] - (MYFLT)frameCount;
            if (read_index < 0.0){read_index += (MYFLT) line_length;}
        for (k=0; k<frameCount; k++){
            read_index += incr;
            if (((int32_t)read_index)>=line_length ){read_index-=(MYFLT)line_length;}
            x0 = (int32_t)(read_index - 1.0);
            x1 = (int32_t) read_index;
            x2 = (int32_t)(read_index + 1.0);
            x3 = (int32_t)(read_index + 2.0);
            if (x0<0){x0+= line_length;}
            if (x2>=line_length){x2-= line_length;}
            if (x3>=line_length){x3-= line_length;}
            x = read_index - ((MYFLT) x1);
            f0 = line[j]->value[x0];
            f1 = line[j]->value[x1];
            f2 = line[j]->value[x2];
            f3 = line[j]->value[x3];
            interpolated= f1 + (((f3 - f0 - 3 * f2 + 3 * f1)* x + 3 * (f2 + f0 - 2*f1))* x - (f3 + 2*f0 - 6*f2 + 3* f1))*x/6.0;
            output[ii] += weighting[j][i]*interpolated;
            ii += num_chans;
                }
            }
    }

}

#define MAX_GRAIN_LENGTH 10000
void* init_partikkel(BSOUND* bsound, USR_IN type){
    PARTIKKEL_OPS* data = (PARTIKKEL_OPS*) malloc(sizeof(PARTIKKEL_OPS));

    switch (type){
        case CLOUD:
            data->grain_length        = 5000;
            data->frequency           =90; //every x samples one grain gets seeded
            data->max_val             = 0.22;
            data->max_dist            = bsound->sample_rate*4;//max displacement of grain in samples; should be smaller then RNGBUFs
            data->curr_dist           = data->max_dist;
            data->transpose_frequency = 30; //every x grains one gets transposed
            data->pitch_factor        =1.0; //transpose interval
            data->lp_freq             = 10000.0;
            //printf("spraying mist...\n\n");
            break;
        case SHIMMER:
            data->grain_length        = 6500;
            data->frequency           =90; //every x samples one grain gets seeded
            data->max_val             = 0.20f;
            data->max_dist            = bsound->sample_rate*3;//max displacement of grain in samples; should be smaller then RNGBUFs
            data->curr_dist           = data->max_dist;
            data->transpose_frequency = 5; //every x grains one gets transposed
            data->pitch_factor        =2.0f; //transpose interval
            data->lp_freq             = 1500.0;
            //printf("sprinkling glitter...\n\n");
            break;
        case TRANSPOSE:
            data->grain_length        = 5000;
            data->frequency           = 500; //every x samples one grain gets seeded
            data->max_val             = 0.55;
            data->max_dist            = 5000;//max displacement of grain in samples; should be smaller than RNGBUFs
            data->curr_dist           = 20;
            data->transpose_frequency = 1; //every x grains one gets transposed
            data->pitch_factor        =1.5; //transpose interval
            data->lp_freq             = 2500.0;
            //printf("activating robospeak...\n\n");
            break;
        default:
            break;
    }

    int32_t i;
    int32_t max_rand;
    double rand_a, rand_b;
    int32_t buf_length = bsound->sample_rate *4;
    data->in=alloc_rngbuf(bsound, buf_length);
    data->out = alloc_rngbuf(bsound, buf_length);
    data->err_term = malloc(sizeof(MYFLT*)*bsound->num_chans);
    for (i=0; i<bsound->num_chans; i++)
        data->err_term[i]=(MYFLT*)calloc(sizeof(MYFLT*)*buf_length, 1);
    data->disttab_index  = 0;
    data->disttab_length =4096<<5;

    //filter coefficients for LP filter
    data->lp_damp = 2.0-cos(data->lp_freq*MY_2_PI/bsound->sample_rate);
    data->lp_damp = data->lp_damp - sqrt(data->lp_damp*data->lp_damp - 1.0);
    data->transposed = malloc(sizeof(DELAY_LINE*)*bsound->num_chans);
    for (i=0; i<bsound->num_chans; i++){
        data->transposed[i]             = alloc_delay_line(bsound, buf_length);
        data->transposed[i]->read_index = 1.0f;
        data->transposed[i]->index      = 1;
    }
    data->prv_y0        = (MYFLT*) calloc(sizeof(MYFLT)*bsound->num_chans, 1);
    data->envtab_length = MAX_GRAIN_LENGTH;
    data->disttab       = (int32_t*) malloc(sizeof(int32_t)*(data->disttab_length));
    data->envtab        = (MYFLT*)malloc(sizeof(MYFLT)*data->envtab_length); //this is currently the max_env_tab_length
    data->phs = 0;
    data->transpose_phs = 0;

    max_rand = data->max_dist;
    for (i=0; i<data->disttab_length-1; ){ //gaussian distribution using box-muller transform
        rand_a = (double)(rand()%max_rand)/max_rand; //pair of random vals
        rand_b = (double)(rand()%max_rand)/max_rand;
        //transform here; fabs turns them into unipolar vals, rescaling makes sure they don't exceed max_rand
        data->disttab[i]=(max_rand/2)*fabs(pow(-2.0 * log(rand_a), 0.5) * cos(MY_2_PI*rand_b));
        data->disttab[i]=data->disttab[i]%max_rand;
        data->disttab[i+1]=(max_rand/2)*fabs(pow(-2.0 * log(rand_a), 0.5) * sin(MY_2_PI*rand_b));
        data->disttab[i+1]=data->disttab[i+1]%max_rand;
        i += 2;
    }
    for (i=0; i<data->envtab_length; i++){ //HANNING WINDOW
        data->envtab[i] = data->max_val*(0.5* (1-cos(2*M_PI*i/(data->envtab_length-1))));
    }
    data->transposed_index = (bsound->bufsize -2) - (data->pitch_factor*((int32_t)(bsound->bufsize/data->pitch_factor)));
    return (void *) data;
}
void dealloc_partikkel(BSOUND* bsound, void* data_st){
    PARTIKKEL_OPS* data = (PARTIKKEL_OPS *) data_st;
    free(data->disttab);
    free(data->envtab);
    dealloc_rngbuf(data->in, bsound);
    dealloc_rngbuf(data->out, bsound);
}

/// TODO: pan grains?
int16_t is_denormal(MYFLT in){
    if (fpclassify(in) == FP_SUBNORMAL)
        return 1;
    else
        return 0;
}
void partikkel(float*input, float*output, void* data_st, const int16_t* attr, const BSOUND* bsound){
    //int32_t counter = 0;
    PARTIKKEL_OPS* data = (PARTIKKEL_OPS*) data_st;
    RNGBUF* in = data->in;
    RNGBUF* out = data->out;
    DELAY_LINE** transposed=data->transposed;
    MYFLT silence = 0.0f;
    MYFLT volume = pow(10.0, attr[4]/20.0);
    MYFLT* env     = data->envtab;
    MYFLT** outval = out->value;
    MYFLT** inval  = in->value;
    MYFLT * inch;
    MYFLT *outch;
    MYFLT* err_term;
    MYFLT* transposed_val;
    int32_t* disttab = data->disttab, d_index = data->disttab_index, d_length = data->disttab_length, d_redraw, i;
    //update from attr
    data->grain_length = (int32_t)(((MYFLT)attr[2])/100.0*MAX_GRAIN_LENGTH);
    data->frequency = bsound->sample_rate/attr[1];
    data->curr_dist = (int32_t)((MYFLT)data->max_dist * (MYFLT)attr[0] /100.0);
    d_redraw = bsound->num_chans*bsound->bufsize / data->frequency + 6;
    MYFLT rand_a, rand_b; int32_t max_rand = data->curr_dist;
    //srand(time(NULL));
    for (i = 0; i<d_redraw; ){
        rand_a = (MYFLT)(rand()%max_rand)/max_rand; //pair of random vals
        rand_b = (MYFLT)(rand()%max_rand)/max_rand;
        //transform here; fabs turns them into unipolar vals, rescaling makes sure they don't exceed max_rand
        disttab[d_index]=(max_rand/2)*fabs(pow(-2.0 * log(rand_a), 0.5) * cos(MY_2_PI*rand_b));
        disttab[d_index]=disttab[d_index]%max_rand;
        if (++d_index>=d_length){d_index-=d_length;}
        disttab[d_index]=(max_rand/2)*fabs(pow(-2.0 * log(rand_a), 0.5) * sin(MY_2_PI*rand_b));
        disttab[d_index]=data->disttab[d_index]%max_rand;
        if(++d_index>=d_length){d_index-=d_length;}
        i += 2;
    }
    int16_t num_chans = bsound->num_chans;
    int64_t frameCount = bsound->bufsize;
    int32_t ii, j, jj, jjj, k,   kk, transposed_length;
    int32_t to_write, grain_length = data->grain_length, frequency = data->frequency, transpose_freq = data->transpose_frequency;
    int64_t inlength = in->length;
    MYFLT pitch_factor = interval_to_float_conv(attr[3]);
    //for filter
    MYFLT y0, hi_damp;//, y1;
    MYFLT dampfactor = data->lp_damp, samp;
    //interpolation
    MYFLT f0, f1, f2, f3;
    MYFLT x, index, max_val = data->max_val;
    MYFLT feedback = (MYFLT)attr[7]/100.0;
    // this limits the amount of available feedback at low spread settings
    //  spread
    if (attr[0]<10){
        int16_t new_attr;
        if (attr[7]>50){
            new_attr = (int16_t) (50.0 + (attr[0])/10.0 * (attr[7]-50.0));
            feedback = (MYFLT) new_attr/100.0;
    }
    }
    int32_t x0, x1, x2, x3;
    //check if the grain_length has changed since last k-cycle
    if (data->envtab_length != grain_length){
        for (i=0; i<grain_length; i++){
            env[i]= max_val*(0.5*(1-cos(2*M_PI*i/(grain_length-1))));
        }
        data->envtab_length = grain_length;
    }
    // read input into input_buffer
    for (j = 0; j<num_chans; j++){
        k=in->index+in->available;
        kk=out->index - bsound->num_chans*bsound->bufsize;
        if (kk<0){kk+=inlength;}
        inch = inval[j];
        outch =outval[j];
        hi_damp = bsound->hi_damp;
        y0 = data->prv_y0[j];
       if(k>=inlength){k-=inlength;}
        ii=j;
        kk+=j;
        for (i=0; i<frameCount; i++){
            MYFLT my_in = input[ii] +atan(feedback*outch[kk++])/1.4; //+0.9*erf(feedback*outch[kk++]);
            samp = my_in;
            samp = (y0+samp)*hi_damp;
            y0 = samp - my_in;
            inch[k]=samp;// reads input
            ii+=num_chans;
            k++;
            if (kk>=inlength){kk=0;}
            if (k>=inlength){k=0;}
        }
        data->prv_y0[j]=y0;
        x2 = k-2;
        if (x2<0){x2+=inlength;}
        f2 = inch[x2];
        x3 = k -1;
        if (x3<0){x3+=inlength;}
        f3 = inch[x3];
        inch[k]= 2*f3 - f2;
    }
        for (j= 0; j<num_chans; j++){
            transposed_length = transposed[j]->length;
            y0 = transposed[j]->prv_y0;
            to_write = (MYFLT)frameCount/pitch_factor;
            transposed_val = transposed[j]->value;
            ii = (int32_t)(transposed[j]->read_index);
            index = data->transposed_index;//(in->index + in->available - 2  + frameCount) - (pitch_factor*((int32_t)(frameCount/pitch_factor))) ;
            if (index<0){index += inlength;}
            for (i=0; i<to_write; i++){
                //index reads inval
                //i is read_index
                index += pitch_factor;
                if (((int32_t)index)<0){index+= inlength;}
                if (((int32_t)index)>=inlength){index-=(MYFLT)inlength;}
                x0 = (int32_t)(index-1.0);
                x1 = (int32_t) index;
                x2 = (int32_t)(index +1.0);
                x3 = (int32_t)(index + 2.0);
                if (x0<0){x0+= inlength;}
                if (x2>=inlength){x2-= inlength;}
                if (x3>=inlength){x3-= inlength;}
                x = index - ((MYFLT) x1);
                f0 = inval[j][x0];
                f1 = inval[j][x1];
                f2 = inval[j][x2];
                f3 = inval[j][x3];
                samp= f1 + (((f3 - f0 - 3 * f2 + 3 * f1)* x + 3 * (f2 + f0 - 2*f1))* x - (f3 + 2*f0 - 6*f2 + 3* f1))*x/6.0;
                //first order IIR filter
                samp = (y0 - samp)*dampfactor +  samp;
                y0 =  samp;
                transposed_val[ii]=samp;
                ii++;
                if (ii>= transposed_length){ii = 0;}
            }
            //update y0
            transposed[j]->prv_y0 = y0;
            transposed[j]->read_index = (MYFLT) ii;
            }
    data->transposed_index = index;
    in->available += frameCount;

    to_write = pitch_factor>1.0?(MYFLT)in->available/pitch_factor: in->available;
    to_write -= grain_length;
    if (to_write > 0) // if there is enough in the RNGBUF to write one grain
    {
        if (pitch_factor != 1.0){
                jj = data->phs; //phs counter to ensure grain freq is constant over ksmp-periods
                jjj = data->transpose_phs; //phs counter to ensure transpose freq is constant over ksmps periods
                for (j=0; j<num_chans; j++){
                    inch     = inval[j];
                    outch    = outval[j];
                    err_term = data->err_term[j];
                    transposed_val=transposed[j]->value;
                for (i=1; i<to_write; i++){
                        if ((jj%frequency)==0){
                            if ((jjj%transpose_freq)==0){
                                k = transposed[j]->read_index - grain_length-to_write + (int32_t)((float)i)/pitch_factor;
                                if (k<0){k+=inlength;}//transposed[j]->index + (int32_t)((float)i)/pitch_factor;
                                //inlength should be the same as transposed length
                                if (k>=inlength){k-=inlength;}
                                kk=(data->disttab[data->disttab_index++]%data->curr_dist)+data->out->index;//read from dist-tab
                                if (data->disttab_index>=data->disttab_length){data->disttab_index =0;}
                                if (kk>=inlength){kk-=inlength;}
                                MYFLT val, prv_val;
                                if (attr[6]==0){
                                    for (ii=0; ii<grain_length; ii++){
                                        val = transposed_val[k++]*env[ii];
                                        prv_val = outch[kk];
                                        outch[kk]   +=val;
                                        err_term[kk]+= outch[kk]- val - prv_val;
                                        kk++;
                                        //at appropriate point (determined by disttab) writes values in "in" multiplied by env (channel j) to out
                                        if (kk>=inlength){kk=0;}
                                        if (k>=inlength){k=0;}
                                    }
                                }
                                else {
                                    k += grain_length;
                                    if (k>=inlength){k-=inlength;}
                                    for (ii=0; ii<grain_length; ii++){
                                        val = transposed_val[k--]*env[ii];
                                        prv_val = outch[kk];
                                        outch[kk]+= val;
                                        err_term[kk]+=outch[kk]-val-prv_val;
                                        kk++;
                                        if (kk>=inlength){kk=0;}
                                        if (k<0){k+=inlength;}
                                    }
                                }

                        }

                            else {
                            if (attr[6]==0)
                                k=in->index + i; //read index
                            else
                                k=in->index+i+grain_length;
                            if (k>= inlength ){k-=inlength;}

                            kk=(data->disttab[data->disttab_index++]%data->curr_dist)+out->index;//read from dist-tab

                            if (data->disttab_index>=data->disttab_length){data->disttab_index =0;}
                            if (kk>=inlength){kk-=inlength;}
                                MYFLT val, prv_val;
                                if (attr[6]==0){
                                for (ii=0; ii<grain_length; ii++){
                                    val = inch[k++]*env[ii];
                                    prv_val = outch[kk];
                                    outch[kk]   +=val;
                                    err_term[kk]+= outch[kk]- val - prv_val;
                                    kk++; //at appropriate point (determined by disttab) writes values in "in" multiplied by env (channel j) to out

                                    if (k>=inlength){k=0;}
                                    if (kk>=inlength){kk=0;}
                                }
                                }
                            else{
                                for (ii=0; ii<grain_length; ii++){
                                    val = inch[k--]*env[ii];
                                    prv_val = outch[kk];
                                    outch[kk]   +=val;
                                    err_term[kk]+= outch[kk]- val - prv_val;
                                    kk++; //at appropriate point (determined by disttab) writes values in "in" multiplied by env (channel j) to out

                                    if (k<0){k+=inlength;}
                                    if (kk>=inlength){kk=0;}
                                }
                                }
                        }

                            jjj++;
                        }
                        jj++;
                }
                }
                data->phs = jj; //updates phs counter in PARTIKKEL_OPS struct
            data->transpose_phs = jjj; //same for transpose counter
            }
        else {
        jj = data->phs; //phs counter to ensure grain freq is constant over ksmp-periods
        for (j=0; j<num_chans; j++){
            inch     = inval[j];
            outch    = outval[j];
            err_term = data->err_term[j];
            for (i=0; i<to_write; i++){
                if ((jj%frequency)==0){
                    if (attr[6]==0)
                    k=in->index + i; //read index
                    else
                        k=in->index+i+grain_length;
                    if (k>= inlength ){k-=inlength;}

                    kk=(data->disttab[data->disttab_index++]%data->curr_dist)+data->out->index;//read from dist-tab

                    if (data->disttab_index>=data->disttab_length){data->disttab_index =0;}
                    if (kk>=inlength){kk-=inlength;}
                    MYFLT val, prv_val;
                    if (attr[6]==0){
                    for (ii=0; ii<grain_length; ii++){
                        val = inch[k++]*env[ii];
                        prv_val = outch[kk];
                        outch[kk]   +=val;
                        err_term[kk]+= outch[kk]- val - prv_val;
                        kk++; //at appropriate point (determined by disttab) writes values in "in" multiplied by env (channel j) to out

                        if (k>=inlength){k=0;}
                        if (kk>=inlength){kk=0;}
                    }
                    }
                else{
                    for (ii=0; ii<grain_length; ii++){
                        val = inch[k--]*env[ii];
                        prv_val = outch[kk];
                        outch[kk]   +=val;
                        err_term[kk]+= outch[kk]- val - prv_val;
                        kk++; //at appropriate point (determined by disttab) writes values in "in" multiplied by env (channel j) to out

                        if (k<0){k+=inlength;}
                        if (kk>=inlength){kk=0;}
                    }
                    }

                }
                jj++;
            }
            data->phs = jj; //updates phs counter in PARTIKKEL_OPS struct
        }
        }
        in->available-=to_write; //updates in status
        in->index+=to_write;
        if(in->index>=inlength){in->index -= inlength;}
        for (i=0; i<num_chans; i++){
            transposed[i]->index += (int32_t)(((float)to_write)/pitch_factor);
            if (transposed[i]->index>=inlength){transposed[i]->index-=inlength;}
        }
    }


    for (j = 0; j<num_chans; j++){
        k=out->index;
        kk = out->index - bsound->bufsize*bsound->num_chans;
        if (kk<0){kk+=inlength;}
        ii=j;
        outch= outval[j];
        err_term = data->err_term[j];
        for (i=0; i<frameCount; i++){
            output[ii]=(outch[k]-err_term[k])*volume; //writes output
            outch[kk]    =silence;
            err_term[kk] = silence;
            kk++;
            if (output[ii]>= 1.0){output[ii]=0.999f; /*bsound->out_of_range += 1;*/}
            if (output[ii]<= -1.0){output[ii]=-0.999f;/*bsound->out_of_range += 1;*/}
            ii+=num_chans;
            k++;
            if (k>=inlength){k=0;}
            if (kk>=inlength){kk=0;}
        }

    }

    out->index = k;
  /*  char outstring[20];
    sprintf(outstring, "%d", counter);
    if (data->phs % 30000 > 1000 && data->phs % 30000 < 2000)
    error_message(outstring, bsound);*/

}
#define MAX_RANDOMIZE_SPEED (4800.0)
void* init_reson(BSOUND* bsound, USR_IN type){
    RESON_OPS* data = NULL;
    data = (RESON_OPS *) calloc(sizeof(RESON_OPS), 1);
    if (data == NULL)
        return NULL;
    int32_t i, padding = 200;
    int32_t linelengths[12];
    double semitone_ratio;
    MYFLT lp_cutoff = bsound->sample_rate/2.0;
    switch (type) {
        case REVERB:
            data->num_lines = 8;
            int32_t sean_carrol[]={2473, 2767, 3217, 3557, 3907, 4127, 2143, 1933}; //these lengths are "adapted" from sean carroll
            for(i=0; i<8; i++) {linelengths[i]=sean_carrol[i];}
            data->fdbk = 0.5;//0.95f;
            data->output_scaling = 0.44;//0.18f;
            lp_cutoff = 10000.0f; //in hz thanks to john ffitch
            data->random_speed = 900; //k-cycles on average
            break;
        case PEDAL:
            data->num_lines = 12;
            linelengths[0]=bsound->sample_rate / (440.0/16.0);
            semitone_ratio = pow(2.0, 1.0/12.0);
            for (i=1; i<12; i++){
                linelengths[i]= ((double)linelengths[i-1])*semitone_ratio;
            }
            data->fdbk =0.5f;
            data->output_scaling  =0.37f;
            lp_cutoff = 15000.0f;
            data->random_speed = MAX_RANDOMIZE_SPEED;
            break;
        default:
            break;
    }
    //housekeeping
    data->in = alloc_rngbuf(bsound, bsound->sample_rate);
    if (data->in == NULL) {
        //puts( "ERROR allocating memory for delay_line");
        return NULL;
    }
    data->out = alloc_rngbuf(bsound, bsound->sample_rate);
    if (data->out == NULL) {
       //puts( "ERROR allocating memory for delay_line");
        return NULL;
    }
    data->resonator = malloc(sizeof(DELAY_LINE*)*data->num_lines);
    if (data->resonator ==NULL){
       // puts( "ERROR allocating memory for delay_line");
        return NULL;
    }
    for (i=0; i<data->num_lines; i++){
        data->resonator[i]=alloc_delay_line(bsound, (linelengths[i])+ padding);
        if (data->resonator[i]==NULL){
          //  puts( "ERROR allocating memory for delay_line");
            return NULL;
        }
        data->resonator[i]->read_index =(MYFLT) padding;
        data->resonator[i]->linseg_rmns = 0;
    }
    data->lp_damp = 2-cos(lp_cutoff *MY_2_PI/bsound->sample_rate);
    data->lp_damp = data->lp_damp - sqrt(data->lp_damp*data->lp_damp - 1.0);
    data->lp_freq = lp_cutoff;
    data->prv_lp_freq = data->lp_freq;
    for (i=0; i<data->num_lines; i++){
        randomize_delay_line(data->resonator[i], bsound, data);
    }
    data->allpass1 = init_allpass(bsound, 1553);
    data->allpass2 = init_allpass(bsound, 1277);
    data->allpass3 = init_allpass(bsound, 1871);
    data->allpass4 = init_allpass(bsound, 1601);
    return data;
}
void dealloc_reson(BSOUND* bsound, void* data_st){
    RESON_OPS* data = (RESON_OPS*) data_st;
    int32_t i;
    for (i=0; i<data->num_lines; i++){
    dealloc_delay_line(data->resonator[i], bsound);
    }
    free (data->resonator);
    dealloc_rngbuf(data->in, bsound);
    dealloc_rngbuf(data->out, bsound);
    dealloc_allpass(bsound, (void*)data->allpass1);
    dealloc_allpass(bsound, (void*)data->allpass2);
    dealloc_allpass(bsound, (void*)data->allpass3);
    dealloc_allpass(bsound, (void*)data->allpass4);
}
void randomize_delay_line(DELAY_LINE* line, const BSOUND* bsound, RESON_OPS* data){
    MYFLT current_delay = fabs( ((MYFLT)line->index) - line->read_index);
    MYFLT max_delay     = (MYFLT) line->length;
    MYFLT rand_max      = max_delay-current_delay<100?max_delay-current_delay :100;
    MYFLT rand_min      = current_delay-100<0?0                               :-100;
    int32_t linseg_x        = rand_int(data->random_speed * 0.9, data->random_speed * 1.1); //k-cycles
    MYFLT linseg_y      = rand_float(rand_min, rand_max);
    if (data->random_speed == -1){
        line->read_incr = 1.0;
        line->linseg_rmns = MAX_RANDOMIZE_SPEED;
    }
    else{
    line->read_incr   = 1.0 + (linseg_y/linseg_x)/bsound->bufsize;
    line->linseg_rmns = linseg_x;
    }
}
void reson(float *input, float* output, void* data_st, const int16_t* attr, const BSOUND* bsound){
    RESON_OPS* data = (RESON_OPS*) data_st;
    RNGBUF* in = data->in;
    RNGBUF* out = data->out;
    MYFLT output_scaling = data->output_scaling * pow(10.0, attr[3]/20.0);
    int16_t num_chans = bsound->num_chans;
    int64_t frameCount = bsound->bufsize;
    MYFLT *inch, *outch;
    MYFLT *line, fdbk, dampfactor = data->lp_damp;
    MYFLT y0;
    //interpolation
    int32_t x0, x1, x2, x3;
    MYFLT f0, f1, f2, f3, int_indx, interpolated, x;
    //reading indices
    int32_t i, ii, j,jj, k, kk;
    int32_t inlength = in->length, line_length;
    data->lp_freq = (attr[1]/100.0)*10000;
    fdbk=data->fdbk = (attr[0]/100.0);
    if (data->num_lines == 12){
        output_scaling = data->output_scaling = 0.61375-0.4875*data->fdbk;
    }
    else if (data->num_lines == 8){
       output_scaling = data->output_scaling = 0.7289-0.5778*data->fdbk;
    }
    if (attr[2]){
    data->random_speed = (int32_t)( MAX_RANDOMIZE_SPEED / 5* ((100.0-attr[2])/100.0))+15;
    data->random_speed *= 128.0;
    data->random_speed /= bsound->bufsize;
    }
    else
        data->random_speed = -1;
    if (data->lp_freq != data->prv_lp_freq){
        dampfactor = 2-cos(data->lp_freq *MY_2_PI/bsound->sample_rate);
        dampfactor = dampfactor - sqrt(dampfactor*dampfactor - 1.0);
        data->lp_damp = dampfactor;
        data->prv_lp_freq = data->lp_freq;
    }
    for (j = 0; j<num_chans; j++){
        k=in->index;
        inch = in->value[j];
        ii=j;
        for (i=0; i<frameCount; i++){
            inch[k]=input[ii];// reads input
            ii+=num_chans;
            k++;
            if (k>=inlength){k=0;}
        }
    }
        allpass(in, 0.3, data->allpass1, bsound);
        allpass(in, 0.3, data->allpass2, bsound);
        allpass(in, 0.3, data->allpass3, bsound);
        allpass(in, 0.3, data->allpass4, bsound);
    for (j=0; j<data->num_lines; j++){
        jj=j%bsound->num_chans;
        //buffers
        inch = in->value[jj];
        outch = out->value[jj];
        line = data->resonator[j]->value;
        //input index
        ii=in->index;
        //delay index
        kk=data->resonator[j]->index;
        //interpolation index
        int_indx = data->resonator[j]->read_index;
        line_length = data->resonator[j]->length;
        //out index
        k = out->index;
        y0 = data->resonator[j]->prv_y0;
        for (i=0; i<frameCount; i++){
            //write to delay lines
            line[kk]=inch[ii];
            //read value with interpolation
            int_indx += data->resonator[j]->read_incr;
            if (((int32_t)int_indx)>=line_length ){int_indx-=(MYFLT)line_length;}
            x0 = (int32_t)(int_indx - 1.0);
            x1 = (int32_t) int_indx;
            x2 = (int32_t)(int_indx + 1.0);
            x3 = (int32_t)(int_indx + 2.0);
            if (x0<0){x0+= line_length;}
            if (x2>=line_length){x2-= line_length;}
            if (x3>=line_length){x3-= line_length;}
            x = int_indx - ((MYFLT) x1);
            f0 = line[x0];
            f1 = line[x1];
            f2 = line[x2];
            f3 = line[x3];
            interpolated= f1 + (((f3 - f0 - 3 * f2 + 3 * f1)* x + 3 * (f2 + f0 - 2*f1))* x - (f3 + 2*f0 - 6*f2 + 3* f1))*x/6.0;
            //combine with y0 for filtering
            //!first order IIR => y[n]=x[n]+b*(y[n-1]-x[n])
            //!b has to be between 0 and 0.5 and equations above make sure dampfactor is within these bounds
            //!and further make sure that lp_cut_off can be in hertz (thanks to Istvan Varga)
//TODO: ???should fdbk multiplication happen before writing to output or before writing to input???
            interpolated = (y0-interpolated)*dampfactor + interpolated;
            // set y0 to value
            y0 = interpolated;
            //add to output and back to input of line
            outch[k++] +=interpolated;
            interpolated*= fdbk;
            line[kk++]+=interpolated;
            ii++;
            if (ii>=inlength){ii =0;}
            if (k>=inlength){k=0;}
            if (kk>=line_length){kk = 0;}

        }
        //save y0 for next k-cycle
        data->resonator[j]->prv_y0 = y0;
        data->resonator[j]->index = kk;
        data->resonator[j]->read_index = int_indx;
        //if we're at the end of our linseg, calculate new read_incr and linseg_rmns
        if(--(data->resonator[j]->linseg_rmns)== 0){
            randomize_delay_line(data->resonator[j], bsound, data);
        }
    }
    for (j = 0; j<num_chans; j++){
        k     = out->index;
        outch = out->value[j];
        ii = j;
        for (i=0; i<frameCount; i++){
            output[ii]=outch[k] * output_scaling; //writes output
            outch[k]=0;
            ii+=num_chans;
            k++;
            if (k>=inlength){k=0;}
        }
    }
    out->index = k;
}
void* init_allpass(BSOUND* bsound, int32_t buf_length){
    ALLPASS_OPS* data = (ALLPASS_OPS*)malloc(sizeof(ALLPASS_OPS));
    data->aux = alloc_rngbuf(bsound, buf_length);
    return data;
}
void dealloc_allpass(BSOUND* bsound, void* data){
    ALLPASS_OPS* data_st = (ALLPASS_OPS*) data;
    dealloc_rngbuf(data_st->aux, bsound);
    free(data_st);
}
void allpass(RNGBUF* buf, const MYFLT feedback, ALLPASS_OPS* data, const BSOUND* bsound){
    int32_t i, ii, j, k, frame_count = bsound->bufsize, inlength, auxlength;
    MYFLT* inch, *aux;
    int16_t num_chans = bsound->num_chans;
    for (j=0; j<num_chans; j++){
        inch = buf->value[j];
        aux = data->aux->value[j];
        ii = buf->index;
        k = data->aux->index;
        inlength = buf->length;
        auxlength = data->aux->length;
        MYFLT y, z;
        for(i=0; i<frame_count; i++){
            y = aux[k];
            z = aux[k++]=inch[ii] + feedback*y;
            inch[ii]= y - z * feedback ;
            ii++;
            if (k>=auxlength){k=0;}
            if (ii>=inlength){ii=0;}
        }
    }
}
///@todo add bias control to moddemod
void* init_moddemod(BSOUND* bsound, USR_IN type){
    MODDEMOD_OPS* data = (MODDEMOD_OPS*) malloc(sizeof(MODDEMOD_OPS));
    int32_t i;
    data->tab_length = 16384; //magic numbers FTW
    data->modulator = (MYFLT *)malloc(sizeof(MYFLT)*data->tab_length);
    MYFLT incr = MY_2_PI/data->tab_length;
    switch (type){
        case MODDEMOD:
            for (i=0; i<data->tab_length; i++){
                data->modulator[i]=(MYFLT) sin(i*(double)MY_2_PI/data->tab_length);
                data->modulator[i]=data->modulator[i]*data->modulator[i]*1.57;
            }
            data->square = 1;
            break;
        case RINGMOD:
            for (i=0; i<data->tab_length; i++){
                data->modulator[i]=sin(i*incr)/0.707;
            }
            data->square = 0;
            break;
        default:
            break;
    }
    data->freq = 500.0;
    data->mod_freq = 173.0;
    data->mod_index = data->tab_length/4.0;
    data->index = 1.0;
    data->prv_y0 = (MYFLT*) calloc(sizeof(MYFLT)*bsound->num_chans, 1);
    data->prv_fold =0;
    data->prv_offset = 0;
    return (void*)data;
}
void dealloc_moddemod(BSOUND* bsound, void* data){
    MODDEMOD_OPS* data_st = (MODDEMOD_OPS *) data;
    free(data_st->modulator);
    free(data_st);
}
void moddemod(float* input, float* output, void* data_st, const int16_t* attr, const BSOUND* bsound){
    MODDEMOD_OPS* data = (MODDEMOD_OPS *) data_st;
    int32_t j, i, ii, frameCount = bsound->bufsize;
    int16_t num_chans = bsound->num_chans;
    int32_t tab_length = data->tab_length;
    MYFLT current_index = data->index, x;
    MYFLT volume = pow(10.0, attr[5]/20.0);
    int32_t x0, x1, x2, x3;
    MYFLT f0, f1, f2, f3, interpolated;
    MYFLT y0, dampfactor = bsound->hi_damp, samp;
    data->freq = 2.5+ 1000.0*(pow((attr[0]/100.0), 3.0));
    data->mod_freq = 2.5+1000.0*(pow((attr[1]/100.0), 3.0));
    MYFLT read_incr = data->freq*data->tab_length/bsound->sample_rate;
    MYFLT mod_incr = data->mod_freq*data->tab_length/bsound->sample_rate;
    MYFLT mod_index;
    MYFLT *modulator = data->modulator;
    if (attr[3]!=data->prv_fold || attr[4]!= data->prv_offset){
        MYFLT fold_val = (MYFLT) attr[3]/20.0;
        fold_val += 1.0;
        MYFLT offset_val = (MYFLT) attr[4]/25.0;
        offset_val *= M_PI_2;
        MYFLT incr = MY_2_PI/tab_length;
        if (data->square){
            for (j=0; j<tab_length; j++){
                modulator[j]=sin(fold_val*sin(j*incr)+offset_val);
                modulator[j]=modulator[j]*modulator[j]*1.57;
            }
        }
        else {
        for (j=0; j<tab_length; j++)
        modulator[j]=sin(fold_val*sin(j*incr)+offset_val)/0.707;
        }
        data->prv_fold=attr[3]; data->prv_offset = attr[4];
    }
    for (j=0; j<num_chans; j++){
        ii = j;
        current_index = data->index;
        mod_index = data->mod_index;
        y0 = data->prv_y0[j];
        for (i=0; i<frameCount; i++){
            current_index +=read_incr;
            mod_index +=mod_incr;
            if (((int32_t)mod_index)>=tab_length){mod_index-=(MYFLT)tab_length;}
            current_index += attr[2]*modulator[(int32_t)mod_index];
            if (((int32_t)current_index)>=tab_length){current_index-=(MYFLT)tab_length;}
            if (((int32_t)current_index)<0){current_index+=(MYFLT)tab_length;}
                x0 = (int32_t)(current_index - 1.0);
                x1 = (int32_t) current_index;
                x2 = (int32_t)(current_index + 1.0);
                x3 = (int32_t)(current_index + 2.0);
                if (x0<0){x0+=tab_length;}
                if (x2>=tab_length){x2-=tab_length;}
                if (x3>=tab_length){x3-=tab_length;}
                x = current_index - ((MYFLT)x1);
                f0 = modulator[x0];
                f1 = modulator[x1];
                f2 = modulator[x2];
                f3 = modulator[x3];
            interpolated = f1 + (((f3 - f0 - 3 * f2 + 3 * f1)* x + 3 * (f2 + f0 - 2*f1))* x - (f3 + 2*f0 - 6*f2 + 3* f1))*x/6.0;
            samp = input[ii];
            samp = (y0+samp)*dampfactor;
            y0 = samp - input[ii];
            output[ii]=samp*interpolated*volume;
            ii += num_chans;
        }
        data->prv_y0[j]=y0;
    }
    data->index = current_index;
    data->mod_index = mod_index;
}
void* init_crush(BSOUND* bsound, USR_IN type){
    CRUSH_OPS* data = (CRUSH_OPS*)malloc(sizeof(CRUSH_OPS));
    int32_t i;
    data->read_factor = 0.15;
    data->interpolate = 1;
    data->tab_length = bsound->bufsize*4;
    data->samp_reduced = malloc(sizeof(MYFLT*)*bsound->num_chans);
    data->in_buffer = malloc(sizeof(MYFLT*)*bsound->num_chans);
    for (i= 0; i<bsound->num_chans; i++){
        data->samp_reduced[i] = (MYFLT*) calloc(sizeof(MYFLT)*(data->tab_length), 1);
        data->in_buffer[i] = (MYFLT*) calloc(sizeof(MYFLT)*data->tab_length, 1);
    }
    data->in_index = 0;
    data->samp_index = 0;
    data->prv_y0 = (MYFLT*)calloc(sizeof(MYFLT)*bsound->num_chans, 1);
    MYFLT damp_factor;
    damp_factor = 2.0-cos(data->read_factor*MY_2_PI/1.5);
    damp_factor = damp_factor-sqrt(damp_factor*damp_factor-1.0);
    data->damp_factor = damp_factor;
    data->prv_y0_hipass = (MYFLT*)calloc(sizeof(MYFLT)*bsound->num_chans, 1);
    data->in_read = (bsound->bufsize -2)-((1.0/data->read_factor)*((int32_t)bsound->bufsize*data->read_factor));
    data->samp_read = (bsound->bufsize -2)-(data->read_factor*((int32_t)bsound->bufsize/data->read_factor));

    return (void*)data;
}
    void dealloc_crush(BSOUND* bsound, void* data){
        CRUSH_OPS* data_st = (CRUSH_OPS*) data;
        free(data_st->samp_reduced);
        free(data_st->prv_y0);
        free(data_st);
}
float signum(float in){
    return (in > 0) ? 1 : ((in < 0) ? -1 : 0);
}
void crush(float* input, float* output, void* data_st, const int16_t* attr, const BSOUND* bsound){
    CRUSH_OPS* data = (CRUSH_OPS*) data_st;
    //if we're not interpolating quantize
    if (attr[1])
        data->read_factor = attr_to_freq_conv(attr[0]) / bsound->sample_rate;
    else{
        data->read_factor = attr_to_freq_conv(attr[0]) / bsound->sample_rate;
        data->read_factor = 1.0/((int32_t)(1.0/data->read_factor));
    }
    data->lp_freq = attr_to_freq_conv(attr[2]);
    MYFLT read_factor =  1.0f / data->read_factor;
    MYFLT volume = pow(10.0, attr[4]/20.0);
    MYFLT* in_tab;
    MYFLT** samp_reduced = data->samp_reduced;
    int32_t x0, x1, x2, x3;
    MYFLT f0, f1, f2, f3, x, index;
    MYFLT samp, y0, damp_factor = data->damp_factor;
    int16_t num_chans = bsound->num_chans;
    int32_t i, ii, j, jj, tab_length = data->tab_length;
    int64_t frameCount = bsound->bufsize;
    if (data->lp_freq != data->prv_lp_freq){
        damp_factor = 2.0-cos(data->lp_freq*MY_2_PI/bsound->sample_rate);
        damp_factor = damp_factor-sqrt(damp_factor*damp_factor-1.0);
        data->prv_lp_freq = data->lp_freq;
        data->damp_factor = damp_factor;
    }
    ///if bit-depth is reduced
    if (attr[3]){
        frameCount= bsound->bufsize*bsound->num_chans;
        // max_val converts attr[3] and scales it
        int32_t val; float max_val = (10.0 - attr[3]) / 9.0;
        max_val = 32.0 + max_val * max_val *max_val *max_val* 8160;
        double mu_denominator = log(1+255), one_by_mu = 1.0/255.0;
        for (i=0; i<frameCount; i++){
            input[i]= signum(input[i])*log(1+255*fabsf(input[i]))/mu_denominator;
            val = (int32_t) (max_val * input[i]);
            input[i]= ((float)val)/max_val;
            input[i]=signum(input[i])*one_by_mu*(pow(266, fabsf(input[i])) -1);
        }
    }
    frameCount = bsound->bufsize;
    for (j=0; j<num_chans; j++){
        jj = j;
        in_tab = data->in_buffer[j];
        ii = data->in_index;
        for (i= 0; i< frameCount; i++){
           in_tab[ii++] = input[jj] * volume;
            if (ii>=tab_length){ii=0;}
            jj += num_chans;
        }
        x2 = ii-2;
        if (x2<0){x2+=tab_length;}
        f2 = in_tab[x2];
        x3 = ii -1;
        if (x3<0){x3+=tab_length;}
        f3 = in_tab[x3];
        in_tab[ii]= 2*f3 - f2;
    }
    data->in_index = ii;

    frameCount = frameCount  * data->read_factor;
    for (j=0; j<num_chans;j++){
                // read in input-buffer
        index = data->in_read;
        if ((int32_t)index < 0){index += (MYFLT)tab_length;}
        if ((int32_t)index > tab_length){index -=tab_length;}
        in_tab = data->in_buffer[j];
        //write index in SR-reduced buffer
        ii =data->samp_index;
        y0 = data->prv_y0[j];
        //frameCount is new frameCount
        read_factor = (MYFLT) bsound->bufsize/frameCount;
        for (i= 0; i<frameCount; i++){
            index += read_factor ;
            if ((int32_t)index >= tab_length){index -= tab_length;}
            x0 = (int32_t) index -1.0;
            if (x0<0){x0+= tab_length;}
            x1 = (int32_t) index ;
            x2 = (int32_t) index + 1.0;
            if (x2>=tab_length){x2-=tab_length;}
            x3 = (int32_t) index + 2.0;
            if (x3>=tab_length){x3-=tab_length;}
            f0 = in_tab[x0];
            f1 = in_tab[x1];
            f2 = in_tab[x2];
            f3 = in_tab[x3];
            x = index - x1;
            if (attr[1]==0)
                samp = f1;
            else if (attr[1]==1)
                samp = (f2-f1)*x + f1;
            else
            samp = f1 + (((f3 - f0 - 3 * f2 + 3 * f1)* x + 3 * (f2 + f0 - 2*f1))* x - (f3 + 2*f0 - 6*f2 + 3* f1))*x/6.0;
            samp = (y0-samp)*damp_factor + samp;
            y0 = samp;
            samp_reduced[j][ii++] = samp;

            if (ii>=tab_length){ii=0;}
        }
            data->prv_y0[j] = y0;
        //this guestimates one more sample
        x2 = ii-2;
        if (x2<0){x2+=tab_length;}
        f2 = samp_reduced[j][x2];
        x3 = ii -1;
        if (x3<0){x3+=tab_length;}
        f3 = samp_reduced[j][x3];
        samp_reduced[j][ii]= 2*f3 - f2;

    }
    data->samp_index = ii;
    data->in_read = index;
    read_factor = data->read_factor;
    for (j=0; j<num_chans;j++){
        frameCount = bsound->bufsize * data->read_factor; //length of samp_reduced buffer
        index  = data->samp_read;
        if ((int32_t)index < 0){index += (MYFLT)tab_length;}
        if ((int32_t)index >tab_length){index-= tab_length;}
        in_tab = data->samp_reduced[j];
        read_factor = (MYFLT) frameCount/bsound->bufsize;
        frameCount = bsound->bufsize;
        ii = j;
        MYFLT y1 = data->prv_y0_hipass[j], damp_factor = bsound->hi_damp;
        for (i= 0; i<frameCount ; i++){
            index += read_factor;
            if ((int32_t)index >= tab_length){index -= tab_length;}
            x0 = (int32_t) index -1.0;
            if (x0<0){x0+= tab_length;}
            x1 = (int32_t) index ;
            x2 = (int32_t) index + 1.0;
            if (x2>=tab_length){x2-=tab_length;}
            x3 = (int32_t) index + 2.0;
            if (x3>=tab_length){x3-=tab_length;}
            f0 = in_tab[x0];
            f1 = in_tab[x1];
            f2 = in_tab[x2];
            f3 = in_tab[x3];
            x = index - x1;
            if (attr[1]==0)
                samp = f1;
            else if (attr[1]==1)
                samp = (f2-f1)*x + f1;
            else
            samp= f1 + (((f3 - f0 - 3 * f2 + 3 * f1)* x + 3 * (f2 + f0 - 2*f1))* x - (f3 + 2*f0 - 6*f2 + 3* f1))*x/6.0;
            output[ii]=samp;
            output[ii]=(y1+output[ii])*damp_factor;
            y1 = output[ii]-samp;
            ii += num_chans;
        }
        data->prv_y0_hipass[j]=y1;
    }
    data->samp_read = index;

}
void* init_bbd(BSOUND* bsound, USR_IN type){
    BBD_OPS* data = (BBD_OPS*)malloc(sizeof(BBD_OPS));
    int32_t i;
    data->read_factor = 0.15;
    data->interpolate = 1;
    data->tab_length = bsound->sample_rate;
    data->samp_reduced = malloc(sizeof(MYFLT*)*bsound->num_chans);
    data->in_buffer = malloc(sizeof(MYFLT*)*bsound->num_chans);
    for (i= 0; i<bsound->num_chans; i++){
        data->samp_reduced[i] = (MYFLT*) calloc(sizeof(MYFLT)*(data->tab_length), 1);
        data->in_buffer[i] = (MYFLT*) calloc(sizeof(MYFLT)*data->tab_length, 1);
    }
    data->in_index = 0;
    data->samp_index = 0;
    data->prv_y0 = (MYFLT*)calloc(sizeof(MYFLT)*bsound->num_chans, 1);
    MYFLT damp_factor;
    damp_factor = 2.0-cos(data->read_factor*MY_2_PI/1.5);
    damp_factor = damp_factor-sqrt(damp_factor*damp_factor-1.0);
    data->damp_factor = damp_factor;
    data->prv_y0_hipass = (MYFLT*)calloc(sizeof(MYFLT)*bsound->num_chans, 1);
    data->in_read = (bsound->bufsize -2)-((1.0/data->read_factor)*((int32_t)bsound->bufsize*data->read_factor));
    data->samp_read = (bsound->bufsize) * 3;
    return (void*)data;
}
void dealloc_bbd(BSOUND* bsound, void* data){
    BBD_OPS* data_st = (BBD_OPS*) data;
    free(data_st->samp_reduced);
    free(data_st->prv_y0);
    free(data_st);
}
void bbd(float* input, float* output, void* data_st, const int16_t* attr, const BSOUND* bsound){
    BBD_OPS* data = (BBD_OPS*) data_st;
    //if we're not interpolating quantize
    if (attr[3])
        data->read_factor = attr[0]/100.0 ;
    else{
        data->read_factor = attr[0]/100.0 ;
        if (data->read_factor<=1.0f)
        data->read_factor = 1.0/((int32_t)(1.0/data->read_factor));
        else {
            if (data->read_factor<2.0)
                data->read_factor = 2.0;
            else if (data->read_factor<3.0)
                data->read_factor = 3.0;
            else
                data->read_factor = 4.0;
        }
    }
    MYFLT read_factor =  1.0f / data->read_factor;
    MYFLT* in_tab;
    MYFLT** samp_reduced = data->samp_reduced;
    int32_t x0, x1, x2, x3;
    MYFLT f0, f1, f2, f3, x, index;
    MYFLT feedback = attr[1]/100.0;
    MYFLT samp, y0, damp_factor = data->damp_factor;
    int16_t num_chans = bsound->num_chans;
    int32_t i, ii, j, jj, k, tab_length = data->tab_length;
    int64_t frameCount = bsound->bufsize;
    data->lp_freq = attr_to_freq_conv(attr[2]);
    if (data->lp_freq != data->prv_lp_freq){
        damp_factor = 2.0-cos(data->lp_freq*MY_2_PI/bsound->sample_rate);
        damp_factor = damp_factor-sqrt(damp_factor*damp_factor-1.0);
        data->prv_lp_freq = data->lp_freq;
        data->damp_factor = damp_factor;
    }
    frameCount = bsound->bufsize;
    for (j=0; j<num_chans; j++){
        jj = j;
        in_tab = data->in_buffer[j];
        ii = data->in_index;
        for (i= 0; i< frameCount; i++){
           in_tab[ii++] = input[jj];
            if (ii>=tab_length){ii=0;}
            jj += num_chans;
        }
        x2 = ii-2;
        if (x2<0){x2+=tab_length;}
        f2 = in_tab[x2];
        x3 = ii -1;
        if (x3<0){x3+=tab_length;}
        f3 = in_tab[x3];
        in_tab[ii]= 2*f3 - f2;
    }
    data->in_index = ii;

    frameCount = frameCount  * data->read_factor;
    for (j=0; j<num_chans;j++){
                // read in input-buffer
        index = data->in_read;
        if ((int32_t)index < 0){index += (MYFLT)tab_length;}
        if ((int32_t)index > tab_length){index -=tab_length;}
        in_tab = data->in_buffer[j];
        //write index in SR-reduced buffer
        ii =data->samp_index;
        k = data->samp_read;
        y0 = data->prv_y0[j];
        //frameCount is new frameCount
        read_factor = (MYFLT) bsound->bufsize/frameCount;
        for (i= 0; i<frameCount; i++){
            index += read_factor ;
            if ((int32_t)index >= tab_length){index -= tab_length;}
            x0 = (int32_t) index -1.0;
            if (x0<0){x0+= tab_length;}
            x1 = (int32_t) index ;
            x2 = (int32_t) index + 1.0;
            if (x2>=tab_length){x2-=tab_length;}
            x3 = (int32_t) index + 2.0;
            if (x3>=tab_length){x3-=tab_length;}
            f0 = in_tab[x0];
            f1 = in_tab[x1];
            f2 = in_tab[x2];
            f3 = in_tab[x3];
            x = index - x1;
            if (attr[3]==0)
                samp = f1;
            else if (attr[3]==1)
                samp = (f2-f1)*x + f1;
            else
            samp = f1 + (((f3 - f0 - 3 * f2 + 3 * f1)* x + 3 * (f2 + f0 - 2*f1))* x - (f3 + 2*f0 - 6*f2 + 3* f1))*x/6.0;
            samp = (y0-samp)*damp_factor + samp;
            y0 = samp;
            f1 = in_tab[k];
            f1 += data->in_buffer[(j+1)%num_chans][k++]; if (k>tab_length){k=0;}
            samp_reduced[j][ii++] = samp + f1*feedback;

            if (ii>=tab_length){ii=0;}
        }
            data->prv_y0[j] = y0;
        //this guestimates one more sample
        x2 = ii-2;
        if (x2<0){x2+=tab_length;}
        f2 = samp_reduced[j][x2];
        x3 = ii -1;
        if (x3<0){x3+=tab_length;}
        f3 = samp_reduced[j][x3];
        samp_reduced[j][ii]= 2*f3 - f2;

    }
    data->samp_index = ii;
    data->in_read = index;
    read_factor = data->read_factor;
    for (j=0; j<num_chans;j++){
        frameCount = bsound->bufsize * data->read_factor; //length of samp_reduced buffer
        index  = data->samp_read;
        if ((int32_t)index < 0){index += (MYFLT)tab_length;}
        if ((int32_t)index >tab_length){index-= tab_length;}
        in_tab = data->samp_reduced[j];
        read_factor = (MYFLT) frameCount/bsound->bufsize;
        frameCount = bsound->bufsize;
        ii = j;
        MYFLT y1 = data->prv_y0_hipass[j], damp_factor = bsound->hi_damp;
        for (i= 0; i<frameCount ; i++){
            index += read_factor;
            if ((int32_t)index >= tab_length){index -= tab_length;}
            x0 = (int32_t) index -1.0;
            if (x0<0){x0+= tab_length;}
            x1 = (int32_t) index ;
            x2 = (int32_t) index + 1.0;
            if (x2>=tab_length){x2-=tab_length;}
            x3 = (int32_t) index + 2.0;
            if (x3>=tab_length){x3-=tab_length;}
            f0 = in_tab[x0];
            f1 = in_tab[x1];
            f2 = in_tab[x2];
            f3 = in_tab[x3];
            x = index - x1;
            if (attr[3]==0)
                samp = f1;
            else if (attr[3]==1)
                samp = (f2-f1)*x + f1;
            else
            samp= f1 + (((f3 - f0 - 3 * f2 + 3 * f1)* x + 3 * (f2 + f0 - 2*f1))* x - (f3 + 2*f0 - 6*f2 + 3* f1))*x/6.0;
            output[ii]=samp;
            output[ii]=(y1+output[ii])*damp_factor;
            y1 = output[ii]-samp;
            ii += num_chans;
        }
        data->prv_y0_hipass[j]=y1;
    }
    data->samp_read = index;

}
void* init_reseq(BSOUND* bsound, USR_IN type){
    RESEQ_OPS* data = (RESEQ_OPS*)malloc(sizeof(RESEQ_OPS));
    bsound->filter_bank_exists = 1;
    data->num_bands = 10;
    data->prv_tilt = 50;
    data->buffer = (MYFLT*)malloc(sizeof(MYFLT)*2048*bsound->num_chans);
    data->peak          = malloc(sizeof(MYFLT*)*bsound->num_chans);
    data->bandwidth     = malloc(sizeof(MYFLT*)*bsound->num_chans);
    data->gain          = malloc(sizeof(MYFLT*)*bsound->num_chans);
    data->b0            = malloc(sizeof(MYFLT)*bsound->num_chans);
    data->b1            = malloc(sizeof(MYFLT)*bsound->num_chans);
    data->b2            = malloc(sizeof(MYFLT)*bsound->num_chans);
    data->a0            = malloc(sizeof(MYFLT)*bsound->num_chans);
    data->a1            = malloc(sizeof(MYFLT)*bsound->num_chans);
    data->a2            = malloc(sizeof(MYFLT)*bsound->num_chans);
    data->xmem1         = malloc(sizeof(MYFLT)*bsound->num_chans);
    data->xmem2         = malloc(sizeof(MYFLT)*bsound->num_chans);
    data->ymem1         = malloc(sizeof(MYFLT)*bsound->num_chans);
    data->ymem2         = malloc(sizeof(MYFLT)*bsound->num_chans);
    int32_t i, j;
    for (i=0; i<bsound->num_chans; i++){
        data->peak[i]          = (MYFLT*) malloc(sizeof(MYFLT)*data->num_bands);
        data->bandwidth[i]     = (MYFLT*) malloc(sizeof(MYFLT)*data->num_bands);
        data->gain[i]          = (MYFLT*) malloc(sizeof(MYFLT)*data->num_bands);
        data->b0[i]            = (MYFLT*) malloc(sizeof(MYFLT)*data->num_bands);
        data->b1[i]            = (MYFLT*) malloc(sizeof(MYFLT)*data->num_bands);
        data->b2[i]            = (MYFLT*) malloc(sizeof(MYFLT)*data->num_bands);
        data->a0[i]            = (MYFLT*) malloc(sizeof(MYFLT)*data->num_bands);
        data->a1[i]            = (MYFLT*) malloc(sizeof(MYFLT)*data->num_bands);
        data->a2[i]            = (MYFLT*) malloc(sizeof(MYFLT)*data->num_bands);
        data->xmem1[i]         = (MYFLT*) calloc(sizeof(MYFLT)*data->num_bands, 1);
        data->xmem2[i]         = (MYFLT*) calloc(sizeof(MYFLT)*data->num_bands, 1);
        data->ymem1[i]         = (MYFLT*) calloc(sizeof(MYFLT)*data->num_bands, 1);
        data->ymem2[i]         = (MYFLT*) calloc(sizeof(MYFLT)*data->num_bands, 1);
    }
    MYFLT SERGE_reseq[]={29.0, 61.0, 115.0, 218.0, 411.0, 777.0, 1500.0, 2800.0, 5200.0, 11000.0};
    //maybe this should be two-dimensional and we should be able to interpolate in between?
    //MYFLT formant_reseq[]={710.0, 1129.0, 2686.0, 3440.0, 4431.0};
    for (i=0; i<data->num_bands; i++){
        data->peak[0][i]      = SERGE_reseq[i];
        data->gain[0][i]      = 48.0;
        data->bandwidth[0][i] = 1.0;
    }
    for (i=1; i<bsound->num_chans; i++){
        for (j=0; j<data->num_bands; j++){
            data->peak[i][j]      = 1.1*i*data->peak[0][j];
            data->bandwidth[i][j] = 1.1*i*data->bandwidth[0][j];
            data->gain[i][j]      = 1.1*i*data->gain[0][j];
        }
    }
    //calculate coefficients according to bjorn roche
    for (i=0; i<bsound->num_chans; i++){
        for (j=0; j<data->num_bands; j++){
        MYFLT A, omega, c, s, alpha;
            A     = pow(10.0, data->gain[i][j]/40.0);
            omega = MY_2_PI*data->peak[i][j]/bsound->sample_rate;
            c     = cos(omega);
            s     = sin(omega);
            alpha = s* sinh(log(2.0)/2.0*data->bandwidth[i][j]*omega/s);

            data->b0[i][j] = 1.0 + alpha*A;
            data->b1[i][j] = -2*c;
            data->b2[i][j] = 1.0-alpha*A;
            data->a0[i][j] = 1.0 + alpha/A;
            data->a1[i][j] = -2*c;
            data->a2[i][j] = 1.0-alpha/A;
            data->b0[i][j]/=data->a0[i][j];
            data->b1[i][j]/=data->a0[i][j];
            data->b2[i][j]/=data->a0[i][j];
            data->a1[i][j]/=data->a0[i][j];
            data->a2[i][j]/=data->a0[i][j];
        }
    }
    return (void*) data;
}
void dealloc_reseq(BSOUND* bsound, void* data){
    RESEQ_OPS* data_st = (RESEQ_OPS*) data;
    free(data_st);
}
void recalculate_coefficients(RESEQ_OPS* data, const BSOUND* bsound){
    int32_t i, j;
    for (i=0; i<bsound->num_chans; i++){
        for (j=0; j<data->num_bands; j++){
            MYFLT A, omega, c, s, alpha;
                A     = pow(10.0, data->gain[i][j]/40.0);
                omega = MY_2_PI*data->peak[i][j]/bsound->sample_rate;
                c     = cos(omega);
                s     = sin(omega);
                alpha = s* sinh(log(2.0)/2.0*data->bandwidth[i][j]*omega/s);

                data->b0[i][j] = 1.0 + alpha*A;
                data->b1[i][j] = -2*c;
                data->b2[i][j] = 1.0-alpha*A;
                data->a0[i][j] = 1.0 + alpha/A;
                data->a1[i][j] = -2*c;
                data->a2[i][j] = 1.0-alpha/A;
                data->b0[i][j]/=data->a0[i][j];
                data->b1[i][j]/=data->a0[i][j];
                data->b2[i][j]/=data->a0[i][j];
                data->a1[i][j]/=data->a0[i][j];
                data->a2[i][j]/=data->a0[i][j];
        }
    }
}
void reseq(float* input, float* output, void* data_st, const int16_t* attr, const BSOUND* bsound){
    RESEQ_OPS* data = (RESEQ_OPS*)data_st;
    ///here we would check whether something has changed and update accordingly
    int32_t i, j, k = 0;
    if (attr[0]!=data->gain[0][0]){
        for (i=0; i<bsound->num_chans; i++){
            for (j=0; j<data->num_bands; j++)
            data->gain[i][j]=attr[0]*(0.1*i + 1);
        }
        recalculate_coefficients(data, bsound);
    }
    if (attr[1]!=data->bandwidth[0][0]){
        for (i=0; i<bsound->num_chans; i++){
            for (j=0; j<data->num_bands; j++)
            data->bandwidth[i][j]=(MYFLT)attr[1]/100.0;
        }
        recalculate_coefficients(data, bsound);
    }
    if (attr[2]!=data->prv_tilt){
        data->gain[0][0] = 50.0 - attr[2]*0.5;
        data->gain[0][1] = 50.0 - attr[2]*0.5;
        data->gain[0][2] = 50.0 - attr[2]*0.5;
        data->gain[0][3] = 50.0 - attr[2]*0.5;
        data->gain[0][4] = 50.0 - attr[2]*0.5;
        data->gain[0][5] = attr[2]*0.5;
        data->gain[0][6] = attr[2]*0.5;
        data->gain[0][7] = attr[2]*0.5;
        data->gain[0][8] = attr[2]*0.5;
        data->gain[0][9] = attr[2]*0.5;
        recalculate_coefficients(data, bsound);
    }
    int16_t numChans = bsound->num_chans;
    int32_t numBands = data->num_bands;
    int64_t numSamps = bsound->num_chans* bsound->bufsize;
    MYFLT xmem1, xmem2, ymem1, ymem2;
    MYFLT b0, b1, b2, a1, a2;
    MYFLT *buffer = data->buffer;
    for (j=0; j<numChans; j++){
        for (k=0; k<numBands; k++){
            xmem1 = data->xmem1[j][k];
            xmem2 = data->xmem2[j][k];
            ymem1 = data->ymem1[j][k];
            ymem2 = data->ymem2[j][k];
            b0 = data->b0[j][k];
            b1 = data->b1[j][k];
            b2 = data->b2[j][k];
            a1 = data->a1[j][k];
            a2 = data->a2[j][k];
            for (i=j; i<numSamps;){
               buffer[i]            = b0*input[i] +b1*xmem1 + b2*xmem2 - a1*ymem1 - a2*ymem2;
                xmem2    = xmem1;
                   xmem1 = input[i];
                   ymem2 = ymem1;
                   ymem1 = buffer[i];
                i+=numChans;
            }
            for (i=j; i<numSamps; ){
                output[i]+=buffer[i];
                i+=numChans;
            }
            data->xmem1[j][k] = xmem1;
            data->xmem2[j][k] = xmem2;
            data->ymem1[j][k] = ymem1;
            data->ymem2[j][k] = ymem2;
        }
    }
    for (i=0; i<numSamps; i++)
    output[i]*=0.25;
}
