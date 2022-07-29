/*
 * created at 2022-07-27 12:00.
 *
 * Company: HashedIn By Deloitte.
 * Copyright (C) 2022 HashedIn By Deloitte
 */

#include <stdio.h>
#include <unistd.h>
#include "cloud_server.h"

void *write_to_cloud(void *arg)
{
    struct cloud_data_struct *cloud_data = (struct cloud_data_struct *)arg;
    while (1)
    {
        sleep(2); /* Display data every 2 sec*/

        printf("\nSTM32 Data : %s\n", cloud_data->stm32_data.sensor_data);
        printf("\nLat: %.4f %c", cloud_data->gps_data.latitude, cloud_data->gps_data.lat_cardinal_sign);
        printf("\t Long: %.4f %c\n", cloud_data->gps_data.longitude, cloud_data->gps_data.long_cardinal_sign);
        printf("\nPDOP:%.2f\tHDOP:%.2f\tVDOP:%.2f\n", cloud_data->gps_data.pdop, cloud_data->gps_data.hdop, cloud_data->gps_data.vdop);
    }
}