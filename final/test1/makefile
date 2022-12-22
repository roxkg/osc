test1: sbuffer.c main.c connmgr.c lib/tcpsock.c lib/dplist.c datamgr.c sensor_db.c
	gcc -g -Wall -Werror -DSET_MAX_TEMP=23 -DSET_MIN_TEMP=16 \
	sbuffer.c main.c connmgr.c lib/tcpsock.c lib/dplist.c datamgr.c sensor_db.c -o test_server
	
sensor_node: sensor_node.c lib/tcpsock.c
	gcc sensor_node.c lib/tcpsock.c -o sensor_node

runserver:
	./test_server 5678

runclient1: sensor_node
	./sensor_node 15 1 127.0.0.1 5678

runclient2: sensor_node
	./sensor_node 21 2 127.0.0.1 5678

runclient3: sensor_node
	./sensor_node 37 4 127.0.0.1 5678

runclient4: sensor_node
	./sensor_node 4 6 127.0.0.1 5678
