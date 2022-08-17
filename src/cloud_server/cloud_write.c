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

/*
 * Name : write_to_cloud
 * Descriptoin: The write_to_cloud function is for diplaying/monitoring vehicle informtion from
 *              STM32 microcontroller, GPS Module, and CAN Bus over the UART protocol.
 * Input parameters: struct cloud_data_struct *
 * Output parameters: void
 */
void *write_to_cloud(void *arg)
{
    struct cloud_data_struct *cloud_data = (struct cloud_data_struct *)arg;

    logger_config_t cfg;
    logger_setup(&cfg);

    while (1)
    {
        logger_info(CLOUD_LOG_MODULE_ID, "\n motion = %d | voltage = %f | pto = %d\n",cloud_data->client_controller_data.motion,
            cloud_data->client_controller_data.voltage, cloud_data->client_controller_data.pto);
        logger_info(CLOUD_LOG_MODULE_ID, "\nLat: %.4f %c", cloud_data->gps_data.latitude, cloud_data->gps_data.lat_cardinal_sign);
        logger_info(CLOUD_LOG_MODULE_ID, "\t Long: %.4f %c\n", cloud_data->gps_data.longitude, cloud_data->gps_data.long_cardinal_sign);
        logger_info(CLOUD_LOG_MODULE_ID, "\nPDOP:%.2f\tHDOP:%.2f\tVDOP:%.2f\n", cloud_data->gps_data.pdop,
                    cloud_data->gps_data.hdop, cloud_data->gps_data.vdop);
        sleep(2); /* Display data every 2 sec*/

    }
}

/*
 * Name : initialize_cloud_data
 * Descriptoin: The initialize_cloud_data function is for initializing vehicle informtion for the 1st time.
 * Input parameters: struct cloud_data_struct *
 * Output parameters: void
 */
void initialize_cloud_data(struct cloud_data_struct *cloud_data)
{
    struct gps_data_struct gps_data;
    struct client_controller_data_struct client_controller_data;

    gps_data.gps_time = "";
    gps_data.latitude = 0.0;
    gps_data.lat_cardinal_sign = 'N';
    gps_data.longitude = 0.0;
    gps_data.long_cardinal_sign = 'E';
    gps_data.hdop = 0.0;
    gps_data.pdop = 0.0;
    gps_data.vdop = 0.0;

    client_controller_data.pto = 0;
    client_controller_data.motion = 0;
    client_controller_data.voltage = 0.0;

    cloud_data->gps_data = gps_data;
    cloud_data->client_controller_data = client_controller_data;
}