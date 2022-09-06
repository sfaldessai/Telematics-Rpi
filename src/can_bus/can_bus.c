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
#include "../logger/logger.h"
#include "../../include/resource.h"

#define EQUALS_SIGN 0x3D

/*
 * Name : get_manufacturer_detail
 * Descriptoin: The get_manufacturer_detail function is for fetching manufaturer detail & vehicle type.
 * Input parameters:
 *                  uint8_t wmi : World manufaturer Identifier
 * Output parameters: char * : manufaturer detail & vehicle type
 */
char *get_manufacturer_detail(uint8_t *wmi)
{
    for (size_t i = 0; i < WMI_LIST_LEN; i++)
    {
        char wmi_key[CAN_FRAME_LENGTH];
        memset(wmi_key, '\0', sizeof(wmi_key));

        char *manufacturer_detail = strchr(manufacturers[i], EQUALS_SIGN);

        for (size_t j = 0; j < WMI_LEN; j++)
        {
            wmi_key[j] = manufacturers[i][j];
        }

        int result = strcmp((char *)wmi, wmi_key);
        if (result == 0)
        {
            return manufacturer_detail + 1;
        }
    }

    return NULL;
}

/*
 * Name : validate_vin
 * Descriptoin: The validate_vin function is for validating VIN.
 * Input parameters:
 *                  char *vin : Vehicle Identification Number 
 *                              Sample VIN = 5YJSA3DG9HFP14703
 * Output parameters: bool : true/false
 */
bool validate_vin(char *vin)
{
    if (strlen(vin) != VIN_LEN)
    {
        return false;
    }

    size_t sum = 0;
    for (size_t i = 0; i < VIN_LEN; i++)
    {
        size_t value;
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
    size_t check_digit_value = (size_t)check_digit - 48;

    if ((sum == 10 && check_digit == 'X') || (sum == check_digit_value))
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
 *                  void *arg : cloud_data_struct to update VIN data
 *
 * Output parameters: void
 */
void *read_can_id_number(void *arg)
{
    struct cloud_data_struct *cloud_data = (struct cloud_data_struct *)arg;

    /* TBD: read from can module once we get CAN module. Hardcaded for testing*/
    char *read_data = "A0000000000000001";

    /*
     *
     * TBD: Write a function to Request the CAN module or virtual CAN for PID 0x02 VIN data,
     * and get respose in read_data
     *
     */

    /* Copy 17 byte VIN data to cloud struct member for displaying on screen from deiplay thread */
    strncpy((char *)cloud_data->can_data.vin, read_data, MAX_LEN_VIN);

    return 0;
}

/*
 * Name : read_can_speed_pid
 *
 * Descriptoin: The read_can_speed_pid function is for fetching Vehicle speed PID data from the CAN module
 *
 * Input parameters:
 *                  void *arg : cloud_data_struct to update vehicle speed data
 *
 * Output parameters: void
 */
void *read_can_speed_pid(void *arg)
{
    struct cloud_data_struct *cloud_data = (struct cloud_data_struct *)arg;

    /* TBD: read from can once we get CAN module. Hardcaded for testing*/
    char *read_data = "85";

    while (1)
    {
        /*
         *
         * TBD: Write a function to Request the CAN module or virtual CAN for PID 0x0d vehicle speed data,
         * and get respose in read_data
         *
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
 *                  void *arg : cloud_data_struct to update supported pid data
 *
 * Output parameters: void
 */
void *read_can_supported_pid(void *arg)
{
    struct cloud_data_struct *cloud_data = (struct cloud_data_struct *)arg;

    /* TBD: read from can once we get CAN module. Hardcaded for testing*/
    char *read_data = "2147483647";

    while (1)
    {
        /*
         *
         * TBD: Write a function to Request the CAN module or virtual CAN for PID 0x00 Vehicle Supported PID data,
         * and get respose in read_data
         *
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
 *                  void *arg : cloud_data_struct to update CAN data
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