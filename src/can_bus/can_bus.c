/*
 * created at 2022-07-29 14:30.
 *
 * Company: HashedIn By Deloitte.
 * Copyright (C) 2022 HashedIn By Deloitte
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "can_bus.h"
#include "../serial_interface/serial_config.h"
#include "../main.h"

/* mutex to lock cloud_data struct for wirte */
pthread_mutex_t can_bus_mutex = PTHREAD_MUTEX_INITIALIZER;

/*
 * Name : read_can_id_number
 *
 * Descriptoin: The read_can_id_number function is for fetching VIN PID data from the CAN module
 *
 * Input parameters:
 *                  void *arg : uart_device_struct can_device and cloud_data_struct
 *
 * Output parameters: void
 */
void *read_can_id_number(void *arg)
{
    /* TBD: read from can module once we get CAN module. Hardcaded for testing*/
    char *read_data = "A0000000000000001";
    uint8_t vin[MAX_LEN_VIN];

    struct arg_struct *args = (struct arg_struct *)arg;
    struct uart_device_struct can_device = args->uart_device;
    struct cloud_data_struct *cloud_data = args->cloud_data;

    /*
     *
     * TBD: Write function to Request the CAN module for PID 0x02 VIN data.
     *
     * uart_writes(can_device, "0x02\r\n");
     */

    sleep(1);

    /*
     *
     * TBD: Write function to Receive the VIN data from CAN module.
     *
     * uart_reads_chunk(can_device, &read_data, MAX_LEN_VIN);
     */

    /* Copy 17 byte VIN data to cloud struct member for displaying on screen from deiplay thread */
    strncpy(vin, read_data, MAX_LEN_VIN);
    cloud_data->can_data.vin = vin;
}

/*
 * Name : read_can_speed_pid
 *
 * Descriptoin: The read_can_speed_pid function is for fetching Vehicle speed PID data from the CAN module
 *
 * Input parameters:
 *                  void *arg : uart_device_struct can_device and cloud_data_struct
 *
 * Output parameters: void
 */
void *read_can_speed_pid(void *arg)
{
    /* TBD: read from can once we get CAN module. Hardcaded for testing*/
    char *read_data = "85";

    struct arg_struct *args = (struct arg_struct *)arg;
    struct uart_device_struct can_device = args->uart_device;
    struct cloud_data_struct *cloud_data = args->cloud_data;

    while (1)
    {
        /*
         *
         * TBD: Write function to Request the CAN module for PID 0x0d Vehicle speed data.
         *
         * uart_writes(&can_device, "0x0d\r\n");
         */

        sleep(1);

        /*
         *
         * TBD: Write function to Receive the Vehicle speed data from CAN module.
         *
         * uart_reads_chunk(&can_device, &read_data, MAX_LEN_SPEED_DATA);
         */

        /* Copy 1 byte (0-255) Vehicle speed data to cloud struct member for displaying on screen from deiplay thread */

        cloud_data->can_data.speed = (uint8_t) atoi(read_data);
    }
}

/*
 * Name : read_can_supported_pid
 *
 * Descriptoin: The read_can_supported_pid function is for fetching supported PID data from the CAN module
 *
 * Input parameters:
 *                  void *arg : uart_device_struct can_device and cloud_data_struct
 *
 * Output parameters: void
 */
void *read_can_supported_pid(void *arg)
{
    /* TBD: read from can once we get CAN module. Hardcaded for testing*/
    char *read_data = "2147483647";

    struct arg_struct *args = (struct arg_struct *)arg;
    struct uart_device_struct can_device = args->uart_device;
    struct cloud_data_struct *cloud_data = args->cloud_data;

    while (1)
    {
        /*
         *
         * TBD: Write function to Request the CAN module for PID 0x00 Vehicle Supported PID data.
         *
         * uart_writes(&can_device, "0x00\r\n");
         */

        sleep(2);

        /*
         *
         * TBD: Write function to Receive the Vehicle Supported PID data from CAN module.
         *
         * uart_reads_chunk(&can_device, &read_data, MAX_LEN_SPEED_DATA);
         */

        /* Copy 32 byte Vehicle Supported PID data to cloud struct member for displaying on screen from deiplay thread */
        cloud_data->can_data.supported_pids = (uint32_t) atoi(read_data);
    }
}

/*
 * Name : read_from_can
 *
 * Descriptoin: The read_from_can function is for fetching CAN data which contains VIN, SPEED, and supported PID data
 *
 * Input parameters:
 *                  void *arg : uart_device_struct can_device and cloud_data_struct
 *                  pthread_t *read_can_supported_thread
 *                  pthread_t *read_can_speed_thread
 *                  pthread_t *read_can_vin_thread
 *
 * Output parameters: void
 */
void read_from_can(void *arg, pthread_t *read_can_supported_thread, pthread_t *read_can_speed_thread, pthread_t *read_can_vin_thread)
{
    struct arg_struct *args = (struct arg_struct *)arg;
    struct uart_device_struct can_device = args->uart_device;
    struct cloud_data_struct *cloud_data = args->cloud_data;

    /* Thread to fetch VIN. */
    pthread_create(read_can_vin_thread, NULL, &read_can_id_number, arg);
    /* Thread to fetch supported pid data. */
    pthread_create(read_can_supported_thread, NULL, &read_can_supported_pid, arg);
    /* Thread to fetch spedd pid data. */
    pthread_create(read_can_speed_thread, NULL, &read_can_speed_pid, arg);
}