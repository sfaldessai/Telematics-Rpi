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

/* mutex to lock can_module_lock */
pthread_mutex_t can_module_lock = PTHREAD_MUTEX_INITIALIZER;

#define EQUALS_SIGN 0x3D

int sockfd = 0;

/*
 * Name : get_manufaturer_detail
 * Descriptoin: The get_manufaturer_detail function is for fetching manufaturer detail & vehicle type.
 * Input parameters:
 *                  uint8_t wmi : World manufaturer Identifier
 * Output parameters: char * : manufaturer detail & vehicle type
 */
char *get_manufaturer_detail(uint8_t *wmi)
{
    for (size_t i = 0; i < WMI_LIST_LEN; i++)
    {
        char wmi_key[WMI_LEN + 1];

        char *manufacturer_detail = strchr(manufacturers[i], EQUALS_SIGN);

        strncpy(wmi_key, manufacturers[i], 3);
        wmi_key[WMI_LEN + 1] = '\0';

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
 * Output parameters: bool : true/false
 */
bool validate_vin(char *vin)
{
    if (strlen(vin) != VIN_LEN)
    {
        return false;
    }
    /* VIN Numerical counterparts */
    static const size_t values[] = {1, 2, 3, 4, 5, 6, 7, 8, 0, 1, 2, 3, 4, 5, 0, 7, 0, 9, 2, 3, 4, 5, 6, 7, 8, 9};
    /* Weights */
    static const size_t weights[] = {8, 7, 6, 5, 4, 3, 2, 10, 0, 9, 8, 7, 6, 5, 4, 3, 2};

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
 * Name : can_request_response
 * Descriptoin: The can_request_response function is for requesting PID data to can module and get response from the CAN module.
 * Input parameters:
 *                  struct can_frame * : response frame, updating from can response frame
 *                  size_t frame_length : number of can response frames - 3 frames for VIN, 1 frame for speed and supported pid
 *                  struct can_frame : can request frame which contains requets PID details
 * Output parameters: void
 */
void can_request_response(struct can_frame *frame, size_t frame_length, struct can_frame request_frame)
{
    struct can_frame response_frame;

    /* locking read write for can data synchronization */
    pthread_mutex_lock(&can_module_lock);

    transmit_can_data(sockfd, request_frame);

    log_can_data(request_frame, CAN_REQUEST);

    sleep(0.2);

    for (size_t i = 0; i < frame_length; i++)
    {
        receive_can_data(sockfd, &response_frame);
        log_can_data(response_frame, CAN_RESPONSE);
        frame[i] = response_frame;
    }

    pthread_mutex_unlock(&can_module_lock);
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
    struct can_frame vin_frame[VIN_DATA_FRAME], request_frame;
    struct cloud_data_struct *cloud_data = (struct cloud_data_struct *)arg;

    /* TBD: read from can module once we get CAN module. Hardcaded for testing*/
    char read_data[VIN_LEN];

    /* prepare CAN request frame */
    get_request_frame(&request_frame, VIN_PID, VIN_MODE);

    /* Send Request and get response for PID 0x02 */
    can_request_response(vin_frame, VIN_DATA_FRAME, request_frame);

    size_t j = 0;
    /* TODO: convert vin bytes to decimal and assign to cloud_data->can_data.vin */
    for (size_t i = 5; i < CAN_FRAME_LENGTH; i++)
    {
        read_data[j] = (char) vin_frame[0].data[i];
        j = j + 1;
    }
    for (size_t i = 1; i < CAN_FRAME_LENGTH; i++)
    {
        read_data[j] = (char) vin_frame[1].data[i];
        j = j + 1;
    }
    for (size_t i = 1; i < CAN_FRAME_LENGTH; i++)
    {
        read_data[j] = (char) vin_frame[2].data[i];
        j = j + 1;
    }

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
    struct can_frame speed_frame[SPEED_DATA_FRAME], request_frame;
    struct cloud_data_struct *cloud_data = (struct cloud_data_struct *)arg;

    /* TBD: read from can once we get CAN module. Hardcaded for testing*/
    char *read_data = "85";

    while (1)
    {
        /* Copy 1 byte (0-255) Vehicle speed data to cloud struct member for displaying on screen from deiplay thread */

        /* prepare CAN request frame */
        get_request_frame(&request_frame, SPEED_PID, LIVE_DATA_MODE);

        /* Send Request and get response for PID 0x0D */
        can_request_response(speed_frame, SPEED_DATA_FRAME, request_frame);

        if (speed_frame[0].data[2] == SPEED_PID)
        {
            cloud_data->can_data.speed = (uint8_t)speed_frame[0].data[3];

            logger_info(CAN_LOG_MODULE_ID, "CAN Vehicle Speed: %d", cloud_data->can_data.speed);
        }

        /* request next data after 1sec */
        sleep(1);
    }
    close_socket(&sockfd);
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
    struct can_frame supported_frame[SUPPORTED_DATA_FRAME], request_frame;
    struct cloud_data_struct *cloud_data = (struct cloud_data_struct *)arg;

    /* TBD: read from can once we get CAN module. Hardcaded for testing*/
    char *read_data = "2147483647";

    while (1)
    {
        /* prepare CAN request frame */
        get_request_frame(&request_frame, SUPPORTED_PID, LIVE_DATA_MODE);

        /* Send Request and get response for PID 0x00 */
        can_request_response(supported_frame, SUPPORTED_DATA_FRAME, request_frame);

        /* TODO: convert supported pid response bytes to binary and assign to can data can_data.supported_pids */

        /* Copy 32 byte Vehicle Supported PID data to cloud struct member for displaying on screen from deiplay thread */
        cloud_data->can_data.supported_pids = (uint32_t)atoi(read_data);

        /* request next data after 30sec */
        sleep(30);
    }
    close_socket(&sockfd);
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

    setup_can_socket(&sockfd);

    /* Thread to fetch VIN. */
    pthread_create(read_can_vin_thread, NULL, &read_can_id_number, arg);
    /* Thread to fetch supported pid data. */
    pthread_create(read_can_supported_thread, NULL, &read_can_supported_pid, arg);
    /* Thread to fetch spedd pid data. */
    pthread_create(read_can_speed_thread, NULL, &read_can_speed_pid, arg);
}