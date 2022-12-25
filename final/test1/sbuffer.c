/**
 * \author YUHAN SUN
 */

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include<unistd.h>
#include <fcntl.h>
#include "sbuffer.h"

/**
 * basic node for the buffer, these nodes are linked together to create the buffer
 */
typedef struct sbuffer_node {
    struct sbuffer_node *next;  /**< a pointer to the next node*/
    sensor_data_t data;         /**< a structure containing the data */
} sbuffer_node_t;

/**
 * a structure to keep track of the buffer
 */
struct sbuffer {
    sbuffer_node_t *head;       /**< a pointer to the first node in the buffer */
    sbuffer_node_t *tail;       /**< a pointer to the last node in the buffer */
    pthread_mutex_t lock;
    pthread_cond_t write_signal;
    pthread_cond_t read_signal;
    short unsigned int read_first;
};

void stormgr_init(FILE* file){
    sensor_data_t* data = malloc(sizeof(sensor_data_t));
    memset(data,0,sizeof(sensor_data_t));
    char log[MAX_SIZE];
    memset(log,0,sizeof(log));
    char result[64];
    memset(result,0,sizeof(result));
    while(1)
    {
        int i = sbuffer_remove(buffer,data,1);
        if(i != SBUFFER_FAILURE ) {
            sprintf(result,"%hu %g %ld\n",data->id,data->value,data->ts);
            printf("readed id: %hu\n",data->id);
            if(data->id==0) {puts("stormgr break");break;}
            fputs(result,file);
            fflush(file);
            sprintf(log,"%ld Data insertion from sensor %d succeeded.",time(NULL),data->id);
            write(fd[WRITE_END], log, 100);
            if(i == SBUFFER_NO_DATA)    {pthread_cond_wait(&(buffer->write_signal),&insert_lock);puts("stormgr unlock");}
        }
        else {perror("sbuffer read failure");break;}
    }
    free(data);
    puts("stormgr_init exit");
}

int sbuffer_init(sbuffer_t **buffer) {
    *buffer = malloc(sizeof(sbuffer_t));
    if (*buffer == NULL) return SBUFFER_FAILURE;
    (*buffer)->head = NULL;
    (*buffer)->tail = NULL;
    pthread_mutex_init(&((*buffer)->lock),NULL);
    pthread_cond_init(&((*buffer)->write_signal),NULL);
    pthread_cond_init(&((*buffer)->read_signal),NULL);
    (*buffer)->read_first = 0;
    return SBUFFER_SUCCESS;
}

int sbuffer_free(sbuffer_t **buffer) {
    sbuffer_node_t *dummy;
    if ((buffer == NULL) || (*buffer == NULL)) {
        return SBUFFER_FAILURE;
    }
    while ((*buffer)->head) {
        dummy = (*buffer)->head;
        (*buffer)->head = (*buffer)->head->next;
        free(dummy);
    }
    pthread_cond_destroy(&((*buffer)->write_signal));
    pthread_cond_destroy(&((*buffer)->read_signal));
    free(*buffer);
    *buffer = NULL;
    puts("buffer free");
    return SBUFFER_SUCCESS;
}

int sbuffer_remove(sbuffer_t *buffer, sensor_data_t *data, short unsigned int remove) {
    sbuffer_node_t *dummy;
    if (buffer == NULL) return SBUFFER_FAILURE;
    pthread_mutex_lock(&(buffer->lock));
    if (buffer->head == NULL) {
        if(remove == 0) {puts("datamgr no value wait");pthread_cond_wait(&insert_signal,&(buffer->lock));}
        else {puts("stormgr no value wait");pthread_cond_wait(&(buffer->write_signal),&(buffer->lock));}

        if(remove == 0) puts("datamgr work");
        else puts("stormgr works");
    }
    if (buffer->head == buffer->tail){  // buffer has only one node
        /*while((remove == 1 && buffer->read_first == 0) || (remove == 0 && buffer->read_first == 1))
        {
            if(remove == 0) pthread_cond_wait(&(buffer->read_signal),&(buffer->lock));
            else pthread_cond_wait(&(buffer->write_signal),&(buffer->lock));
        }*/
        *data = buffer->head->data;
        if(remove == 1)
        {
            dummy = buffer->head;
            buffer->head = buffer->tail = NULL;
            free(dummy);
            buffer->read_first = 0;
            pthread_cond_signal(&(buffer->read_signal));
        }
        else{
            buffer->read_first = 1;
            pthread_cond_signal(&(buffer->write_signal));
        }
        pthread_mutex_unlock(&(buffer->lock));
        return SBUFFER_NO_DATA;
    }
    else{
        while((remove == 1 && buffer->read_first == 0) || (remove == 0 && buffer->read_first == 1))
        {
            if(remove == 0) puts("datamgr stop");
            else puts("stormgr stop");
            if(remove == 0) pthread_cond_wait(&(buffer->read_signal),&(buffer->lock));
            else pthread_cond_wait(&(buffer->write_signal),&(buffer->lock));
            if(remove == 0) puts("datamgr continue");
            else puts("stormgr continue");
        }
        *data = buffer->head->data;
        if(remove == 1)
        {
            dummy = buffer->head;
            buffer->head = buffer->head->next;
            free(dummy);
            puts("here");
            buffer->read_first = 0;
            pthread_cond_signal(&(buffer->read_signal));
            puts("tell datamgr continue");
        }
        else{
            buffer->read_first = 1;
            pthread_cond_signal(&(buffer->write_signal));
            puts("tell stormgr continue");
        }
    }
    pthread_mutex_unlock(&(buffer->lock));
    return SBUFFER_SUCCESS;
}

int sbuffer_insert(sbuffer_t *buffer, sensor_data_t *data) {
    sbuffer_node_t *dummy;
    if (buffer == NULL) return SBUFFER_FAILURE;
    dummy = malloc(sizeof(sbuffer_node_t));
    if (dummy == NULL) return SBUFFER_FAILURE;
    dummy->data = *data;
    dummy->next = NULL;
    pthread_mutex_lock(&(buffer->lock));
    if (buffer->tail == NULL) // buffer empty (buffer->head should also be NULL
    {
        buffer->head = buffer->tail = dummy;
        buffer->read_first = 0;
        //pthread_cond_broadcast(&(buffer->broadcast_signal));
        pthread_cond_signal(&insert_signal);
        puts("\ninsert");
        //pthread_mutex_unlock(&(buffer->lock));
    } else // buffer not empty
    {
        buffer->tail->next = dummy;
        buffer->tail = buffer->tail->next;
    }
    //printf("insert: %d-%g-%ld\n",data->id,data->value,data->ts);
    pthread_mutex_unlock(&(buffer->lock));
    return SBUFFER_SUCCESS;
}
