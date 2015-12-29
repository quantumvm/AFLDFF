#include "afldff_access.h"
#include "../networking/afldff_networking.h"
#include "../../afl-dff.h"

#include <glib.h>
#include <pthread.h>
#include <string.h>

extern pthread_mutex_t ll_mutex;
extern GSList * GLOBAL_JOB_MATRIX;

long long get_test_case(unsigned int id ){
    long long test_cases;

    pthread_mutex_lock(&ll_mutex);
        
        //jlp is "job list pointer"
        for(GSList * jlp=GLOBAL_JOB_MATRIX; jlp; jlp=jlp->next){
            job_node * job = jlp->data; 
            for(GSList * gslp = job->packet_info_list; gslp; gslp=gslp->next){
                if(((packet_info *) gslp->data)->instance_id == id){
                    test_cases = ((packet_info *) gslp->data)->test_cases;
                    pthread_mutex_unlock(&ll_mutex);
                    return test_cases; 
                }
            }
        }

    pthread_mutex_unlock(&ll_mutex);

    return -1;
}

long long get_crash_case(unsigned int id){
    long long crash_cases;

    pthread_mutex_lock(&ll_mutex);
        
        //jlp is "job list pointer"
        for(GSList * jlp=GLOBAL_JOB_MATRIX; jlp; jlp=jlp->next){
            job_node * job = jlp->data; 
            for(GSList * gslp = job->packet_info_list; gslp; gslp=gslp->next){
                if(((packet_info *) gslp->data)->instance_id == id){
                    crash_cases = ((packet_info *) gslp->data)->crashes;
                    pthread_mutex_unlock(&ll_mutex);
                    return crash_cases; 
                }
            }
        }       
    
    pthread_mutex_unlock(&ll_mutex);

    return -1;
}

//returns the sum of all crash cases
long long get_all_crash_cases(){
    long long crash_cases = 0;

    pthread_mutex_lock(&ll_mutex);
         
        //jlp is "job list pointer"
        for(GSList * jlp=GLOBAL_JOB_MATRIX; jlp; jlp=jlp->next){
            job_node * job = jlp->data; 
            for(GSList * gslp = job->packet_info_list; gslp; gslp=gslp->next){
                crash_cases = crash_cases + ((packet_info *) gslp->data)->crashes;
            }
         }

    pthread_mutex_unlock(&ll_mutex);
    return crash_cases;
}

//returns sum of all test cases
long long get_all_test_cases(){
    long long test_cases = 0;

    pthread_mutex_lock(&ll_mutex);
         
        //jlp is "job list pointer"
        for(GSList * jlp=GLOBAL_JOB_MATRIX; jlp; jlp=jlp->next){
            job_node * job = jlp->data; 
            for(GSList * gslp = job->packet_info_list; gslp; gslp=gslp->next){
                test_cases = test_cases + ((packet_info *) gslp->data)->test_cases;
            }
         }

    pthread_mutex_unlock(&ll_mutex);
    return test_cases;


}

