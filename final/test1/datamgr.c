/**
 * \author Yuhan Sun
 */
#include <stdlib.h>
#include <stdio.h>
#include<unistd.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include "config.h"
#include "datamgr.h"
#include "sbuffer.h"

//#define BUS_SIZE 1024

static void * element_copy(void * element);
static void element_free(void ** element);
static int element_compare(void * x, void * y);

static dplist_t* list = NULL;
void datamgr_parse_sensor_files(FILE *fp_sensor_map)
//void datamgr_parse_sensor_files(FILE *fp_sensor_map, FILE *fp_sensor_data)
{
    list = dpl_create(element_copy,element_free,element_compare);
    char store[BUS_SIZE];
    element_t element;
    while(fgets(store,BUS_SIZE,fp_sensor_map) != NULL)
    {
        sscanf(store, "%hu%hu", &(element.room_id), &(element.sensor_id));
        dpl_insert_at_index(list,&element,9,true);
        printf("room id %hu -- sensor id %hu\n",element.room_id,element.sensor_id);
    }
    sensor_value_t start[dpl_size(list)][RUN_AVG_LENGTH];
    memset(start,0,sizeof(start));
    sensor_data_t* data = malloc(sizeof(sensor_data_t));
    while(1)
    {
        //pthread_mutex_lock(&insert_lock);
        int i = sbuffer_remove(buffer,data,0);
        //if(i == SBUFFER_NO_DATA)   {puts("datamgr break");break;}
        if(i != SBUFFER_FAILURE){
        char result[BUS_SIZE];
        memset(result,0,sizeof(result));
        sprintf(result,"%hu %g %ld\n",data->id,data->value,data->ts);
        if(data->id==0) {puts("datamgr break");break;}
        char log[100];
        memset(log,0,sizeof(log));
        if(datamgr_get_node_by_sensor(data->id) == NULL)
        {
            sprintf(log,"%ld Received sensor data with invalid sensor node ID %d",time(NULL),data->id);
            write(fd[WRITE_END], log, 100);
        }
        else{
            element = *datamgr_get_node_by_sensor(data->id);
            element.running_avg = get_zeros(start,datamgr_get_index_by_sensor(data->id),data->value);
            element.last_modified = time(NULL);
            //printf("sensor id = %hu - Avgtemperature = %lf - timestamp = %ld\n\n", element.sensor_id, element.running_avg,
            //        element.last_modified);
            if(element.running_avg < SET_MIN_TEMP)
            sprintf(log,"%ld Sensor node %hu reports it's too cold (avg temp = %lf)",time(NULL),element.sensor_id,element.running_avg);
            if(element.running_avg > SET_MAX_TEMP)
            sprintf(log,"%ld Sensor node %hu reports it's too hot (avg temp = %lf)",time(NULL),element.sensor_id,element.running_avg);
            write(fd[WRITE_END], log, 100); 
        }
        if(i == SBUFFER_NO_DATA)    {pthread_cond_wait(&insert_signal,&insert_lock);
        pthread_mutex_unlock(&insert_lock);}
        }//pthread_mutex_unlock(&lock);
    }
    free(data);
}

void datamgr_free()
{
    dpl_free(&list,true);
    puts("list free");
}

room_id_t datamgr_get_room_id(sensor_id_t sensor_id)
{
    short unsigned int find = 0;
    element_t* x;
    for(int i = 0; i < dpl_size(list);i++)
    {
        x = (element_t*)dpl_get_element_at_index(list,i);
        if(x->sensor_id == sensor_id)
        {
            find = 1;
            break;
        }
    }
    if(find == 1)   return x->room_id;
    else return 0;
}

sensor_value_t datamgr_get_avg(sensor_id_t sensor_id)
{
    short unsigned int find = 0;
    element_t* x;
    for(int i = 0; i < dpl_size(list);i++)
    {
        x = (element_t*)dpl_get_element_at_index(list,i);
        if(x->sensor_id == sensor_id)
        {
            find = 1;
            break;
        }
    }
    if(find == 1)   return x->running_avg;
    else return -1;
}

