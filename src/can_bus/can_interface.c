#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "can_bus.h"

void get_request_frame(struct can_frame *request_frame, int pid)
{
	request_frame->can_id = 0x7DF;
	request_frame->can_dlc = 8;

	request_frame->data[0] = 0x02;
	request_frame->data[1] = 0x01;
	request_frame->data[2] = pid;
	request_frame->data[3] = 0xAA;
	request_frame->data[4] = 0xAA;
	request_frame->data[5] = 0xAA;
	request_frame->data[6] = 0xAA;
	request_frame->data[7] = 0xAA;
}

void transmit_can_data(int sockfd, struct can_frame request_frame)
{
	if (write(sockfd, &request_frame, sizeof(struct can_frame)) != sizeof(struct can_frame))
	{
		perror("Write");
	}
}

void receive_can_data(int sockfd, struct can_frame *frame)
{
	int nbytes, i;
	nbytes = read(sockfd, frame, sizeof(struct can_frame));
	if (nbytes < 0)
	{
		perror("Read");
	}
}

void setup_can_socket(int *sockfd)
{
	struct sockaddr_can addr;
	struct ifreq ifr;

	if ((*sockfd = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0)
	{
		perror("Socket");
	}

	strcpy(ifr.ifr_name, "vcan0");
	ioctl(*sockfd, SIOCGIFINDEX, &ifr);

	struct timeval timeout;
	timeout.tv_sec = 3;
	timeout.tv_usec = 0;

	if (setsockopt(*sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout,
				   sizeof timeout) < 0)
	{
		perror("setsockopt failed");
	}

	memset(&addr, 0, sizeof(addr));
	addr.can_family = AF_CAN;
	addr.can_ifindex = ifr.ifr_ifindex;

	if (bind(*sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
	{
		perror("Bind");
	}
}

void close_socket(int *sockfd)
{
	if (close(*sockfd) < 0)
	{
		perror("Close");
	}
}