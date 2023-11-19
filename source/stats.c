#include <pthread.h>

//Start timer on t
void start_timing(struct timeval* t){
    gettimeofday(t, (void*)0);
}

//Finish timer and return time in ms
double stop_timing(const struct timeval* t){
    struct timeval t2;
    gettimeofday(&t2, (void*)0);
    
    double ms = (t2.tv_sec - t->tv_sec) * 1000.0;
    ms += (t2.tv_usec - t->tv_usec) / 1000.0;
    return ms;
}

//Update the stats
void update_stats(struct statistics* stats, int op_code, double ms){

}


