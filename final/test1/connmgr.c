#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <pthread.h>
#include<fcntl.h>
#include<unistd.h>
#include "config.h"
#include "lib/tcpsock.h"
#include "connmgr.h"

#define PORT 5678
#define MAX_CONN 2

//extern conn_list_t * connList;

void* create_conn(void* client);

void* connect()
{
    tcpsock_t *server, *client;
    printf("Test server is started\n");
    int i = tcp_passive_open(&server, PORT);
    if ( i!= TCP_NO_ERROR) {printf("error codea: %d\n",i);exit(EXIT_FAILURE);}
    do {
        printf("%d\n",conn_counter);
        int j = tcp_wait_for_connection(server, &client);
        if (j != TCP_NO_ERROR) {printf("error codeb: %d\n",j);exit(EXIT_FAILURE);}
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
    for(int i = 0; i < MAX_CONN;i++)
    {
        pthread_join(threads[i],NULL);
    }
    sensor_data_t data;
    data.id = 0;
    sbuffer_insert(buffer,&data);
    pthread_exit(SBUFFER_SUCCESS);
}

void* create_conn(void* node)
{
    sensor_data_t data;
    int bytes, result;
    short unsigned int first= 0; 
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
            //printf("sensor id = %" PRIu16 " - temperature = %g - timestamp = %ld\n", data.id, data.value,
            //           (long int) data.ts);
            //fflush(stdout);
            if(first == 0 )
            {
                char log[100];
                sprintf(log,"%ld Sensor node %d has opened a new connection.",time(NULL),data.id);
                write(fd[WRITE_END], log, 100);
                first = 1;
            }
            sbuffer_insert(buffer,&data);
        }
    } while (result == TCP_NO_ERROR);
    if (result == TCP_CONNECTION_CLOSED)
    {
        //printf("Peer has closed connection\n");
        char log[100];
        sprintf(log,"%ld Sensor node %d has closed the connection.",time(NULL),data.id);
        write(fd[WRITE_END], log, 100);
    }
    else
        printf("Error occured on connection to peer\n");
    conn_counter--;
    tcp_close(&client);
    pthread_exit(SBUFFER_SUCCESS);
}