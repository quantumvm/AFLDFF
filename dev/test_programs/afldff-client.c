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
    
    packet_info pi;
    memset(&pi, 0, sizeof(struct packet_info));
    pi.instance_id = 42;

    int client_sfd = get_udp_client(argv[1], argv[2]);
    
    srand(time(NULL));
    
    for(int i =0; i< 100; i++){
        pi.instance_id = rand()%3;
        pi.test_cases = rand(); 
        pi.crashes = rand();
        send_packet(client_sfd, &pi);
        sleep(5);
    }
    
    return 0;
}
