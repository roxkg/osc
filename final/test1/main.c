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
#define BUS_SIZE 64
#define TIME_OUT 5
/*
    try to combine connmgr & sbuffer. Realize print data within sbuffer_remove
    and exit correctly. one in and one out
*/
sbuffer_t *buffer;
//pthread_mutex_t lock;
int conn_counter = 0;
int total_counter = 0;
pthread_t threads[99];
int fd[2];

void* init_connmgr();
void* init_stormgr();
void* init_datamgr();

int main()
{
    sbuffer_init(&buffer);
    pthread_t connmgr,datamgr;//,stormgr;
    //pthread_mutex_init(&lock,NULL);
    sbuffer_init(&buffer);
    if(pthread_create(&connmgr, NULL, init_connmgr,NULL)!=0) 
                printf("\ncan't create thread connmgr"); 
    //if(pthread_create(&stormgr, NULL, init_stormgr,NULL)!=0) 
                //printf("\ncan't create storage connmgr"); 
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
        //TODO: WRITE LOG 
        time_t start,end;
        start = 0; end = 0;
        while(1)
        {
            if(conn_counter == 0)
            {
                if(start == 0)    {time(&start);}
                else {time(&end);}
                if((end - start)>TIME_OUT)  
                {
                    printf("total counter: %d\n",total_counter);
                    for(int i = 0; i < total_counter;i++)
                    {
                        printf("cancel\n");
                        int x = pthread_join(threads[i],NULL);
                        printf("%d\n",x);
                    }
                    int x = pthread_cancel(connmgr);
                    printf("connmgr: %d\n",x);
                    sensor_data_t data;
                    data.id = 0;
                    sbuffer_insert(buffer,&data);
                    //x = pthread_cancel(stormgr);
                    //printf("stormgr: %d\n",x);
                    //x = pthread_cancel(datamgr);
                    //printf("datamgr: %d\n",x);
                    //over = 1;
                    write(fd[WRITE_END],"close",6);
                    break;
                }
            }
            else
            {
                start = 0; end = 0;
            }
        }
        pthread_join(connmgr,NULL);
        //pthread_join(stormgr,NULL);
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
    FILE* file = fopen("data.csv","w");
    char log[100];
    sprintf(log,"%ld A new data.csv file has been created.",time(NULL));
    write(fd[WRITE_END], log, 100);
    printf("stor init return: %d\n",stormgr_init(file));
    puts("herea");
    fclose(file);
    puts("hereb");
    sprintf(log,"%ld The data.csv file has been closed. ",time(NULL));
    write(fd[WRITE_END], log, 100);
    write(fd[WRITE_END],"close",6);
    puts("stormgr close");
    pthread_exit(SBUFFER_SUCCESS);
}

void* init_datamgr()
{
    FILE * map = fopen("room_sensor.map", "r");
    if(map == NULL) {perror("fail to open room_sensor");pthread_exit(SBUFFER_SUCCESS);}
    datamgr_parse_sensor_files(map);
    puts("datamgr close");
    pthread_exit(SBUFFER_SUCCESS);
}
