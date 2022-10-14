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
#define MAX_READ_SIZE_CHECKSUM 50
#define CR 0x0d
#define NMEA_END_CHAR '\n'
#define SUCESS_CODE 1
#define HASH_SIGN 0x23
#define DOLLAR_SIGN 0x24

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
    client_controller_data.voltage = (float)error_code;
    client_controller_data.pto = (uint16_t)error_code;
    client_controller_data.motion = (uint16_t)error_code;
    client_controller_data.acc_x = error_code;
    client_controller_data.acc_y = error_code;
    client_controller_data.acc_z = error_code;

    /* update gps_data to cloud_data struct */
    pthread_mutex_lock(&cloud_data_mutex);
    cloud_data->client_controller_data = client_controller_data;
    pthread_mutex_unlock(&cloud_data_mutex);
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
    client_controller_data->motion = (uint16_t)atoi(stmc_data + 1);

    stmc_data = strchr(stmc_data + 1, COMMA);
    client_controller_data->voltage = (float)atof(stmc_data + 1);

    stmc_data = strchr(stmc_data + 1, COMMA);

    client_controller_data->pto = (uint16_t)atoi(stmc_data + 1);

    stmc_data = strchr(stmc_data + 1, COMMA);
    client_controller_data->acc_x = atoi(stmc_data + 1);

    stmc_data = strchr(stmc_data + 1, COMMA);
    client_controller_data->acc_y = atoi(stmc_data + 1);

    stmc_data = strchr(stmc_data + 1, COMMA);
    client_controller_data->acc_z = atoi(stmc_data + 1);
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
                
                 /* Checksum Read */
                for (size_t j = 0; j < 2; j++)
                {
                    read_data_len = uart_reads(&client_controller_device, &read_data, MAX_READ_SIZE);
                    if (read_data_len > 0)
                    {
                        i++;
                        stm32_serial_data[i] = read_data;
                    }
                }
                stm32_serial_data[i + 1] = '\0';

                logger_info(CC_LOG_MODULE_ID, "COMPLETE STM32 DATA: %s\n", stm32_serial_data);

                if (stm32_serial_data[1] == 'S' && stm32_serial_data[2] == 'T' && stm32_serial_data[3] == 'M' && stm32_serial_data[4] == 'C')
                {
                    get_client_controller_data(stm32_serial_data, &client_controller_data);

                    if (verify_stm32_checksum(stm32_serial_data)) {
                        /* update stm32 data to cloud_data struct which is used to combile all module data and send to cloud */
                        pthread_mutex_lock(&cloud_data_mutex);
                        cloud_data->client_controller_data = client_controller_data;
                        pthread_mutex_unlock(&cloud_data_mutex);
                    }
                    else {
                        logger_error(CC_LOG_MODULE_ID, "checksum error");
                    }
                }
                else
                {
                    update_cc_error_code(cloud_data, STM32_INVALID_DATA);
                }
            }
        }
    }
    uart_stop(&client_controller_device);
}

/*
 * Name : verify_stm32_checksum
 * Descriptoin: The verify_stm32_checksum function is for verifying STM32 checksum.
 * Input parameters:
 *                  const char *sentence : STM senetence data
 *
 * Output parameters: uint8_t: return 1 for valid and 0 for invalid
 */
bool verify_stm32_checksum(const char* sentence)
{
    int checksum = 0;
    uint8_t checksum_hex[8];

    if (strlen(sentence) > MAX_READ_SIZE_CHECKSUM || strchr(sentence, HASH_SIGN) == NULL || strchr(sentence, DOLLAR_SIGN) == NULL)
    {
        logger_error(MAIN_LOG_MODULE_ID, "Invalid STM32 sentence: %s\n", __func__);
        return 0;
    }
    while ('#' != *sentence && NMEA_END_CHAR != *sentence)
    {
        if (DOLLAR_SIGN == *sentence)
        {
            sentence = sentence + 1;
            continue;
        }
        if ('\0' == *sentence)
        {
            logger_error(MAIN_LOG_MODULE_ID, "Invalid STM32 sentence: %s\n", __func__);
            return 0;
        }
        checksum = checksum ^ (uint8_t)*sentence;
        sentence = sentence + 1;
    }

    sentence = sentence + 1;

    if (strlen(sentence) >= 2)
    {
        checksum_hex[0] = sentence[0];
        checksum_hex[1] = sentence[1];
        checksum_hex[2] = '\0';
    }
    else
    {
        logger_error(MAIN_LOG_MODULE_ID, " Invalid Checksum from STM32: %s\n", __func__);
        return 0;
    }

    uint16_t checksum_dec = hex_to_decimal(checksum_hex);
    if (checksum == checksum_dec)
    {
        return SUCESS_CODE;
    }
    else
    {
        return 0;
    }
}