
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include <linux/can.h>
#include <linux/can/raw.h>

int get_random_number(int lower, int upper)
{
	int number = 0;
	number = (rand() %
			   (upper - lower + 1)) +
			  lower;
	return number;
}


int main(int argc, char **argv)
{
	int s, i;
	int nbytes;
	struct sockaddr_can addr;
	struct ifreq ifr;
	struct can_frame frame, request_frame;

	if ((s = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0)
	{
		perror("Socket");
		return 1;
	}

	strcpy(ifr.ifr_name, "vcan0");
	ioctl(s, SIOCGIFINDEX, &ifr);

	memset(&addr, 0, sizeof(addr));
	addr.can_family = AF_CAN;
	addr.can_ifindex = ifr.ifr_ifindex;

	if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0)
	{
		perror("Bind");
		return 1;
	}

	while (1)
	{
		nbytes = read(s, &request_frame, sizeof(struct can_frame));
		printf("\nRequest Receved: ");
		for (i = 0; i < request_frame.can_dlc; i++)
		{
			printf(" %02X ", request_frame.data[i]);
		}
		printf("\n");
		if (nbytes > 0)
		{
			switch (request_frame.data[2])
			{
			case 0x0D:
				frame.can_id = 0x7E8;
				frame.can_dlc = 8;

				frame.data[0] = 3;
				frame.data[1] = 41;
				frame.data[2] = 0x0D;

				int speed = get_random_number(0, 255);
				
				frame.data[3] = speed;
				frame.data[4] = 0xAA;
				frame.data[5] = 0xAA;
				frame.data[6] = 0xAA;
				frame.data[7] = 0xAA;
				if (write(s, &frame, sizeof(struct can_frame)) != sizeof(struct can_frame))
				{
					perror("Write");
					return 1;
				}
				break;

			case 0x00:
				frame.can_id = 0x7E8;
				frame.can_dlc = 8;

				frame.data[0] = 0x06;
				frame.data[1] = 0x41;
				frame.data[2] = 0x00;
				frame.data[3] = 0x12;
				frame.data[4] = 0x34;
				frame.data[5] = 0x56;
				frame.data[6] = 0x78;
				frame.data[7] = 0xAA;
				if (write(s, &frame, sizeof(struct can_frame)) != sizeof(struct can_frame))
				{
					perror("Write");
					return 1;
				}
				break;

			case 0x02:
				frame.can_id = 0x7E8;
				frame.can_dlc = 8;

				frame.data[0] = 3;
				frame.data[1] = 41;
				frame.data[2] = 0x0D;
				frame.data[3] = 0x12;
				frame.data[4] = 0;
				frame.data[5] = 0;
				frame.data[6] = 0;
				frame.data[7] = 0;
				if (write(s, &frame, sizeof(struct can_frame)) != sizeof(struct can_frame))
				{
					perror("Write");
					return 1;
				}
				break;

			default:
				break;
			}
		}
	}

	if (close(s) < 0)
	{
		perror("Close");
		return 1;
	}

	return 0;
}
