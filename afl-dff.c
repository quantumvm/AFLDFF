#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

#include <glib.h>
#include <pthread.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>


#include "afl-dff.h"
#include "dev/networking/afldff_networking.h"
#include "dev/networking/afldff_access.h"
#include "dev/interface/afldff_ncurses.h"
#include "dev/patch/afldff_patch.h"

//startup flags
typedef struct command_args{
    char * ip;
    char * port;
    char * path_afl_tar;

}command_args;
//command line arguments
command_args flags;


//Data structure that contains our machine ids and their data.
//
// |------|   |------|
// | name |-> | name | -> NULL
// |------|   |------|
//    |          |
//    V          V
// |------|     NULL
// | ID   |
// |------|
//
GSList * GLOBAL_JOB_MATRIX = NULL;
//mutex for accessing linked list
pthread_mutex_t ll_mutex;
pthread_mutexattr_t ll_mutex_attr;

GQueue * NEW_NODE_QUEUE = NULL;
//mutex for accessing queue
pthread_mutex_t q_mutex;

/********************************************************************
 * Start our udp listener (starts up listener in seperate thread)   *
 ********************************************************************/

void * start_server(void *ptr){
 
    int sfd_server = get_udp_server(flags.ip, flags.port);
    
    if(sfd_server == -1){
        fprintf(stderr, "Failed to start server!\n");
        exit(EXIT_FAILURE);
    }
   
    while(1){
        
        packet_info * pi = get_packet_info(sfd_server);
         
        pthread_mutex_lock(&ll_mutex);
            
            //Look for a job node that has the hash found in the incoming packet
            GSList * job_node_list_pointer = GLOBAL_JOB_MATRIX;    
            for(; job_node_list_pointer; job_node_list_pointer=job_node_list_pointer->next){
                job_node * data = job_node_list_pointer->data; 
                if(memcmp( data->hash, pi->p->hash, 16)== 0){
                    break;
                }
            }
            

            //If the hash is not found create a new job node and add it to the
            //job matrix.
            if(job_node_list_pointer == NULL){
                job_node * data = calloc(1, sizeof(struct job_node));
                memcpy(data->hash, pi->p->hash, 16);
                GLOBAL_JOB_MATRIX = g_slist_append(GLOBAL_JOB_MATRIX, data);
                job_node_list_pointer = g_slist_last(GLOBAL_JOB_MATRIX);
            }
            
            
            //Try and find matching ID in job
            job_node * job = job_node_list_pointer->data;
            GSList * pilp = job->packet_info_list;
            for(; pilp; pilp=pilp->next){
                packet_info * data = pilp->data;
                if(data->p->instance_id == pi->p->instance_id){
                     free_packet_info(pilp->data);
                     pilp->data = pi;
                     break;
                }
            }
            
            //if the id is not in our linked list append it.
            if(pilp == NULL){
                job->packet_info_list = g_slist_append(job->packet_info_list, pi);
                
                pthread_mutex_lock(&q_mutex);
                    g_queue_push_head(NEW_NODE_QUEUE, pi);
                pthread_mutex_unlock(&q_mutex);
                
            }
        pthread_mutex_unlock(&ll_mutex);
       
        //struct sockaddr_in * sockin = (struct sockaddr_in *) &pi->src_addr;
        //printf("sockaddr: %s\n", inet_ntoa(sockin->sin_addr));

    }
       
}


//start server thread
static void start_server_thread(pthread_t * thread){
    int ret = pthread_create(thread, NULL, start_server, NULL);
    if(ret){
        fprintf(stderr, "Could not start server thread!");
        exit(EXIT_FAILURE);
    }
}

static void print_help(){
    puts("afldff [ options ]");
    puts("  -i ip       - IP address to listen on defaults to 0.0.0.0 if left blank");
    puts("  -p port     - Port to listen on");
    puts("  -m tar      - Patch afl to be network compatable");
    puts("  -h          - Print help screen");   
}

int main(int argc, char * argv[]){
    pthread_t udp_listener;

//Read our arguments from standard in and set the appropriate flags to 
//use in the command_args structure
 
    int opt;
    
    //This should be initialized to zero but we will set it again anyway
    memset(&flags, 0, sizeof(struct command_args));

    while((opt = getopt(argc, argv, "m:i:p:h")) != -1){
        switch(opt){
        case 'i':
            flags.ip = optarg;
            break;
        case 'p':
            flags.port = optarg;
            break;
        case 'm':
            flags.path_afl_tar = optarg;
            break;
        case 'h':
            print_help();
            exit(0);
            break;
        default:
            print_help();
            exit(EXIT_FAILURE);
        }
    }   
    
    if(flags.path_afl_tar != NULL){
        patch_afl(flags.path_afl_tar, 1);
        exit(0);
    }

    if( flags.port == NULL ){
        print_help();
        exit(EXIT_FAILURE);
    }



//Create queue mutex and initialize queue data structure
    NEW_NODE_QUEUE = g_queue_new(); 
    if(pthread_mutex_init(&q_mutex, NULL)!=0){
        fprintf(stderr,"Failed to create mutex\n");
        exit(EXIT_FAILURE);       
    }


    
//Start up the udp listener and initialize GLOBAL_JOB_MATRIX mutex
    
    //set up ll_mutex to use recursive a recursive mutex
    if(pthread_mutexattr_init(&ll_mutex_attr)!=0){
        fprintf(stderr,"Failed to create ll_mutex attribute\n");
        exit(EXIT_FAILURE);       
    }
    
    if(pthread_mutexattr_settype(&ll_mutex_attr, PTHREAD_MUTEX_RECURSIVE)!=0){
        fprintf(stderr,"Failed to set ll_mutex attribute\n");
        exit(EXIT_FAILURE);       
    }

    if(pthread_mutex_init(&ll_mutex, &ll_mutex_attr)!=0){
        fprintf(stderr,"Failed to create mutex\n");
        exit(EXIT_FAILURE);       
    }   
    start_server_thread(&udp_listener);

    

    draw_afldff_interface();
}


