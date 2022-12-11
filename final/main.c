#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include "sensor_db.h"
#include "sbuffer.h"
#define BUS_SIZE 64
int fd[2];
sbuffer_t *buffer;
void* init_connmgr();
void* init_datamgr();
void* init_stormgr();


int main()
{
    pthread_t connmgr,datamgr,stormgr;
    pthread_mutex_init(&lock,NULL);
    sbuffer_init(&buffer);
    if(pthread_create(&connmgr, NULL, init_connmgr,NULL)!=0) 
                printf("\ncan't create thread connmgr"); 

    if(pthread_create(&datamgr, NULL, init_datamgr, NULL)!=0)
                printf("\ncan't create thread datamgr");           

    if(pthread_create(&stormgr, NULL, init_stormgr, NULL)!=0)
                printf("\ncan't create thread stormgr");
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
        FILE* f = open_db("test.txt",false);
        /*for(int i = 0; i < 10; i++)
        {
            insert_sensor(f,1,14.5+i,time(NULL));
        }*/
        //TODO: WRITE LOG 
        close_db(f);
        write(fd[WRITE_END],"close",100);
        printf("sensor file done!\n");
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
            printf("looping... \n");
            char buff[100];
            read(fd[0], buff, 100);
            printf(" %s", buff);
            fflush(stdout);
            if(strcmp(buff, "close")==0) break;
            char* result = strtok(buff, "\n");
            counter++;
            fprintf(logFile, "%d %s\n",counter,result);    
        }    
        fclose(logFile);       
        printf("child finish\n");
        exit(0);
    }
    close(fd[READ_END]);
    close(fd[WRITE_END]);
    pthread_join(datamgr,NULL);
    pthread_join(connmgr,NULL);
    pthread_join(stormgr,NULL);
    sbuffer_free(&buffer);
    return 0;
}

void* init_connmgr()
{

}

void* init_datamgr()
{
    FILE * map = fopen("room_sensor.map", "r");

    if(map == NULL) return -1;

    datamgr_parse_sensor_files(map);
}

void* init_stormgr()
{
    FILE* file = fopen("data.csv","w");
    sensor_data_t* data = malloc(sizeof(sensor_data_t));
    while(!(sbuffer_remove(buffer,data) != SBUFFER_SUCCESS && over == 1))
    {
        pthread_mutex_lock(&lock);
        char result[BUS_SIZE];
        sprintf(result,"%hu %lf %ld\n",data->id,data->value,data->ts);
        if(data->id == 0 && over ==1) break;
        if(data->id > 0 ) fputs(result,file);
        pthread_mutex_unlock(&lock);
    }
    free(data);
    fclose(file);
    pthread_exit(SBUFFER_SUCCESS);
}