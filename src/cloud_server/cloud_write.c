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
#include "cloud_server.h"
#include "../logger/logger.h"
#include "../database/db_handler.h"
#include "../utils/c_json/cJSON.h"
#include "mqtt_demo_mutual_auth.h"
#define HOURS 3600
#define RPM_OFFSET 0
#define SPEED_THRESHOLD 0
#define IDLE_THRESHOLD 10

bool idle_timer_started = false;
time_t begin, end;

char *jsonObj(struct cloud_data_struct *cloud_data)
{
    cJSON *cjson_vehicle = NULL;
    cJSON *cjson_telematic = NULL;
    cJSON *cjson_can = NULL;
    cJSON *cjson_location = NULL;
    char *str = NULL;

    cjson_telematic = cJSON_CreateObject();
    cjson_can = cJSON_CreateObject();
    cjson_vehicle = cJSON_CreateObject();

    // change these values in json file
    cJSON_AddStringToObject(cjson_can, "serial", (char *)cloud_data->mac_address);
    cJSON_AddStringToObject(cjson_can, "from", "SHREE");
    cJSON_AddStringToObject(cjson_can, "vin", (char *)cloud_data->can_data.vin);
    cJSON_AddNumberToObject(cjson_can, "speed", cloud_data->can_data.speed);
    // cJSON_AddNumberToObject(cjson_can, "distance_travelled", distanceTravelled);
    cJSON_AddNumberToObject(cjson_can, "idle_time", cloud_data->idle_time_secs);
    // cJSON_AddNumberToObject(cjson_can,"veh_in_service", inService);
    cJSON_AddItemToObject(cjson_telematic, "can", cjson_can);

    cjson_location = cJSON_CreateObject();
    cJSON_AddNumberToObject(cjson_location, "lat", cloud_data->gps_data.longitude);
    cJSON_AddNumberToObject(cjson_location, "long", cloud_data->gps_data.longitude);
    cJSON_AddNumberToObject(cjson_location, "dop", cloud_data->gps_data.hdop);
    cJSON_AddItemToObject(cjson_telematic, "location", cjson_location);

    cJSON_AddNumberToObject(cjson_telematic, "motion", cloud_data->client_controller_data.motion);
    cJSON_AddNumberToObject(cjson_telematic, "pto", cloud_data->client_controller_data.pto);
    cJSON_AddNumberToObject(cjson_telematic, "battery", cloud_data->client_controller_data.voltage);

    cJSON_AddItemToObject(cjson_vehicle, "telematic", cjson_telematic);

    /* Prints all the data of the JSON object (the whole list) */
    str = cJSON_Print(cjson_vehicle);
    return str;
}

void display_cloud_struct_data_logger(struct cloud_data_struct *cloud_data)
{
    logger_info(CLOUD_LOG_MODULE_ID, "\tVIN = %s | CAN SPEED = %d | GPS SPEED = %f |  idle_time_sec = %lld \n", cloud_data->can_data.vin,
                cloud_data->can_data.speed, cloud_data->gps_data.speed, cloud_data->idle_time_secs);
    logger_info(CLOUD_LOG_MODULE_ID, "\tMOTION = %d | VOLTAGE = %f | PTO = %d\n", cloud_data->client_controller_data.motion,
                cloud_data->client_controller_data.voltage, cloud_data->client_controller_data.pto);
    logger_info(CLOUD_LOG_MODULE_ID, "\tLAT: %.4f", cloud_data->gps_data.latitude);
    logger_info(CLOUD_LOG_MODULE_ID, "\tLONG: %.4f\n", cloud_data->gps_data.longitude);
    logger_info(CLOUD_LOG_MODULE_ID, "\tPDOP:%.2f\tHDOP:%.2f\tVDOP:%.2f\n", cloud_data->gps_data.pdop,
                cloud_data->gps_data.hdop, cloud_data->gps_data.vdop);
}

/*
 * Name : write_to_cloud
 * Description: The write_to_cloud function is for diplaying/monitoring vehicle informtion from
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
            display_cloud_struct_data_logger(cloud_data);

            send_data = jsonObj(cloud_data);
            // printf("\n TEST DATA======%s\n", send_data);

            calculate_idle_time(cloud_data);

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
 * Name : calculate_idle_time
 * Description: This function calculates the idle time based on the speed and RPM
 * Input parameters: struct cloud_data_struct *
 * Output parameters: void
 */
void calculate_idle_time(struct cloud_data_struct *cloud_data)
{
    /* Start the Idle time when the speed is 0 and RPM is greater than some threshold when Ignition is ON */
    if ((cloud_data->can_data.speed == SPEED_THRESHOLD && cloud_data->gps_data.speed == SPEED_THRESHOLD) &&
        (cloud_data->can_data.rpm >= RPM_OFFSET))
    {
        if (idle_timer_started == false)
        {
            idle_timer_started = true;
            begin = time(NULL);
        }
    }
    else if (idle_timer_started)
    {
        end = time(NULL);
        int idle_time_sec = (end - begin);

        /* Write to cloud idle time only if the minimulm threshold is greater than 10 seconds. Otherwise, ignore */
        if (idle_time_sec > IDLE_THRESHOLD)
        {
            cloud_data->idle_time_secs = cloud_data->idle_time_secs + (uint64_t)idle_time_sec;
        }
        idle_timer_started = false;
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
    gps_data.longitude = 0.0;
    gps_data.hdop = 0.0;
    gps_data.pdop = 0.0;
    gps_data.vdop = 0.0;
    gps_data.speed = 0.0;

    client_controller_data.pto = 0;
    client_controller_data.motion = 0;
    client_controller_data.voltage = 0.0;

    memset(can_data.vin, '\0', MAX_LEN_VIN);
    can_data.speed = 0;
    can_data.rpm = 0.0;
    memset(can_data.supported_pids, 0, CAN_PID_LENGTH * sizeof(uint32_t));

    cloud_data->gps_data = gps_data;
    cloud_data->client_controller_data = client_controller_data;
    cloud_data->can_data = can_data;

    /* Fetching last updated idele_time value from db for calculating idle_time */
    uint8_t idle_time_db_value[COLUMN_VALUE_MAX_LEN];
    get_single_column_value(IDLE_TIME, SORT_BY_DESC, idle_time_db_value);
    cloud_data->idle_time_secs = (uint64_t)atoi((char *)idle_time_db_value);
}

/*
 * Name : gps_error_codes
 * Descriptoin: The gps_error_codes function is for updating erro codes for gps struct member
 * Input parameters: struct cloud_data_struct * : clout struct to update gps data member
 *                   int error_code : error code to update
 * Output parameters: void
 */
void gps_error_codes(struct cloud_data_struct *cloud_data, int error_code)
{
    struct gps_data_struct gps_data;

    gps_data.hdop = error_code;
    gps_data.vdop = error_code;
    gps_data.pdop = error_code;
    gps_data.latitude = error_code;
    gps_data.longitude = error_code;
    gps_data.speed = error_code;

    cloud_data->gps_data = gps_data;
}