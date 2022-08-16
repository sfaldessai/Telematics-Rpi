/*
 * created at 2022-07-29 14:30.
 *
 * Company: HashedIn By Deloitte.
 * Copyright (C) 2022 HashedIn By Deloitte
 */

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include "can_bus.h"
#include "../serial_interface/serial_config.h"
#include "../main.h"

/* mutex to lock cloud_data struct for wirte */
pthread_mutex_t can_bus_mutex = PTHREAD_MUTEX_INITIALIZER;

char *read_can_id_number(struct uart_device_struct *can_device)
{
    char *read_data = NULL;

    uart_writes(can_device, "0x02\r\n");

    sleep(1);

    uart_reads_chunk(can_device, &read_data, MAX_LEN_VIN);

    return read_data;
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

        uart_writes(&can_device, "0x0d\r\n");

        sleep(1);

        uart_reads_chunk(&can_device, &read_data, MAX_LEN_SPEED_DATA);

        cloud_data->can_data.speed = (uint8_t)atoi(read_data);

        uint8_t speed;

        pthread_mutex_unlock(&can_bus_mutex);
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

        uart_writes(&can_device, "0x00\r\n");

        sleep(30);

        uart_reads_chunk(&can_device, &read_data, MAX_LEN_SUPPORTED_DATA);

        cloud_data->can_data.supported_pids = (uint32_t)atoi(read_data);

        pthread_mutex_unlock(&can_bus_mutex);
    }
}

void *read_from_can(void *arg)
{
    char *read_data = NULL;
    pthread_t read_can_supported_thread, read_can_speed_thread;

    struct arg_struct *args = (struct arg_struct *)arg;
    struct uart_device_struct can_device = args->uart_device;
    struct cloud_data_struct *cloud_data = args->cloud_data;

    cloud_data->can_data.vin[MAX_LEN_VIN] = (uint8_t *)read_can_id_number(&can_device);

    pthread_create(&read_can_supported_thread, NULL, &read_can_supported_pid, &arg);
    pthread_create(&read_can_speed_thread, NULL, &read_can_speed_pid, &arg);

    pthread_join(read_can_supported_thread, NULL);
    pthread_join(read_can_speed_thread, NULL);
}