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
#include <stdbool.h>

#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include <linux/can.h>
#include <linux/can/raw.h>

#include "../../include/resource.h"

/* ODB2 PID contain a 17-character VIN */
#define MAX_LEN_VIN 24
#define VIN_LEN 17
#define WMI_LEN 3
#define MAX_LEN_SPEED_DATA 8
#define MAX_LEN_SUPPORTED_DATA 32
#define SUPPORTED_PID 0x00
#define SPEED_PID 0x0D
#define VIN_PID 0x02
#define CAN_REQUEST_ID 0x7DF
#define VIN_MODE 0x09
#define LIVE_DATA_MODE 0x01
#define CAN_EMPTY_DATA 0xAA

/* TOD: replace with real can name while using read can module */
#define CAN_FILE "vcan0"
#define CAN_REQUEST "CAN REQUEST"
#define CAN_RESPONSE "CAN RESPONSE"
#define VIN_DATA_FRAME 3
#define SPEED_DATA_FRAME 1
#define SUPPORTED_DATA_FRAME 1
#define CAN_FRAME_LENGTH 8
#define CAN_PID_LENGTH 32

#define DEBUG

struct can_data_struct
{
	uint8_t vin[MAX_LEN_VIN];
	uint8_t speed;
	uint32_t supported_pids[CAN_PID_LENGTH];
	char vehicle_type[WMI_STRING_LEN];
};

/* pthread to display all serial data */
void read_from_can(void *, pthread_t *, pthread_t *, pthread_t *);

char *get_manufacturer_detail(uint8_t *);
bool validate_vin(char *);

void get_request_frame(struct can_frame *, int, int);
void transmit_can_data(int, struct can_frame);
void receive_can_data(int, struct can_frame *);
int setup_can_socket(int *);
void close_socket(int *);
void log_can_data(struct can_frame, char *);
void vin_from_can_frame_data(struct can_frame *, char *);
void hex_to_binary(struct can_frame, uint8_t *);
void log_can_supported_data(uint8_t *);

#endif