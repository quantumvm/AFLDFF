#include <stdio.h>
#include <glib.h>

#include "networking.h"

int main(){
    server_info s;
    s.ip = "0.0.0.0";
    s.port = NULL;

    int socket = set_udp_listener(&s);
    
    while(1){
        packet_info * parse_udp_packet(socket);
        printf("id: %d, test_cases: %d, crashes: %d\n", 
                packet_info->instance_id,
                packet_info->test_cases,
                packet_info->crashes);
    }


}
