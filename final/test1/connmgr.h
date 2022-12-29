#include "config.h"
#include "sbuffer.h"

extern sbuffer_t* buffer;
extern int conn_counter;
extern int total_counter;
extern int PORT;
extern pthread_mutex_t pip_lock;
extern pthread_t threads[MAX_CONN];

void* get_connect();

