#ifndef STATS_H
#define STATS_H

#include <pthread.h>

struct statistics_t
{
    int total_operations;
    long total_time;
    int connected_clients;
    pthread_mutex_t stats_mutex;
};

#endif // STATS_H
