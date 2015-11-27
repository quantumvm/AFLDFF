#include <stdio.h>
#include <glib.h>
#include <stdlib.h>

#include "dev/networking/afldff_networking.h"

int main(){
    int sfd_server = get_udp_server("127.0.0.1", "31337");
    packet_info * pi = NULL;

    
    while(1){
        pi = get_packet(sfd_server);

        printf("contact from node: %d\n", pi->instance_id);
        printf("test case: %lld\n", pi->test_cases);
        printf("crashes: %lld\n\n", pi->crashes);
        free(pi);
        pi =NULL;
    }
    

}
