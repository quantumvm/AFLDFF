#ifndef BASIC_TYPES
#define BASIC_TYPES

typedef struct server_info{
    char * ip;
    char * port;
    int verbose;
}server_info;

typedef struct packet_info{
    int instance_id;
    long long test_cases;
    long long crashes;
}packet_info;

#endif
