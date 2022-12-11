#include<stdio.h>
#include<stdlib.h>
#include"config.h"
#include<stdbool.h>
#include <time.h>

# define MAX_SIZE 100
# define READ_END 0
# define WRITE_END 1

extern int fd[2];
//extern bool pipe_empty;

FILE * open_db(char * filename, bool append);

int insert_sensor(FILE * f, sensor_id_t id,sensor_value_t value,sensor_ts_t ts);

int close_db(FILE *f);

void write_log(char* log);
