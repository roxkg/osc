#include<stdio.h>
#include<stdlib.h>
#include<assert.h>
#include"config.h"
#include<stdbool.h>
#include<fcntl.h>
#include <sys/wait.h>
#include<unistd.h>
#include<string.h>
#include"sensor_db.h"

FILE * open_db(char * filename, bool append)
{
    FILE * file;
    if(append)
    {
        file = fopen(filename, "a");
    }
    else
    {
        file = fopen(filename, "w");
    }
    char log[100] = "A new csv file is created or an existing file has been opened.\n";
    write_log(log);
    assert(file!=NULL);
    return file;
}
    
int insert_sensor(FILE * f, sensor_id_t id,sensor_value_t value,sensor_ts_t ts)
{
    int i = fprintf(f, "%hu,%f,%ld\n",id,value,ts);
    char log[100];
    if(i < 0) 
    {
        strcpy(log,"An error occured when writing to the csv file.\n");
    }
    else
    {
        strcpy(log,"Data insertion succeeded.\n");     
    }
    write_log(log);
    return i;
}

int close_db(FILE *f)
{
    int i = fclose(f);
    char log[100];
    if(i < 0) 
    {
        strcpy(log,"An error occured when closing the csv file.\n");       
    }
    else
    {
        strcpy(log,"The csv file has been closed.\n");
    }
    write_log(log);
    return i;
}

