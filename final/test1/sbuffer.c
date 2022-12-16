/**
 * \author YUHAN SUN
 */

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
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
    pthread_cond_t cond_signal;
    bool read_first;
};

int sbuffer_init(sbuffer_t **buffer) {
    *buffer = malloc(sizeof(sbuffer_t));
    if (*buffer == NULL) return SBUFFER_FAILURE;
    (*buffer)->head = NULL;
    (*buffer)->tail = NULL;
    pthread_mutex_init(&((*buffer)->lock),NULL);
    pthread_cond_init(&((*buffer)->cond_signal),NULL);
    (*buffer)->read_first = false;
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
    pthread_cond_destroy(&((*buffer)->cond_signal));
    free(*buffer);
    *buffer = NULL;
    return SBUFFER_SUCCESS;
}

int sbuffer_remove(sbuffer_t *buffer, sensor_data_t *data, bool remove) {
    sbuffer_node_t *dummy;
    if (buffer == NULL) return SBUFFER_FAILURE;
    if (buffer->head == NULL) return SBUFFER_NO_DATA;
    pthread_mutex_lock(&(buffer->lock));
    *data = buffer->head->data;
    dummy = buffer->head;
    while(buffer->head == NULL) {pthread_cond_wait(&(buffer->cond_signal),&(buffer->lock));}
    if (buffer->head == buffer->tail) // buffer has only one node
    {
        if(remove)
        {
            buffer->head = buffer->tail = NULL;
        }
        pthread_cond_wait(&(buffer->cond_signal),&(buffer->lock));
    } else  // buffer has many nodes empty
    {
        if(remove)
        {
            buffer->head = buffer->head->next;
        }
    }
    //int id = data->id;
    free(dummy);
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
        pthread_cond_broadcast(&(buffer->cond_signal));
        pthread_mutex_unlock(&(buffer->lock));
    } else // buffer not empty
    {
        buffer->tail->next = dummy;
        buffer->tail = buffer->tail->next;
    }
    pthread_mutex_unlock(&(buffer->lock));
    return SBUFFER_SUCCESS;
}
