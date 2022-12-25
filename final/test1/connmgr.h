#include "config.h"
#include "sbuffer.h"

extern sbuffer_t* buffer;
extern int conn_counter;
extern int total_counter;
extern pthread_t threads[MAX_CONN];

void* connect();

