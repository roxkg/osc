test1: sbuffer.c test1.c connmgr.c lib/tcpsock.c lib/dplist.c datamgr.c
	gcc -g -Wall -Werror sbuffer.c test1.c connmgr.c lib/tcpsock.c -o test_server
	
sensor_node: sensor_node.c lib/tcpsock.c
	gcc sensor_node.c lib/tcpsock.c -o sensor_node

runserver: test_server
	./test_server

runclient1: sensor_node
	./sensor_node 1 2 127.0.0.1 5678

runclient2: sensor_node
	./sensor_node 2 5 127.0.0.1 5678

runclient3: sensor_node
	./sensor_node 3 4 127.0.0.1 5678

runclient4: sensor_node
	./sensor_node 4 6 127.0.0.1 5678
