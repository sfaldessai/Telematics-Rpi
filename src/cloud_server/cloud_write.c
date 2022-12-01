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
#include <math.h>
#include <sys/time.h>
#include <time.h>
#include "cloud_server.h"
#include "../logger/logger.h"
#include "../database/db_handler.h"
#include "../utils/c_json/cJSON.h"
#include "mqtt_demo_mutual_auth.h"

#define HOURS 3600
#define RPM_OFFSET 0
#define SPEED_THRESHOLD 0
#define IDLE_THRESHOLD 10
#define PI 3.142
#define EARTH_RADIUS 6371
#define KMPH_TO_MPS_CONVERTER 0.277778 /* 1 km/hr = 5/18 m/sec or 0.277778 m/sec*/

time_t tval_start, tval_stop, dtval_start, dtval_stop;
bool service_timer_start = false, distance_timer_start = false;
float prev_speed = 0;

bool idle_timer_started = false;
time_t begin, end;

char *create_json_obj(struct cloud_data_struct *cloud_data)
{
    cJSON *cjson_vehicle = NULL;
    cJSON *cjson_telematic = NULL;
    cJSON *cjson_can = NULL;
    cJSON *cjson_location = NULL;
    cJSON *cjson_client_controller = NULL;
    char *json_string = NULL;

    cjson_telematic = cJSON_CreateObject();
    cjson_can = cJSON_CreateObject();
    cjson_vehicle = cJSON_CreateObject();
    cjson_location = cJSON_CreateObject();
    cjson_client_controller = cJSON_CreateObject();

    char supported_pids[CAN_PID_LENGTH + 1];
    size_t i = 0;

    /* Prepare Query String START */
    for (i = 0; i < CAN_PID_LENGTH; i++)
    {
        sprintf(&supported_pids[i], "%d", cloud_data->can_data.supported_pids[i]);
    }
    supported_pids[i] = '\0';

    cJSON_AddStringToObject(cjson_can, "vin", (char *)cloud_data->can_data.vin);
    cJSON_AddStringToObject(cjson_can, "vehicleType", (char *)cloud_data->can_data.vehicle_type);
    cJSON_AddNumberToObject(cjson_can, "speed", cloud_data->can_data.speed,0);
    cJSON_AddNumberToObject(cjson_can, "rpm", cloud_data->can_data.rpm,1);
    cJSON_AddStringToObject(cjson_can, "supportedPid", supported_pids);
    cJSON_AddNumberToObject(cjson_can, "temperature", cloud_data->can_data.temperature,1);
    cJSON_AddItemToObject(cjson_telematic, "can", cjson_can);

    cJSON_AddNumberToObject(cjson_location, "latitude", cloud_data->gps_data.latitude,6);
    cJSON_AddNumberToObject(cjson_location, "longitude", cloud_data->gps_data.longitude,6);
    cJSON_AddNumberToObject(cjson_location, "hdop", cloud_data->gps_data.hdop,2);
    cJSON_AddNumberToObject(cjson_location, "vdop", cloud_data->gps_data.vdop,2);
    cJSON_AddNumberToObject(cjson_location, "pdop", cloud_data->gps_data.pdop,2);
    cJSON_AddItemToObject(cjson_telematic, "location", cjson_location);

    cJSON_AddNumberToObject(cjson_client_controller, "motion", cloud_data->client_controller_data.motion,0);
    cJSON_AddNumberToObject(cjson_client_controller, "pto", cloud_data->client_controller_data.pto,0);
    cJSON_AddNumberToObject(cjson_client_controller, "battery", cloud_data->can_data.battery,2);
    cJSON_AddNumberToObject(cjson_client_controller, "accX", cloud_data->client_controller_data.acc_x,0);
    cJSON_AddNumberToObject(cjson_client_controller, "accY", cloud_data->client_controller_data.acc_y,0);
    cJSON_AddNumberToObject(cjson_client_controller, "accZ", cloud_data->client_controller_data.acc_z,0);
    cJSON_AddItemToObject(cjson_telematic, "clientController", cjson_client_controller);

    cJSON_AddStringToObject(cjson_telematic, "serial", (char *)cloud_data->mac_address);
    cJSON_AddNumberToObject(cjson_telematic, "idleTime", (double)cloud_data->idle_time_secs,0);
    cJSON_AddNumberToObject(cjson_telematic, "serviceTime", (double)cloud_data->service_time,0);
    cJSON_AddNumberToObject(cjson_telematic, "distanceTravelled", (double)cloud_data->distance_travelled,0);

    time_t seconds = time(NULL);
    cJSON_AddNumberToObject(cjson_telematic, "timeStampInSeconds", seconds,0);

    cJSON_AddItemToObject(cjson_vehicle, "telematic", cjson_telematic);

    /* Prints all the data of the JSON object (the whole list) */
    json_string = cJSON_Print(cjson_vehicle);

    logger_info(CLOUD_LOG_MODULE_ID, "COMBINED JSON DATA: \n%s\n", json_string);
    return json_string;
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
            if (cloud_data->gps_data.hdop >= GPS_ERROR_RANGE_BEGIN && cloud_data->gps_data.hdop <= GPS_ERROR_RANGE_END
                && cloud_data->client_controller_data.acc_x < CLIENT_CONTROLLER_ERROR_RANGE_BEGIN ) {
                /*TBD: Test and note the min-max limits of Acceleration values and rectify the error code ranges accordingly*/
                get_last_two_lat_log(cloud_data->prev_latitude, cloud_data->prev_longitude);
                logger_info(CLOUD_LOG_MODULE_ID, "last(nth) updated value: latitude = %lf longitude = %lf", cloud_data->prev_latitude[0], cloud_data->prev_longitude[0]);
                logger_info(CLOUD_LOG_MODULE_ID, "last(nth-1) updated value: latitude = %lf longitude = %lf", cloud_data->prev_latitude[1], cloud_data->prev_longitude[1]);
                handle_gps_signal_lost(cloud_data);
            }
            calculate_service_time(cloud_data);
            calculate_distance_travelled(cloud_data);
            calculate_idle_time(cloud_data);
            send_data = create_json_obj(cloud_data);

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
    if (((cloud_data->can_data.speed == SPEED_THRESHOLD || cloud_data->can_data.speed > MAX_CAR_SPEED) &&
         (cloud_data->gps_data.speed == SPEED_THRESHOLD || cloud_data->gps_data.speed > MAX_CAR_SPEED)) &&
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
    memset(gps_data.dop_accuracy, '\0', DOP_ACCURACY_STRING);
    gps_data.latitude = 0.0;
    gps_data.longitude = 0.0;
    gps_data.hdop = 0.0;
    gps_data.pdop = 0.0;
    gps_data.vdop = 0.0;
    gps_data.speed = 0.0;

    client_controller_data.pto = 0;
    client_controller_data.motion = 0;
    client_controller_data.voltage = 0.0;
    client_controller_data.acc_x = 0;
    client_controller_data.acc_y = 0;
    client_controller_data.acc_z = 0;
    client_controller_data.mode = 1;

    memset(can_data.vin, '\0', MAX_LEN_VIN);
    memset(can_data.vehicle_type, '\0', WMI_STRING_LEN);
    can_data.speed = 0;
    can_data.rpm = 0.0;
    can_data.temperature = 0;
    can_data.mode = 1;
    can_data.battery=0;
    memset(can_data.supported_pids, '\0', CAN_PID_LENGTH * sizeof(uint8_t));

    cloud_data->gps_data = gps_data;
    cloud_data->client_controller_data = client_controller_data;
    cloud_data->can_data = can_data;

    /* Fetching last updated idele_time value from db for calculating idle_time */
    uint8_t idle_time_db_value[COLUMN_VALUE_MAX_LEN];
    get_single_column_value(IDLE_TIME, SORT_BY_DESC, idle_time_db_value);
    cloud_data->idle_time_secs = (uint64_t)atoi((char *)idle_time_db_value);

    uint8_t service_value[COLUMN_VALUE_MAX_LEN];
    get_single_column_value(VEHICLE_IN_SERVICE, SORT_BY_DESC, service_value);
    cloud_data->service_time = (int)atoi((char *)service_value);

    uint8_t distance_value[COLUMN_VALUE_MAX_LEN];
    get_single_column_value(DISTANCE_TRAVLLED, SORT_BY_DESC, distance_value);
    cloud_data->distance_travelled = (float)atof((char *)distance_value);
}

/*
 * Name : caluclate_service_time
 * Description: This function calculates the service time based on the CAN speed and GPS Speed
 * Input parameters: struct cloud_data_struct *
 * Output parameters: void
 */
void calculate_service_time(struct cloud_data_struct *cloud_data)
{
    /* Start the Service time when the Either GPS or CAN speed is greater than 0 */
    if (((cloud_data->can_data.speed > SPEED_THRESHOLD && cloud_data->can_data.speed < MAX_CAR_SPEED) ||
         (cloud_data->gps_data.speed > SPEED_THRESHOLD && cloud_data->gps_data.speed > MAX_CAR_SPEED)))
    {
        if (!service_timer_start)
        {
            service_timer_start = true;
            tval_start = time(NULL);
        }
    } /* Test and check if both CAN and GPS speed is required to handle stop timer when either of the modules malfunction */
    if (((cloud_data->can_data.speed == SPEED_THRESHOLD || cloud_data->can_data.speed > MAX_CAR_SPEED) &&
         (cloud_data->gps_data.speed == SPEED_THRESHOLD || cloud_data->gps_data.speed > MAX_CAR_SPEED)) &&
        service_timer_start)
    {
        tval_stop = time(NULL);
        int tval_inServiceTime = (tval_stop - tval_start);
        cloud_data->service_time = cloud_data->service_time + tval_inServiceTime;
        service_timer_start = false;
    }
}

/*
 * Name : calculate_distance_travelled
 * Description: This function calculates the  distance travelled based on the CAN speed and GPS Speed
 * Input parameters: struct cloud_data_struct *
 * Output parameters: void
 */
void calculate_distance_travelled(struct cloud_data_struct *cloud_data)
{
    if (cloud_data->can_data.speed > SPEED_THRESHOLD || cloud_data->can_data.speed < MAX_CAR_SPEED)
    {
        distance_travelled_calculator(cloud_data, cloud_data->can_data.speed);
    }
    else if (cloud_data->gps_data.speed > SPEED_THRESHOLD || cloud_data->gps_data.speed < MAX_CAR_SPEED)
    {
        distance_travelled_calculator(cloud_data, cloud_data->gps_data.speed);
    }
    else
    {
        logger_error(CLOUD_LOG_MODULE_ID, "No speed data available");
    }
}

void distance_travelled_calculator(struct cloud_data_struct *cloud_data, int speed)
{
    if (speed > SPEED_THRESHOLD)
    {
        if (!distance_timer_start)
        {
            prev_speed = speed;
            distance_timer_start = true;
            dtval_start = time(NULL);
        }
    }
    if (speed != prev_speed && distance_timer_start)
    {
        dtval_stop = time(NULL);
        int time_diff = (dtval_stop - dtval_start);
        prev_speed = KMPH_TO_MPS_CONVERTER * prev_speed;
        cloud_data->distance_travelled = cloud_data->distance_travelled + (float)(prev_speed * time_diff);
        distance_timer_start = false;
    }
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

/*
 * Name : client_controller_error_codes
 * Descriptoin: The client_controller_error_codes function is for updating erro codes for STM32 struct member
 * Input parameters: struct cloud_data_struct * : clout struct to update STM32 data member
 *                   int error_code : error code to update
 * Output parameters: void
 */
void client_controller_error_codes(struct cloud_data_struct *cloud_data, int error_code)
{
    struct client_controller_data_struct cc_data;

    cc_data.motion = (uint16_t)error_code;
    cc_data.pto = (uint16_t)error_code;
    cc_data.voltage = (float)error_code;
    cc_data.acc_x = error_code;
    cc_data.acc_y = error_code;
    cc_data.acc_z = error_code;
    cloud_data->client_controller_data = cc_data;
}

/*
 * Name : handle_gps_signal_lost
 * Description: This function handles scenarios when the GPS signal is lost
 * Input parameters: struct cloud_data_struct *
 * Output parameters: void
 */
void handle_gps_signal_lost(struct cloud_data_struct *cloud_data)
{
    double Lat2, Lon2 = 0;
    double distance = 0;
    double bearing_theta;
    double X, Y;
    double x_distance = 0, y_distance = 0, z_distance = 0;
    cloud_data->prev_latitude[0] = (cloud_data->prev_latitude[0] * PI) / 180;
    cloud_data->prev_latitude[1] = (cloud_data->prev_latitude[1] * PI) / 180;
    cloud_data->prev_longitude[0] = (cloud_data->prev_longitude[0] * PI) / 180;
    cloud_data->prev_longitude[1] = (cloud_data->prev_longitude[0] * PI) / 180;

    /* calculate distance based on accx, accy and accz */
    x_distance = 0.5 * cloud_data->client_controller_data.acc_x * (1 / 3600) * (1 / 3600);
    y_distance = 0.5 * cloud_data->client_controller_data.acc_y * (1 / 3600) * (1 / 3600);
    z_distance = 0.5 * cloud_data->client_controller_data.acc_z * (1 / 3600) * (1 / 3600);
    distance = sqrt(x_distance * x_distance + y_distance * y_distance + z_distance * z_distance);
    logger_info(CLOUD_LOG_MODULE_ID, "The distance between source and destination is: %lf", distance);

    /* calculate bearing based on previous lat, lon */
    X = cos(cloud_data->prev_latitude[1]) * sin(cloud_data->prev_longitude[0] - cloud_data->prev_longitude[1]);
    Y = cos(cloud_data->prev_latitude[1]) * sin(cloud_data->prev_latitude[0]) - sin(cloud_data->prev_latitude[1]) * cos(cloud_data->prev_latitude[0]) * cos(cloud_data->prev_longitude[0] - cloud_data->prev_longitude[1]);
    bearing_theta = atan2(X, Y);
    bearing_theta = (bearing_theta * 180) / PI;
    logger_info(CLOUD_LOG_MODULE_ID, "Bearing Theta values is %lf", bearing_theta);

    /* calculate unknown lat-Lon based on previous lat, lon, bearing angle and distance calculated based on Acceleration */
    Lat2 = asin(sin(cloud_data->prev_latitude[0]) * cos(distance / EARTH_RADIUS) + cos(cloud_data->prev_latitude[0]) * sin(distance / EARTH_RADIUS) * cos(bearing_theta));
    Lon2 = cloud_data->prev_longitude[0] + atan2(sin(bearing_theta) * sin(distance / EARTH_RADIUS) * cos(cloud_data->prev_latitude[0]),
                                                 cos(distance / EARTH_RADIUS) - sin(cloud_data->prev_latitude[0]) * sin(Lat2));
    cloud_data->gps_data.latitude = (Lat2 * 180) / PI;
    cloud_data->gps_data.longitude = (Lon2 * 180) / PI;
    logger_info(CLOUD_LOG_MODULE_ID, "Unknown Lat2 value is approx: %lf", cloud_data->gps_data.latitude);
    logger_info(CLOUD_LOG_MODULE_ID, "Unknown Lon2 value is approx: %lf", cloud_data->gps_data.longitude);
}
