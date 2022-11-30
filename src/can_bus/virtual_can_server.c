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
#include <arpa/inet.h>

#include <linux/can.h>
#include <linux/can/raw.h>
#define PORT 8080
#define IP "192.168.218.234"
#define SA struct sockaddr
#define COMMA 0x2C
#define MAX_LEN_VIN 17
#define CAN_PID_LENGTH 32
#define SPEED_PID 0x0D
#define VIN_PID 0x02
#define RPM_PID 0x0C
#define SUPPORTED_PID 0x00
#define TEMPERATURE_PID 0x05

 /*
  * virtual_can_server.c is using for accessing vcan0 (virtual can) and simulating odb2 data.
  * TBD: We can remove this file once we ready to connect the real can module.
  */

int speed_value = 40; /* default set to 40 */
int rpm_byte_1 = 20;
int rpm_byte_2 = 20;
int temperature_value = 20;
int sock_tcp_fd = 0, client_fd;
int can_type = 0;
struct can_data_struct_tcp
{
	uint8_t vin[MAX_LEN_VIN];
	uint8_t supported_pids[CAN_PID_LENGTH];
	uint8_t temperature;
	uint16_t speed;
	uint8_t rpm_byte_1;
	uint8_t rpm_byte_2;
};

struct can_data_struct_tcp tcp_can_data;

uint8_t get_random_number(uint8_t lower, uint8_t upper)
{
	int number = 0;
	number = (rand() % (upper - lower + 1)) + lower;
	return (uint8_t)number;
}

void* user_input(void* arg)
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
			printf("\nset to %d\n", temperature_value);
			break;
		default:
			break;
		}
	} while (1);
	printf("Thread ID %ld EXIT\n", tid);
}

void get_can_data(char* received_data, struct can_data_struct_tcp* decoded_data, int pid)
{
	char* can_data = NULL;
	char i;
	int rpm1, rpm2;

	if (received_data[1] == 'C' && received_data[2] == 'A' && received_data[3] == 'N')
	{
		if (pid == SPEED_PID)
		{
			can_data = strchr(received_data, COMMA);
			decoded_data->speed = atoi(can_data + 1);
		}

		else if (pid == RPM_PID)
		{
			can_data = strchr(received_data, COMMA);
			decoded_data->rpm_byte_1 = atoi(can_data + 1);
			can_data = strchr(received_data, COMMA);
			decoded_data->rpm_byte_2 = atoi(can_data + 1);
		}
		else if (pid == SUPPORTED_PID)
		{
			for (i = 0;i < 4;i++) {
				can_data = strchr(received_data, COMMA);
				decoded_data->supported_pids[i] = atoi(can_data + 1);
			}
		}
		else if (pid == TEMPERATURE_PID)
		{
			can_data = strchr(received_data, COMMA);
			decoded_data->temperature = atoi(can_data + 1);
		}
	}
}

