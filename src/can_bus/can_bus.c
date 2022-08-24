/*
 * created at 2022-07-29 14:30.
 *
 * Company: HashedIn By Deloitte.
 * Copyright (C) 2022 HashedIn By Deloitte
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "can_bus.h"
#include "../serial_interface/serial_config.h"
#include "../main.h"
#include "../logger/logger.h"
#include "../../include/resource.h"

#define EQUALS_SIGN 0x3D

/* mutex to lock cloud_data struct for wirte */
pthread_mutex_t can_bus_mutex = PTHREAD_MUTEX_INITIALIZER;

/*
 * Name : get_manufaturer_detail
 * Descriptoin: The get_manufaturer_detail function is for fetching manufaturer detail & vehicle type.
 * Input parameters:
 *                  char *wmi : World manufaturer Identifier
 * Output parameters: char * : manufaturer detail & vehicle type
 */
char *get_manufaturer_detail(char *wmi)
{
    for (size_t i = 0; i < strlen(manufacturers); i++)
    {
        uint8_t wmi_key[3];
        size_t index = 0;

        char *manufacturer_detail = strchr(manufacturers[i], EQUALS_SIGN);
        index = (int)(manufacturer_detail - manufacturers[i]);

        strncpy(wmi_key, manufacturers[i], WMI_LEN);

        if (wmi == wmi_key)
        {
            return manufacturer_detail + 1;
        }
    }
}

/*
 * Name : validate_VIN
 * Descriptoin: The validate_VIN function is for validating VIN.
 * Input parameters:
 *                  char *vin : Vehicle Identification Number
 * Output parameters: bool : true/false
 */
bool validate_VIN(char *vin)
{
    if (strlen(vin) != VIN_LEN)
    {
        return false;
    }
    /* VIN Numerical counterparts */
    static const int values[] = {1, 2, 3, 4, 5, 6, 7, 8, 0, 1, 2, 3, 4, 5, 0, 7, 0, 9, 2, 3, 4, 5, 6, 7, 8, 9};
    /* Weights */
    static const int weights[] = {8, 7, 6, 5, 4, 3, 2, 10, 0, 9, 8, 7, 6, 5, 4, 3, 2};

    int sum = 0;
    for (int i = 0; i < VIN_LEN; i++)
    {
        int value;
        char vin_character = vin[i];

        /* Numerical counterparts for VIN characters */
        if (vin_character > ('A' - 1) && vin_character < ('Z' + 1))
        {
            value = values[vin_character - 'A'];
            // don't allow 0
            if (!value)
            {
                return false;
            }
        }
        /* Numerical counterparts for VIN numeric */
        else if (vin_character > ('0' - 1) && vin_character < ('9' + 1))
        {
            value = vin_character - '0';
        }
        /* return false for bad characters */
        else
        {
            return false;
        }

        /*
         * Multiply Numerical counterparts new number with the assigned weight.
         * Sum the resulting products.
         */
        sum = sum + weights[i] * value;
    }

    /* Modulus the sum of the products by 11, to find the remainder. */
    sum = sum % 11;

    /* The check digit is the character on position 9 and can be a number from 0 to 9 and X (for 10) */
    char check_digit = vin[8];

    if ((sum == 10 && check_digit == 'X') || (sum == (check_digit - 48)))
    {
        return true;
    }
    else
    {
        return false;
    }
}

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

    struct arg_struct *args = (struct arg_struct *)arg;
    struct uart_device_struct can_device = args->uart_device;
    struct cloud_data_struct *cloud_data = args->cloud_data;

    /*
     *
     * TBD: Write function to Request the CAN module for PID 0x02 VIN data.
     *
     * uart_writes(can_device, "0x02\r\n");
     */

    /*
     *
     * TBD: Write function to Receive the VIN data from CAN module.
     *
     * uart_reads_chunk(can_device, &read_data, MAX_LEN_VIN);
     */

    /* Copy 17 byte VIN data to cloud struct member for displaying on screen from deiplay thread */
    strncpy(cloud_data->can_data.vin, read_data, MAX_LEN_VIN);
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

        /*
         *
         * TBD: Write function to Receive the Vehicle speed data from CAN module.
         *
         * uart_reads_chunk(&can_device, &read_data, MAX_LEN_SPEED_DATA);
         */

        /* Copy 1 byte (0-255) Vehicle speed data to cloud struct member for displaying on screen from deiplay thread */

        cloud_data->can_data.speed = (uint8_t)atoi(read_data);

        /* request next data after 1sec */
        sleep(1);
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

        /*
         *
         * TBD: Write function to Receive the Vehicle Supported PID data from CAN module.
         *
         * uart_reads_chunk(&can_device, &read_data, MAX_LEN_SPEED_DATA);
         */

        /* Copy 32 byte Vehicle Supported PID data to cloud struct member for displaying on screen from deiplay thread */
        cloud_data->can_data.supported_pids = (uint32_t)atoi(read_data);

        /* request next data after 30sec */
        sleep(30);
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
    /* Thread to fetch VIN. */
    pthread_create(read_can_vin_thread, NULL, &read_can_id_number, arg);
    /* Thread to fetch supported pid data. */
    pthread_create(read_can_supported_thread, NULL, &read_can_supported_pid, arg);
    /* Thread to fetch spedd pid data. */
    pthread_create(read_can_speed_thread, NULL, &read_can_speed_pid, arg);
}