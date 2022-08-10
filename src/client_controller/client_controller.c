/*
 * created at 2022-07-29 09:00.
 *
 * Company: HashedIn By Deloitte.
 * Copyright (C) 2022 HashedIn By Deloitte
 */

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include "client_controller.h"
#include "../serial_interface/serial_config.h"
#include "../main.h"

#define COMMA 0x2C

/* mutex to lock cloud_data struct for wirte */
pthread_mutex_t cloud_data_mutex = PTHREAD_MUTEX_INITIALIZER;

#define MAX_READ_SIZE 32 /* set to 80 for temporary. TBD: message format & size */

/*
 * Name : get_clinet_controller_data
 * Descriptoin: The get_clinet_controller_data function is for processing vehicle motion, PTO,
 *              and batter voltage data from the STM32 microcontroller.
 * Input parameters:
 *                  char * (stm32 raw data)
 *                  client_controller_data_struct * (reference type to update stm32 data)
 * Output parameters: void
 */
void get_clinet_controller_data(char *read_data, struct client_controller_data_struct *client_controller_data)
{
    char *stmc_data = NULL;

    /* Get UTC Time from GGA message */
    stmc_data = strchr(read_data, COMMA);
    client_controller_data->motion = atof(stmc_data + 1);

    stmc_data = strchr(stmc_data + 1, COMMA);
    client_controller_data->voltage = atof(stmc_data + 1);

    stmc_data = strchr(stmc_data + 1, COMMA);
    client_controller_data->pto = atof(stmc_data + 1);

    free(stmc_data);
}

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
             * "$STMC,<MOTION>,<VOLT>,<PTO>,#""
             * '$' & '#' used to identify starting and ending.
             * microcontroller will send new data in every 2 sec
             */

            if (read_data[1] == 'S' && read_data[2] == 'T' && read_data[3] == 'M' && read_data[4] == 'C')
            {
                get_clinet_controller_data(read_data, &client_controller_data);

                pthread_mutex_lock(&cloud_data_mutex);
                cloud_data->client_controller_data = client_controller_data;
                pthread_mutex_unlock(&cloud_data_mutex);
            }

            free(read_data);
        }
    }
    uart_stop(&client_controller_device);
}