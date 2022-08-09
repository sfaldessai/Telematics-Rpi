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
#define COMMA 0x2C
#define CR 0x0d
#define GPS_LOG_MODULE_ID 2

pthread_mutex_t cloud_data_gps_mutex = PTHREAD_MUTEX_INITIALIZER;

void get_lat_log(double *latitude)
{
    int degrees;

    degrees = (int)(*latitude / 100);
    *latitude = degrees + (*latitude - degrees * 100) / 60;
}

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

void get_gps_data(char *nmea_data, struct gps_data_struct *gps_data)
{
    char *gga_data = NULL;
    char *gsa_data = NULL;

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
}

void *read_from_gps(void *arg)
{
    char *read_data = NULL;
    int read_data_len = 0;

    struct arg_struct *args = (struct arg_struct *)arg;
    struct uart_device_struct gps_device = args->uart_device;
    struct cloud_data_struct *cloud_data = args->cloud_data;
    struct gps_data_struct gps_data;

    do
    {
        read_data_len = uart_reads_chunk(&gps_device, &read_data, MAX_READ_SIZE); /* read char from serial port */

        if (read_data_len > 0)
        {
            get_gps_data(read_data, &gps_data);

            /* update gps_data to cloud_data struct */
            pthread_mutex_lock(&cloud_data_gps_mutex);
            cloud_data->gps_data = gps_data;
            pthread_mutex_unlock(&cloud_data_gps_mutex);
        }

    } while (1);
    uart_stop(&gps_device);
}
