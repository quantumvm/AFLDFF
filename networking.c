#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

#include "types.h"

int set_udp_listener(struct server_info * sinfo){
    int fd;
    struct sockaddr_in bind_info;
    
    memset(&bind_info, 0, sizeof(struct sockaddr_in));
    
    if(sinfo == NULL){
        perror("No serverinfo");
        return 0;
    }

    if((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
        perror("Failed to create socket");
        return 0;
    }
    bind_info.sin_family = AF_INET;
    bind_info.sin_addr.s_addr = htonl(INADDR_ANY); 

    if(sinfo->port != NULL){
        bind_info.sin_port = htons((short) atoi(sinfo->port));
    }else{
        //if user did not give port bind to 31337
        bind_info.sin_port = htons(31337);
    }

    if(bind(fd, (struct sockaddr *) &bind_info, sizeof(bind_info)) < 0){
        perror("Failed to bind to port");
        return 0;
    }

    return fd;

}


packet_info * parse_udp_packet(int socket){
    packet_info * packet = malloc(sizeof(struct packet_info));
    recv(socket, packet, sizeof(struct packet_info), 0);
}




