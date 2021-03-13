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
#include <stdint.h>
#include <unistd.h>

#include "../portaudio/portaudio.h"
#include "../portaudio/pa_mac_core.h"

#include "callback_functions.h"
#include "data_types.h"
#include "input_handling.h"
#include "log_actions.h"
#include "programm_state.h"
#include "opcodes.h"
#include "util_opcodes.h"

///this was originally in stack_actions, but has been moved here since stack_actions was decomissioned
void free_op_stack(OP_STACK* head, BSOUND* bsound){
    OP_STACK * current= head;
    int32_t i;
    if (bsound->num_ops != 0){
        for (i= 0; i<bsound->num_ops; i++){
            current->dealloc(bsound, current->func_st);
            current = current->next_op;
        }
    }
}

int main(int argc, const char * argv[]) {
    BSOUND * bsound = init_bsound();
    bsound->programm_loc = argv[0];
    OP_STACK* head = init_head();
    bsound->head = head;
    int32_t i; bool OutOfRangeFlag;
    int32_t num_devices;
    //port_audio vars
    CALLBACK_DATA* my_callback_data;
    PaError  err = paNoError;
    const PaDeviceInfo *inputinfo, *outputinfo;
    PaStreamParameters inparam, outparam;
    PaStream *handle;
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
        while (inputinfo->maxInputChannels < 1 ){
            printf("Your input device does not allow audio input\nPlease choose an appropriate device:\n");
            int32_t devicecount = Pa_GetDeviceCount(), my_device;
            const PaDeviceInfo* myinfo;
            for (i=0; i<devicecount; i++){
                myinfo = Pa_GetDeviceInfo(i);
                printf("Device number %d: %s\nNumber of input channels: %d\n", i, myinfo->name, myinfo->maxInputChannels);
            }
            printf("Please choose a device by typing it's number and hitting enter\n");
            scanf("%d", &my_device);
            inputinfo = Pa_GetDeviceInfo(my_device);
            inparam.device = my_device;
        }
        while (outputinfo->maxOutputChannels < 1 ){
            printf("Your output device does not allow audio output\nPlease choose an appropriate device:\n");
            int32_t devicecount = Pa_GetDeviceCount(), my_device;
            const PaDeviceInfo* myinfo;
            for (i=0; i<devicecount; i++){
                myinfo = Pa_GetDeviceInfo(i);
                printf("Device number %d: %s\nNumber of output channels: %d\n", i, myinfo->name, myinfo->maxOutputChannels);
            }
            printf("Please choose a device by typing it's number and hitting enter\n");
            scanf("%d", &my_device);
            outputinfo = Pa_GetDeviceInfo(my_device);
            outparam.device = my_device;
        }
        if (outputinfo->maxOutputChannels > inputinfo->maxInputChannels )
            bsound->num_chans = outputinfo->maxOutputChannels;
        else
            bsound->num_chans = inputinfo->maxInputChannels;
        if (outputinfo->maxOutputChannels > inputinfo->maxInputChannels){
            bsound->mono_input = true;
            bsound->in_out_chanmatch= false;
            printf("maxIn: %d; maxOut: %d", inputinfo->maxInputChannels, outputinfo->maxOutputChannels);
            sleep(1);
        }
        inparam.channelCount = inputinfo->maxInputChannels;
        bsound->in_chans = inputinfo->maxInputChannels;
        outparam.channelCount = outputinfo->maxOutputChannels;
        bsound->out_chans = outputinfo->maxOutputChannels;
        inparam.suggestedLatency = inputinfo->defaultLowInputLatency ;

            inparam.hostApiSpecificStreamInfo = NULL;
        outparam.suggestedLatency = outputinfo->defaultLowOutputLatency ;
            outparam.hostApiSpecificStreamInfo = NULL;
            my_callback_data = init_callbackdata(bsound);
      err = Pa_OpenStream(&handle,
           &inparam,
           &outparam,
           SR,
           bsound->bufsize,
           paNoFlag | (paMacCoreChangeDeviceParameters &paPlatformSpecificFlags),
           test_callback,
           my_callback_data);
        if (err == paNoError){
            err = Pa_StartStream(handle);
            bsound->stream_handle = handle;
            if (err==paNoError){

                pthread_create(&input_handling, NULL, *(input_handler), (void *)bsound);
                while(!bsound->quit_flag){
                      // isn't this always true when requested_bufsize and bufsize don't match?
                      if (bsound->bufsize != bsound->requested_bufsize){
                           Pa_StopStream(handle);
                           bsound->bufsize = bsound->requested_bufsize;
                           err = Pa_OpenStream(&handle,
                           &inparam,
                           &outparam,
                           SR,
                           bsound->bufsize,
                           paNoFlag | (paMacCoreChangeDeviceParameters &paPlatformSpecificFlags),
                           test_callback,
                           my_callback_data);
                           if (err!=paNoError)
                           break;
                           //implicit else
                           Pa_StartStream(handle);
                           bsound->stream_handle = handle;
                           if (err!=paNoError)
                           break;
                           bsound->pause_flag = 0;
                     }
                     //this keeps cpu usage in check
                    usleep(15000);
                }
                Pa_StopStream(handle);
                save_to_log(argv[0], bsound);
                pthread_join(input_handling, NULL);
                printf("...\tstopping audio-stream\t");
                //deallocate resources!!!
                free_op_stack(bsound->head, bsound);
                if (bsound->num_ops!=0)
                    free(bsound->head);
                printf("...\tdeallocating resources\t");
            }
            else printf("%s \n", Pa_GetErrorText(err));
            Pa_Terminate();
        }
        else printf("%s \n", Pa_GetErrorText(err));

    }
    bool error_occurred = false;
    for (i=0; i<BERROR_NUM_ERRORS; i++){
          if (bsound->errors[i]!=0){
             error_occurred = true;
            printf("BERROR %d occurred %d times\n", i, bsound->errors[i]);
      }
   }
   if (error_occurred)
   printf("\nPlease report this error at https://github.com/oskarvonephesos/bsound\n");
   free(bsound);
    printf("...\t quit\n\n");
    return 0;
}
