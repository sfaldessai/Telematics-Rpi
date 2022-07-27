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
#include "uart.h"

#define MAX_READ_SIZE 1

int main()
{
    struct uart_device_struct stm32_device, gps_device;
    struct cloud_data_struct cloud_data;
    struct arg_struct stm32_args, gps_args;
    pthread_t stm32_read_thread, gps_read_thread, serial_write_thread;

    stm32_device.file_name = "/dev/ttyACM0";
    gps_device.file_name = "/dev/ttyUSB0"; /* connected neo gps module to rapi using UART to USB converter */
    stm32_device.baud_rate = B9600;
    gps_device.baud_rate = B9600;

    uart_start(&stm32_device, false);
    uart_start(&gps_device, false);

    char read_data;
    size_t read_data_len;

    /* STM32 Microcontroller Read Thread */
    stm32_args.uart_device = stm32_device;
    stm32_args.cloud_data = &cloud_data;
    pthread_create(&stm32_read_thread, NULL, (void *)&read_from_stm32, &stm32_args);

    /* NEO GPS Module Read Thread */
    gps_args.uart_device = gps_device;
    gps_args.cloud_data = &cloud_data;
    pthread_create(&gps_read_thread, NULL, (void *)&read_from_gps, &gps_args);

    /* NEO GPS Module Read Thread */
    pthread_create(&serial_write_thread, NULL, (void *)&write_to_cloud, &cloud_data);

    pthread_join(stm32_read_thread, NULL);
    pthread_join(gps_read_thread, NULL);
    pthread_join(serial_write_thread, NULL);

    return 0;
}
