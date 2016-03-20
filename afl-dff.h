#ifndef AFL_DFF
#define AFL_DFF

#include <glib.h>

typedef struct job_node{
    char hash[16];
    char * name;
    int is_open;
    GSList * packet_info_list;
}job_node;

#endif
