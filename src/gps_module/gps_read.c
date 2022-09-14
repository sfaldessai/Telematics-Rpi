/*
 * created at 2022-07-25 11:30.
 *
 * Company: HashedIn By Deloitte.
 * Copyright (C) 2022 HashedIn By Deloitte
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <termios.h>
#include <pthread.h>
#include "gps_module.h"
#include "../serial_interface/serial_config.h"
#include "../main.h"

#define MAX_READ_SIZE 80 /* GPS at most, sends 80 or so chars per message string.*/
#define HUNDRED 100
#define SIXTY 60
#define COMMA 0x2C
#define CR 0x0d
#define SPEED_POS ((uint8_t)7)
#define GPS_KMPH_PER_KNOT 1.852
#define NMEA_END_CHAR '\n'
#define SUCESS_CODE 0
#define ASTERISK_SIGN 0x2A
#define DOLLAR_SIGN 0x24

/* mutex to lock cloud_data struct for wirte */
pthread_mutex_t cloud_data_gps_mutex = PTHREAD_MUTEX_INITIALIZER;

/*
 * Name : nmea_verify_checksum
 * Descriptoin: The nmea_verify_checksum function is for verifying GPS checksum.
 * Input parameters:
 *                  const char *sentence : NMEA senetence
 *                                        
 * Output parameters: uint8_t: return 1 for invalid and 0 for valid
 */
