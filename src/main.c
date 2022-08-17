/*
 * created at 2022-07-19 18:46.
 *
 * Company: HashedIn By Deloitte.
 * Copyright (C) 2022 HashedIn By Deloitte
 */

#include <stdio.h>
#include <fcntl.h>
#include <termios.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "main.h"
#include <getopt.h>
#include <ctype.h>

#define MAX_READ_SIZE 1
#define CLIENT_CONTROLLER "/dev/ttyACM0"
#define GPS_MODULE "/dev/ttyUSB0"
int module_flag = 1;
int write_to_file = 0;


int main(int argc, char *argv[])
{
    struct uart_device_struct client_controller_device, gps_device;
    struct cloud_data_struct cloud_data;
    struct arg_struct client_controller_args, gps_args;
    pthread_t client_controller_read_thread, gps_read_thread, serial_write_thread;

    client_controller_device.file_name = "/dev/ttyACM0";
    gps_device.file_name = "/dev/ttyUSB0"; /* connected neo gps module to rapi using UART to USB converter */
    client_controller_device.baud_rate = B115200;
    gps_device.baud_rate = B9600;
    int opt;
    /* uart set-up*/
    uart_setup(&client_controller_device, CLIENT_CONTROLLER, B115200, true);
    uart_setup(&gps_device, GPS_MODULE, B9600, true);

    while ((opt = getopt(argc, argv, "m:f:")) != -1)
    {
        switch (opt)
        {
        case 'm':
            module_flag = atoi(optarg);
            break;
        case 'f':
            write_to_file = atoi(optarg);;
            break;
        default:
            break;
        }
    }
    
    /* Pointer char initializing to null*/
    initialize_cloud_data(&cloud_data);

    /* Thread Creation */
    if (client_controller_device.fd > 0)
    {
        client_controller_args.uart_device = client_controller_device;
        client_controller_args.cloud_data = &cloud_data;
        /* client_controller Microcontroller Read Thread */
        pthread_create(&client_controller_read_thread, NULL, &read_from_client_controller, &client_controller_args);
    }
    if (gps_device.fd > 0)
    {
        gps_args.uart_device = gps_device;
        gps_args.cloud_data = &cloud_data;
        /* NEO GPS Module Read Thread */
        pthread_create(&gps_read_thread, NULL, &read_from_gps, &gps_args);
    }
    /* Cloud Write Thread */
    pthread_create(&serial_write_thread, NULL, &write_to_cloud, &cloud_data);
    /* Thread End */

    /* Join Thread */
    if (client_controller_device.fd > 0)
    {
        pthread_join(client_controller_read_thread, NULL);
    }
    if (gps_device.fd > 0)
    {
        pthread_join(gps_read_thread, NULL);
    }
    pthread_join(serial_write_thread, NULL);
    /* Join Thread End*/

    return 0;
}