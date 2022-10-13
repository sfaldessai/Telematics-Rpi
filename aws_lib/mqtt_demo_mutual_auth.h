/*
 * created at 2022-10-12 14:30.
 *
 * Company: HashedIn By Deloitte.
 * Copyright (C) 2022 HashedIn By Deloitte
 */

#ifndef MQTT_H_
#define MQTT_H_

#define DEBUG

#include <unistd.h>
extern char *send_data;
extern void *mqtt_send(void *arg);
struct aws_arg
{
	char *client_id;
	char *topic;
	char *aws_iot_endpoint;
};
#endif