#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <stdint.h>
#include "connmgr.h"
#include "datamgr.h"
#include "sensor_db.h"

#ifndef TIMEOUT
#define TIMEOUT 5
#endif


/*
    try to combine connmgr & sbuffer. Realize print data within sbuffer_remove
    and exit correctly. one in and one out
*/
sbuffer_t *buffer;
pthread_mutex_t insert_lock;
pthread_cond_t insert_signal;
pthread_cond_t write_signal;
int conn_counter = 0;
int total_counter = 0;
pthread_t threads[MAX_CONN];
int fd[2];

void* init_connmgr();
void* init_stormgr();
void* init_datamgr();

int main()
{
    sbuffer_init(&buffer);
    pthread_t connmgr,datamgr,stormgr;
    pthread_mutex_init(&insert_lock,NULL);
    pthread_cond_init(&insert_signal,NULL);
    pthread_cond_init(&write_signal,NULL);
    sbuffer_init(&buffer);
    if(pthread_create(&connmgr, NULL, init_connmgr,NULL)!=0) 
                printf("\ncan't create thread connmgr"); 
    if(pthread_create(&stormgr, NULL, init_stormgr,NULL)!=0) 
                printf("\ncan't create storage connmgr"); 
    if(pthread_create(&datamgr, NULL, init_datamgr,NULL)!=0) 
                printf("\ncan't create datamgr connmgr"); 
    pid_t p;
    if (pipe(fd) == -1) {
        fprintf(stderr, "Pipe Failed");
        return 1;
    }
    p = fork();
    if (p < 0) {
        fprintf(stderr, "fork Failed");
        return 1;
    }
    // Parent process
    else if (p > 0) { 
        time_t start,end;
        start = 0; end = 0;
        while(1)
        {
            if(conn_counter == 0)
            {
                if(start == 0)    {time(&start);}
                else {time(&end);}
                if((end - start)>TIMEOUT)  
                {
                    printf("total counter: %d\n",total_counter);
                    if(total_counter < MAX_CONN){
                        for(int i = 0; i < total_counter;i++)
                        {
                            pthread_join(threads[i],NULL);
                        }
                    }
                    int x = pthread_cancel(connmgr);
                    printf("connmgr: %d\n",x);
                    sensor_data_t data;
                    data.id = 0;
                    sbuffer_insert(buffer,&data);
                    break;
                }
            }
            else
            {
                start = 0; end = 0;
            }
        }
        pthread_join(connmgr,NULL);
        pthread_join(stormgr,NULL);
        pthread_join(datamgr,NULL);
        printf("sensor file done!\n");
        sbuffer_free(&buffer);
        wait(NULL);
        exit(0);
    }
 
    // child process
    else { 
        FILE* logFile = fopen("gateway.log", "w");
        int counter = 0; 
        if(logFile == NULL)
        {
            perror("fail to open gateway.log");
            exit(0);
        }
        while(1) {   
            char buff[100];
            read(fd[0], buff, 100);
            if(strcmp(buff,"")!=0){
                printf("%s\n", buff);
                if(strcmp(buff, "close")==0) break;
                counter++;
                fprintf(logFile, "%d %s\n",counter,buff);
                fflush(logFile);}    
        }    
        fclose(logFile);       
        printf("child finish\n");
        exit(0);
    }
    pthread_cond_destroy(&insert_signal);
    pthread_cond_destroy(&write_signal);
    close(fd[READ_END]);
    close(fd[WRITE_END]);
    return 0;
}

void* init_connmgr()
{
    connect();
    pthread_exit(SBUFFER_SUCCESS);
}

void* init_stormgr()
{
    FILE* file = open_db("data.csv",false);
    //stormgr_init(file);
    sensor_data_t* data = malloc(sizeof(sensor_data_t));
    memset(data,0,sizeof(sensor_data_t));
    //char log[MAX_SIZE];
    //memset(log,0,sizeof(log));
    //char result[BUS_SIZE];
    //memset(result,0,sizeof(result));
    while(1)
    {
        int i = sbuffer_remove(buffer,data,1);
        if(i != SBUFFER_FAILURE ) {
            if(data->id==0) {puts("stormgr break");break;}
            insert_sensor(file,data->id,data->value,data->ts);
            pthread_cond_wait(&write_signal,&insert_lock);
        }
        else {perror("sbuffer read failure");break;}
    }
    free(data);
    puts("stormgr_init exit");
    close_db(file);
    pthread_exit(SBUFFER_SUCCESS);
}

void* init_datamgr()
{
    FILE * map = fopen("room_sensor.map", "r");
    if(map == NULL) {perror("fail to open room_sensor");pthread_exit(SBUFFER_SUCCESS);}
    datamgr_parse_sensor_files(map);
    puts("datamgr close");
    datamgr_free();
    pthread_exit(SBUFFER_SUCCESS);
}
