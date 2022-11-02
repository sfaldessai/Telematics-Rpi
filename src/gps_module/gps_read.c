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
#include <math.h>
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

uint8_t is_ignition_on = IGNITION_ON;

/* mutex to lock cloud_data struct for wirte */
pthread_mutex_t cloud_data_gps_mutex = PTHREAD_MUTEX_INITIALIZER;

/*
 * Name : update_gps_error_code
 * Descriptoin: The update_gps_error_code function is for updating erro codes for gps struct member
 * Input parameters: struct cloud_data_struct * : clout struct to update gps data member
 *                   int error_code : error code to update
 * Output parameters: void
 */
void update_gps_error_code(struct cloud_data_struct *cloud_data, int error_code)
{
    struct gps_data_struct gps_data;

    gps_data.hdop = error_code;
    gps_data.vdop = error_code;
    gps_data.pdop = error_code;
    gps_data.latitude = error_code;
    gps_data.longitude = error_code;
    gps_data.speed = error_code;

    /* update gps_data to cloud_data struct */
    pthread_mutex_lock(&cloud_data_gps_mutex);
    cloud_data->gps_data = gps_data;
    pthread_mutex_unlock(&cloud_data_gps_mutex);
}

/*
 * Name : nmea_verify_checksum
 * Descriptoin: The nmea_verify_checksum function is for verifying GPS checksum.
 * Input parameters:
 *                  const char *sentence : NMEA senetence
 *
 * Output parameters: uint8_t: return 1 for invalid and 0 for valid
 */
