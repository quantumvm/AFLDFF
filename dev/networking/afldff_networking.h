#ifndef AFLDFF_NETWORKING
#define AFLDFF_NETWORKING

typedef struct packet_info{
    unsigned int instance_id;
    char hash[16];
    long long test_cases;
    long long crashes;
}packet_info;

int get_udp_server(char * ip, char * port);
int get_udp_client(char * ip, char * port);

packet_info * get_packet(int sfd);
int send_packet(int sfd, packet_info * packet);

#endif
