#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

#include "types.h"

int get_udp_client_socket(char * ip, char * port){
    struct addrinfo hints;
    struct addrinfo * result;
    int getaddr_ok;
    
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family=AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = 0;
    hints.ai_protocol = 0;
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;
 
    getaddr_ok = getaddrinfo(ip, port, &hints, &result);
    

    for (struct addrinfo * rp = result; rp != NULL; rp = rp->ai_next) {
        sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sfd == -1){
            continue;
        }else{
            return sfd;
        }
    }

    return sfd;
}

int get_udp_server_socket(char * ip, char * port){
    struct addrinfo hints;
    struct addrinfo * result;
    int getaddr_ok;
    
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family=AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = 0;
    hints.ai_protocol = 0;
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;
 
    getaddr_ok = getaddrinfo(ip, port, &hints, &result);
    

    for (struct addrinfo * rp = result; rp != NULL; rp = rp->ai_next) {
        sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sfd == -1){
            continue;
        }else{
            return sfd;
        }
    }

    return sfd;
}


int get_client_socket(){
    packet_info * packet = malloc(sizeof(struct packet_info));
    recv(socket, packet, sizeof(struct packet_info), 0);
}



