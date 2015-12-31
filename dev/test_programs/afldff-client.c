#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>

#include "../networking/afldff_networking.h"

int main(int argc, char ** argv){
    if(argc != 3){
        puts("USE: [ip] [port]");
        exit(1);
    }
    
   
    int client_sfd = get_udp_client(argv[1], argv[2]);
    
    //set random seed
    srand(time(NULL));
    
    //Create three diffrent machines and initialize their IDs
    //and other values to NULL
    packet pi[3];
    
    for(int i=0; i<3; i++){
        memset(&pi[i], 0, sizeof(struct packet));
        pi[i].instance_id = i;
    }

    //initialize hash
    memcpy(pi[0].hash,"AAAAAAAAAAAAAAAA", 16);
    memcpy(pi[1].hash,"AAAAAAAAAAAAAAAA", 16);
    memcpy(pi[2].hash,"BBBBBBBBBBBBBBBB", 16);
 

    int id;
    
    while(1){
        id = rand()%3;
        pi[id].crashes = pi[id].crashes + (rand()%3);
        pi[id].test_cases = pi[id].test_cases + (rand()%1000);

        send_packet(client_sfd, &pi[id]);
        sleep(1);
    }
    
    return 0;
}
