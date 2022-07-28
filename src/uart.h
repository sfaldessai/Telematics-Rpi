/*
 * created at 2022-07-19 19:07.
 *
 * Company: HashedIn By Deloitte.
 * Copyright (C) 2022 HashedIn By Deloitte
 */

#include <termios.h>
#include <unistd.h>
#include <stdbool.h>

#ifndef UART_H_
#define UART_H_

#define DEBUG

struct uart_device_struct
{
	char *file_name;
	int baud_rate;

	int fd;
	struct termios *tty;
};

struct stm32_data_struct
{
	char *sensor_data;
};

struct gps_data_struct
{
	float latitude;
	float longitude;
	char *gps_time;
	char lat_cardinal_sign;
	char long_cardinal_sign;
	float pdop;
	float vdop;
	float hdop;
};

struct cloud_data_struct
{
	struct gps_data_struct gps_data;
	struct stm32_data_struct stm32_data;
};

struct arg_struct
{
	struct uart_device_struct uart_device;
	struct cloud_data_struct *cloud_data;
};

int uart_start(struct uart_device_struct *dev, bool canonic);
int uart_writen(struct uart_device_struct *dev, char *buf, size_t buf_len);
int uart_writes(struct uart_device_struct *dev, char *string);
int uart_reads(struct uart_device_struct *dev, char *buf, size_t buf_len);
void uart_stop(struct uart_device_struct *dev);

void *read_from_stm32(void *); /* pthread to handle stm32 read */
void *read_from_gps(void *);   /* pthread to handle gps read */

void *write_to_cloud(void *); /* pthread to display all serial data */

#endif