#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>



int main(int argc, char *argv[]){

    
    // if init command -> create ".bgit" file
    if(argc <2) return 1; // check if we have 2 arguments at least (first always filename)
    char *command = argv[1];
    
    if(strcmp(command,"init") == 0){
        char *suffix = "/.bgit"; // we call it bgit for now

        char cwd[64]; //hopefully more than enough
        getcwd(cwd,sizeof(cwd)); 

        char *filepath = malloc(strlen(suffix)+strlen(cwd)+1); // allocate memory
        if(!filepath)return 1; // check for malloc 

        strcpy(filepath,cwd);
        strcat(filepath,suffix);

        int status;
        if(mkdir(filepath,0755)<0){ // normal permission settings
            if(errno == EACCES){
                printf("Fail with access\n");
            }else if(errno == EEXIST){
                printf("Fail because directory already exists\n");
            }
            // TODO: add all Errors
            return -1; // early return
        } 

        printf("initialize Repo");
        free(filepath); // free memory
    }
}