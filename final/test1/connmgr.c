#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <pthread.h>
#include "config.h"
#include "lib/tcpsock.h"
#include "connmgr.h"

#define PORT 5678
#define MAX_CONN 99

//extern conn_list_t * connList;

void* create_conn(void* client);

void* connect()
{
    tcpsock_t *server, *client;
    //int conn_counter = 0;
    //pthread_t threads[MAX_CONN];
    //&threads[0] = (pthread_t* )malloc(total_nodes*sizeof(pthread_t));
    printf("Test server is started\n");
    if (tcp_passive_open(&server, PORT) != TCP_NO_ERROR) exit(EXIT_FAILURE);
    do {
        printf("%d\n",conn_counter);
        if (tcp_wait_for_connection(server, &client) != TCP_NO_ERROR) exit(EXIT_FAILURE);
        printf("Incoming client connection\n");
        conn_counter++;
        total_counter = conn_counter;
        if(pthread_create(&threads[conn_counter-1], NULL, create_conn,client)!=0)
        {
            perror("can't create thread");
            conn_counter--;
            total_counter = conn_counter;
        } 
    } while (conn_counter < MAX_CONN);
    if (tcp_close(&server) != TCP_NO_ERROR) exit(EXIT_FAILURE);
    printf("Test server is shutting down\n");
    pthread_exit(SBUFFER_SUCCESS);
}

void* create_conn(void* node)
{
    sensor_data_t data;
    int bytes, result;
    tcpsock_t * client = (tcpsock_t*) node;
    do {
        // read sensor ID
        bytes = sizeof(data.id);
        result = tcp_receive(client, (void *) &data.id, &bytes);
        // read temperature
        bytes = sizeof(data.value);
        result = tcp_receive(client, (void *) &data.value, &bytes);
        // read timestamp
        bytes = sizeof(data.ts);
        result = tcp_receive(client, (void *) &data.ts, &bytes);
        if ((result == TCP_NO_ERROR) && bytes) {
            
            sbuffer_insert(buffer,&data);
        }
    } while (result == TCP_NO_ERROR);
    if (result == TCP_CONNECTION_CLOSED)
        printf("Peer has closed connection\n");
    else
        printf("Error occured on connection to peer\n");
    conn_counter--;
    tcp_close(&client);
    pthread_exit(SBUFFER_SUCCESS);
}