int nmea_verify_checksum(const char *sentence,int* gps)
{
    int checksum = 0;
    uint8_t gps_checksum_hex[8];

    if (strlen(sentence) > MAX_READ_SIZE || strchr(sentence, ASTERISK_SIGN) == NULL
    	||strchr(sentence, HASH_SIGN) == NULL || strchr(sentence, DOLLAR_SIGN) == NULL)
    {
    	{
     if(gps == 1)
     	{
    	 	 logger_info(GPS_LOG_MODULE_ID, "Invalid NMEA sentence: %s\n", __func__);
    	 	 return GPS_NMEA_SENTENCE_CHECKSUM_ERROR;
     	}
     	else
     	 {
        	logger_info(GPS_LOG_MODULE_ID, "Invalid STM32 sentence: %s\n", __func__);
        	return STM_CHECKSUM_ERROR;
         }
    }
    while (('*' != *sentence || '#' != *sentence) && NMEA_END_CHAR != *sentence)
    {
        if (DOLLAR_SIGN == *sentence)
        {
            sentence = sentence + 1;
            continue;
        }
        if ('\0' == *sentence)
        {
            logger_info(GPS_LOG_MODULE_ID, "Invalid NMEA sentence: %s\n", __func__);
            return GPS_NMEA_SENTENCE_CHECKSUM_ERROR;
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

    	if(stm == 2)
    		{
            	logger_info(GPS_LOG_MODULE_ID, "Invalid NMEA sentence: %s\n", __func__);
            	return STM_NMEA_SENTENCE_CHECKSUM_ERROR;
            }
    	else
    		{
            	logger_info(GPS_LOG_MODULE_ID, "Invalid STM32 sentence: %s\n", __func__);
            	return STM_CHECKSUM_ERROR;
    		}
    else
    {
        logger_info(GPS_LOG_MODULE_ID, " Invalid Checksum from GPS: %s\n", __func__);
        return GPS_NMEA_SENTENCE_CHECKSUM_ERROR;
    }
    }

    uint16_t gps_checksum_dec = hex_to_decimal(gps_checksum_hex);
    if (checksum == gps_checksum_dec)
    {
        return SUCESS_CODE;
    }
    else
    {
        return GPS_NMEA_SENTENCE_CHECKSUM_ERROR;
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
 * Output parameters: int return success/error code
 */
int get_gps_data(char *nmea_data, struct gps_data_struct *gps_data)
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

        if (gga_data[1] == 'S' || gga_data[1] == 's')
        {
            gps_data->latitude = -1 * gps_data->latitude;
        }

        /* Get Longitude from GGA message */
        gga_data = strchr(gga_data + 1, COMMA);
        gps_data->longitude = atof(gga_data + 1);
        get_lat_log(&gps_data->longitude);

        /* Get Longitude cardinal sign from GGA message */
        gga_data = strchr(gga_data + 1, COMMA);

        if (gga_data[1] == 'W' || gga_data[1] == 'w')
        {
            gps_data->longitude = -1 * gps_data->longitude;
        }

        gga_data = strchr(gga_data + 1, COMMA);

        /* GPS quality indicator (0=invalid; 1=GPS fix; 2=Diff. GPS fix) */
        int gps_quality = atoi(gga_data + 1);

        gga_data = strchr(gga_data + 1, COMMA);

        /* Number of satellites in use [not those in view] */
        /* int number_of_satellites = atoi(gga_data + 1); */

        gga_data = strchr(gga_data + 1, COMMA);

        /* Horizontal dilution of position */
        double hdop = atof(gga_data + 1);
        if (gps_quality <= 0 || hdop >= INVALID_DOP_VALUE)
        {
            return GPS_INVALID_QUALITY;
        }
        else if (hdop == NO_SIGNAL_DOP_VALUE)
        {
            return LOST_GPS_SIGNAL_ERROR;
        }
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

        if (gps_data->hdop == NO_SIGNAL_DOP_VALUE)
        {
            return LOST_GPS_SIGNAL_ERROR;
        }
        else if (gps_data->hdop >= INVALID_DOP_VALUE)
        {
            return GPS_INVALID_QUALITY;
        }
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
    return SUCESS_CODE;
}

/*
 * Name : send_ubx_cfg_command
 * Descriptoin: The send_ubx_cfg_command function is for sending ubx commands or message for configuring m8n gps module,
 *
 * Input parameters: struct uart_device_struct gps_device : serial port
 *                   const uint8_t *cmd : ubx command
 *                   uint8_t size : message length
 *
 * Output parameters: int : # byte written
 */
int send_ubx_cfg_command(struct uart_device_struct gps_device, const uint8_t *cmd, uint8_t size)
{
    uint8_t ubx_cmd[size];

    for (size_t i = 0; i < size; i++)
    {
        ubx_cmd[i] = cmd[i];
    }

    /* chesum calculation */
    uint8_t CK_A = 0, CK_B = 0;
    for (int i = 2; i < size - 2; i++)
    {
        CK_A = CK_A + ubx_cmd[i];
        CK_B = CK_B + CK_A;
    }
    /* adding checksum */
    ubx_cmd[size - 2] = CK_A;
    ubx_cmd[size - 1] = CK_B;

    int bytes = uart_gps_write(&gps_device, ubx_cmd, size);
    if (bytes == size)
    {
        logger_info(GPS_LOG_MODULE_ID, "%d bytes written to the gps\n", size);
    }
    else
    {
        logger_error(GPS_LOG_MODULE_ID, "failed to write %d bytes to the gps\n", size);
        return GPS_INITIAL_CONFIGURATION_WRITE_FAILED;
    }
    return SUCESS_CODE;
}

/*
 * Name : initialize_gps_module
 * Descriptoin: The initialize_gps_module function is for sending initial ubx commands or message for configuring m8n gps module,
 *
 * Input parameters: struct uart_device_struct gps_device : serial port
 *
 *
 * Output parameters: int
 */
int initialize_gps_module(struct uart_device_struct gps_device)
{
    /* Start GNSS with hot boot */
    int bytes = send_ubx_cfg_command(gps_device, set_gnss_start, GNSS_STOP_START_CMD_LEN);

    if (bytes == SUCESS_CODE)
    {
        /* Turn on NMEA sentence */
        send_ubx_cfg_command(gps_device, set_NMEA_sentence_on, NMEA_SENTENCE_CMD_LEN);
    }
    if (bytes == SUCESS_CODE)
    {
        /* Turn on power save mode */
        send_ubx_cfg_command(gps_device, set_power_save_mode, POWER_SAVE_MODE_CMD_LEN);
    }
    if (bytes == SUCESS_CODE)
    {
        /* Turn on assistnow feature */
        send_ubx_cfg_command(gps_device, set_assistNow_autonomous, ASSIST_NOW_AUTONOMOUS_CMD_LEN);
    }
    if (bytes == SUCESS_CODE)
    {
        /* Save Configuration */
        send_ubx_cfg_command(gps_device, save_configuration, SAVE_CONFIG_CMD_LEN);
    }

    if (bytes == GPS_INITIAL_CONFIGURATION_WRITE_FAILED)
    {
        return GPS_INITIAL_CONFIGURATION_WRITE_FAILED;
    }
    return SUCESS_CODE;
}

/*
 * Name : ignition_on
 * Descriptoin: The ignition_on function is for start or wakeup gnss when ignition on
 *
 * Input parameters: struct uart_device_struct gps_device : serial port
 *
 *
 * Output parameters: void
 */
void ignition_on(struct uart_device_struct gps_device)
{
    int byte = send_ubx_cfg_command(gps_device, set_gnss_start, GNSS_STOP_START_CMD_LEN);
    if (byte == GNSS_STOP_START_CMD_LEN)
    {
        is_ignition_on = IGNITION_ON;
    }
}

/*
 * Name : ignition_off
 * Descriptoin: The ignition_off function is for stop or sleep gnss when ignition off
 *
 * Input parameters: struct uart_device_struct gps_device : serial port
 *
 *
 * Output parameters: void
 */
void ignition_off(struct uart_device_struct gps_device)
{
    int byte = send_ubx_cfg_command(gps_device, set_gnss_stop, GNSS_STOP_START_CMD_LEN);
    if (byte == GNSS_STOP_START_CMD_LEN)
    {
        is_ignition_on = IGNITION_OFF;
    }
}

/*
 * Name : gps_data_processing
 * Descriptoin: The gps_data_processing function is for processing NMEA sentence
 * Input parameters: struct cloud_data_struct *cloud_data : cloud_data member to update gps data
 *                   char *read_data : NMEA sentence data
 * Output parameters: int
 */
int gps_data_processing(char *read_data, struct gps_data_struct *gps_data)
{
    logger_info(GPS_LOG_MODULE_ID, "COMPLETE GPS DATA: %s\n", read_data);

    int is_valid_checksum = nmea_verify_checksum(read_data);

    if (is_valid_checksum == SUCESS_CODE)
    {
        return get_gps_data(read_data, gps_data);
    }
    else
    {
        return is_valid_checksum;
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

    /* gps initial configuration */
    int rc = initialize_gps_module(gps_device);
    if (rc != SUCESS_CODE)
    {
        update_gps_error_code(cloud_data, rc);
    }

    do
    {
        if (cloud_data->client_controller_data.voltage <= VOLTAGE_THRESHOLD)
        {
            if (IGNITION_ON != is_ignition_on)
            {
                /* turn on gps when ignition on */
                ignition_on(gps_device);
            }

            read_data_len = uart_reads_chunk(&gps_device, read_data, MAX_READ_SIZE); /* read char from serial port */

            if (read_data_len > 0)
            {
                rc = gps_data_processing(read_data, &gps_data);
                if (rc != SUCESS_CODE)
                {
                    update_gps_error_code(cloud_data, rc);
                    gps_data = cloud_data->gps_data;
                }
                else
                {
                    /* update gps_data to cloud_data struct */
                    pthread_mutex_lock(&cloud_data_gps_mutex);
                    cloud_data->gps_data = gps_data;
                    pthread_mutex_unlock(&cloud_data_gps_mutex);
                }
            }
            else if (read_data_len == 0 && gps_device.fd > 0)
            {
                uart_stop(&gps_device);
                update_gps_error_code(cloud_data, GPS_DEVICE_DISCONNECTED);
            }
            else if (gps_device.fd <= 0)
            {
                sleep(3); /* Re-connecting in 3 seconds */
                uart_setup(&gps_device, GPS_MODULE, B9600, true);
                if (gps_device.fd <= 0)
                {
                    update_gps_error_code(cloud_data, FAILED_TO_OPEN_GPS_DEVICE);
                }
            }
        }
        else if (is_ignition_on != IGNITION_OFF)
        {
            /* turn on gps when ignition off */
            ignition_off(gps_device);
            logger_info(GPS_LOG_MODULE_ID, "GNSS POWER TURNING OFF, and voltage value %f\n", cloud_data->client_controller_data.voltage);

            update_gps_error_code(cloud_data, GPS_POWERED_OFF);
        }
        else
        {
            logger_info(GPS_LOG_MODULE_ID, "GNSS POWER is OFF, and voltage value %f\n", cloud_data->client_controller_data.voltage);
        }
    } while (1);
    uart_stop(&gps_device);
}
