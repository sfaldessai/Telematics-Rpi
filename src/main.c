/*
 * created at 2022-07-19 18:46.
 *
 * Company: HashedIn By Deloitte.
 * Copyright (C) 2022 HashedIn By Deloitte
 */

#include <stdio.h>
#include <fcntl.h>
#include <termios.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <getopt.h>
#include <ctype.h>
#include "mqtt_demo_mutual_auth.h"
#include "main.h"

int main(int argc, char *argv[])
{
    struct uart_device_struct client_controller_device, gps_device;
    struct cloud_data_struct cloud_data;
    struct aws_arg aws_arg_data;
    struct arg_struct client_controller_args, gps_args;
    pthread_t client_controller_read_thread, gps_read_thread, serial_write_thread;
    pthread_t read_can_supported_thread, read_can_speed_thread, read_can_vin_thread, read_can_rpm_thread, read_can_temperature_thread, cloud_send_thread, read_ble_can_thread;
    int opt;
    char *client_controller_path = NULL, *gps_device_path= NULL;
    char *cc_device_list[CC_DEVICE_LIST_LENGTH] = {CC_MANUFACTURE_NAME, TEST_CC_MANUFACTURE_NAME};
    char *gps_device_list[GPS_DEVICE_LIST_LENGTH] = {GPS_MANUFACTURE_NAME, TEST_GPS_MANUFACTURE_NAME};
    int can_server=0;
    cloud_data.build_version = RELEASE_VERSION;

    client_controller_path = get_device_path(cc_device_list, CC_DEVICE_LIST_LENGTH);
    gps_device_path = get_device_path(gps_device_list, GPS_DEVICE_LIST_LENGTH);

    if (client_controller_path == NULL || strlen(client_controller_path) <= 0)
    {
        /* setting default path when client_controller_path is NULL */
        client_controller_path = CLIENT_CONTROLLER;
    }
    if (gps_device_path == NULL || strlen(gps_device_path) <= 0)
    {
        /* setting default path when gps_device_path is NULL */
        gps_device_path = GPS_MODULE;
    }

    /* uart set-up*/
    uart_setup(&client_controller_device, client_controller_path, B115200, true);
    uart_setup(&gps_device, gps_device_path, B9600, true);

    while ((opt = getopt(argc, argv, "m:f:c:")) != -1)
    {
        switch (opt)
        {
        case 'm':
            module_flag = atoi(optarg);
            break;
        case 'f':
            write_to_file = atoi(optarg);
            break;
        case 'c':
            can_server = atoi(optarg);
        default:
            break;
        }
    }

    /* Get Master (RPI) MAC Address and update cloud_data.mac_address */
    get_master_mac_address(cloud_data.mac_address);

    /* Pointer char initializing to null*/
    initialize_cloud_data(&cloud_data);

    /* Thread Creation */
    if (client_controller_device.fd > 0)
    {
        client_controller_args.uart_device = client_controller_device;
        client_controller_args.cloud_data = &cloud_data;
        /* client_controller Microcontroller Read Thread */
        pthread_create(&client_controller_read_thread, NULL, &read_from_client_controller, &client_controller_args);
    }
    if (gps_device.fd > 0)
    {
        gps_args.uart_device = gps_device;
        gps_args.cloud_data = &cloud_data;
        /* NEO GPS Module Read Thread */
        pthread_create(&gps_read_thread, NULL, &read_from_gps, &gps_args);
    }
    /* Cloud Write Thread */
    pthread_create(&serial_write_thread, NULL, &write_to_cloud, &cloud_data);

    /* CAN Module Read Thread */
    if (can_server == BLE_CAN_MODULE_ID)
    {
        pthread_create(&read_ble_can_thread,NULL,&read_from_ble_can,&cloud_data);
    }
    else
    {
        read_from_can(&cloud_data, &read_can_supported_thread, &read_can_speed_thread, &read_can_vin_thread, &read_can_rpm_thread, &read_can_temperature_thread);
    }
    aws_arg_data.client_id = AWS_CLIENT_ID;
    aws_arg_data.topic = AWS_TOPIC;
    aws_arg_data.aws_iot_endpoint = AWS_IOT_ENDPOINT;
    pthread_create(&cloud_send_thread, NULL, &mqtt_send, &aws_arg_data);

    /* Thread Creation End */

    /* Join Thread */
    if (client_controller_device.fd > 0)
    {
        pthread_join(client_controller_read_thread, NULL);
    }
    else
    {
        client_controller_error_codes(&cloud_data, FAILED_TO_OPEN_STM32_DEVICE);
    }
    if (gps_device.fd > 0)
    {
        pthread_join(gps_read_thread, NULL);
    }
    else
    {
        gps_error_codes(&cloud_data, FAILED_TO_OPEN_GPS_DEVICE);
    }

    pthread_join(serial_write_thread, NULL);
    pthread_join(read_can_supported_thread, NULL);
    pthread_join(read_can_speed_thread, NULL);
    pthread_join(read_can_vin_thread, NULL);
    pthread_join(read_can_rpm_thread, NULL);
    pthread_join(read_can_temperature_thread, NULL);
    if (can_server == BLE_CAN_MODULE_ID){
        pthread_join(read_ble_can_thread,NULL);
    }
    /* Join Thread End*/

    return 0;
}
