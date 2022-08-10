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

#define SERIAL_LOG_MODULE_ID 3


int uart_start(struct uart_device_struct *device, bool canonical)
{
	struct termios *tty;
	int fd;
	int rc;

	/* Used variables */
    slog_config_t cfg;
	// uint16_t nLogFlags = SLOG_ERROR;

	/* Initialize slog and allow only error and not tagged output */
    slog_config_get(&cfg);

	/* Enable all logging flags */
    slog_enable(SLOG_FLAGS_ALL);



	fd = open(device->file_name, O_RDWR | O_NOCTTY);
	if (fd < 0)
	{
		/* Error message with errno string (in this case must be 'Success')*/
    	slog_error(SERIAL_LOG_MODULE_ID,"Error: failed to open UART device - %s\r\n", __func__);
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
		printf("%s: failed to set attributes\r\n", __func__);
		return rc;
	}

	device->fd = fd;
	device->tty = tty;

	return 0;
}

int uart_reads(struct uart_device_struct *device, char *buf, size_t buf_len)
{
	int rc;

	rc = read(device->fd, buf, buf_len);
	if (rc < 0)
	{
		printf("%s: failed to read uart data\r\n", __func__);
		return rc;
	}

	buf[rc] = '\0';
	return rc;
}

int uart_reads_chunk(struct uart_device_struct *device, char **buf, size_t buf_len)
{
	int rc;
	char chunk_data[buf_len];
	rc = read(device->fd, chunk_data, buf_len);

	if (rc < 0)
	{
		printf("%s: failed to read uart data\r\n", __func__);
		return rc;
	}

	chunk_data[rc] = '\0';

	free(*buf);
	*buf = (char *)malloc(rc + 1); /* strcpy adds a null terminator character '\0' */

	if (!*buf)
	{
		printf("%s: failed to allocate UART TTY instance\r\n", __func__);
		return -ENOMEM;
	}

	strncpy(*buf, chunk_data, rc);

	return rc;
}

int uart_writen(struct uart_device_struct *device, char *buf, size_t buf_len)
{
	return write(device->fd, buf, buf_len);
}

int uart_writes(struct uart_device_struct *device, char *string)
{
	size_t len = strlen(string);
	return uart_writen(device, string, len);
}

void uart_stop(struct uart_device_struct *device)
{
	free(device->tty);
}
