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
#define MAX_LEN_SUPPORTED_DATA 40
#define SUPPORTED_PID 0x00
#define SPEED_PID 0x0D
#define TEMPERATURE_PID 0x05
#define RPM_PID 0x0C
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
#define TEMPERATURE_DATA_FRAME 1
#define RPM_DATA_FRAME 1
#define SUPPORTED_DATA_FRAME 1
#define CAN_FRAME_LENGTH 8
#define CAN_PID_LENGTH 32

/* CAN ERROR CODE */
#define CAN_SUCESS 0
#define CAN_SOCKET_ERROR 801
#define CAN_READ_ERROR 802
#define INVALID_VIN_ERROR 803
#define INVALID_WMIs_ERROR 804
#define SUPPORTED_READ_ERROR 805
#define CAN_RPM_READ_ERROR 806
#define CAN_SPEED_READ_ERROR 807
#define CAN_DISCONNECTED 808
#define CAN_WRITE_ERROR 809
#define CAN_SOCKET_CLOSED 810
#define CAN_PID_NOT_SUPPORTED 811

#define PID_SUPPORTED 1

#define RPM_PID_POSITION 11
#define SPEED_PID_POSITION 12
#define TEMPERATURE_PID_POSITION 4

#define DEBUG

struct can_data_struct
{
	uint8_t vin[MAX_LEN_VIN];
	uint16_t speed;
	float rpm;
	uint8_t supported_pids[CAN_PID_LENGTH];
	int temperature;
	char vehicle_type[WMI_STRING_LEN];
	int mode; /* mode=1 for infinite loop - build mode  || mode=0 for test mode used to test infinte loops and other cases */
};

/* pthread to display all serial data */
void read_from_can(void *, pthread_t *, pthread_t *, pthread_t *, pthread_t *, pthread_t *);

char *get_manufacturer_detail(uint8_t *);
bool validate_vin(char *);

void get_request_frame(struct can_frame *, int, int);
int transmit_can_data(int, struct can_frame);
int receive_can_data(int, struct can_frame *);
int setup_can_socket(int *);
void close_socket(int *);
void log_can_data(struct can_frame, char *);
void vin_from_can_frame_data(struct can_frame *, char *);
void hex_to_binary(struct can_frame, uint8_t *);
void log_can_supported_data(uint8_t *);

#endif