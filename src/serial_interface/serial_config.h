/*
 * created at 2022-07-19 19:07.
 *
 * Company: HashedIn By Deloitte.
 * Copyright (C) 2022 HashedIn By Deloitte
 */

#include <termios.h>
#include <unistd.h>
#include <stdbool.h>

#ifndef SERIAL_CONFIG_H
#define SERIAL_CONFIG_H

#define DEBUG

struct uart_device_struct
{
	char *file_name;
	unsigned int baud_rate;

	int fd;
	struct termios *tty;
};

int uart_start(struct uart_device_struct *dev, bool canonic);
int uart_writen(struct uart_device_struct *dev, char *buf, size_t buf_len);
int uart_writes(struct uart_device_struct *dev, char *string);
int uart_reads(struct uart_device_struct *dev, char *buf, size_t buf_len);
void uart_stop(struct uart_device_struct *dev);

#endif