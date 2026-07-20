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
#include <sys/types.h>

// Definiton for searchIndex()
#define FOUND 0
#define NOT_FOUND 1
#define ERROR 2

// TODO: function for easy directory switching

// Struct for holding File Metadata
struct FileMeta
{
    ino_t inode;
    off_t size;
    time_t mtime; //modification time
    char name[128];
};

// searches if we are in a .bgit repository 
// chInto = 0 ->chdir into top level folder
int searchbGit(int chInto){
    DIR *dir;
    struct dirent *dirent;
    char cwd[128];
    getcwd(cwd,sizeof(cwd));
    //printf("In: %s \n",cwd);
    // optimization of using first string and rm until next '/' instead of cwd overwriting
    while ((strcmp(cwd,"/"))!=0)
    {
        //printf("In: %s \n",cwd);
        if((dir=opendir("."))==NULL){
            printf("Error opening directory\n");
            return 1;
        }
        if ((dirent =readdir(dir))==NULL){
            printf("Error reading directory\n");
            return 1;
        }
        while (dirent!=NULL)
        {
            
            if((strcmp(dirent->d_name,".bgit")==0)){
                getcwd(cwd,sizeof(cwd));
                if(chInto==0){
                    if(chdir(cwd)!=0){
                        printf("Error changing directory\n");
                    }
                }
                //printf("Found '.bgit' in %s \n",cwd);
                //printf("Changed into %s \n",cwd);
                return 0;
            }
            
            dirent =readdir(dir);
        }
        closedir(dir);
        if((chdir(".."))!=0){
            printf("Error changing directory\n");
            return 1;
        }
        getcwd(cwd,sizeof(cwd));
    }
    printf("Not in a repository\n");
    return 1;
}
// Searches IndexFile and returns struct on found
int searchIndex(ino_t Inode,struct FileMeta *result){
    if((searchbGit(0))!=0) return 1;
    if(chdir(".bgit")!=0){
        perror("chdir");
        return ERROR;
    }
    
    FILE *indx;
    indx = fopen("index","rb");
    // TODO: handle ALL Errors!
    if(indx==NULL){
        chdir("..");
        return NOT_FOUND;
    }
    size_t bytesRead;
    while((bytesRead=fread(result,sizeof(*result),1,indx))>0){
        if(result->inode==Inode){
            //printf("Got old data:%s \n" ,result->name);
            fclose(indx);
            if(chdir("..")!=0){
                printf("Error changing directory\n");
                return ERROR;
            }
            return FOUND;
        }
    }
    if(chdir("..")!=0){
        printf("Error changing directory\n");
        return ERROR;
    }
    //printf("Did not find old data in Index\n");
    return NOT_FOUND;
}
// Copies a file from source to .bgit folder
int makeFile(char *source){

    // TODO: Add directory with hashall 
    FILE *dst,*src;
    char cwd2[64];
    getcwd(cwd2,sizeof(cwd2));
    printf("We are in %s\n",cwd2);
    printf("Adress:%s\n",source);
    char *hash = hashFile(source);
    printf("Hash is :%s \n",hash);
    src = fopen(source,"rb");


    if(src==NULL){
        perror("fopen");
        return 1;
    }

    if(chdir(".bgit")!=0){
        printf("Error switchting directory\n");
        return 1;
    }
    
    char cwd[64]; //hopefully more than enough
    getcwd(cwd,sizeof(cwd));
    printf("We are in: %s \n",cwd); 

    dst = fopen(hash,"wb");
    free(hash);
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
        printf("Error switchting directory\n");
        return 1;
    }
    fclose(src);
    return 0;
}




