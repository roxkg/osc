/**
 * \author Yuhan Sun
 */
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include "config.h"
#include "datamgr.h"
#include "sbuffer.h"

#define BUS_SIZE 1024

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
    }
    short unsigned int start[dpl_size(list)][RUN_AVG_LENGTH];
    sensor_data_t* data = malloc(sizeof(sensor_data_t));
    while(over!=1)
    {
        pthread_mutex_lock(&lock);
        sbuffer_remove(buffer,data,false);
        char result[BUS_SIZE];
        sprintf(result,"%hu %lf %ld\n",data->id,data->value,data->ts);
        //if(data->id == 0 && over ==1) break;
        //if(data->id > 0 ) fputs(result,file);
        if(datamgr_get_node_by_sensor(data->id) == NULL)
        {
            //TODO:加id
            //char log[100] = "Received sensor data with invalid sensor node ID\n";
            //write_log(log);
        }
        else{
            element = *datamgr_get_node_by_sensor(data->id);
            get_zeros(start,datamgr_get_index_by_sensor(data->id),data->value);
            //TODO: send error Running average exceeds a minimum or maximum temperature value.
            sensor_value_t total = 0;
            for(int i = 0; i < RUN_AVG_LENGTH ; i++)
            {
                total += start[datamgr_get_index_by_sensor(data->id)][i];
            }
            element.running_avg = total/RUN_AVG_LENGTH;
            element.last_modified = time(NULL);
            buffer->read_first = true;
            printf("sensor id = %hu - Avgtemperature = %g - timestamp = %ld\n", element.sensor_id, element.running_avg,
                    element.last_modified);
            //TODO: inform storage manager to remove data from buffer. 
        }
        pthread_mutex_unlock(&lock);
    }
    free(data);
}

void datamgr_free()
{
    dpl_free(&list,true);
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
    for(int i = 0 ; i < dpl_size(list);i++){
        element = (element_t *)dpl_get_element_at_index(list,i);
        sensor_id_t id = element->sensor_id;
        if(id == sensor_id) {
            find = 1;
            break;
        }
    }
    if(find == 1)   return element;
    else    return NULL;
}

int datamgr_get_index_by_sensor(sensor_id_t sensor_id)
{
    element_t* element = datamgr_get_node_by_sensor(sensor_id);
    return dpl_get_index_of_element(list,element);
}

void * element_copy(void * element) {
    element_t* copy = malloc(sizeof (element_t));
    //copy -> sensor_id = ((element_t*)element)->sensor_id;
    //copy -> room_id = ((element_t*)element)->room_id;
    //copy -> running_avg = ((element_t*)element)->running_avg;
    //copy -> last_modified = ((element_t*)element)->last_modified;
    *copy = *((element_t *) element);
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
    //return ((((element_t*)x)->last_modified < ((element_t*)y)->last_modified) ? 
    //-1 : (((element_t*)x)->last_modified == ((element_t*)y)->last_modified) ? 0 : 1);
    return -1;
}

void get_zeros(int arr[][RUN_AVG_LENGTH],int i,sensor_value_t value)
{
    int counter = 0;
    for(int j = 0; j < RUN_AVG_LENGTH; j++){
        if(arr[i][j]!=0)  counter++;  
    }
    if(counter == RUN_AVG_LENGTH){
        arr[i][0] = 0;
        for(int m = 1; m < RUN_AVG_LENGTH; m++)
        {
            arr[i][m - 1] = arr[i][m];
        }
        arr[i][RUN_AVG_LENGTH-1] = value;
    }
    else{
        arr[i][counter] = value;
    }
}

