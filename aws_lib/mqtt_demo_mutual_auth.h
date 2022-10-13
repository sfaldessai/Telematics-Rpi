#ifndef MQTT_H_
#define MQTT_H_
#define DEBUG
/* POSIX includes. */
#include <unistd.h>
extern char *send_data;
extern void *mqtt_send(void *arg);
struct aws_arg
{
	char *client_id;
	char *topic;
};
#endif