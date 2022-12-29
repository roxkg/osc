#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include "config.h"
#include "lib/tcpsock.h"
#include "connmgr.h"

struct timeval timeout;      

void* create_conn(void* client);

void* get_connect()
{
    tcpsock_t *server, *client;
    timeout.tv_sec = TIMEOUT;
    timeout.tv_usec = 0; 
    printf("Test server is started\n");
    if ( tcp_passive_open(&server, PORT)!= TCP_NO_ERROR) {exit(EXIT_FAILURE);}
    do {
        printf("%d\n",conn_counter);
        if (tcp_wait_for_connection(server, &client) != TCP_NO_ERROR) {exit(EXIT_FAILURE);}   
        printf("Incoming client connection\n");
        //conn_counter++;
        //total_counter = conn_counter;
        if(pthread_create(&threads[conn_counter-1], NULL, create_conn,client)!=0)
        {
            perror("can't create thread");
            //conn_counter--;
            //total_counter = conn_counter;
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
    int sd;
    tcp_get_sd(client,&sd);
    conn_counter++;
    total_counter = conn_counter;
    if(setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout))<0)
        printf("set socket error\n");
    else puts("set socket success");
    do {
        // read sensor ID
        bytes = sizeof(data.id);
        result = tcp_receive(client, (void *) &data.id, &bytes);
        if(result == TCP_CONNECTION_CLOSED)  break;
        // read temperature
        bytes = sizeof(data.value);
        result = tcp_receive(client, (void *) &data.value, &bytes);
        if(result == TCP_CONNECTION_CLOSED)  break;
        // read timestamp
        bytes = sizeof(data.ts);
        result = tcp_receive(client, (void *) &data.ts, &bytes);
        if(result == TCP_CONNECTION_CLOSED)  break;
        if ((result == TCP_NO_ERROR) && bytes) {
            printf("\nsensor id = %" PRIu16 " - temperature = %g - timestamp = %ld\n", data.id, data.value,
                       (long int) data.ts);
            if(first == 0 )
            {
                char log[100];
                memset(log,0,sizeof(log));
                sprintf(log,"%ld Sensor node %d has opened a new connection.",time(NULL),data.id);
                pthread_mutex_lock(&pip_lock);
                write(fd[WRITE_END], log, 100);
                pthread_mutex_unlock(&pip_lock);
                first = 1;
            }
            sbuffer_insert(buffer,&data);
        }
    } while (result == TCP_NO_ERROR);
    if (result == TCP_CONNECTION_CLOSED || result == TCP_SOCKOP_ERROR)
    {
        //printf("Peer has closed connection\n");
        char log[100];
        memset(log,0,sizeof(log));
        sprintf(log,"%ld Sensor node %d has closed the connection.",time(NULL),data.id);
        pthread_mutex_lock(&pip_lock);
        write(fd[WRITE_END], log, 100);
        pthread_mutex_unlock(&pip_lock);
        conn_counter--;
    }
    else
        printf("Error occured on connection to peer\n");
    tcp_close(&client);
    pthread_exit(SBUFFER_SUCCESS);
}