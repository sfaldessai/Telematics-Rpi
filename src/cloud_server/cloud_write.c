/*
 * created at 2022-07-27 12:00.
 *
 * Company: HashedIn By Deloitte.
 * Copyright (C) 2022 HashedIn By Deloitte
 */

#include <stdio.h>
#include <unistd.h>
#include "cloud_server.h"
#include "../logger/logger.h"

#define CLOUD_LOG_MODULE_ID 6

void *write_to_cloud(void *arg)
{
    struct cloud_data_struct *cloud_data = (struct cloud_data_struct *)arg;

    logger_config_t cfg;
    logger_setup(&cfg);

    while (1)
    {
        logger_info(CLOUD_LOG_MODULE_ID, "\nSTM32 Sensor: %s\n", cloud_data->stm32_data.sensor_data);
        logger_info(CLOUD_LOG_MODULE_ID, "\nLat: %.4f %c", cloud_data->gps_data.latitude, cloud_data->gps_data.lat_cardinal_sign);
        logger_info(CLOUD_LOG_MODULE_ID, "\t Long: %.4f %c\n", cloud_data->gps_data.longitude, cloud_data->gps_data.long_cardinal_sign);
        logger_info(CLOUD_LOG_MODULE_ID, "\nPDOP:%.2f\tHDOP:%.2f\tVDOP:%.2f\n", cloud_data->gps_data.pdop,
                    cloud_data->gps_data.hdop, cloud_data->gps_data.vdop);
    }
}

void initialize_cloud_data(struct cloud_data_struct *cloud_data)
{
    struct gps_data_struct gps_data;
    struct stm32_data_struct stm32_data;

    gps_data.gps_time = "";
    gps_data.latitude = 0.0;
    gps_data.lat_cardinal_sign = 'N';
    gps_data.longitude = 0.0;
    gps_data.long_cardinal_sign = 'E';
    gps_data.hdop = 0.0;
    gps_data.pdop = 0.0;
    gps_data.vdop = 0.0;

    stm32_data.sensor_data = "";

    cloud_data->gps_data = gps_data;
    cloud_data->stm32_data = stm32_data;
}