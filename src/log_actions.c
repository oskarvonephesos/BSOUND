#include "log_actions.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"
#include "unistd.h"

//@todo: text files should be in separate "resources folder"
//@todo: startup message a bit corny?
LOG_STATE init_log(const char* programm_location, BSOUND* bsound){
    int loc_length = strlen(programm_location) - 6; //the six chars are "BSOUND"
    char* log_loc = (char*)malloc((20+loc_length)*sizeof(char)); //magic numbers FTW
    char* read_loc = (char *)malloc((20+loc_length)*sizeof(char));
    char* line = (char*)malloc(256*sizeof(char));
    if (log_loc == NULL){
        puts("SERIOUS MALLOC ERROR\n");
        return LOG_ERROR;
    }
    FILE* fp, *f_read;
    int first_load;
    memset(log_loc, '\0', loc_length + 20 );
    memset(read_loc, '\0', loc_length + 20);
    memcpy(log_loc, programm_location, loc_length);
    memcpy(read_loc, programm_location, loc_length);
    strcat(log_loc, "log.txt");
    strcat(read_loc, "WELCOMETEXT.txt");
    if (access(log_loc, F_OK) == 0){//file exists
        fp = fopen(log_loc, "r");
        fscanf(fp, "%d", &first_load);
        if (first_load == 0){
            f_read = fopen(read_loc, "r");
            if (f_read){
            while(fgets(line, 256, f_read)){
                fprintf(stdout, "%s", line);
            }
            fclose(f_read);
            }
            else
                fprintf(stdout, "couldn't find resources \n");
            fprintf(fp, "1");
            fclose(fp);
            free(read_loc);
            free (log_loc);
            return FIRST_LOAD;
        }
        else {
            fscanf(fp, " %hd %ld", &bsound->mono_input, &bsound->bufsize);
        }
    }
    else {
        fp = fopen(log_loc, "w");
        fprintf(fp, "1 0 0");
        fclose(fp);
        f_read = fopen(read_loc, "r");
        if (f_read){
        while(fgets(line, 256, f_read)){
            fprintf(stdout, "%s", line);
        }
        fclose(f_read);
        }
        else
            fprintf(stdout, "couldn't find resources \n");
        free(read_loc);
        free (log_loc);
        return FIRST_LOAD;
    }
    free(log_loc);
    return NORMAL_LOAD;
}
void save_to_log(const char* programm_loc, BSOUND* bsound){
    int loc_length = strlen(programm_loc) - 6; //the six chars are "BSOUND"
    char* log_loc = (char*)malloc((20+loc_length)*sizeof(char)); //magic numbers FTW
    FILE* fp;
    memset(log_loc, '\0', loc_length + 20 );
    memcpy(log_loc, programm_loc, loc_length);
    strcat(log_loc, "log.txt");
    fp = fopen(log_loc, "w");
    fprintf(fp, "1 %d %ld", bsound->mono_input, bsound->bufsize);
    fclose(fp);
    free (log_loc);
}
