/*
 * created at 2022-07-29 09:00.
 *
 * Company: HashedIn By Deloitte.
 * Copyright (C) 2022 HashedIn By Deloitte
 */

#include <stdio.h>
#include <pthread.h>
#include "client_controller.h"
#include "../serial_interface/serial_config.h"
#include "../main.h"

pthread_mutex_t cloud_data_mutex = PTHREAD_MUTEX_INITIALIZER;

#define MAX_READ_SIZE 80 /* set to 80 for temporary. TBD: message format & size */

void *read_from_stm32(void *arg)
{
    char *read_data = NULL;
    int read_data_len;

    struct arg_struct *args = (struct arg_struct *)arg;
    struct uart_device_struct stm32_device = args->uart_device;
    struct cloud_data_struct *cloud_data = args->cloud_data;
    struct stm32_data_struct stm32_data;

    while (1)
    {

        /* Reading data byte by byte */
        read_data_len = uart_reads_chunk(&stm32_device, &read_data, MAX_READ_SIZE);

        if (read_data_len > 0)
        {
            printf("\n STM32 DATA: %s\n", read_data);
            /*
             * Message protocol used in microcontroller:
             * *<SERIALNO><LOCATION><VIN><BATTERY><SPEED><IDLETIME><SERVICE>$""
             * '*' & '$' used to identify starting and ending.
             * microcontroller will send new data in every 2 sec
             */

            if (read_data[0] == '*')
            {
                stm32_data.sensor_data = read_data;

                pthread_mutex_lock(&cloud_data_mutex);
                cloud_data->stm32_data = stm32_data;
                pthread_mutex_unlock(&cloud_data_mutex);
            }
        }
    }
    uart_stop(&stm32_device);
}