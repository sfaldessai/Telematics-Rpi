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

void read_can_id_number(struct uart_device_struct *can_device)
{
    char *read_data = "A0000000000000001";
    uint8_t vin[MAX_LEN_VIN];

    // uart_writes(can_device, "0x02\r\n");

    sleep(1);

    // uart_reads_chunk(can_device, &read_data, MAX_LEN_VIN);
    strncpy(vin, read_data, MAX_LEN_VIN);
    printf("%s", vin);
}

// for every 1000ms read vehicle speed data
void *read_can_speed_pid(void *arg)
{
    char *read_data = NULL;

    struct arg_struct *args = (struct arg_struct *)arg;
    struct uart_device_struct can_device = args->uart_device;
    struct cloud_data_struct *cloud_data = args->cloud_data;

    while (1)
    {
        pthread_mutex_lock(&can_bus_mutex);

        // uart_writes(&can_device, "0x0d\r\n");

        pthread_mutex_unlock(&can_bus_mutex);

        sleep(1);

        // uart_reads_chunk(&can_device, &read_data, MAX_LEN_SPEED_DATA);

        // cloud_data->can_data.speed = (uint8_t)atoi(read_data);
    }
}

// for every 30ms read PID 0x00
void *read_can_supported_pid(void *arg)
{
    char *read_data = NULL;

    struct arg_struct *args = (struct arg_struct *)arg;
    struct uart_device_struct can_device = args->uart_device;
    struct cloud_data_struct *cloud_data = args->cloud_data;

    while (1)
    {
        pthread_mutex_lock(&can_bus_mutex);

        // uart_writes(&can_device, "0x00\r\n");

        pthread_mutex_unlock(&can_bus_mutex);
        sleep(2);

        // uart_reads_chunk(&can_device, &read_data, MAX_LEN_SUPPORTED_DATA);

        // cloud_data->can_data.supported_pids = (uint32_t)atoi(read_data);
    }
}

void read_from_can(void *arg, pthread_t *read_can_supported_thread, pthread_t *read_can_speed_thread)
{

    struct arg_struct *args = (struct arg_struct *)arg;
    struct uart_device_struct can_device = args->uart_device;
    struct cloud_data_struct *cloud_data = args->cloud_data;
    uint8_t vin[MAX_LEN_VIN];
    // cloud_data->can_data.vin[MAX_LEN_VIN] =
    read_can_id_number(&can_device);

    pthread_create(read_can_supported_thread, NULL, &read_can_supported_pid, &arg);
    pthread_create(read_can_speed_thread, NULL, &read_can_speed_pid, &arg);
}