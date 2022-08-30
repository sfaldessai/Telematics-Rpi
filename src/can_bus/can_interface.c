#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "can_bus.h"
#include "../logger/logger.h"

/*
 * Name : get_request_frame
 *
 * Descriptoin: The get_request_frame function is for preparing a request frame for PID data.
 *
 * Input parameters:
 *                  struct can_frame  *request_frame: for updating request frame and send back
 * 					int pid: Requesting PID hex value
 * 					int mode: CAN service or mode number hex value
 *
 * Output parameters: void
 */
void get_request_frame(struct can_frame *request_frame, int pid, int mode)
{
	/* can request ID: 0x7DF for all request*/
	request_frame->can_id = 0x7DF;
	/* Data length code: 0 .. 8 */
	request_frame->can_dlc = 8;

	/* CAN Data Fields */
	/* #bytes using 0x02 for all PIDs */
	request_frame->data[0] = 0x02;

	/*
	 * MODE, tere 10 different mode, here we using 2 differnt mode
	 * 0x01 : Show Current Data - for speed and supported PID
	 * 0x09 : Request vehicle information
	 */
	request_frame->data[1] = 0x01;
	/* PID
	 * 0x00 for supported pids
	 * 0x0D for speed
	 * 0x02 for VIN
	 */
	request_frame->data[2] = pid;

	/* Below bytes are not used and set to 0xAA */
	request_frame->data[3] = 0xAA;
	request_frame->data[4] = 0xAA;
	request_frame->data[5] = 0xAA;
	request_frame->data[6] = 0xAA;
	request_frame->data[7] = 0xAA;
}

/*
 * Name : transmit_can_data
 *
 * Descriptoin: The transmit_can_data function is for sending can frame request to can module.
 *
 * Input parameters:
 *                  int sockfd: can socket file descriptor
 * 					struct can_frame request_frame:  Request frame for PID data
 *
 * Output parameters: void
 */
void transmit_can_data(int sockfd, struct can_frame request_frame)
{
	if (write(sockfd, &request_frame, sizeof(struct can_frame)) != sizeof(struct can_frame))
	{
		logger_error(CAN_LOG_MODULE_ID, "Error: Write failed- %s\r\n", __func__);
	}
}

/*
 * Name : receive_can_data
 *
 * Descriptoin: The receive_can_data function is for reading can frame response from can module.
 *
 * Input parameters:
 *                  int sockfd: can socket file descriptor
 * 					struct can_frame *frame:  response frame for PID data
 *
 * Output parameters: void
 */
void receive_can_data(int sockfd, struct can_frame *frame)
{
	int nbytes, i;
	nbytes = read(sockfd, frame, sizeof(struct can_frame));
	if (nbytes < 0)
	{
		logger_error(CAN_LOG_MODULE_ID, "Error: Read failed- %s\r\n", __func__);
	}
}

/*
 * Name : setup_can_socket
 *
 * Descriptoin: The setup_can_socket function is for can module setup for transmit and receive can frame data.
 *
 * Input parameters:
 *                  int sockfd: can socket file descriptor, update and return
 *
 * Output parameters: void
 */
void setup_can_socket(int *sockfd)
{
	struct sockaddr_can addr;
	struct ifreq ifr;

	if ((*sockfd = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0)
	{
		logger_error(CAN_LOG_MODULE_ID, "Error: Socket Open- %s\r\n", __func__);
	}

	strcpy(ifr.ifr_name, CAN_FILE);
	ioctl(*sockfd, SIOCGIFINDEX, &ifr);

	/* setting receive time for 5 sec*/
	struct timeval timeout;
	timeout.tv_sec = 5;
	timeout.tv_usec = 0;

	if (setsockopt(*sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout,
				   sizeof timeout) < 0)
	{
		logger_error(CAN_LOG_MODULE_ID, "Error: setsockopt failed- %s\r\n", __func__);
	}

	memset(&addr, 0, sizeof(addr));
	addr.can_family = AF_CAN;
	addr.can_ifindex = ifr.ifr_ifindex;

	if (bind(*sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
	{
		logger_error(CAN_LOG_MODULE_ID, "Error: Socket Bind- %s\r\n", __func__);
	}
}

/*
 * Name : close_socket
 *
 * Descriptoin: The close_socket function is for closing can socket.
 *
 * Input parameters:
 *                  int sockfd: can socket file descriptor
 *
 * Output parameters: void
 */
void close_socket(int *sockfd)
{
	if (close(*sockfd) < 0)
	{
		logger_error(CAN_LOG_MODULE_ID, "Error: Socket Close- %s\r\n", __func__);
	}
}

/*
 * Name : log_can_data
 *
 * Descriptoin: The log_can_data function is for logging can request frame data and response frame data.
 *
 * Input parameters:
 *                  struct can_frame frame: can frame data to log
 * 					char *type: type string to identify requets data or response data (CAN REQUEST or CAN RESPONSE)
 *
 * Output parameters: void
 */
void log_can_data(struct can_frame frame, char *type)
{
	logger_info(CAN_LOG_MODULE_ID, "%s :0x%03X [%d]  %02X %02X %02X %02X %02X %02X %02X %02X \n\n",
				type, frame.can_id, frame.can_dlc, frame.data[0], frame.data[1], frame.data[2],
				frame.data[3], frame.data[4], frame.data[5], frame.data[6], frame.data[7]);
}