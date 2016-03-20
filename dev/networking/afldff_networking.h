#ifndef AFLDFF_NETWORKING
#define AFLDFF_NETWORKING

#include <sys/types.h>
#include <sys/socket.h>
#include <time.h>

typedef struct packet{
    unsigned int instance_id;
    char hash[16];
    long long test_cases;
    long long crashes;
}packet;

typedef struct packet_info{
    packet * p;
    socklen_t addrlen;
    struct sockaddr src_addr;
    time_t time_joined;
    int is_selected;
}packet_info;

int get_udp_server(char * ip, char * port);
int get_udp_client(char * ip, char * port);

packet_info * get_packet_info(int sfd);
void free_packet_info(packet_info * pi);
int send_packet(int sfd, packet * packet);

#endif
