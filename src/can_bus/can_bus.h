/*
 * created at 2022-07-29 14:30.
 *
 * Company: HashedIn By Deloitte.
 * Copyright (C) 2022 HashedIn By Deloitte
 */

#ifndef CAN_BUS_H_
#define CAN_BUS_H_

#define DEBUG
void *read_canAble(void *);
void *read_from_can(void *); /* pthread to display all serial data */
void *read_canRx_1000ms(void *); /* to read data every 1 second */
void *read_canRx_30ms(void *); /* to read data every 30ms */

struct can_data_Struct
{
	float data;
	unsigned int* pid;
};

#endif