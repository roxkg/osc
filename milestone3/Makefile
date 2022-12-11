try:
	gcc -g -Wall -Werror sbuffer.c main.c  \
	-o main
zip:
	zip milestone3.zip *.c *.h

file_creator:
	gcc -g -Wall -Werror -DDEBUG file_creator.c \
	-o file_creator

clean:
	rm sensor_data_out.txt

