/*
 * created at 2022-07-29 09:00.
 *
 * Company: HashedIn By Deloitte.
 * Copyright (C) 2022 HashedIn By Deloitte
 */

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include "client_controller.h"
#include "../serial_interface/serial_config.h"
#include "../main.h"

/* mutex to lock cloud_data struct for wirte */
pthread_mutex_t cloud_data_mutex = PTHREAD_MUTEX_INITIALIZER;

#define MAX_READ_SIZE 80 /* set to 80 for temporary. TBD: message format & size */

/*
 * Name : read_from_client_controller
 * Descriptoin: The read_from_client_controller function is for reading vehicle motion, PTO,
 *              and batter voltage data from the STM32 microcontroller over the UART protocol.
 * Input parameters: struct arg_struct
 * Output parameters: void
 * Notes: cloud_data_struct *cloud_data will update with STM32 data.
 */
void *read_from_client_controller(void *arg)
{
    char *read_data = NULL;
    int read_data_len = 0;

    struct arg_struct *args = (struct arg_struct *)arg;
    struct uart_device_struct client_controller_device = args->uart_device;
    struct cloud_data_struct *cloud_data = args->cloud_data;
    struct client_controller_data_struct client_controller_data;

    while (1)
    {

        /* Reading data byte by byte */
        read_data_len = uart_reads_chunk(&client_controller_device, &read_data, MAX_READ_SIZE);

        if (read_data_len > 0)
        {
            /*
             * Message protocol used in microcontroller:
             * "*STMC,<MOTION>,<VOLT>,<PTO>,#""
             * '$' & '#' used to identify starting and ending.
             * microcontroller will send new data in every 2 sec
             */

            if (read_data[0] == '*')
            {
                client_controller_data.sensor_data = read_data;

                printf("\nclient_controller Sensor: %s\n", client_controller_data.sensor_data);

                pthread_mutex_lock(&cloud_data_mutex);
                cloud_data->client_controller_data = client_controller_data;
                pthread_mutex_unlock(&cloud_data_mutex);
            }

            free(read_data);
        }
    }
    uart_stop(&client_controller_device);
}