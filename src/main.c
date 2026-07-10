#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <openssl/evp.h>
#include <time.h>



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
    }else if (strcmp(command,"status")==0){
        // TODO: apply with branches
        // want to check if files have changed by  first looking at changetime and filesize 
        // if the chance is high the file actually changed we compute the hash and compare it to the old hash from the file we computed before
        printf("Status");
        struct stat fileMeta; // struct from stat.h to save fileinfo/Metadata
        // from stat we get st_size for bytesize
        // st_ino for the inode (unique identifier)
        // and st_mtime for last 
        if(stat("main.c",&fileMeta)!=0){
            printf("Error reading file");
            return -1;
        }
        printf("Änderung: %s \n",ctime(&fileMeta.st_mtime));
        printf( "Size: %ld",&fileMeta.st_size);
    }else{
        printf("%s is not a valid command for Bombagit",command);
    }
}