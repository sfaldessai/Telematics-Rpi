#ifndef MQTT_H_
#define MQTT_H_

#define DEBUG

#include <unistd.h>

#define SIZE 4000
#define RECORDS 1024

extern char *send_data;
extern void *mqtt_send(void *arg);

extern char telematic_json_array[RECORDS][SIZE];

extern int insert_queue(char *data);
extern int delete_queue();

struct aws_arg
{
	char *client_id;
	char *topic;
    	char *aws_iot_endpoint;
};
#endif
