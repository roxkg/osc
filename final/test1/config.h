/**
 * \author YUHAN SUN
 */

#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <stdint.h>
#include <time.h>

# define MAX_SIZE 100
# define READ_END 0
# define WRITE_END 1
# define BUS_SIZE 64

#ifndef MAX_CONN
#define MAX_CONN 3
#endif

extern int fd[2];

typedef uint16_t sensor_id_t;
typedef uint16_t room_id_t;
typedef double sensor_value_t;
typedef time_t sensor_ts_t;         // UTC timestamp as returned by time() - notice that the size of time_t is different on 32/64 bit machine

typedef struct {
    sensor_id_t id;
    sensor_value_t value;
    sensor_ts_t ts;
} sensor_data_t;

typedef struct {
    sensor_id_t sensor_id;
    room_id_t room_id;
    sensor_value_t running_avg;
    sensor_ts_t last_modified;
}element_t;

#endif /* _CONFIG_H_ */