int makeIndx(char *filename,struct stat FileMetaData){
    if((searchbGit(0))!=0) return -1;
    FILE *indx;

    struct FileMeta content ;
    strcpy(content.name,filename);
    content.inode =FileMetaData.st_ino;
    content.size =FileMetaData.st_size;
    content.mtime = FileMetaData.st_mtime;

    struct FileMeta old;
    if(chdir(".bgit")!=0){
        printf("Error changing directory\n");
        return 1;
    }
    // check if file is already in index file
    size_t bytesRead;
    indx =fopen("index","a+b"); // append and not overwrite 
    // position of reading pointer still at the start of the file
    while((bytesRead=fread(&old,sizeof(content),1,indx))>0){
        if(old.inode==content.inode){
            printf("Got old data:%s \n",old.name);
            fclose(indx);
            if(chdir("..")!=0){
                printf("Error changing directory\n");
                return 1;
            }
            return 0;
        }
    }
    printf("Did not found in index");
    // If not indexed yet, first create file/directory
    // making sure it worked and then adding to index
    if(chdir("..")!=0){
        printf("Error changing directory\n");
        return 1;
    }
    if(makeFile(filename)!=0) return 1;

    printf("Added new data to Index: %s \n",content.name);
    fwrite(&content,sizeof(content),1,indx);

    fclose(indx);
    return 0;
}
int buildSortArray(){
    // It is given that the index is always sorted by namen
    // This hold because we put the elements in one after another
    // could implement when we have many elements 10+ or so we do mergesort
    if(chdir(".bgit")!=0){
        perror("chdir");
        return 1;
    }
    struct stat fileData;
    stat("index",&fileData);
    FILE *indx;
    indx = fopen("index","rb");
    if(indx==NULL){
        chdir("..");
        return NOT_FOUND;
    }
    int fileCount = (fileData.st_size/sizeof(struct FileMeta));
    struct FileMeta *files = malloc(fileCount * sizeof(struct FileMeta));
    
    size_t bytesRead;
    for (size_t i = 0; i < (size_t)fileCount; i++)
    {
        if ((bytesRead=fread(&files[i],sizeof(struct FileMeta),1,indx))>0)
        {
            printf("%ld is %s\n",i,files[i].name);
        }
        
    }

    
    if(files ==NULL){
        printf("Malloc error");
        free(files);
        return 1;
    }
    fclose(indx);
    free(files);
    return 0;
}
int readIndx(){
    if((searchbGit(0))!=0) return -1;
    FILE *indx;
    
    struct FileMeta content;
    
    if(chdir(".bgit")!=0){
        printf("Error changing directory\n");
        return 1;
    }

    size_t bytesRead;
    indx =fopen("index","rb");
    while ((bytesRead=fread(&content,sizeof(content),1,indx))>0)
    {
        printf("Name: %s \n",content.name);
        printf("Inode: %ld \n",content.inode);
        printf("Size: %ld \n",content.size);
        printf("Change time: %ld \n",content.mtime);
    }
   
    //fseek(indx,sizeof(struct FileMeta)-sizeof(char[128]),SEEK_CUR);
    
    
    
    fclose(indx);
    return 0;
}
// Change into the Top-Level Folder of the Repository
// assuming we are in a Repositorys
int chdirTop(){
    char cwd[64];
    getcwd(cwd,sizeof(cwd));
    return 0;
}


int hashAll(char *filepath){
    struct stat fileMeta; // struct from stat.h to save fileinfo/Metadata
        // from stat we get st_size for bytesize
        // st_ino for the inode (unique identifier)
        // and st_mtime for last 
        // check with inode if we have file already if not git add
        DIR *dir; // directory pointer
        struct dirent *dirent; // directory entry

        char cwd[64];
        getcwd(cwd,sizeof(cwd));
        //printf("Before hashing %s in: %s \n",filepath,cwd);
        if((dir=opendir(filepath))==NULL){
            printf("Error opening directory1\n");
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
                    struct FileMeta old;
                    //printf("This is the path %s \n",fullpath);
                    stat(fullpath,&fileMeta);//get fileMetaData
                    int ret = searchIndex(fileMeta.st_ino,&old);
                    if(ret == FOUND){
                    printf("got metadata for %s \n",dirent->d_name);
                    if(fileMeta.st_mtime > old.mtime && fileMeta.st_size != old.size){
                        char *hash = hashFile(fullpath);
                        if(hash){
                            printf("Hash: %s \n",hash);
                            makeFile(dirent->d_name);
                            free(hash);
                        }

                    }
                    }else if (ret == NOT_FOUND){
                        // Not tracked
                        printf("Not Indexed yet: %s \n",fullpath);
                    }else return 1;
                    break;

                case 4:
                    // dir 
                    // recursion

                    //printf("Dir: %s using recursion\n",fullpath);

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

        if(mkdir(filepath,0755)<0){ // normal permission settings
            if(errno == EACCES){
                printf("Fail with access\n");
            }else if(errno == EEXIST){
                printf("Fail because directory already exists\n");
            }
            // TODO: add all Errors
            return 1; // early return
        } 

        printf("initialize Repo\n");
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
        printf("Took %fms \n",time_spent);
        return 0;

        
    }else if(strcmp(command,"add")==0){
        // add 
        
        // TODO: add directories for add and remove 
        char *path = argv[2];
        // writes to index file
        struct stat file;
        stat(path,&file);
        if(S_ISREG(file.st_mode)) makeIndx(path,file);
        else if(S_ISDIR(file.st_mode))hashAll(path);
        else{
            printf("Path given is not a file or Dir");
            return 1;
        }
        return 0;
        
        // logic with metafile
        // also adding "."
    }else if(strcmp(command,"rm")==0){
        char *path = argv[2];
        // also adding "."
        if(chdir(".bgit")!=0){
            printf("Error switchting directory\n");
            return 1;
        }
        
        remove(path);

        if(chdir("..")!=0){
            printf("Error switchting directory\n");
            return 1;
        }
    }else if(strcmp(command,"test")==0){
        struct stat file;
        char *fileName ="sha256.c";
        stat(fileName,&file);
        //makeIndx(fileName,file);
        //readIndx();
        buildSortArray();
    }else{
        printf("%s is not a valid command for Bombagit \n",command);
    }
}