#define _GNU_SOURCE

#include <unistd.h>
#include <string.h>
#include <openssl/md5.h>
#include <stdio.h>
#include <stdlib.h>
#include <glib.h>

#include <sys/types.h>
#include <sys/wait.h>


#include "afldff_patch.h"

typedef struct afl_version_info{
    char * hash;
    char * version;
}afl_version_info;


static GSList* afl_valid_versions = NULL;
static afl_version_info * valid_version = NULL;

static void set_afl_version_info(){
    
    //version 1.96b
    afl_version_info * version = malloc(sizeof(struct afl_version_info));
    version->hash = "a87164448a1e9a4007919a26c29e7c76";
    version->version = "afl-1.96b";
    
    afl_valid_versions = g_slist_append(afl_valid_versions, version);
}

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
    for(GSList * gslp = afl_valid_versions ; gslp; gslp = gslp->next){
        char * tar_hash_string = hash_to_string((unsigned char *)hash);        

        if(strcmp(tar_hash_string, ((struct afl_version_info *) gslp->data)->hash) == 0){
            fclose(tar_file);
            free(tar_hash_string);
            valid_version = (struct afl_version_info *) gslp->data;
            return 0;
        }
        
        free(tar_hash_string);
    }
    
    fclose(tar_file);
    return -1;
}


/************************************************************
 * Untar afl return 0 on success and -1 on failure          *
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
            fprintf(stderr, "Failed to fork.\n");
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
 * Patch afl return 0 on success and -1 on failure          *
 ************************************************************/

static int patch_file(char * source_code, char * patch_file, int cli){
    char * args[4];
    args[0] = "patch";
    args[1] = source_code;
    args[2] = patch_file;
    args[3] = NULL;
    
    if(access(patch_file, R_OK) == -1){
        if(cli){
            fprintf(stderr, "Could not get patch file [did you run make install?]\n");
        }
        return -1;
    }

    if(access(source_code, R_OK | W_OK) == -1){
        if(cli){
            fprintf(stderr, "Could not get afl source\n");
        }
        return -1;
    }

    pid_t pid;
    if((pid = fork()) == 0){
        execvp(args[0], args);  
    }
    else if(pid == -1){
        if(cli){
            fprintf(stderr, "Failed to fork.\n");
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



//we will use the shoddy method of calling cp at the moment
static int copy_resources(char * source, char * destination, int cli){
    char * args[4];
    args[0] = "cp";
    args[1] = source;
    args[2] = destination;
    args[3] = NULL;
    
    if(access(source, R_OK) == -1){
        if(cli){
            fprintf(stderr, "Could not find source file %s\n", source);
        }
        return -1;
    }

    pid_t pid;
    if((pid = fork()) == 0){
        execvp(args[0], args);  
    }
    else if(pid == -1){
        if(cli){
            fprintf(stderr, "Failed to fork.\n");
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
    //create linked list of valid afl versions
    if((valid_version == NULL) || (afl_valid_versions == NULL)){
        set_afl_version_info();
    }
    
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
    if(untar_afl(afl_tar_path, cli) == -1){
        if(cli){
            fprintf(stderr, "Failed to untar afl\n");
        }
        return -1;
    }
    
    //If untar was successful get the path into the extracted tar
    char * afl_c_source;
    char * afl_makefile_source;
    asprintf(&afl_c_source,         "%s/%s", valid_version->version, "afl-fuzz.c");
    asprintf(&afl_makefile_source,  "%s/%s", valid_version->version, "Makefile");
    //patch c file
    if(patch_file(afl_c_source, "/opt/afldff/patches/afl-fuzz.patch", cli) == -1){
        if(cli){
            fprintf(stderr, "Failed to patch afl-fuzz.c\n");
        }
        return -1;
    }   
    
    //patch Makefile
    if(patch_file(afl_makefile_source, "/opt/afldff/patches/Makefile.patch", cli) == -1){
        if(cli){
            fprintf(stderr, "Failed to patch Makefile\n");
        }
        return -1;
    }


    //copy the necissary source files to run the patched version of afl
    if(copy_resources("/opt/afldff/afldff_networking.c", valid_version->version, cli) == -1){
        if(cli){
            fprintf(stderr, "Failed to copy afldff_networking.c\n");
        }
        return -1;
    }
    if(copy_resources("/opt/afldff/afldff_networking.h", valid_version->version, cli) == -1){
        if(cli){
            fprintf(stderr, "Failed to copy afldff_networking.h\n");
        }
        return -1;
    }    
    
    free(afl_c_source);
    free(afl_makefile_source);
    
    return 0;
}
