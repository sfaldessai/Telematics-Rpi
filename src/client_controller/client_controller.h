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

/* STM32 ERROR CODE */
#define FAILED_TO_OPEN_STM32_DEVICE 1001
#define STM32_CHECKSUM_ERROR 1002
#define MOTION_READ_ERROR 1003
#define PTO_READ_ERROR 1004
#define VOLTAGE_READ_ERROR 1005
#define STM32_DEVICE_DISCONNECTED 1006
#define MEMS_ERROR 1007

/* client_controller_data_struct struct to hold STM32 data*/
struct client_controller_data_struct
{
	uint8_t motion;
	uint8_t pto;
	float voltage;
	int acc_x;
	int acc_y;
	int acc_z;
};

void *read_from_client_controller(void *); /* pthread to handle client_controller read */
void get_client_controller_data(char *, struct client_controller_data_struct *);

#endif