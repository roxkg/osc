#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include "sbuffer.h"
#define BUS_SIZE 64
void* write_to_buffer();
void* read_from_buffer();
sbuffer_t *buffer;
short unsigned int over = 0;
pthread_mutex_t lock;

int main(void)
{
    pthread_t threads[3];
    
    pthread_mutex_init(&lock,NULL);
    sbuffer_init(&buffer);
    if(pthread_create(&(threads[0]), NULL, write_to_buffer,NULL)!=0) 
                printf("\ncan't create thread"); 

    if(pthread_create(&(threads[1]), NULL, read_from_buffer, NULL)!=0)
                printf("\ncan't create thread");           

    if(pthread_create(&(threads[2]), NULL, read_from_buffer, NULL)!=0)
                printf("\ncan't create thread");  
   
    for(int i = 0 ; i < 3;i++)
    {
        pthread_join(threads[i],NULL);
    }
    sbuffer_free(&buffer);

}

void* write_to_buffer()
{
    
    FILE* file = fopen("sensor_data","rb");
    
    //char load[BUS_SIZE];
    sensor_data_t *data = malloc(sizeof(sensor_data_t));
    /*while(fgets(load,BUS_SIZE,file) != NULL)
    //for(int i = 0; i < 10; i++)
    {
        //fgets(load,BUS_SIZE,file);
        sscanf(load, "%hu %lf %ld", &(data->id),&(data->value),&(data->ts));
        sbuffer_insert(buffer,data);
        //puts(load);
        //puts("write\n");
    }*/
    while(1){
        if(!fread(&(data->id), sizeof(sensor_id_t), 1, file))   break;
        fread(&(data->value), sizeof(sensor_value_t), 1, file);
        fread(&(data->ts), sizeof(sensor_ts_t), 1, file);
        sbuffer_insert(buffer, data);
        fflush(stdout);
    }
    over = 1;
    free(data);
    fclose(file);
    pthread_exit(SBUFFER_SUCCESS);
}

void* read_from_buffer()
{
    FILE* file = fopen("sensor_data_out.txt","a");
    
    sensor_data_t* data = malloc(sizeof(sensor_data_t));
    while(!(sbuffer_remove(buffer,data) != SBUFFER_SUCCESS && over == 1))
    //for(int i = 0; i<10;i++)
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