void* start_can_communication(void* arg)
{

	int s;
	int nbytes;
	struct sockaddr_can addr;
	struct ifreq ifr;
	struct can_frame frame, request_frame;
	char received_data[80];

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

	if (bind(s, (struct sockaddr*)&addr, sizeof(addr)) < 0)
	{
		perror("Bind");
	}

	while (1)
	{
		nbytes = read(s, &request_frame, sizeof(struct can_frame));

		if (nbytes > 0)
		{
			if (can_type == 1)
			{
				if (sock_tcp_fd >= 0)
				{
					char p[4];

					/* Converting Hex to String */
					sprintf(p, "%02X", request_frame.data[2]);

					send(sock_tcp_fd, p, strlen(p), 0);

					printf("sent");
				}

				sleep(1);
				read(sock_tcp_fd, received_data, sizeof(received_data));
			}
			switch (request_frame.data[2])
			{
			case 0x0C: // RPM
				if (can_type == 1) {
					get_can_data(received_data, &tcp_can_data, RPM_PID);
				}
				frame.can_id = 0x7E8;
				frame.can_dlc = 8;

				frame.data[0] = 3;
				frame.data[1] = 41;
				frame.data[2] = 0x0C;

				if (can_type == 1) {
					frame.data[3] = (uint8_t)tcp_can_data.rpm_byte_1; //get rpm data from TCP server
					frame.data[4] = (uint8_t)tcp_can_data.rpm_byte_2; //get rpm data from TCP server
				}
				else {
					frame.data[3] = (uint8_t)rpm_byte_1; // get_random_number(0, 40);
					frame.data[4] = (uint8_t)rpm_byte_2; // get_random_number(0, 99);
				}

				frame.data[5] = 0xAA;
				frame.data[6] = 0xAA;
				frame.data[7] = 0xAA;
				if (write(s, &frame, sizeof(struct can_frame)) != sizeof(struct can_frame))
				{
					perror("Write");
				}
				break;

			case 0x0D: // SPEED
				if (can_type == 1) {
					get_can_data(received_data, &tcp_can_data, SPEED_PID);
				}
				frame.can_id = 0x7E8;
				frame.can_dlc = 8;

				frame.data[0] = 3;
				frame.data[1] = 41;
				frame.data[2] = 0x0D;
				if (can_type == 1) {
					frame.data[3] = (uint8_t)tcp_can_data.speed; // get_random_speed_from_TCP(0, 255);
				}
				else {
					frame.data[3] = (uint8_t)speed_value; // get_random_number(0, 255);
				}
				frame.data[4] = 0xAA;
				frame.data[5] = 0xAA;
				frame.data[6] = 0xAA;
				frame.data[7] = 0xAA;
				if (write(s, &frame, sizeof(struct can_frame)) != sizeof(struct can_frame))
				{
					perror("Write");
				}
				break;

			case 0x00: //SUPPORTED PID
				if (can_type == 1) {
					get_can_data(received_data, &tcp_can_data, SUPPORTED_PID);
				}
				frame.can_id = 0x7E8;
				frame.can_dlc = 8;

				frame.data[0] = 0x06;
				frame.data[1] = 0x41;
				frame.data[2] = 0x00;
				if (can_type == 1) {
					frame.data[3] = tcp_can_data.supported_pids[0];
					frame.data[4] = tcp_can_data.supported_pids[0];
					frame.data[5] = tcp_can_data.supported_pids[0];
					frame.data[6] = tcp_can_data.supported_pids[0];
				}
				else {
					frame.data[3] = 255;
					frame.data[4] = 255;
					frame.data[5] = 255;
					frame.data[6] = 255;
				}
				frame.data[7] = 0xAA;
				if (write(s, &frame, sizeof(struct can_frame)) != sizeof(struct can_frame))
				{
					perror("Write");
				}
				break;

			case 0x02: //VIN

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
				if (can_type == 1) {
					printf("temp");
					frame.data[3] = (uint8_t)tcp_can_data.temperature; //get_random temperature from TCP
				}
				else {
					frame.data[3] = (uint8_t)temperature_value; // get_random_number(0, 40);
				}
				frame.data[4] = 0xAA;
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

void* connect_tcp_server(void* arg)
{
	long tid;
	tid = (long)arg;

	struct sockaddr_in serv_addr;
	char buffer[1024] = { 0 };
	if ((sock_tcp_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("\n Socket creation error \n");
		return NULL;
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);

	if (inet_pton(AF_INET, IP, &serv_addr.sin_addr) <= 0)
	{
		printf("\nInvalid address/ Address not supported \n");
		return NULL;
	}
	client_fd = connect(sock_tcp_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
	printf("clientfd %d", client_fd);

	if (client_fd < 0)
	{
		printf("\nConnection Failed \n");
		return NULL;
	}
	printf("connected sucsessfully");
	return NULL;
}

int main(int argc, char* argv[])
{
	int opt;
	pthread_t input_thread, can_communication_thread, tcp_client_thread;

	while ((opt = getopt(argc, argv, "t:")) != -1)
	{
		switch (opt)
		{
		case 't':
			can_type = atoi(optarg);
			break;
		default:
			break;
		}
	}
	if (can_type != 1)
	{
		pthread_create(&input_thread, NULL, &user_input, NULL);
	}
	pthread_create(&can_communication_thread, NULL, &start_can_communication, NULL);
	pthread_create(&tcp_client_thread, NULL, &connect_tcp_server, NULL);
	if (can_type != 1)
	{
		pthread_join(input_thread, NULL);
	}
	pthread_join(can_communication_thread, NULL);
	pthread_join(tcp_client_thread, NULL);

	return 0;
}