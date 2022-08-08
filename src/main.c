/*
 * created at 2022-07-19 18:46.
 *
 * Company: HashedIn By Deloitte.
 * Copyright (C) 2022 HashedIn By Deloitte
 */

#include <stdio.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <pthread.h>
#include "main.h"

#define CLINT_CONTROLLER "/dev/ttyACM0"
#define GPS_MODULE "/dev/ttyUSB0"

int main(void)
{
    struct uart_device_struct clinet_controller_device, gps_device;
    struct cloud_data_struct cloud_data;
    struct arg_struct clinet_controller_args, gps_args;
    pthread_t clinet_controller_read_thread, gps_read_thread, serial_write_thread;

    /* uart set-up*/
    uart_setup(&clinet_controller_device, CLINT_CONTROLLER, B115200, true);
    uart_setup(&gps_device, GPS_MODULE, B9600, true);

    /* Pointer char initializing to null*/
    initialize_cloud_data(&cloud_data);

    /* Thread Creation */
    if (clinet_controller_device.fd > 0)
    {
        clinet_controller_args.uart_device = clinet_controller_device;
        clinet_controller_args.cloud_data = &cloud_data;
        /* clinet_controller Microcontroller Read Thread */
        pthread_create(&clinet_controller_read_thread, NULL, &read_from_clinet_controller, &clinet_controller_args);
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
    if (clinet_controller_device.fd > 0)
    {
        pthread_join(clinet_controller_read_thread, NULL);
    }
    if (gps_device.fd > 0)
    {
        pthread_join(gps_read_thread, NULL);
    }
    pthread_join(serial_write_thread, NULL);
    /* Join Thread End*/

    return 0;
}