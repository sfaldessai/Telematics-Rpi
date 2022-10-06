/*
 * created at 2022-08-30 14:30.
 *
 * Company: HashedIn By Deloitte.
 * Copyright (C) 2022 HashedIn By Deloitte
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <pthread.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include <linux/can.h>
#include <linux/can/raw.h>

/*
 * virtual_can_server.c is using for accessing vcan0 (virtual can) and simulating odb2 data.
 * TBD: We can remove this file once we ready to connect the real can module.
 */

int speed_value = 40; /* default set to 40 */
int rpm_byte_1 = 20;
int rpm_byte_2 = 20;
int temperature_value = 20;

uint8_t get_random_number(uint8_t lower, uint8_t upper)
{
	int number = 0;
	number = (rand() % (upper - lower + 1)) + lower;
	return (uint8_t)number;
}

void *user_input(void *arg)
{
	long tid;
	tid = (long)arg;
	do
	{
		printf("\nPlease select the below option to update the value:\n");
		printf("\n\t1: SPEED\t2: RPM\t3:TEMPERATURE\n");

		int speed, rpm, option, temp;
		while (scanf("%d", &option) != 1)
		{
			getchar();
			printf("Must be an integer value\n");
		}
		switch (option)
		{
		case 1:
			printf("\nPlease enter speed value between 0-255\n");
			while (scanf("%d", &speed) != 1)
			{
				getchar();
				printf("Must be an integer value\n");
			}
			speed_value = speed;
			printf("\nset to %d\n", speed_value);
			break;
		case 2:
			printf("\nPlease enter rpm 1st byte value\n");
			while (scanf("%d", &rpm) != 1)
			{
				getchar();
				printf("Must be an integer value\n");
			}
			rpm_byte_1 = rpm;
			printf("\nPlease enter rpm 2nd byte value\n");
			while (scanf("%d", &rpm) != 1)
			{
				getchar();
				printf("Must be an integer value\n");
			}
			rpm_byte_2 = rpm;
			printf("\nset to %d %d\n", rpm_byte_1, rpm_byte_2);
			break;
		case 3:
			printf("\nPlease enter temeprature value between -40-215\n");
			while (scanf("%d", &temp) != 1)
			{
				getchar();
				printf("Must be an integer value\n");
			}
			temperature_value = temp;
			printf("\nset to %d\n", speed_value);
			break;
		default:
			break;
		}
	} while (1);
	printf("Thread ID %ld EXIT\n", tid);
}

