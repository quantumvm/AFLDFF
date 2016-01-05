#include <unistd.h>
#include <string.h>
#include <openssl/md5.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/wait.h>

#include "afldff_patch.h"

static char * afl_valid_hash[] = {
    "a87164448a1e9a4007919a26c29e7c76" //afl-1.96b
};

/************************************************************
 * Return an ascii representation of the hash pointed to by *
 * char * hash                                              *
 ************************************************************/

static char * hash_to_string(unsigned char * hash){
    char * hash_string = calloc(1,32+1);

    for(int i=0; i<16;i++){
        sprintf(&hash_string[i*2], "%02x", hash[i]);
    }

    hash_string[32] = '\x00';
    
    return hash_string;
}


/************************************************************
 * Check if the hash of the tar is equal to a supported     *
 * hash return 0 on success and -1 on failure.              *
 ************************************************************/

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


/************************************************************
 * Untar afl return 0 on success and -1 on failure          *
 * cli is not currently supported and does nothing          *
 ************************************************************/

static int untar_afl(char * path, int cli){
    
    //tar -xf [path to afl tar]
    char * args[4];
    args[0] = "tar";
    args[1] = "-xf";
    args[2] = path;
    args[3] = NULL;
    
    pid_t pid;
    if((pid = fork()) == 0){
        execvp(args[0], args);  
    }
    else if(pid == -1){
        if(cli){
            fprintf(stderr, "Failed to fork.");
        }
        return -1;
    }
    
    int status;
    waitpid(pid, &status, 0);

    if(!WIFEXITED(status)){
        return -1;
    }

    return 0;


}


/************************************************************
 * Function for applying patch to afl. Mark cli (command    *
 * line) 1 for console output                               *
 ************************************************************/

int patch_afl(char * afl_tar_path, int cli){
    //check that the tar file exists before trying to extract it.
    if(access(afl_tar_path, R_OK) == -1){
        if(cli){
            fprintf(stderr, "Could not access tar file.\n");
        }
        return -1;
    }
    
    //Make sure our patches support this version of afl
    if(afl_valid_tar(afl_tar_path) == -1){
        if(cli){
            fprintf(stderr, "This version of AFL is not supported!\n");
        }
        return -1;
    }
    
    //untar afl
    if(untar_afl(afl_tar_path, 1) == -1){
        if(cli){
            fprintf(stderr, "Failed to untar afl");
        }
        return -1;
    }

    
    return 0;
}
