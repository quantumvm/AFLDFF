#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>

#include "afldff_networking.h"

/*
 * Vincent Moscatello - 11/27/2015
 *
 * Basic udp networking implementation. Needs to be compiled using posix 
 * standard. May run into some issues with ansii c.
 *
 */


typedef struct udp_socket_info{
    struct addrinfo * si;
    int sfd;
}udp_socket_info;

static udp_socket_info * get_udp_socket(char * ip, char * port){
    struct addrinfo hints;
    struct addrinfo * result;
    
    int getaddr_ok;
    int sfd;
    
    if(ip == NULL){
        ip = "0.0.0.0";
    }

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family=AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    getaddr_ok = getaddrinfo(ip, port, &hints, &result);
    
    if(getaddr_ok !=0){
        fprintf(stderr,"Error getting address information! This may be the" 
                       " result of an invalid IP or port.\n");
        exit(EXIT_FAILURE);
    }

    for (struct addrinfo * rp = result; rp != NULL; rp = rp->ai_next) {
        sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sfd == -1){
            continue;
        }else{
            udp_socket_info * return_info =  malloc(sizeof(struct udp_socket_info));
            return_info->sfd = sfd;
            return_info->si = rp;
            return return_info;
        }
    }

    return NULL;
}


int get_udp_server(char * ip, char * port){
    udp_socket_info * usi = get_udp_socket(ip, port);
    if(bind(usi->sfd, usi->si->ai_addr, usi->si->ai_addrlen) != -1){
        return usi->sfd;
    }else{
        return -1;
    }
}

int get_udp_client(char * ip, char * port){
    udp_socket_info * usi = get_udp_socket(ip, port);
    if(connect(usi->sfd, usi->si->ai_addr, usi->si->ai_addrlen) != -1){
        return usi->sfd;
    }else{
        return -1;
    }   
}
 
packet_info * get_packet_info(int sfd){
    packet_info * pinfo = calloc(1,sizeof(struct packet_info));
    pinfo->p = calloc(1,sizeof(struct packet));

    pinfo->addrlen = sizeof(struct sockaddr); 

    int rok = recvfrom( sfd, 
                        
                        //buf, len, flags
                        pinfo->p, 
                        sizeof(struct packet), 
                        0, 
                        
                        //src_addr, addrlen
                        &(pinfo->src_addr), 
                        &(pinfo->addrlen));
    
    if(rok != sizeof(struct packet)){
        fprintf(stderr, "recvfrom failed!");
        exit(1);
    }
    
    time(&(pinfo->time_joined));

    return pinfo;
}

void free_packet_info(packet_info * pi){ 
    if(pi->p != NULL){
        free(pi->p);
    }
    free(pi);
}

int send_packet(int sfd, packet * p){
    send(sfd, p, sizeof(struct packet), 0);
    return 0;
}

