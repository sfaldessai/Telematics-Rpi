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
#include "uart.h"

#define MAX_READ_SIZE 1
#define COMMA 0x2C
#define MAXSIZE 100 /* GPS at most, sends 80 or so chars per message string.*/
#define CR 0x0d

void get_lat_log(float *latitude)
{
    int degrees;

    degrees = (int)(*latitude / 100);
    *latitude = degrees + (float)(*latitude - degrees * 100) / 60;
}

void *read_from_gps(void *arg)
{
    char read_data;
    size_t read_data_len;
    unsigned int i;
    unsigned char nmea_data[MAXSIZE];
    char *gga_data = NULL;

    struct uart_device_struct *gps_device = (struct uart_device_struct *)arg;
    struct gps_data_struct gps_data;

    do
    {
        read_data_len = uart_reads(gps_device, &read_data, MAX_READ_SIZE); /* read char from serial port */

        if (read_data == '$')
        {
            /* GPS messages start with $ char */
            i = 0;
            nmea_data[i] = read_data;
            do
            {
                read_data_len = uart_reads(gps_device, &read_data, MAX_READ_SIZE);
                if ((read_data_len > 0) && (isalnum(read_data) || isspace(read_data) || ispunct(read_data)))
                {
                    i++;
                    nmea_data[i] = read_data;
                }
            } while (read_data != CR);

            nmea_data[i + 1] = '\0';

            /* Check if string we collected is the $GNGGA or $GPGGA message */
            if (nmea_data[3] == 'G' && nmea_data[4] == 'G' && nmea_data[5] == 'A')
            {
                /* Get UTC Time from GGA message */
                gga_data = strchr(nmea_data, ',');
                gps_data.gps_time = gga_data + 1;

                /* Get Latitude from GGA message */
                gga_data = strchr(gga_data + 1, ',');
                gps_data.latitude = atof(gga_data + 1);
                get_lat_log(&gps_data.latitude);

                /* Get Latitude cardinal sign from GGA message */
                gga_data = strchr(gga_data + 1, ',');
                gps_data.lat_cardinal_sign = gga_data[1];

                /* Get Longitude from GGA message */
                gga_data = strchr(gga_data + 1, ',');
                gps_data.longitude = atof(gga_data + 1);
                get_lat_log(&gps_data.longitude);

                /* Get Longitude cardinal sign from GGA message */
                gga_data = strchr(gga_data + 1, ',');
                gps_data.long_cardinal_sign = gga_data[1];

                if (gps_data.latitude == 0 && gps_data.longitude == 0)
                {
                    printf("\nInvalid GPS Data\n");
                }
                else
                {
                    printf("\nLat: %.4f %c \t Long: %.4f %c\n", gps_data.latitude, gps_data.lat_cardinal_sign, gps_data.longitude, gps_data.long_cardinal_sign);
                }
            }
        }
    } while (1);
    uart_stop(gps_device);
}
