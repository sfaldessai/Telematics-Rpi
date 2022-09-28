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

#define SPEED_THRESHOLD 0

time_t tval_start, tval_stop,dtval_start,dtval_stop;
bool service_timer_start = false,distance_timer_start=false;
int prev_speed = 0;

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

    int rc = initialize_db();
    while (1)
    {
        if (cloud_data != NULL)
        {
            calculate_service_time(cloud_data);
            calculate_distance_travelled(cloud_data);
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
            sleep(1); /* Display data every 1 sec*/
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

    uint8_t service_value[COLUMN_VALUE_MAX_LEN];
    get_single_column_value(VEHICLE_IN_SERVICE, SORT_BY_DESC, service_value);
    cloud_data->service_time = (int) atoi((char*)service_value);

    uint8_t distance_value[COLUMN_VALUE_MAX_LEN];
    get_single_column_value(DISTANCE_TRAVLLED, SORT_BY_DESC, distance_value);
    cloud_data->distance_travelled = (float)atof((char*)distance_value);
}

/*
 * Name : caluclate_service_time
 * Description: This function calculates the service time based on the CAN speed and GPS Speed
 * Input parameters: struct cloud_data_struct *
 * Output parameters: void
 */
void calculate_service_time(struct cloud_data_struct* cloud_data) {
    /* Start the Service time when the Either GPS or CAN speed is greater than 0 */
    if (cloud_data->can_data.speed > SPEED_THRESHOLD || cloud_data->gps_data.speed > SPEED_THRESHOLD) {
        if (!service_timer_start) {
            service_timer_start = true;
            tval_start = time(NULL);
        }
    } /* Test and check if both CAN and GPS speed is required to handle stop timer when either of the modules malfunction */
    else if (cloud_data->can_data.speed == SPEED_THRESHOLD && cloud_data->gps_data.speed == SPEED_THRESHOLD && service_timer_start) {
        tval_stop = time(NULL);
        int tval_inServiceTime = (tval_stop - tval_start);
        cloud_data->service_time = cloud_data->service_time +  tval_inServiceTime;
        service_timer_start = false;
    }
}

void calculate_distance_travelled(struct cloud_data_struct* cloud_data) {
    if (cloud_data->can_data.speed > SPEED_THRESHOLD) {
        distance_travelled_calculator(cloud_data,cloud_data->can_data.speed);
    }
    else if(cloud_data->gps_data.speed > SPEED_THRESHOLD) {
        distance_travelled_calculator(cloud_data,cloud_data->gps_data.speed);
    }
    else {
        logger_error(CLOUD_LOG_MODULE_ID, "No speed data available");
    }
}

void distance_travelled_calculator(struct cloud_data_struct* cloud_data,int speed) {
    if (speed > SPEED_THRESHOLD) {
        if (!distance_timer_start) {
            prev_speed = speed;
            distance_timer_start = true;
            dtval_start = time(NULL);
        }
    }
    if (speed != prev_speed && distance_timer_start) {
        dtval_stop = time(NULL);
        int time_diff = (dtval_stop - dtval_start);
        cloud_data->distance_travelled = cloud_data->distance_travelled + (float)(prev_speed * time_diff);
        distance_timer_start = false;
    }
}