/*
 * created at 2022-07-19 19:39.
 *
 * Company HashedIn By Deloitte
 * Copyright (C) 2022 HashedIn By Deloitte
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <limits.h>

#include "serial_config.h"

/*
 * Name : uart_setup
 * Descriptoin: The uart_start function is for serial port setting port name and baud rate.
 * Input parameters: struct uart_device_struct * (for serial device information)
 * 					 char * (to set file name)
 * 					 int (to set baud rate)
 * 					 bool (to enable canonic mode)
 * Output parameters: void
 */
void uart_setup(struct uart_device_struct *device, char *file_name, int baud_rate, bool canonic)
{
	device->file_name = file_name;
	device->baud_rate = (unsigned int)baud_rate;

	uart_start(device, canonic);
}

/*
 * Name : uart_start
 * Descriptoin: The uart_start function is for initial setup.
 * Input parameters: struct uart_device_struct * (for serial device information)
 * 					 bool (to enable canonic mode)
 * Output parameters: int
 */
int uart_start(struct uart_device_struct *device, bool canonical)
{
	struct termios *tty;
	int fd;
	int rc;

	logger_setup();

	fd = open(device->file_name, O_RDWR | O_NOCTTY);
	if (fd < 0)
	{
		/* Error message with errno string (in this case must be 'Success')*/
		logger_error(SERIAL_LOG_MODULE_ID, "Error: failed to open UART device - %s\r\n", __func__);
		device->fd = fd;
		return fd;
	}

	tty = malloc(sizeof(*tty));
	if (!tty)
	{
		printf("%s: failed to allocate UART TTY instance\r\n", __func__);
		return -ENOMEM;
	}

	memset(tty, 0, sizeof(*tty));

	/* Set baud-rate. */
	tty->c_cflag |= device->baud_rate;

	/* Ignore framing and parity errors in input. */
	tty->c_iflag |= IGNPAR;

	/* Use 8-bit characters. */
	tty->c_cflag |= CS8;

	/* Enable receiver. */
	tty->c_cflag |= CREAD;

	if (canonical)
	{
		/*
		 * Enable canonical mode.
		 * Input is made available line by line.
		 */
		tty->c_lflag |= ICANON;
	}
	else
	{
		/* read() will block until at least one byte is available. */
		tty->c_cc[VTIME] = 0;
		tty->c_cc[VMIN] = 1;
	}

	/* Flush port. */
	tcflush(fd, TCIFLUSH);

	/* Apply attributes. */
	rc = tcsetattr(fd, TCSANOW, tty);
	if (rc)
	{
		logger_error(SERIAL_LOG_MODULE_ID, "Error: failed to set attributes - %s\r\n", __func__);
		return rc;
	}

	device->fd = fd;
	device->tty = tty;

	return 0;
}

/*
 * Name : uart_reads
 * Descriptoin: The uart_reads function is for reading data from serail port byte by byte.
 * Input parameters: struct uart_device_struct * (for serial device information)
 * 					 char * (buffer to hold serail data)
 * 					 size_t (buffer size)
 * Output parameters: int
 */
int uart_reads(struct uart_device_struct *device, char *buf, size_t buf_len)
{
	int rc;

	if (device->fd < 0)
	{
		logger_error(SERIAL_LOG_MODULE_ID, "failed to open UART device - %s\r\n", __func__);
		return device->fd;
	}

	rc = read(device->fd, buf, buf_len);
	if (rc < 0)
	{
		logger_error(SERIAL_LOG_MODULE_ID, "failed to read uart data - %s\r\n", __func__);
		return rc;
	}
	return rc;
}

/*
 * Name : uart_reads_chunk
 * Descriptoin: The uart_reads_chunk function is for reading data from serail port line by line.
 * Input parameters: struct uart_device_struct * (for serial device information)
 * 					 char * (buffer to hold serail data)
 * 					 size_t (buffer size)
 * Output parameters: int
 */
int uart_reads_chunk(struct uart_device_struct *device, char *buf, size_t buf_len)
{
	int rc;

	if (device->fd < 0)
	{
		logger_error(SERIAL_LOG_MODULE_ID, "failed to open UART device - %s\r\n", __func__);
		return device->fd;
	}

	rc = read(device->fd, buf, buf_len);

	if (rc < 0)
	{
		logger_error(SERIAL_LOG_MODULE_ID, "failed to read uart data - %s\r\n", __func__);
		return rc;
	}

	buf[rc] = '\0';

	return rc;
}

/*
 * Name : uart_writen
 * Descriptoin: The uart_writen function is for send data to serail port.
 * Input parameters: struct uart_device_struct * (for serial device information)
 * 					 char * (buffer to hold serail data)
 * 					 size_t (buffer size)
 * Output parameters: int
 */
int uart_writen(struct uart_device_struct *device, char *buf, size_t buf_len)
{
	return write(device->fd, buf, buf_len);
}

/*
 * Name : uart_writes
 * Descriptoin: The uart_writen function is for send data to serail port.
 * Input parameters: struct uart_device_struct * (for serial device information)
 * 					 char * (buffer to hold serail data)
 * 					 size_t (buffer size)
 * Output parameters: int
 */
int uart_writes(struct uart_device_struct *device, char *string)
{
	size_t len = strlen(string);
	return uart_writen(device, string, len);
}

/*
 * Name : uart_stop
 * Descriptoin: The uart_stop function is for closing serail port.
 * Input parameters: struct uart_device_struct * (for serial device information)
 * Output parameters: void
 */
void uart_stop(struct uart_device_struct *device)
{
	free(device->tty);
}
