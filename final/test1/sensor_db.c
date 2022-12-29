#include<stdio.h>
#include<stdlib.h>
#include<assert.h>
#include"config.h"
#include<stdbool.h>
#include<fcntl.h>
#include<unistd.h>
#include<pthread.h>
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
    char log[100];
    memset(log,0,sizeof(log));
    sprintf(log,"%ld A new csv file is created.",time(NULL));
    pthread_mutex_lock(&pip_lock);
    write(fd[WRITE_END], log, 100);
    pthread_mutex_unlock(&pip_lock);
    assert(file!=NULL);
    return file;
}
    
int insert_sensor(FILE * f, sensor_id_t id,sensor_value_t value,sensor_ts_t ts)
{
    //int i = fprintf(f, "%hu,%f,%ld\n",id,value,ts);
    char log[MAX_SIZE];
    memset(log,0,sizeof(log));
    char result[64];
    memset(result,0,sizeof(result));
    sprintf(result,"%hu %g %ld\n",id,value,ts);
    int i = fputs(result,f);
    fflush(f);
    if(i < 0) 
    {
        sprintf(log,"%ld An error occured when writing to the csv file.",ts);
    }
    else
    {
        sprintf(log,"%ld Data insertion from sensor %d succeeded.",ts,id);    
    }
    pthread_mutex_lock(&pip_lock);
    write(fd[WRITE_END], log, MAX_SIZE);
    pthread_mutex_unlock(&pip_lock);
    return i;
}

int close_db(FILE *f)
{
    int i = fclose(f);
    char log[100];
    memset(log,0,sizeof(log));
    if(i < 0) 
    {
        sprintf(log,"%ld An error occured when closing the csv file.",time(NULL));       
    }
    else
    {
        sprintf(log,"%ld The csv file has been closed.",time(NULL));
    }
    pthread_mutex_lock(&pip_lock);
    write(fd[WRITE_END], log, 100);
    write(fd[WRITE_END],"close",6);
    pthread_mutex_unlock(&pip_lock);
    return i;
}

