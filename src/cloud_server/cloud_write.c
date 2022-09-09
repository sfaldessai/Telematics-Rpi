/*
 * created at 2022-07-27 12:00.
 *
 * Company: HashedIn By Deloitte.
 * Copyright (C) 2022 HashedIn By Deloitte
 */

#include <stdio.h>
#include <unistd.h>
#include <sqlite3.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "cloud_server.h"
#include "../logger/logger.h"
#include "../database/db_handler.h"

struct timeval tval_start, tval_stop, tval_inServiceTime;
static bool isStartTime = false, isStopTime = false;

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

    /* Initializing logger */
    logger_setup();

    if (cloud_data->can_data.speed > 0 || cloud_data->gps_data.speed > 0 && !isStartTime) {
        gettimeofday(&tval_start, NULL);
        isStartTime = true;
    }

    if (cloud_data->can_data.speed <= 0 && cloud_data->gps_data.speed <= 0 && &tval_start > 0) {
        gettimeofday(&tval_stop, NULL);
        isStopTime = true;
    }

    /*if (&tval_stop < &tval_start) {

    }*/

    if (isStopTime) {
        timersub(&tval_stop, &tval_start, &tval_inServiceTime);
        cloud_data->service_time = (double)tval_inServiceTime.tv_sec + ((double)tval_inServiceTime.tv_usec / 1000000.0f);

        double current_inServiceTime = retrive_previous_inServiceTime() + cloud_data->service_time;
        insert_parameter_in_db(current_inServiceTime);
        cloud_data->service_time = current_inServiceTime;

        timerclear(&tval_start);
        timerclear(&tval_stop);
        isStartTime = false;
        isStopTime = false;
    }

    int rc = initialize_db();
    while (1)
    {
        if (cloud_data != NULL)
        {
            logger_info(CLOUD_LOG_MODULE_ID, "\tVIN = %s | CAN SPEED = %d | GPS SPEED = %f \n", cloud_data->can_data.vin,
                cloud_data->can_data.speed, cloud_data->gps_data.speed);
            logger_info(CLOUD_LOG_MODULE_ID, "\tMOTION = %d | VOLTAGE = %f | PTO = %d\n", cloud_data->client_controller_data.motion,
                        cloud_data->client_controller_data.voltage, cloud_data->client_controller_data.pto);
            logger_info(CLOUD_LOG_MODULE_ID, "\tLAT: %.4f %c", cloud_data->gps_data.latitude, cloud_data->gps_data.lat_cardinal_sign);
            logger_info(CLOUD_LOG_MODULE_ID, "\tLONG: %.4f %c\n", cloud_data->gps_data.longitude, cloud_data->gps_data.long_cardinal_sign);
            logger_info(CLOUD_LOG_MODULE_ID, "\tPDOP:%.2f\tHDOP:%.2f\tVDOP:%.2f\n", cloud_data->gps_data.pdop,
                        cloud_data->gps_data.hdop, cloud_data->gps_data.vdop);

            if (rc == SQLITE_OK)
            {
                int result = insert_telematics_data(cloud_data);
                if (result != 0)
                {
                    logger_info(CLOUD_LOG_MODULE_ID, "DB write failed, TODO: Write to file temporarily");
                }
            }
            sleep(2); /* Display data every 2 sec*/
        }
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
    struct can_data_struct can_data;

    gps_data.gps_time = "";
    gps_data.latitude = 0.0;
    gps_data.lat_cardinal_sign = 'N';
    gps_data.longitude = 0.0;
    gps_data.long_cardinal_sign = 'E';
    gps_data.hdop = 0.0;
    gps_data.pdop = 0.0;
    gps_data.vdop = 0.0;
    gps_data.speed = 0.0;

    client_controller_data.pto = 0;
    client_controller_data.motion = 0;
    client_controller_data.voltage = 0.0;

    memset(can_data.vin, '\0', MAX_LEN_VIN);
    can_data.speed = 0;
    memset(can_data.supported_pids, 0, CAN_PID_LENGTH * sizeof(uint32_t));

    cloud_data->gps_data = gps_data;
    cloud_data->client_controller_data = client_controller_data;
    cloud_data->can_data = can_data;
    cloud_data->service_time = 0.0;
}