
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
#include "utils/common_utils.h"
#include "bluetooth_can/bluetooth_can.h"

#define DEBUG

#define CLIENT_CONTROLLER "/dev/ttyACM0"
#define GPS_MODULE "/dev/ttyUSB0"

#define AWS_CLIENT_ID "testclient"
#define AWS_TOPIC "testclient/example/topic"

/* Update AWS IOT ENDPOINT */
#define AWS_IOT_ENDPOINT "aws.iot.endpoint"

/*BLE Device*/
#define BLE_DEVICE_NAME "OBDII"
#define BLE_DEVICE_SERIAL_PROFILE_UUID "00001101-0000-1000-8000-00805f9b34fb"


/* arg_struct struct to holds required thread arguments*/
struct arg_struct
{
	struct uart_device_struct uart_device;
	struct cloud_data_struct *cloud_data;
};

#endif
