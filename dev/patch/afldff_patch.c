#include <unistd.h>
#include <string.h>
#include <openssl/md5.h>
#include <stdio.h>
#include <stdlib.h>

#include "afldff_patch.h"

static char * afl_valid_hash[] = {
    "a87164448a1e9a4007919a26c29e7c76" //afl-1.96b
};

static char * hash_to_string(unsigned char * hash){
    char * hash_string = calloc(1,32+1);

    for(int i=0; i<16;i++){
        sprintf(&hash_string[i*2], "%02x", hash[i]);
    }

    hash_string[32] = '\x00';
    
    return hash_string;
}

//Check if the hash of the tar is equal to a supported hash 

static int afl_valid_tar(char * file_name){
    
    FILE * tar_file;
    
    if((tar_file = fopen(file_name, "r")) == NULL){
        return -1;
    }
    
    int bytes;
    char hash[16]; 
    char hash_file_in[512];

    MD5_CTX md5_context;
    MD5_Init(&md5_context);

    //read the tar file in chunks of 512 bytes
    while((bytes=fread(hash_file_in, 1,sizeof(hash_file_in), tar_file))){
       MD5_Update(&md5_context, hash_file_in, bytes); 
    }

    //generate the hash for the tar
    MD5_Final((unsigned char *) hash, &md5_context); 
     
    //check that the tar hash matches a supported version of afl
    for(int i=0; i<(sizeof(afl_valid_hash)/sizeof(char *)); ++i){
        char * tar_hash_string = hash_to_string((unsigned char *)hash);        

        if(strcmp(tar_hash_string, afl_valid_hash[i]) == 0){
            fclose(tar_file);
            free(tar_hash_string);
            return 0;
        }
        
        free(tar_hash_string);
    }
    
    fclose(tar_file);
    return -1;
}

int patch_afl(char * afl_tar_path, int cli){
    //check that the tar file exists before trying to extract it.
    if(access(afl_tar_path, R_OK) == -1){
        if(cli){
            fprintf(stderr, "Could not access tar file.\n");
        }
        return -1;
    }
    
    //Make sure our patches support this version of afl
    if(afl_valid_tar(afl_tar_path) == 0){
        puts("Valid version of AFL");
    }else{
        if(cli){
            fprintf(stderr, "This version of AFL is not supported!\n");
        }
        return -1;
    }
    
    return 0;
}
