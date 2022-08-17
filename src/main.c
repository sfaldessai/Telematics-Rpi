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

#define CLIENT_CONTROLLER "/dev/ttyACM0"
#define GPS_MODULE "/dev/ttyUSB0"
#define CAN_MODULE "/dev/ttyUSB0"

int main(void)
{
    struct uart_device_struct client_controller_device, gps_device, can_bus_device;
    struct cloud_data_struct cloud_data;
    struct arg_struct client_controller_args, gps_args, can_bus_arg;
    pthread_t client_controller_read_thread, gps_read_thread, serial_write_thread, CAN_read_thread;
    pthread_t read_can_supported_thread, read_can_speed_thread, read_can_vin_thread;

    /* uart set-up*/
    uart_setup(&client_controller_device, CLIENT_CONTROLLER, B115200, true);
    uart_setup(&gps_device, GPS_MODULE, B9600, true);
    uart_setup(&can_bus_device, CAN_MODULE, B115200, true);

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
    can_bus_arg.uart_device = can_bus_device;
    can_bus_arg.cloud_data = &cloud_data;
    read_from_can(&can_bus_arg, &read_can_supported_thread, &read_can_speed_thread, &read_can_vin_thread);

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
    pthread_join(serial_write_thread, NULL);
    pthread_join(read_can_supported_thread, NULL);
    pthread_join(read_can_speed_thread, NULL);
    pthread_join(read_can_vin_thread, NULL);
    /* Join Thread End*/

    return 0;
}