/*
 * created at 2022-07-29 14:30.
 *
 * Company: HashedIn By Deloitte.
 * Copyright (C) 2022 HashedIn By Deloitte
 */

#ifndef CAN_BUS_H_
#define CAN_BUS_H_

#include <stdint.h>
#include <pthread.h>

/* ODB2 PID contain a 17-character VIN */
#define MAX_LEN_VIN 24
#define VIN_LEN 17
#define WMI_LEN 3
#define MAX_LEN_SPEED_DATA 8
#define MAX_LEN_SUPPORTED_DATA 32 

#define DEBUG

struct can_data_struct
{
	uint8_t vin[MAX_LEN_VIN];
	uint8_t speed;
	uint32_t supported_pids;
};

/* pthread to display all serial data */
void read_from_can(void *, pthread_t *, pthread_t *, pthread_t *);

#endif