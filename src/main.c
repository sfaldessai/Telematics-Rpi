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
#include <getopt.h>
#include <ctype.h>
#include "main.h"

int main(int argc, char *argv[])
{
    struct uart_device_struct client_controller_device, gps_device;
    struct cloud_data_struct cloud_data;
    struct arg_struct client_controller_args, gps_args;
    pthread_t client_controller_read_thread, gps_read_thread, serial_write_thread;
    pthread_t read_can_supported_thread, read_can_speed_thread, read_can_vin_thread, read_can_rpm_thread;
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
            write_to_file = atoi(optarg);
            break;
        default:
            break;
        }
    }

    /* Get Master (RPI) MAC Address and update cloud_data.mac_address */
    get_master_mac_address(cloud_data.mac_address);

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

    /* CAN Module Read Thread */
    read_from_can(&cloud_data, &read_can_supported_thread, &read_can_speed_thread, &read_can_vin_thread, &read_can_rpm_thread);

    /* Thread Creation End */

    /* Join Thread */
    if (client_controller_device.fd > 0)
    {
        pthread_join(client_controller_read_thread, NULL);
    }
    if (gps_device.fd > 0)
    {
        pthread_join(gps_read_thread, NULL);
    }
    else
    {
        gps_error_codes(&cloud_data, FAILED_TO_OPEN_GPS_DEVICE);
    }

    pthread_join(serial_write_thread, NULL);
    pthread_join(read_can_supported_thread, NULL);
    pthread_join(read_can_speed_thread, NULL);
    pthread_join(read_can_vin_thread, NULL);
    pthread_join(read_can_rpm_thread, NULL);
    /* Join Thread End*/

    return 0;
}