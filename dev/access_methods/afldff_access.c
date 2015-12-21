#include "afldff_access.h"
#include "../networking/afldff_networking.h"

#include <glib.h>
#include <pthread.h>

extern pthread_mutex_t ll_mutex;
extern GSList * N_NODE;

long long get_test_cases(unsigned int id ){
    long long test_cases;

    pthread_mutex_lock(&ll_mutex);
        for(GSList * gslp = N_NODE; gslp; gslp=gslp->next){
            if(((packet_info *) gslp->data)->instance_id == id){
                test_cases = ((packet_info *) gslp->data)->test_cases;
                pthread_mutex_unlock(&ll_mutex);
                return test_cases; 
            }
        }       
    pthread_mutex_unlock(&ll_mutex);

    return -1;
}

long long get_crash_cases(unsigned int id){
    long long crash_cases;

    pthread_mutex_lock(&ll_mutex);
        for(GSList * gslp = N_NODE; gslp; gslp=gslp->next){
            if(((packet_info *) gslp->data)->instance_id == id){
                crash_cases = ((packet_info *) gslp->data)->crashes;
                pthread_mutex_unlock(&ll_mutex); 
                return crash_cases;
            }
        }       
    pthread_mutex_unlock(&ll_mutex);

    return -1;
}

