/*
 * created at 2022-07-29 14:00.
 *
 * Company: HashedIn By Deloitte.
 * Copyright (C) 2022 HashedIn By Deloitte
 */

#ifndef CLOUD_SERVER_H_
#define CLOUD_SERVER_H_

#include "../gps_module/gps_module.h"
#include "../client_controller/client_controller.h"
#include "../can_bus/can_bus.h"

#define DEBUG

#define MAX_LEN_MAC_ADDRESS 12

/* cloud_data_struct struct to holds STM32, GPS, and CAN bus data*/
struct cloud_data_struct
{
	uint8_t mac_address[MAX_LEN_MAC_ADDRESS];
	struct gps_data_struct gps_data;
	struct client_controller_data_struct client_controller_data;
	struct can_data_struct can_data;
	uint64_t idle_time_secs;
};

void *write_to_cloud(void *); /* pthread to display all serial data */

void initialize_cloud_data(struct cloud_data_struct *);

void calculate_idle_time(struct cloud_data_struct *);

void display_cloud_struct_data_logger(struct cloud_data_struct *);

void gps_error_codes(struct cloud_data_struct *, int);

#endif