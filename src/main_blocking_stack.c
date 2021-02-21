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
void write_audio(float* input, float* output, op_stack* head, BSOUND* bsound, float* temp1, float* temp2){
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
    PaError  err = paNoError;
    PaDeviceInfo *inputinfo, *outputinfo;
    PaStreamParameters inparam, outparam;
    PaStream *handle;
    //buffers portaudio writes to
    float *samplein, *sampleout, *temp1, *temp2;
    //input_handling thread
    pthread_t input_handling;
    ///@todo: this has to be changed!!!///welcome text for new version
    if (init_log(argv[0], bsound) == FIRST_LOAD){
        i=3;
        sleep(1); //count-down
        while(i>0){
        fprintf(stdout, "\t%d...\t", i-- );
        fflush(stdout);
        sleep(1);
        }
        }
    err = Pa_Initialize();
    if (err== paNoError){
        memset(&inparam, 0, sizeof(PaStreamParameters));
        inparam.device = Pa_GetDefaultInputDevice();
        if (inparam.device == paNoDevice){
            printf("can't get default output device\n");
            num_devices = Pa_GetDeviceCount();
            printf("Please choose an input device! Number of devices: %d\n", num_devices);
            for (i=0; i<num_devices; i++){
                inputinfo = Pa_GetDeviceInfo(i);
                printf("%d: ", i);
            }
        }
        inparam.sampleFormat = paFloat32;

        inputinfo=Pa_GetDeviceInfo(inparam.device);

        memset(&outparam, 0, sizeof(PaStreamParameters));
        outparam.device = Pa_GetDefaultOutputDevice();
        if (outparam.device < 0){
            printf("can't get default output device\n");
        }
        outparam.sampleFormat = paFloat32;

        outputinfo=Pa_GetDeviceInfo(outparam.device);
        if (outputinfo->maxOutputChannels > inputinfo->maxInputChannels )
            bsound->num_chans = outputinfo->maxOutputChannels;
        else if (inputinfo->maxInputChannels == 0)
              bsound->num_chans = 2;
        else
            bsound->num_chans = inputinfo->maxInputChannels;
        inparam.channelCount = bsound->num_chans;
        outparam.channelCount = bsound->num_chans;
        inparam.suggestedLatency = inputinfo->defaultLowInputLatency ;

            inparam.hostApiSpecificStreamInfo = NULL;
        outparam.suggestedLatency = outputinfo->defaultLowOutputLatency ;
            outparam.hostApiSpecificStreamInfo = NULL;

        err = Pa_OpenStream(&handle, &inparam, &outparam, SR, bsound->bufsize, paNoFlag, NULL, NULL);
        if (err == paNoError){
            err = Pa_StartStream(handle);
            if (err==paNoError){
              samplein = (float *)calloc(sizeof(float)*2048*bsound->num_chans, 1);
                sampleout = (float *)calloc(sizeof(float)*2048*bsound->num_chans, 1);
                temp1 = (float *)calloc(sizeof(float)*2048*bsound->num_chans, 1);
                temp2 = (float *)calloc(sizeof(float)*2048*bsound->num_chans, 1);
                pthread_create(&input_handling, NULL, *(input_handler), (void *)bsound);
                bool bypass_active = 0;
                while(1){
                    OutOfRangeFlag = 0;
                    if (bsound->bypass_flag){
                        if (!bypass_active){
                            err = Pa_ReadStream(handle, samplein, bsound->bufsize);
                            int j; MYFLT incr = 0.99;
                            for (j=0; j<512; j++){
                            samplein[j]=samplein[j]*incr;//samplein[j]*(1.0/(j+1));
                                incr = incr*incr;
                            }
                            for (j=512; j<2048*bsound->num_chans; j++)
                            samplein[j]=0.0f;
                            bypass_active = 1;
                        }
                           else {
                        int j; //this is necessary because of in/out swapping
                        for (j=0; j<2048*bsound->num_chans; j++)
                        samplein[j]=0.0f;
                            }
                    }
                    else{
                        err = Pa_ReadStream(handle, samplein, bsound->bufsize);
                        if (bypass_active){
                            int j; MYFLT factor = pow(pow(10, 20), 1.0/1024);
                            MYFLT incr = pow(10, -20);
                            for (j=0; j<1024; j++){
                            samplein[j]=samplein[j]*incr;
                            incr *= factor;}
                            bypass_active = 0;
                        }

                    }
                    if (err!= paNoError){
                        if (err==paInputUnderflow){
                            error_message("input underflow", bsound);
                        }
                    }
                    if (bsound->mono_input)
                    copylefttoright(samplein, bsound, 1);
                    write_audio(samplein, sampleout, head, bsound, temp1, temp2);
                    for (i=0; i<bsound->bufsize*bsound->num_chans; i++){
                        if (sampleout[i]>1.0f){
                            sampleout[i]=0.0f;
                            OutOfRangeFlag = 1;
                        }
                        if (sampleout[i]<-1.0f){
                            sampleout[i]=0.0f;
                            OutOfRangeFlag = 1;
                        }
                    }
                    if (bsound->pause_flag){
                        for (i =0; i<bsound->bufsize*bsound->num_chans; i++)
                        sampleout[i]= 0.0f;
                    }
                    err = Pa_WriteStream(handle, sampleout, bsound->bufsize);
                    if (err!= paNoError){
                        error_message(Pa_GetErrorText(err), bsound);
                        if (err==paOutputUnderflow){
                            error_message("output underflow", bsound);
                        }
                        else if (err==paOutputOverflow){
                            error_message("output overflow", bsound);
                        }
                    }
                    if (bsound->quit_flag)
                        break;

                }
                Pa_StopStream(handle);
                save_to_log(argv[0], bsound);
                pthread_join(input_handling, NULL);
                printf("...\tstopping audio-stream\t");
                free(samplein);free(sampleout);free(temp1); free(temp2);
                //deallocate resources!!!
                free_op_stack(bsound->head, bsound);
                if (bsound->num_ops!=0)
                    free(bsound->head);
                free(bsound);
                printf("...\tdeallocating resources\t");
            }
            else printf("%s \n", Pa_GetErrorText(err));
            Pa_Terminate();
        }
        else printf("%s \n", Pa_GetErrorText(err));

    }

    printf("...\t quit\n\n");
    return 0;
}
