/*
 * created at 2022-07-29 13:37.
 *
 * Company: HashedIn By Deloitte.
 * Copyright (C) 2022 HashedIn By Deloitte
 */

#ifndef CLIENT_CONTROLLER_H_
#define CLIENT_CONTROLLER_H_

#define DEBUG

struct stm32_data_struct
{
	char *sensor_data;
};

void *read_from_stm32(void *); /* pthread to handle stm32 read */

#endif