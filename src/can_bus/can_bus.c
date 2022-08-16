/*
 * created at 2022-07-29 14:30.
 *
 * Company: HashedIn By Deloitte.
 * Copyright (C) 2022 HashedIn By Deloitte
 */

#include <stdio.h>
#include "can_bus.h"

void read_can_id_number(void *arg)
{
    struct can_data_Struct v_in;
    unsigned int l_data;
    unsigned int l_pid;
    if (l_pid == 0x02)
    {
    }
}

// for every 1000ms read vehicle speed data
void *read_can_speed_pid(void *arg)
{
    struct can_data_Struct v_speed;
    unsigned int l_data;
    unsigned int l_pid;
    while (1)
    {
        l_pid = 0;
        l_data = 0;
        sleep(1);
        if (l_pid == 0x0d)
        {
        }
    }
}

// for every 30ms read PID 0x00
void *read_can_supported_pid(void *arg)
{
    struct can_data_Struct poll_data;
    unsigned int l_data;
    unsigned int l_pid;
    while (1)
    {
        l_pid = 0;
        l_data = 0;
        sleep(30);
        if (l_pid == 0x00)
        {
        }
    }
}

// for every 30ms read PID 0x00
void *read_canAble(void *arg)
{
    /* TODO */
    printf("%p", arg);
    return 0;
}

void *read_from_can(void *arg)
{
    char *read_data = NULL;

    struct arg_struct *args = (struct arg_struct *)arg;
    struct uart_device_struct can_device = args->uart_device;
    struct cloud_data_struct *cloud_data = args->cloud_data;
    struct can_data_Struct can_data;

    read_can_id_number(&can_device);

    do
    {
        // read_can_id_number(&can_data);
    } while (1);
    uart_stop(&can_device);
}