uint8_t nmea_verify_checksum(const char *sentence)
{
    uint8_t checksum = 0, gps_checksum_hex[8];

    if (strlen(sentence) > MAX_READ_SIZE || strchr(sentence, ASTERISK_SIGN) == NULL || strchr(sentence, DOLLAR_SIGN) == NULL)
    {
        logger_info(GPS_LOG_MODULE_ID, "Invalid NMEA sentence: %s\n", __func__);
        return 1;
    }
    while ('*' != *sentence && NMEA_END_CHAR != *sentence)
    {
        if (DOLLAR_SIGN == *sentence)
        {
            sentence = sentence + 1;
            continue;
        }
        if ('\0' == *sentence)
        {
            logger_info(GPS_LOG_MODULE_ID, "Invalid NMEA sentence: %s\n", __func__);
            return 1;
        }
        checksum = checksum ^ (uint8_t)*sentence;
        sentence = sentence + 1;
    }
    sentence = sentence + 1;

    if (strlen(sentence) >= 2)
    {
        gps_checksum_hex[0] = sentence[0];
        gps_checksum_hex[1] = sentence[1];
        gps_checksum_hex[2] = '\0';
    }
    else
    {
        logger_info(GPS_LOG_MODULE_ID, " Invalid Checksum from GPS: %s\n", __func__);
        return 1;
    }

    uint16_t gps_checksum_dec = hex_to_decimal(gps_checksum_hex);
    if (checksum == gps_checksum_dec)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

/*
 * Name : get_lat_log
 * Descriptoin: The get_lat_log function is for calculating latitude
 *              and lagitude value and store it in latitude.
 * Input parameters: double * (which holds result latitude data)
 * Output parameters: void
 */
void get_lat_log(double *latitude)
{
    int degrees;
    degrees = (int)(*latitude / HUNDRED);
    *latitude = degrees + (*latitude - degrees * HUNDRED) / SIXTY;
}

/*
 * Name : get_dops
 * Descriptoin: The get_dops function is for extracting latitude, longitude,
 *              and dop data from gps module data/sentences.
 *              GGA sentence is for latitude & longitude.
 *              GSA sentence is for DOP
 * Input parameters: char ** (which hold extracted dop characters)
 *                   char * (gps module GSA sentence)
 * Output parameters: void
 */
void get_dops(char **gsa_data, char *nmea_data)
{
    unsigned int k = 0;

    *gsa_data = strchr(nmea_data, COMMA);
    /* DOP data start from 15th COMMA. */
    while (k < 14)
    {
        *gsa_data = strchr(*gsa_data + 1, COMMA);
        k++;
    }
}

/*
 * Name : get_gps_param_by_position
 * Descriptoin: The get_gps_param_by_position function is to get requested parameter from GPS sentence
 * Input parameters: char ** (extracted param)
 *                   char * (gps module sentence)
 *                   uint8_t    (param position)
 * Output parameters: void
 */
void get_gps_param_by_position(char **param, char *nmea_data, uint8_t position)
{
    uint8_t k = 0;
    if (position > 0)
    {
        *param = strchr(nmea_data, COMMA);

        while (k < position - 1)
        {
            *param = strchr(*param + 1, COMMA);
            k++;
        }
        *param = *param + 1;
    }
}

/*
 * Name : get_gps_data
 * Descriptoin: The get_gps_data function is for extracting latitude, longitude,
 *              and dop data from gps module data/sentences.
 *              GGA sentence is for latitude & longitude.
 *              GSA sentence is for DOP
 *              VTG sentence is for speed
 *              RMC sentence is for speed
 * Input parameters: char * (gps module sentence)
 *                   struct gps_data_struct *
 * Output parameters: void
 */
void get_gps_data(char *nmea_data, struct gps_data_struct *gps_data)
{
    char *gga_data = NULL;
    char *gsa_data = NULL;
    char *vtg_data = NULL;
    char *rmc_data = NULL;

    if (nmea_data[3] == 'G' && nmea_data[4] == 'G' && nmea_data[5] == 'A')
    {
        /* Get UTC Time from GGA message */
        gga_data = strchr(nmea_data, COMMA);
        gps_data->gps_time = gga_data + 1;

        /* Get Latitude from GGA message */
        gga_data = strchr(gga_data + 1, COMMA);
        gps_data->latitude = atof(gga_data + 1);
        get_lat_log(&gps_data->latitude);

        /* Get Latitude cardinal sign from GGA message */
        gga_data = strchr(gga_data + 1, COMMA);
        gps_data->lat_cardinal_sign = gga_data[1];

        /* Get Longitude from GGA message */
        gga_data = strchr(gga_data + 1, COMMA);
        gps_data->longitude = atof(gga_data + 1);
        get_lat_log(&gps_data->longitude);

        /* Get Longitude cardinal sign from GGA message */
        gga_data = strchr(gga_data + 1, COMMA);
        gps_data->long_cardinal_sign = gga_data[1];
    }
    else if (nmea_data[3] == 'G' && nmea_data[4] == 'S' && nmea_data[5] == 'A')
    {
        /* Get gps PDOP from GGA message */
        get_dops(&gsa_data, nmea_data);
        gps_data->pdop = atof(gsa_data + 1);

        /* Get gps HDOP from GGA message */
        gsa_data = strchr(gsa_data + 1, COMMA);
        gps_data->hdop = atof(gsa_data + 1);

        /* Get gps VDOP from GGA message */
        gsa_data = strchr(gsa_data + 1, COMMA);
        gps_data->vdop = atof(gsa_data + 1);
    }
    else if (nmea_data[3] == 'R' && nmea_data[4] == 'M' && nmea_data[5] == 'C')
    {
        /* Get Speed from RMC message*/
        get_gps_param_by_position(&rmc_data, nmea_data, SPEED_POS);
        gps_data->speed = atof(rmc_data) * GPS_KMPH_PER_KNOT;
    }
    else if (nmea_data[3] == 'V' && nmea_data[4] == 'T' && nmea_data[5] == 'G')
    {
        /* Get Speed from VTG message*/
        get_gps_param_by_position(&vtg_data, nmea_data, SPEED_POS);
        gps_data->speed = atof(vtg_data);
    }
}

/*
 * Name : read_from_gps
 * Descriptoin: The read_from_gps function is for reading vehicle location
 *              data from the neo GPS module over the UART protocol.
 * Input parameters: struct arg_struct *
 * Output parameters: void
 */
void *read_from_gps(void *arg)
{
    char read_data[MAX_READ_SIZE];
    int read_data_len = 0;

    struct arg_struct *args = (struct arg_struct *)arg;
    struct uart_device_struct gps_device = args->uart_device;
    struct cloud_data_struct *cloud_data = args->cloud_data;
    struct gps_data_struct gps_data;

    do
    {
        read_data_len = uart_reads_chunk(&gps_device, read_data, MAX_READ_SIZE); /* read char from serial port */

        if (read_data_len > 0)
        {
            logger_info(GPS_LOG_MODULE_ID, "COMPLETE GPS DATA: %s\n", read_data);

            uint8_t is_valid_checksum = nmea_verify_checksum(read_data);

            if (is_valid_checksum == SUCESS_CODE)
            {
                get_gps_data(read_data, &gps_data);

                /* update gps_data to cloud_data struct */
                pthread_mutex_lock(&cloud_data_gps_mutex);
                cloud_data->gps_data = gps_data;
                pthread_mutex_unlock(&cloud_data_gps_mutex);
            }
        }
    } while (1);
    uart_stop(&gps_device);
}
