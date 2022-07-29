/*
 * created at 2022-07-26 09:00.
 *
 * Company: HashedIn By Deloitte.
 * Copyright (C) 2022 HashedIn By Deloitte
 */

#include <stdio.h>
#include <pthread.h>
#include "client_controller.h"
#include "../serial_interface/serial_config.h"
#include "../main.h"

pthread_mutex_t cloud_data_mutex;

#define MAX_READ_SIZE 1
#define MAXSIZE 100 /* set to 100 for temporary. TBD: message format & size */

void *read_from_stm32(void *arg)
{
    char read_data;
    size_t read_data_len;
    unsigned int i;
    unsigned char stem32_serial_data[MAXSIZE];

    struct arg_struct *args = (struct arg_struct *)arg;
    struct uart_device_struct stm32_device = args->uart_device;
    struct cloud_data_struct *cloud_data = args->cloud_data;
    struct stm32_data_struct stm32_data;

    while (1)
    {
        /* Reading data byte by byte */
        read_data_len = uart_reads(&stm32_device, &read_data, MAX_READ_SIZE);

        if (read_data_len > 0)
        {
            /*
             * Message protocol used in microcontroller:
             * *<SERIALNO><LOCATION><VIN><BATTERY><SPEED><IDLETIME><SERVICE>$""
             * '*' & '$' used to identify starting and ending.
             * microcontroller will send new data in every 2 sec
             */

            if (read_data == '*')
            {
                i = 0;
                stem32_serial_data[i] = read_data;
                do
                {
                    read_data_len = uart_reads(&stm32_device, &read_data, MAX_READ_SIZE);
                    if (read_data_len > 0)
                    {
                        i++;
                        stem32_serial_data[i] = read_data;
                    }
                } while (read_data != '$');

                stem32_serial_data[i + 1] = '\0';
                stm32_data.sensor_data = stem32_serial_data;

                pthread_mutex_lock(&cloud_data_mutex);
                cloud_data->stm32_data = stm32_data;
                pthread_mutex_unlock(&cloud_data_mutex);
            }
        }
    }
    uart_stop(&stm32_device);
}