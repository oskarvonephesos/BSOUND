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


//                  CORRELATE OPCODE
//               ???WET/DRY OPCODES???
//TODO: SWITCH A B; first ternary command!!!
//TODO: insert
//TODO: do more exciting things with panning
#include <stdlib.h>
#include <stdio.h>
#include <curses.h>
#include <pthread.h>
#include <string.h>
#include <stdbool.h>
#include "data_types.h"
#include "programm_state.h"
#include "opcodes.h"
#include "portaudio.h"
#include "pa_mac_core.h"
#include "input_handling.h"
#include "opcodes.h"
#include "util_opcodes.h"
#include "log_actions.h"
#include <unistd.h>

///this was originally in stack_actions, but has been moved here since stack_actions was decomissioned
void free_op_stack(op_stack* head, BSOUND* bsound){
    op_stack * current= head;
    int i;
    if (bsound->num_ops != 0){
        for (i= 0; i<bsound->num_ops; i++){
            current->dealloc(bsound, current->func_st);
            current = current->next_op;
        }
    }
}
int g_inch, g_outch;
void test_input(PaStream* handle, float* samplein, BSOUND* bsound){
    int i, j;
    int frameCount = bsound->bufsize * bsound->in_chans;
    MYFLT incr = MY_2_PI/bsound->bufsize;
    short numChans = bsound->in_chans;
    for (j=0; j<numChans; j++){
        int k = 0;
        for (i=j; i<frameCount;){
            samplein[i] = 0.5 * sin(k*incr);
            k++;
            i+= numChans;
        }
    }
}
#define TAB_LENGTH 512
void write_sine(float* samplein, BSOUND* bsound, long counter, FILE* fp, float* sin_tab){
    int i, ii = 0, my_counter, index, j;
    long bufsize = bsound->bufsize * bsound->num_chans;
    short num_chans = bsound->num_chans;
    my_counter  = 4;//(counter/5)%20 + 1;
    for (j=0; j<num_chans; j++){
        ii = 0;
    for (i=j; i<bufsize; ){
        index = my_counter*ii++;
        if (index>=TAB_LENGTH){
            while(index>=TAB_LENGTH)
            {index-=TAB_LENGTH;}
        }
        samplein[i] = 0.5*sin_tab[index];
        i += num_chans;
    }
    }
    for (i=0; i<bufsize; i++)
        fwrite(&(samplein[i]), sizeof(float), 1, fp);
}
void write_out(float* sampleout, BSOUND* bsound, FILE* fp){
    int i;
    for (i=0; i<bsound->bufsize*bsound->out_chans; i++){
        fwrite(&(sampleout[i]), sizeof(float), 1, fp);
    }
}
typedef struct {
    bool bypass_active;
    bool record_active;
    bool crosses_zero;
    int recordbuflength;
    int recordstart;
    int recordend;
    int recordzero;
    int readhead;
}Record_info;
Record_info* init_recordinfo(BSOUND* bsound){
Record_info* r = (Record_info*) malloc(sizeof(Record_info));
    r->bypass_active = 0;
    r->record_active = 0;
    r->recordbuflength =300000;//bsound->sample_rate*4* bsound->num_chans;
    r->readhead = 0;
    r->crosses_zero = false;
    return r;
}
void write_input(float* input, PaStream* handle,  float* record_buf, BSOUND* bsound, Record_info* r){
    int i, recordhead = r->readhead;
    //audio in
    //Pa_ReadStream(handle, input, bsound->bufsize);
    test_input(handle, input, bsound);
    if (bsound->mono_input)
    copylefttoright(input, bsound, 1);
    if (bsound->record_flag){
        //record_start case
        if (!r->record_active){
            r->recordstart = r->readhead;
            r->record_active = true;
            r->crosses_zero = false;
        }
        int sampCount = bsound->bufsize * bsound->num_chans;
        int rbuflength = r->recordbuflength;
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
            if (--recordhead<0)
                recordhead += r->recordbuflength;
            r->recordend = recordhead;
            if (r->recordstart > r->recordend)
                r->recordzero = 0;
            else //recordstart != recordend, so this is recordstart<recordend
                r->recordzero = r->recordstart;
            //make sure not to call again || playbackflag is set in input_handling
            r->record_active = 0;
        recordhead = r->recordstart;
        }
    if (bsound->bypass_flag){
        if (!r->bypass_active){
            int j; MYFLT incr = 0.99;
            for (j=0; j<512; j++){
            input[j]=input[j]*incr;//samplein[j]*(1.0/(j+1));
                incr = incr*incr;
            }
            for (j=512; j<2048*bsound->num_chans; j++)
            input[j]=0.0f;
            r->bypass_active = 1;
        }
           else {
        int j; //this is necessary because of in/out swapping
        for (j=0; j<2048*bsound->num_chans; j++)
        input[j]=0.0f;
            }
    }
    else{
        if (r->bypass_active){
            int j; MYFLT factor = pow(pow(10, 20), 1.0/1024);
            MYFLT incr = pow(10, -20);
            for (j=0; j<1024; j++){
            input[j]=input[j]*incr;
            incr *= factor;}
            r->bypass_active = 0;
        }

    }

    if (bsound->playback_flag){
        int sampCount = bsound->bufsize * bsound->num_chans;
        int rend       = r->recordend,
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
    r->readhead = recordhead;
}
void apply_fx(float* input, float* output, op_stack* head, BSOUND* bsound, float* temp1, float* temp2){
    int i, skip_total = 0;
    float* temp;
    op_stack* current_op = head;
    if (bsound->num_ops == 0){
        for (i=0; i<bsound->bufsize*bsound->num_chans; i++){
            output[i]=0.0f;
        }
    }
    else{
        short attr_num;
        long sampcount = bsound->bufsize * bsound->num_chans;
        for (i=0; i<sampcount; i++){
            temp1[i]=input[i];
            output[i]=0.0f;
        }
        current_op = bsound->head;
    for (i= 0; i<bsound->num_ops;){
        current_op->func(temp1, temp2, current_op->func_st, current_op->attr , bsound);
        i++;
        if (i<bsound->num_ops){
            attr_num = which_attr_is_skip(current_op->type);
            if (current_op->attr[attr_num]){
                int j;
                skip_total += current_op->attr[attr_num];
                MYFLT skip_amount = current_op->attr[attr_num]/100.0;
                for (j=0; j<sampcount; j++)
                output[j]+=temp2[j]*skip_amount;
            }

            temp = temp1;
            temp1 = temp2;
            temp2 = temp;
        }
        if (current_op->next_op != NULL)
            current_op = current_op->next_op;
    }
        for (i=0; i<sampcount; i++)
        output[i]+=temp2[i];
    }
}
int main(int argc, const char * argv[]) {
    BSOUND * bsound = init_bsound();
    bsound->programm_loc = argv[0];
    op_stack* head = init_head();
    bsound->head = head;
    int i; bool OutOfRangeFlag;
    int num_devices;
    //port_audio vars
    PaStream *handle;
    //buffers portaudio writes to
    float *samplein, *sampleout, *temp1, *temp2, *recordbuf;
    //recordbuflength wraps index around at end of buffer
    //recordbegin points to reset point when we pass recordend
    //recordzero points to reset point when we pass buffer end
    ///@todo: this has to be changed!!!///welcome text for new version
    printf("GET SETTINGS FROM LOG FILE? (y/n)?\n");
    char reply;
    scanf("%c", &reply);
    if (reply == 'y'){
    if (init_log(argv[0], bsound) == FIRST_LOAD){
        i=3;
        sleep(1); //count-down
        while(i>0){
        fprintf(stdout, "\t%d...\t", i-- );
        fflush(stdout);
        sleep(1);
        }
        }
    }
    int in_chans, out_chans;
    printf("NUMBER OF IN CHANS?\n");
    scanf("%d", &in_chans);
    printf("NUMBER OF OUT CHANS?\n");
    scanf("%d", &out_chans);
    bsound->in_chans = in_chans;
    bsound->out_chans = out_chans;
        if (out_chans > in_chans )
            bsound->num_chans = out_chans;
        else
            bsound->num_chans = in_chans;
        if (out_chans > in_chans){
            bsound->mono_input = true;
            bsound->in_out_chanmatch= false;
            printf("maxIn: %d; maxOut: %d\n", in_chans, out_chans);
            sleep(1);
        }
    samplein  = (float *)calloc(sizeof(float)*2048*(bsound->num_chans+bsound->in_chans), 1);
    sampleout = (float *)calloc(sizeof(float)*2048*(bsound->num_chans+bsound->out_chans), 1);
    temp1     = (float *)calloc(sizeof(float)*2048*bsound->num_chans, 1);
    temp2     = (float *)calloc(sizeof(float)*2048*bsound->num_chans, 1);
    Record_info* myrecordinfo = init_recordinfo(bsound);
    recordbuf = (float*) calloc(sizeof(float)*myrecordinfo->recordbuflength, 1);

    printf("Opcode to test?\n");
    char opcode_name[256];
    scanf("%s", opcode_name);
    COMMAND* usr_in = parse(opcode_name, 20);
    usr_in->bsound = bsound; usr_in->cursor = NULL;
    insert_op(bsound, usr_in);
    int j = 0;
    FILE *fout;
    long loc_length = strlen(argv[0]) - 6;
    char* write_out_loc = (char*)malloc(sizeof(char)* (loc_length + 16));
    memset(write_out_loc, '\0', loc_length + 16);
    memcpy(write_out_loc, argv[0], loc_length);
    strcat(write_out_loc, "audio_out");
    fout = fopen(write_out_loc, "wb");
    if (fout == NULL){
        fprintf(stderr, "ERROR WRITING TO OUT FILE\n");
    }
    while(j<2500){
            write_input(samplein, handle, recordbuf, bsound, myrecordinfo);
            apply_fx(samplein, sampleout, head, bsound, temp1, temp2);
            if (bsound->out_chans != bsound->num_chans){
                  match_outputchannels(sampleout, bsound);
            }
            write_out(sampleout, bsound, fout);
            j++;
                }
                save_to_log(argv[0], bsound);
                //pthread_join(input_handling, NULL);
                free(samplein);free(sampleout);free(temp1); free(temp2);
                //deallocate resources!!!
                free_op_stack(bsound->head, bsound);
                if (bsound->num_ops!=0)
                    free(bsound->head);
                free(bsound);
                printf("...\tdeallocating resources\t");

    printf("...\t quit\n\n");
    return 0;
}
