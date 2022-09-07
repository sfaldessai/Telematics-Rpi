/*
 * created at 2022-07-29 13:48.
 *
 * Company: HashedIn By Deloitte.
 * Copyright (C) 2022 HashedIn By Deloitte
 */

#ifndef MAIN_H_
#define MAIN_H_

#include <stdbool.h>
#include "serial_interface/serial_config.h"
#include "cloud_server/cloud_server.h"
#include "can_bus/can_bus.h"
#include "global/global.h"

#define DEBUG

/* arg_struct struct to holds required thread arguments*/
struct arg_struct
{
	struct uart_device_struct uart_device;
	struct cloud_data_struct *cloud_data;
};

#endif