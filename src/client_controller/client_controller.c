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

#define CC_LOG_MODULE_ID 5
#define MAX_READ_SIZE 1 /* max stm32 LL data length is 1 */
#define MAXSIZE 32

/*
 * Name : update_cc_error_code
 * Descriptoin: The update_cc_error_code function is for updating erro codes for can struct member
 * Input parameters: struct cloud_data_struct * : clout struct to update can data member
 *                   int error_code : error code to update
 * Output parameters: void
 */
void update_cc_error_code(struct cloud_data_struct *cloud_data, int error_code)
{
    struct client_controller_data_struct client_controller_data;
    client_controller_data.voltage = error_code;
    client_controller_data.pto = error_code;
    client_controller_data.motion = error_code;
    client_controller_data.acc_x = error_code;
    client_controller_data.acc_y = error_code;
    client_controller_data.acc_z = error_code;

    /* update gps_data to cloud_data struct */
    pthread_mutex_lock(&can_module_lock);
    cloud_data->client_controller_data = client_controller_data;
    pthread_mutex_unlock(&can_module_lock);
}

/*
 * Name : get_client_controller_data
 * Descriptoin: The get_client_controller_data function is for extracting vehicle motion, PTO,
 *              and batter voltage data from the STM32 microcontroller sentence.
 * Input parameters:
 *                  char * (stm32 raw data)
 *                  client_controller_data_struct * (reference type to update stm32 data)
 * Output parameters: void
 */
void get_client_controller_data(char *read_data, struct client_controller_data_struct *client_controller_data)
{
    char *stmc_data = NULL;

    /* extracting required data from stm32 data sentence */
    stmc_data = strchr(read_data, COMMA);
    client_controller_data->motion = (uint8_t)atoi(stmc_data + 1);

    stmc_data = strchr(stmc_data + 1, COMMA);
    client_controller_data->voltage = (float)atof(stmc_data + 1);

    stmc_data = strchr(stmc_data + 1, COMMA);
    client_controller_data->pto = (uint8_t)atoi(stmc_data + 1);

    stmc_data = strchr(stmc_data + 1, COMMA);
    client_controller_data->acc_x = (uint8_t)atoi(stmc_data + 1);

    stmc_data = strchr(stmc_data + 1, COMMA);
    client_controller_data->acc_y = (uint8_t)atoi(stmc_data + 1);

    stmc_data = strchr(stmc_data + 1, COMMA);
    client_controller_data->acc_z = (uint8_t)atoi(stmc_data + 1);
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
    char read_data;
    int read_data_len = 0;
    char stm32_serial_data[MAXSIZE];
    size_t i = 0;

    struct arg_struct *args = (struct arg_struct *)arg;
    struct uart_device_struct client_controller_device = args->uart_device;
    struct cloud_data_struct *cloud_data = args->cloud_data;
    struct client_controller_data_struct client_controller_data;

    while (1)
    {

        /* Reading data byte by byte */
        read_data_len = uart_reads(&client_controller_device, &read_data, MAX_READ_SIZE);

        if (read_data_len > 0)
        {
            /*
             * Message protocol used in microcontroller:
             * "$STMC,<MOTION>,<VOLT>,<PTO>,#""
             * '$' & '#' used to identify starting and ending.
             */
            if (read_data == '$')
            {
                i = 0;
                stm32_serial_data[i] = read_data;
                do
                {
                    read_data_len = uart_reads(&client_controller_device, &read_data, MAX_READ_SIZE);
                    if (read_data_len > 0)
                    {
                        i++;
                        stm32_serial_data[i] = read_data;
                    }
                } while (read_data != '#');

                stm32_serial_data[i + 1] = '\0';

                logger_info(CC_LOG_MODULE_ID, "COMPLETE STM32 DATA: %s\n", stm32_serial_data);

                if (stm32_serial_data[1] == 'S' && stm32_serial_data[2] == 'T' && stm32_serial_data[3] == 'M' && stm32_serial_data[4] == 'C')
                {
                    get_client_controller_data(stm32_serial_data, &client_controller_data);

                    /* update stm32 data to cloud_data struct which is used to combile all module data and send to cloud */
                    pthread_mutex_lock(&cloud_data_mutex);
                    cloud_data->client_controller_data = client_controller_data;
                    pthread_mutex_unlock(&cloud_data_mutex);
                }
                else
                {
                }
            }
        }
    }
    uart_stop(&client_controller_device);
}