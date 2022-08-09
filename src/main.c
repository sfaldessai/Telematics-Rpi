/*
 * created at 2022-07-19 18:46.
 *
 * Company: HashedIn By Deloitte.
 * Copyright (C) 2022 HashedIn By Deloitte
 */

#include <stdio.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <pthread.h>
#include "main.h"

#define MAX_READ_SIZE 1

int main((int argc, char* argv[])
{
    struct uart_device_struct stm32_device, gps_device;
    struct cloud_data_struct cloud_data;
    struct arg_struct stm32_args, gps_args;
    pthread_t stm32_read_thread, gps_read_thread, serial_write_thread;
    
    int opt;
    SLogConfig.log_module = 0;
    int option_index = 0;
    int verbose_flag = 0;

    stm32_device.file_name = "/dev/ttyACM0";
    gps_device.file_name = "/dev/ttyUSB0"; /* connected neo gps module to rapi using UART to USB converter */
    stm32_device.baud_rate = B115200;
    gps_device.baud_rate = B9600;
    static bool keepRunning = true;

    /* Used variables */
    slog_config_t cfg;

    static struct option long_options[] =
    { 
        { "verbose", no_argument, &verbose_flag, 1 },
        { "help", no_argument, 0, 'h' },
        { "module", required_argument, &verbose_flag, 'm'},
        { 0, 0, 0, 0 } 
    };


    while ((opt = getopt_long(argc, argv, "h:m", long_options, &option_index)) != -1) {
        switch (opt) {
        case 0: {
            cout << "The following long option was set: " << long_options[option_index].name
                << endl;
            break;
        }
        case 'h': {
            cout << "Allowed options:" << endl;
            cout << "  -h [ --help ]          : produce help message" << endl;
            cout << "  -m [ --log module ] arg   : filter the module to display only its logs" << endl;
            cout << "  -m [ --log module ] arg => 0 : All Modules, 1 => CAN, 2 => GPS, 3 => Serial_Interface" << endl;
            keepRunning = false;
            break;
        }
        case 'm': {
            cfg.log_module = optarg;
            cout << "The hmi display config path was set to: " << displayConfigPath << endl;
            break;
        }
        case '?': {
            cout << "Got unknown option." << endl;
            keepRunning = false;
            break;
        }
        default: {
            cout << "Got unknown parse returns: " << opt << endl;
            keepRunning = false;
        }
        }
    }

    if (!keepRunning) {
        return 1;
    }


    if (verbose_flag == 1) {
        uint16_t nLogFlags = SLOG_INFO | SLOG_WARN | SLOG_DEBUG | SLOG_ERROR;
    }
    else {
        uint16_t nLogFlags = SLOG_ERROR;
    }

    slog_init("", nLogFlags, 0);

    /* Initialize slog and allow only error and not tagged output */
    slog_config_get(&cfg);

    /* Enable all logging flags */
   // slog_enable(SLOG_FLAGS_ALL);


    /* Pointer char initializing to null*/
    initialize_cloud_data(&cloud_data);

    uart_start(&stm32_device, true);
    uart_start(&gps_device, true);

    if (stm32_device.fd > 0)
    {
        /* STM32 Microcontroller Read Thread */
        stm32_args.uart_device = stm32_device;
        stm32_args.cloud_data = &cloud_data;
        pthread_create(&stm32_read_thread, NULL, &read_from_stm32, &stm32_args);
    }
    if (gps_device.fd > 0)
    {
        /* NEO GPS Module Read Thread */
        gps_args.uart_device = gps_device;
        gps_args.cloud_data = &cloud_data;
        pthread_create(&gps_read_thread, NULL, &read_from_gps, &gps_args);
    }

    /* Cloud Write Thread */
    pthread_create(&serial_write_thread, NULL, &write_to_cloud, &cloud_data);

    if (stm32_device.fd > 0)
    {
        pthread_join(stm32_read_thread, NULL);
    }
    if (gps_device.fd > 0)
    {
        pthread_join(gps_read_thread, NULL);
    }
    pthread_join(serial_write_thread, NULL);

    return 0;
}
