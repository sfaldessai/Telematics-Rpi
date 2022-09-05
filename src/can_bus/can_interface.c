#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "can_bus.h"
#include "../../include/resource.h"
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
	request_frame->can_id = CAN_REQUEST_ID;
	/* Data length code: 0 .. 8 */
	request_frame->can_dlc = CAN_FRAME_LENGTH;

	/* CAN Data Fields */
	/* #bytes using 0x02 for all PIDs */
	request_frame->data[0] = 0x02;

	/*
	 * MODE, tere 10 different mode, here we using 2 differnt mode
	 * 0x01 : Show Current Data - for speed and supported PID
	 * 0x09 : Request vehicle information
	 */
	request_frame->data[1] = mode;
	/* PID
	 * 0x00 for supported pids
	 * 0x0D for speed
	 * 0x02 for VIN
	 */
	request_frame->data[2] = pid;

	/* Below bytes are not used and set to 0xAA */
	request_frame->data[3] = CAN_EMPTY_DATA;
	request_frame->data[4] = CAN_EMPTY_DATA;
	request_frame->data[5] = CAN_EMPTY_DATA;
	request_frame->data[6] = CAN_EMPTY_DATA;
	request_frame->data[7] = CAN_EMPTY_DATA;
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
	int nbytes;
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

/*
 * Name : log_can_supported_data
 *
 * Descriptoin: The log_can_supported_data function is for logging can supported PID with support status.
 *
 * Input parameters:
 *                  uint8_t *supported_binary_value: supported PID binary values
 *
 * Output parameters: void
 */
void log_can_supported_data(uint8_t *supported_binary_value)
{
	for (size_t i = 0; i < CAN_PID_LENGTH; i++)
	{
		if (supported_binary_value[i] == 1)
		{
			logger_info(CAN_LOG_MODULE_ID, "PID = %s \t SUPPORTED = YES\n", can_pids[i]);
		}
		else
		{
			logger_info(CAN_LOG_MODULE_ID, "PID = %s \t SUPPORTED = NO\n", can_pids[i]);
		}
	}
}

/*
 * Name : vin_from_can_frame_data
 *
 * Descriptoin: The vin_from_can_frame_data function is for extracting VIN data from 3 can frames and converting into string value.
 *
 * Input parameters:
 *                  struct can_frame *vin_frame: array of vin can frames (3 frames for VIN)
 *					char *read_data: reference type, updating extracted vin
 * Output parameters: void
 */
void vin_from_can_frame_data(struct can_frame *vin_frame, char *read_data)
{
	size_t j = 0;
	for (size_t i = 5; i < CAN_FRAME_LENGTH; i++)
	{
		read_data[j] = (char)vin_frame[0].data[i];
		j = j + 1;
	}
	for (size_t i = 1; i < CAN_FRAME_LENGTH; i++)
	{
		read_data[j] = (char)vin_frame[1].data[i];
		j = j + 1;
	}
	for (size_t i = 1; i < CAN_FRAME_LENGTH; i++)
	{
		read_data[j] = (char)vin_frame[2].data[i];
		j = j + 1;
	}
}

/*
 * Name : add_binary_values
 *
 * Descriptoin: The add_binary_values function is for extracting VIN data from 3 can frames and converting into string value.
 *
 * Input parameters:
 *                  uint8_t *supported_binary: reference type, appedning all 32 PIDs binary data.
 *					int *index: referenc type, updating index value for supported_binary array index.
 *					char *binary: hex byte binary value, appending to supported_binary.
 * Output parameters: void
 */
void add_binary_values(uint8_t *supported_binary, int *index, char *binary)
{
	for (size_t i = 0; i < 4; i++)
	{
		supported_binary[*index] = (binary[i] - '0');
		*index = *index + 1;
	}
}

/*
 * Name : hex_to_binary
 *
 * Descriptoin: The hex_to_binary function is for extracting VIN data from 3 can frames and converting into string value.
 *
 * Input parameters:
 *                  struct can_frame supported_frame: can frame data for supported PIDs.
 *					uint8_t *supported_binary: referenc type, updating binary supported PIDs data from can frame data.
 *
 * Output parameters: void
 */
void hex_to_binary(struct can_frame supported_frame, uint8_t *supported_binary)
{
	size_t i = 0;
	int index = 0;

	uint8_t supported_value[16];
	sprintf((char *)supported_value, "%x%x%x%x", supported_frame.data[3], supported_frame.data[4], supported_frame.data[5], supported_frame.data[3]);

	while (supported_value[i])
	{

		switch (supported_value[i])
		{
		case '0':
			add_binary_values(supported_binary, &index, "0000");
			break;
		case '1':
			add_binary_values(supported_binary, &index, "0001");
			break;
		case '2':
			add_binary_values(supported_binary, &index, "0010");
			break;
		case '3':
			add_binary_values(supported_binary, &index, "0011");
			break;
		case '4':
			add_binary_values(supported_binary, &index, "0100");
			break;
		case '5':
			add_binary_values(supported_binary, &index, "0101");
			break;
		case '6':
			add_binary_values(supported_binary, &index, "0110");
			break;
		case '7':
			add_binary_values(supported_binary, &index, "0111");
			break;
		case '8':
			add_binary_values(supported_binary, &index, "1000");
			break;
		case '9':
			add_binary_values(supported_binary, &index, "1001");
			break;
		case 'A':
		case 'a':
			add_binary_values(supported_binary, &index, "1010");
			break;
		case 'B':
		case 'b':
			add_binary_values(supported_binary, &index, "1011");
			break;
		case 'C':
		case 'c':
			add_binary_values(supported_binary, &index, "1100");
			break;
		case 'D':
		case 'd':
			add_binary_values(supported_binary, &index, "1101");
			break;
		case 'E':
		case 'e':
			add_binary_values(supported_binary, &index, "1110");
			break;
		case 'F':
		case 'f':
			add_binary_values(supported_binary, &index, "1111");
			break;
		default:
			break;
		}
		i++;
	}
}