void *start_can_communication(void *arg)
{

	int s;
	int nbytes;
	struct sockaddr_can addr;
	struct ifreq ifr;
	struct can_frame frame, request_frame;

	long tid;
	tid = (long)arg;
	if ((s = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0)
	{
		perror("Socket");
	}

	strcpy(ifr.ifr_name, "vcan0");
	ioctl(s, SIOCGIFINDEX, &ifr);

	memset(&addr, 0, sizeof(addr));
	addr.can_family = AF_CAN;
	addr.can_ifindex = ifr.ifr_ifindex;

	if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0)
	{
		perror("Bind");
	}
	while (1)
	{
		nbytes = read(s, &request_frame, sizeof(struct can_frame));
		/*
		printf("\nRequest Receved: ");
		for (i = 0; i < request_frame.can_dlc; i++)
		{
			printf(" %02X ", request_frame.data[i]);
		}
		printf("\n");
		*/
		if (nbytes > 0)
		{
			switch (request_frame.data[2])
			{
			case 0x0C: // RPM
				frame.can_id = 0x7E8;
				frame.can_dlc = 8;

				frame.data[0] = 3;
				frame.data[1] = 41;
				frame.data[2] = 0x0C;

				frame.data[3] = (uint8_t)rpm_byte_1; // get_random_number(0, 40);
				frame.data[4] = (uint8_t)rpm_byte_2; // get_random_number(0, 99);
				frame.data[5] = 0xAA;
				frame.data[6] = 0xAA;
				frame.data[7] = 0xAA;
				if (write(s, &frame, sizeof(struct can_frame)) != sizeof(struct can_frame))
				{
					perror("Write");
				}
				break;

			case 0x0D: // SPEED
				frame.can_id = 0x7E8;
				frame.can_dlc = 8;

				frame.data[0] = 3;
				frame.data[1] = 41;
				frame.data[2] = 0x0D;

				frame.data[3] = (uint8_t)speed_value; // get_random_number(0, 255);
				frame.data[4] = 0xAA;
				frame.data[5] = 0xAA;
				frame.data[6] = 0xAA;
				frame.data[7] = 0xAA;
				if (write(s, &frame, sizeof(struct can_frame)) != sizeof(struct can_frame))
				{
					perror("Write");
				}
				break;

			case 0x00:
				frame.can_id = 0x7E8;
				frame.can_dlc = 8;

				frame.data[0] = 0x06;
				frame.data[1] = 0x41;
				frame.data[2] = 0x00;
				frame.data[3] = get_random_number(0, 255);
				frame.data[4] = get_random_number(0, 255);
				frame.data[5] = get_random_number(0, 255);
				frame.data[6] = get_random_number(0, 255);
				frame.data[7] = 0xAA;
				if (write(s, &frame, sizeof(struct can_frame)) != sizeof(struct can_frame))
				{
					perror("Write");
				}
				break;

			case 0x02:
				frame.can_id = 0x7E8;
				frame.can_dlc = 8;

				frame.data[0] = 0x10;
				frame.data[1] = 0x14;
				frame.data[2] = 0x49;
				frame.data[3] = 0x02;
				frame.data[4] = 0x01;
				frame.data[5] = 0x33;
				frame.data[6] = 0x46;
				frame.data[7] = 0x41;
				if (write(s, &frame, sizeof(struct can_frame)) != sizeof(struct can_frame))
				{
					perror("Write");
				}
				usleep(200000);
				frame.data[0] = 0x21;
				frame.data[1] = 0x44;
				frame.data[2] = 0x50;
				frame.data[3] = 0x34;
				frame.data[4] = 0x46;
				frame.data[5] = 0x4A;
				frame.data[6] = 0x32;
				frame.data[7] = 0x42;
				if (write(s, &frame, sizeof(struct can_frame)) != sizeof(struct can_frame))
				{
					perror("Write");
				}
				usleep(200000);
				frame.data[0] = 0x22;
				frame.data[1] = 0x4D;
				frame.data[2] = 0x31;
				frame.data[3] = 0x31;
				frame.data[4] = 0x33;
				frame.data[5] = 0x39;
				frame.data[6] = 0x31;
				frame.data[7] = 0x33;
				usleep(200000);
				if (write(s, &frame, sizeof(struct can_frame)) != sizeof(struct can_frame))
				{
					perror("Write");
				}
				break;
			case 0x05: // TEMPERATURE
				frame.can_id = 0x7E8;
				frame.can_dlc = 8;

				frame.data[0] = 3;
				frame.data[1] = 41;
				frame.data[2] = 0x05;

				frame.data[3] = (uint8_t)temperature_value; // get_random_number(0, 40);
				frame.data[4] = 0xAA; // get_random_number(0, 99);
				frame.data[5] = 0xAA;
				frame.data[6] = 0xAA;
				frame.data[7] = 0xAA;
				if (write(s, &frame, sizeof(struct can_frame)) != sizeof(struct can_frame))
				{
					perror("Write");
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
	}

	printf("Thread ID %ld EXIT\n", tid);
}

int main(void)
{
	pthread_t input_thread, can_communication_thread;

	pthread_create(&input_thread, NULL, &user_input, NULL);
	pthread_create(&can_communication_thread, NULL, &start_can_communication, NULL);

	pthread_join(input_thread, NULL);
	pthread_join(can_communication_thread, NULL);

	return 0;
}