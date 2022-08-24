/*
 * created at 2022-07-29 13:37.
 *
 * Company: HashedIn By Deloitte.
 * Copyright (C) 2022 HashedIn By Deloitte
 */

#ifndef CLIENT_CONTROLLER_H_
#define CLIENT_CONTROLLER_H_

#include <stdint.h>

#define DEBUG

#include <stdint.h>

/* client_controller_data_struct struct to hold STM32 data*/
struct client_controller_data_struct
{
	uint8_t motion;
	uint8_t pto;
	float voltage;
};

void *read_from_client_controller(void *); /* pthread to handle client_controller read */
void get_client_controller_data(char *, struct client_controller_data_struct *);

#endif