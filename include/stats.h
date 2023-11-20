#ifndef STATS_H
#define STATS_H

#include <pthread.h>
#include <sys/time.h>

struct statistics_t
{
    int total_operations;
    long total_time;
    int connected_clients;
    pthread_mutex_t stats_mutex;
};

// Start timer on t
void start_timing(struct timeval *t);

// Finish timer and return time in ms
double stop_timing(const struct timeval *t);

#endif // STATS_H