time_t datamgr_get_last_modified(sensor_id_t sensor_id)
{
    short unsigned int find = 0;
    element_t* x;
    for(int i = 0; i < dpl_size(list);i++)
    {
        x = (element_t*)dpl_get_element_at_index(list,i);
        if(x->sensor_id == sensor_id)
        {
            find = 1;
            break;
        }
    }
    if(find == 1)   return x->last_modified;
    else return 0;
}

int datamgr_get_total_sensors()
{
    //assert(list!=NULL);
    return dpl_size(list);
}

element_t* datamgr_get_node_by_sensor(sensor_id_t sensor_id)
{
    short unsigned int find = 0;
    element_t* element;
    //if(sensor_id!=0)
    //printf("start to find sensor %hu\n",sensor_id);
    for(int i = 0 ; i < dpl_size(list);i++){
        element = (element_t *)dpl_get_element_at_index(list,i);
        sensor_id_t id = element->sensor_id;
        //if(sensor_id!= 0)
        //printf("current sensor id %hu--current index %d\n",id,i);
        if(id == sensor_id) {
            find = 1;
            break;
        }
    }
    if(find == 1)   {return element;}
    else    return NULL;
}

int datamgr_get_index_by_sensor(sensor_id_t sensor_id)
{
    element_t* element = datamgr_get_node_by_sensor(sensor_id);
    //if(sensor_id == 15)
    //printf("the sensor id of the element is %hu\n",element->sensor_id);
    return dpl_get_index_of_element(list,element);
}

void * element_copy(void * element) {
    element_t* copy = malloc(sizeof (element_t));
    copy -> sensor_id = ((element_t*)element)->sensor_id;
    copy -> room_id = ((element_t*)element)->room_id;
    copy -> running_avg = ((element_t*)element)->running_avg;
    copy -> last_modified = ((element_t*)element)->last_modified;
    //*copy = *((element_t *) element);
    return (void *) copy;
}

void element_free(void ** element) {
    //free((((element_t*)*element))->sensor_id);
    //free((((element_t*)*element))->room_id);
    //free((((element_t*)*element))->running_avg);
    //free((((element_t*)*element))->last_modified);
    free((element_t*)*element);
    *element = NULL; 
}

static int element_compare(void * x, void * y) {
    return ((((element_t*)x)->sensor_id < ((element_t*)y)->sensor_id) ? 
    -1 : (((element_t*)x)->sensor_id == ((element_t*)y)->sensor_id) ? 0 : 1);
    //return -1;
}

sensor_value_t get_zeros(sensor_value_t arr[][RUN_AVG_LENGTH],int i,sensor_value_t value)
{
    int counter = 0;
    for(int j = 0; j < RUN_AVG_LENGTH; j++){
        if(arr[i][j]==0)  {break;} 
        counter++;
    }
    if(counter == RUN_AVG_LENGTH){
        arr[i][0] = 0;
        for(int m = 0; m < RUN_AVG_LENGTH - 1; m++)
        {
            arr[i][m] = arr[i][m+1];
        }
        arr[i][RUN_AVG_LENGTH - 1] = value;
    }
    else{
        arr[i][counter] = value;
        counter++;
    }
    //for(int j = 0; j < RUN_AVG_LENGTH;j++)
    //{
    //    printf("arr%d--%G\n",j,arr[i][j]); 
    //}
    sensor_value_t total = 0;
    //counter = (counter == RUN_AVG_LENGTH) ? counter : counter+1;
    for(int x = 0; x < counter ; x++)
    {
        total += arr[i][x];
    }
    if(counter == RUN_AVG_LENGTH)
        return total/RUN_AVG_LENGTH;
    else    
        return total/counter;
}

