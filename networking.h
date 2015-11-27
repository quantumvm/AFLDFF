#ifndef NETWORKING
#define NETWORKING

#include "types.h"

int set_udp_listener(struct server_info * sinfo);

packet_info * parse_udp_packet(int socket);

#endif
