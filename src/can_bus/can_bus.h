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

/* ODB2 PID contain a 17-character VIN */
#define MAX_LEN_VIN 24
#define VIN_LEN 17
#define WMI_LEN 3
#define MAX_LEN_SPEED_DATA 8
#define MAX_LEN_SUPPORTED_DATA 32
#define SUPPORTED_PID 0x00
#define SPEED_PID 0x0D


#define DEBUG

struct can_data_struct
{
	uint8_t vin[MAX_LEN_VIN];
	uint8_t speed;
	uint32_t supported_pids;
};

/* pthread to display all serial data */
void read_from_can(void *, pthread_t *, pthread_t *, pthread_t *);

char *get_manufaturer_detail(uint8_t *);
bool validate_vin(char *);

void get_request_frame(struct can_frame *, int);
void transmit_can_data(int, struct can_frame);
void receive_can_data(int, struct can_frame *);
void setup_can_socket(int *);
void close_socket(int *);

#endif