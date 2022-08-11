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

#define MAX_READ_SIZE 1
int module_flag = 1;

bool isNumber(char number[])
{
    int i = 0;

    //checking for negative numbers
    if (number[0] == '-')
        i = 1;
    for (; number[i] != 0; i++)
    {
        //if (number[i] > '9' || number[i] < '0')
        if (!isdigit(number[i]))
            return false;
    }
    return true;
}

int main(int argc, char *argv[])
{
    struct uart_device_struct stm32_device, gps_device;
    struct cloud_data_struct cloud_data;
    struct arg_struct stm32_args, gps_args;
    pthread_t stm32_read_thread, gps_read_thread, serial_write_thread;


    stm32_device.file_name = "/dev/ttyACM0";
    gps_device.file_name = "/dev/ttyUSB0"; /* connected neo gps module to rapi using UART to USB converter */
    stm32_device.baud_rate = B115200;
    gps_device.baud_rate = B9600;

    /* Used variables */
    logger_config_t cfg;

    /* Initialize logger and allow only error and not tagged output */
    logger_config_get(&cfg);

    /* Enable all logging flags */
    logger_enable(LOGGER_FLAGS_ALL);


    if( argc == 2 ) {
        module_flag = atoi(argv[1]);
        // if (isNumber(atoi(argv[1]))) {
        //     module_flag = atoi(argv[1]);
        // }
        // else {
        //     module_flag = 1;
        // }
    }

    logger_info(1, "module flag is %d", module_flag);

    /* Pointer char initializing to null*/
    initialize_cloud_data(&cloud_data);

    uart_start(&stm32_device, true);
    uart_start(&gps_device, true);

    if (stm32_device.fd > 0)
    {
        /* STM32 Microcontroller Read Thread */
        stm32_args.uart_device = stm32_device;
        stm32_args.cloud_data = &cloud_data;
        pthread_create(&stm32_read_thread, NULL, &read_from_stm32, &stm32_args);
    }
    if (gps_device.fd > 0)
    {
        /* NEO GPS Module Read Thread */
        gps_args.uart_device = gps_device;
        gps_args.cloud_data = &cloud_data;
        pthread_create(&gps_read_thread, NULL, &read_from_gps, &gps_args);
    }

    /* Cloud Write Thread */
    pthread_create(&serial_write_thread, NULL, &write_to_cloud, &cloud_data);

    if (stm32_device.fd > 0)
    {
        pthread_join(stm32_read_thread, NULL);
    }
    if (gps_device.fd > 0)
    {
        pthread_join(gps_read_thread, NULL);
    }
    pthread_join(serial_write_thread, NULL);

    return 0;
}
