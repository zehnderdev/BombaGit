#include <stdio.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <stdlib.h>
#include <string.h>

#define BUFF_SIZE 4096
#define SHA256_SIZE 32

// Using sha256 from openssl

// Cleanup funktion after error and when success
void cleanUp(EVP_MD_CTX *context,FILE *file){
    ERR_print_errors_fp(stderr);

    if(context) EVP_MD_CTX_free(context);
   
    if(file) fclose(file);
}

char *hashFile(const char *filename){

    if(filename==NULL) return NULL;
    FILE *file = fopen(filename,"rb");
    if(file == NULL){
        perror("Could not open file");
        return NULL;
    }

    EVP_MD_CTX *context = EVP_MD_CTX_new(); // state of hash

    if(context == NULL){
        // gives error ?
        printf("Error initializing hash context");
        fclose(file);
        return NULL;
    }

    if(EVP_DigestInit_ex(context,EVP_sha256(), NULL) != 1){
        printf("SHA-256 initialziation failed");
        cleanUp(context,file);
        return NULL;
    }
    
    unsigned char buffer[BUFF_SIZE];
    size_t bytesRead;

    // reading file blockwise, Blocksize = 4096
    while((bytesRead = fread(buffer,1,BUFF_SIZE,file))>0){
        
        if(EVP_DigestUpdate(context,buffer,bytesRead)!=1){
            printf("Hash update failed");
            cleanUp(context,file);
            return NULL;
        }
    }
    // check for reading error after while
    if(ferror(file)){
        printf("Error reading file");
        cleanUp(context,file);
        return NULL;
    }


    unsigned char *hash = malloc(SHA256_SIZE); // sha256 always 32Bytes
    if(!hash){
        cleanUp(context,file);
        return NULL;
    }
    unsigned int hashLength;

    if(EVP_DigestFinal_ex(context,hash, &hashLength)!= 1){
        printf("Hash finalization failed");
        free(hash);
        cleanUp(context,file);
        return NULL;
    }

    // change to hex representation
    char *hexHash = malloc(hashLength *2 +1);

    if(hexHash == NULL){
        free(hash);
        cleanUp(context,file);
        return NULL;
    }
    

    for (unsigned int i = 0; i < hashLength; i++)
    {
        sprintf(hexHash + (i * 2), "%02x",hash[i]); //lowercase hexadecimal 
    }
    hexHash[hashLength *2] ='\0';

    free(hash);

    if(context) EVP_MD_CTX_free(context);

    if(file) fclose(file);

    return hexHash;
}



