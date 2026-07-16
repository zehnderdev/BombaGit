#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <openssl/evp.h>
#include <time.h>
#include <dirent.h>
#include "sha256.h"


// searches if we are in a .bgit repository 
// chInto = 0 ->chdir into top level folder
int searchbGit(int chInto){
    DIR *dir;
    struct dirent *dirent;
    char cwd[128];
    getcwd(cwd,sizeof(cwd));
    printf("In: %s",cwd);
    // optimization of using first string and rm until next '/' instead of cwd overwriting
    while ((strcmp(cwd,"/"))!=0)
    {
        printf("In: %s \n",cwd);
        if((dir=opendir("."))==NULL){
            printf("Error opening directory1");
            return 1;
        }
        if ((dirent =readdir(dir))==NULL){
            printf("Error reading directory1");
            return 1;
        }
        while (dirent!=NULL)
        {
            
            if((strcmp(dirent->d_name,".bgit")==0)){
                getcwd(cwd,sizeof(cwd));
                if(chInto==0){
                    if(chdir(cwd)!=0){
                        printf("Error changing directory");
                    }
                }
                printf("Found '.bgit' in %s ",cwd);
                return 0;
            }
            
            dirent =readdir(dir);
        }
        closedir(dir);
        if((chdir(".."))!=0){
            printf("Error changing directory");
            return 1;
        }
        getcwd(cwd,sizeof(cwd));
    }
    printf("Not in a repository");
    return 1;
}

struct FileMeta
{
    dev_t inode;
    off_t size;
    time_t mtime; //modification time
    char name[128];
};

int makeIndx(char *filename,struct stat FileMetaData){
    if((searchbGit(0))!=0) return -1;
    FILE *indx;

    struct FileMeta content ;
    strcpy(content.name,filename);
    content.inode =FileMetaData.st_ino;
    content.size =FileMetaData.st_size;
    content.mtime = FileMetaData.st_mtime;


    if(chdir(".bgit")!=0){
        printf("Error changing directory");
        return 1;
    }

    indx =fopen("index","wb");
    fwrite(&content,sizeof(content),1,indx);

    fclose(indx);

}
int readIndx(){
    if((searchbGit(0))!=0) return -1;
    FILE *indx;

    struct FileMeta content;
    
    if(chdir(".bgit")!=0){
        printf("Error changing directory");
        return 1;
    }

    indx =fopen("index","rb");
    //fseek(indx,sizeof(struct FileMeta)-sizeof(char[128]),SEEK_CUR);
    fread(&content,sizeof(content),1,indx);
   
    
    printf("Name: %s",content.name);
    fclose(indx);
}
// Change into the Top-Level Folder of the Repository
// assuming we are in a Repositorys
int chdirTop(){
    char cwd[64];
    getcwd(cwd,sizeof(cwd));

}
// Copies a file from destination to source
int makeFile(char *destination,char *source){
    FILE *dst,*src;

    src = fopen(source,"rb");


    if(src==NULL){
        perror("fopen");
        return 1;
    }

    if(chdir(".bgit")!=0){
        printf("Error switchting directory");
        return 1;
    }
    
    char cwd[64]; //hopefully more than enough
    getcwd(cwd,sizeof(cwd));
    printf("We are in: %s",cwd); 

    dst = fopen(destination,"wb");

    if(dst==NULL){
        perror("fopen");
        chdir("..");
        return 1;
    }

    unsigned char buffer[4096];
    size_t bytesRead;

    while ((bytesRead =fread(buffer,1,sizeof(buffer),src))>0)
    {
        fwrite(buffer,1,bytesRead,dst);
    }
    fclose(dst);

    if(chdir("..")!=0){
        printf("Error switchting directory");
        return 1;
    }
    fclose(src);
    return 0;
}

int hashAll(char *filepath){
    struct stat fileMeta; // struct from stat.h to save fileinfo/Metadata
        // from stat we get st_size for bytesize
        // st_ino for the inode (unique identifier)
        // and st_mtime for last 
        // check with inode if we have file already if not git add
        printf(filepath);
        DIR *dir; // directory pointer
        struct dirent *dirent; // directory entry

        if((dir=opendir(filepath))==NULL){
            printf("Error opening directory");
            return 1;
        }
        do
        {
            if ((dirent =readdir(dir))!=NULL)
            {
                // ignore . and ..

                if(strcmp(dirent->d_name,".")==0 ||
                strcmp(dirent->d_name,"..")==0 || strcmp(dirent->d_name,".bgit")==0) continue;
                

                char fullpath[4096];
                //append filepath with dir path
                snprintf(fullpath,sizeof(fullpath),"%s/%s",filepath,dirent->d_name);

            
                // TODO: check if the file is a newly created file(untracked)
                switch (dirent->d_type){

                case 8:
                    // file
                    // check contents
                    // load old stats
                    struct stat old;
                    stat(fullpath,&fileMeta);//get fileMetaData
                    printf("got metadata for %s \n",dirent->d_name);
                    if(fileMeta.st_ctime > old.st_ctime && fileMeta.st_size != old.st_size){
                        char *hash = hashFile(fullpath);
                        if(hash){
                            printf("Hash: %s \n",hash);
                            makeFile(dirent->d_name,dirent->d_name);
                            free(hash);
                        }

                    }
                    break;

                case 4:
                    // dir 
                    // recursion

                    printf("Dir: %s using recursion\n",fullpath);

                    hashAll(fullpath);
                    break;
                
                case 12:
                    // symbolic link
                    break;
                
                case 0:
                    // unknown could also be symlink
                    break;

                default:
                    break;
                }
            }
            
        } while (dirent!=NULL);
    closedir(dir);
    return 0;

}

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
            return 1; // early return
        } 

        printf("initialize Repo");
        free(filepath); // free memory
    }else if (strcmp(command,"status")==0){
        // TODO: apply with branches
        // want to check if files have changed by  first looking at changetime and filesize 
        // if the chance is high the file actually changed we compute the hash and compare it to the old hash from the file we computed before
        clock_t t = clock();
        printf("Status\n");
        
        hashAll(".");
        t = clock() - t;
        double time_spent = ((double)t) *1000 / CLOCKS_PER_SEC;
        printf("Took %fms",time_spent);
        return 0;

        
    }else if(strcmp(command,"add")==0){
        // add 
        
        // TODO: add directories for add and remove 
        char *path = argv[2];
        // writes to index file
        //makeIndx();
        makeFile(path,path);
        // logic with metafile
        // also adding "."
    }else if(strcmp(command,"rm")==0){
        char *path = argv[2];
        // also adding "."
        if(chdir(".bgit")!=0){
            printf("Error switchting directory");
            return 1;
        }
        
        remove(path);

        if(chdir("..")!=0){
            printf("Error switchting directory");
            return 1;
        }
    }else if(strcmp(command,"test")==0){
        struct stat file;
        stat("main.c",&file);
        makeIndx("main.c",file);
        readIndx();
    }else{
        printf("%s is not a valid command for Bombagit",command);
    }
}