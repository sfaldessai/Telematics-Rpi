/*
 * created at 2022-07-29 14:30.
 *
 * Company: HashedIn By Deloitte.
 * Copyright (C) 2022 HashedIn By Deloitte
 */
#include "../serial_interface/serial_config.h"
#include "../main.h"

#ifndef CAN_BUS_H_
#define CAN_BUS_H_

/* ODB2 PID contain a 17-character VIN */
#define MAX_LEN_VIN 17

#define DEBUG

void *read_from_can(void *);	 /* pthread to display all serial data */

struct can_data_Struct
{
	uint8_t vin[MAX_LEN_VIN];
	uint8_t speed;
	uint32_t supported_pids;
};

#endif