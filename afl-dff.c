#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

#include <glib.h>

#include "dev/networking/afldff_networking.h"

//startup flags
typedef struct command_args{
    char * ip;
    char * port;
}command_args;


//Data structure that contains our machine ids and their data.
GSList * N_NODE = NULL;

//Location of our command line arguments
command_args flags;


/********************************************************************
 * Start our udp listener (starts up listener in seperate thread)
 ********************************************************************/
static void print_packet_list();

static void start_server(){
 
    int sfd_server = get_udp_server(flags.ip, flags.port);
    packet_info * pi = NULL;
    
    if(sfd_server == -1){
        fprintf(stderr, "Failed to start server!\n");
        exit(EXIT_FAILURE);
    }

   
    GSList * gslp;
    while(1){
        gslp = NULL;
        pi = get_packet(sfd_server);

        //iterate through our linked list. Update it if we find a matching machine ID.
        for(gslp = N_NODE; gslp; gslp=gslp->next){
            if(((packet_info *) gslp->data)->instance_id == pi->instance_id){
                 free(gslp->data);
                 gslp->data = pi;
                 break;
            }
        }
        
        //if the id is not in our linked list append it.
        if(gslp == NULL){
            N_NODE = g_slist_append(N_NODE, pi);
        }

        print_packet_list();
    }
       
}

//debug function for printing packet list
static void print_packet_list(){
    puts("\n|---NODE---|");
    for(GSList * p = N_NODE; p; p=p->next){
        printf("contact from node: %d\n", ((packet_info *) p->data)->instance_id);
        printf("test case: %lld\n",       ((packet_info *) p->data)->test_cases);
        printf("crashes: %lld\n\n",       ((packet_info *) p->data)->crashes);
    }
}


/********************************************************************
 * Enter the ncurses management interface. 
 ********************************************************************/
void start_graphics(){
     
}

int main(int argc, char * argv[]){


//Read our arguments from standard in and set the appropriate flags to 
//use in the command_args structure
 
    int opt;

    //This should be initialized to zero but we will set it again anyway
    memset(&flags, 0, sizeof(struct command_args));

    while((opt = getopt(argc, argv, "i:p:h")) != -1){
        switch(opt){
        case 'i':
            flags.ip = optarg;
            break;
        case 'p':
            flags.port = optarg;
            break;
        case 'h':
            puts("-i = ip address to listen on. If left blank default to" 
                 " 0.0.0.0 ");
            puts("-p = port to listen on");
            puts("-h help");
            exit(0);
            break;
        default:
            fprintf(stderr,"Invalid arguments. -h for help\n");
            exit(EXIT_FAILURE);
        }
    }   

    if( flags.port == NULL ){
        fprintf(stderr,"Missing command line option '-p'! [-h for help]\n");
        exit(EXIT_FAILURE);
    }

//Start up the udp listener. Relocate to a seperate thread later.
    start_server();
}
