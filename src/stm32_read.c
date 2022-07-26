/*
 * created at 2022-07-26 09:00.
 *
 * Company: HashedIn By Deloitte.
 * Copyright (C) 2022 HashedIn By Deloitte
 */

#include <stdio.h>
#include "uart.h"

#define MAX_READ_SIZE 1

void *read_from_stm32(void *arg)
{
    char read_data;
    size_t read_data_len;
    bool receive_data = false;

    struct uart_device_struct *stm32_device = (struct uart_device_struct *)arg;
    while (1)
    {
        /* Reading data byte by byte */
        read_data_len = uart_reads(stm32_device, &read_data, MAX_READ_SIZE);

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
                receive_data = true;
            }
            else if (receive_data && read_data == '$')
            {
                printf("\nReceived complete data from STM32 and starting receiving new data.\r\n");
            }
            else if (receive_data)
            {
                printf("%c", read_data);
            }
        }
    }
    uart_stop(stm32_device);
}