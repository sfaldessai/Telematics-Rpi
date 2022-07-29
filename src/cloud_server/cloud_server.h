/*
 * created at 2022-07-29 14:00.
 *
 * Company: HashedIn By Deloitte.
 * Copyright (C) 2022 HashedIn By Deloitte
 */

#include "../gps_module/gps_module.h"
#include "../client_controller/client_controller.h"

#ifndef CLOUD_WRITE_H_
#define CLOUD_WRITE_H_

#define DEBUG

struct cloud_data_struct
{
	struct gps_data_struct gps_data;
	struct stm32_data_struct stm32_data;
};

void *write_to_cloud(void *); /* pthread to display all serial data */

#endif