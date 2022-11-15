/*
 * created at 2022-07-29 13:37.
 *
 * Company: HashedIn By Deloitte.
 * Copyright (C) 2022 HashedIn By Deloitte
 */

#ifndef GPS_MODULE_H_
#define GPS_MODULE_H_

#include <stdint.h>
#include "../serial_interface/serial_config.h"

#define DEBUG

#define GPS_INVALID_CHECKSUM_ERROR 16
#define ASSIST_NOW_AUTONOMOUS_CMD_LEN 48
#define NMEA_SENTENCE_CMD_LEN 28
#define POWER_SAVE_MODE_CMD_LEN 10
#define GNSS_STOP_START_CMD_LEN 12
#define GNSS_DEFAULT_SETTING_CMD_LEN 12
#define SAVE_CONFIG_CMD_LEN 21
#define NO_SIGNAL_DOP_VALUE 99.99
#define INVALID_DOP_VALUE 20

#define IGNITION_ON 1
#define IGNITION_OFF 0
#define VOLTAGE_THRESHOLD 12.5

/* ERROR CODES */
#define GPS_WRITE_ERROR 902
#define GPS_POWERED_OFF 904
#define GPS_BAD_SIGNAL 905
#define GPS_DEVICE_DISCONNECTED 906
#define FAILED_TO_OPEN_GPS_DEVICE 907
#define GPS_LATITUDE_EMPTY_DATA 908
#define GPS_LONGITUDE_EMPTY_DATA 909
#define GPS_SPEED_EMPTY_DATA 910
#define GPS_DOP_EMPTY_DATA 911
#define GPS_INITIAL_CONFIGURATION_WRITE_FAILED 912
#define GPS_GNSS_FAILED_TURN_ON_POWER 913
#define GPS_GNSS_FAILED_TURN_OFF_POWER 914
#define LOST_GPS_SIGNAL_ERROR 915
#define GPS_SIGNAL_IS_IDEAL 916
#define GPS_SIGNAL_IS_EXCELLENT 917
#define GPS_SIGNAL_IS_GOOD 918
#define GPS_SIGNAL_IS_MODERATE 919
#define GPS_SIGNAL_IS_FAIR 920
#define GPS_SIGNAL_IS_POOR 921
#define INVALID_NMEA_SENTENCES 922
#define GPS_INVALID_QUALITY 923

#define CC_ERROR_START_CODE 1000

/* UBX CMD */
#define HEADER_1 0xB5
#define HEADER_2 0x62
#define CLASS_ID 0x62

/* DOP Accuracy */
#define IDEAL "IDEAL"
#define EXCELLENT "EXCELLENT"
#define GOOD "GOOD"
#define MODERATE "MODERATE"
#define FAIR "FAIR"
#define POOR "POOR"

#define DOP_ACCURACY_STRING 32

/* UBX-CFG-CFG */
static const uint8_t save_configuration[SAVE_CONFIG_CMD_LEN] = {
	HEADER_1, HEADER_2,
	CLASS_ID,
	0x09,					// message id
	0x0D, 0x00,				// payload length little-endian unsigned int
	0x00, 0x00, 0x00, 0x00, // Clear mask
	0xFF, 0xFF, 0x00, 0x00, // save mask
	0x00, 0x00, 0x00, 0x00, // load mask
	0x01					// device mask
};

/* UBX-CFG-PRT */
static const uint8_t set_NMEA_sentence_on[NMEA_SENTENCE_CMD_LEN] = {
	HEADER_1, HEADER_2,
	CLASS_ID,
	0x00,					// message id
	0x14, 0x00,				// payload length little-endian unsigned int
	0x01,					// portID
	0x00,					// reserved
	0x00, 0x00,				// txReady
	0xD0, 0x08, 0x00, 0x00, // Mode
	0x80, 0x25, 0x00, 0x00, // baudrate 9600 (little-endian)
	0x07, 0x00,				// inProtoMask
	0x03, 0x00,				// outProtoMask
	0x00, 0x00,				// flags
	0x00, 0x00				// reserved[2]
};

/* UBX-CFG-RST { SYNC-CHAR (B5), SYNC-CHAR (0X62), CLASS-FIELD, MESSAGE-ID, PAYLOAD-LENGTH, PAYLOAD } */
static const uint8_t set_gnss_start[GNSS_STOP_START_CMD_LEN] = {
	HEADER_1, HEADER_2,
	CLASS_ID,
	0x04,		// message id
	0x04, 0x00, // payload length little-endian unsigned int
	0x00, 0x00, // hot start (0x0001 : Warm & 0xFFFF : Cold)
	0x09,		// GNSS start
	0x00		// reserved
};

/* UBX-CFG-RST */
static const uint8_t set_gnss_stop[GNSS_STOP_START_CMD_LEN] = {
	HEADER_1, HEADER_2,
	CLASS_ID,
	0x04,		// message id
	0x04, 0x00, // payload length little-endian unsigned int
	0x00, 0x00, // hot start (0x0001 : Warm & 0xFFFF : Cold)
	0x08,		// GNSS stop
	0x00		// reserved
};

/* UBX-CFG-RXM */
static const uint8_t set_power_save_mode[POWER_SAVE_MODE_CMD_LEN] = {
	HEADER_1, HEADER_2,
	CLASS_ID,
	0x11,		// message id
	0x02, 0x00, // payload length little-endian unsigned int
	0x00,		// reserved
	0x01		// Power Save mode (0: continuous)
};

static const uint8_t set_gps_default_setting_cmd[SAVE_CONFIG_CMD_LEN] = {
	HEADER_1, HEADER_2,
	CLASS_ID,
	0x09,																			   // message id
	0x0D, 0x00,																		   // payload length little-endian unsigned int
	0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x01 // payload
};

/* UBX-CFG-NAVX5 */
static const uint8_t set_assistNow_autonomous[ASSIST_NOW_AUTONOMOUS_CMD_LEN] = {
	HEADER_1, HEADER_2,
	CLASS_ID,
	0x23,								// message id
	0x28, 0x00,							// payload length little-endian unsigned int
	0x00, 0x00,							// version
	0x0C, 0x04,							// mask1
	0x80, 0x00, 0x00, 0x00,				// mask2
	0x00, 0x00,							// reserved[2]
	0x03,								// min number of satellite for navigation
	0x10,								// max number of satellite for navigation
	0x00,								// min satellite signal level for navigation
	0x00,								// reserved
	0x00,								// iniFix3D
	0x00, 0x00,							// reserved[2]
	0x01,								// ackAiding
	0x04, 0x00,							// wknRollover
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // reserved[6]
	0x01,								// usePPP
	0x01,								// aopCfg
	0x00, 0x00,							// reserved[2]
	0x00, 0x64,							// aopOrbMaxErr
	0x00, 0x00, 0x00, 0x00,				// reserved[4]
	0x00, 0x00, 0x00,					// reserved[3]
	0x00								// useAdr
};

/* gps_data_struct struct to holds GPS data*/
struct gps_data_struct
{
	double latitude;
	double longitude;
	char *gps_time;
	double pdop;
	double vdop;
	double hdop;
	int speed;
	char dop_accuracy[DOP_ACCURACY_STRING];
};

void *read_from_gps(void *); /* pthread to handle gps read */

void get_lat_log(double *);
void get_dops(char **, char *);
int get_gps_data(char *, struct gps_data_struct *);
void get_gps_param_by_position(char **, char *, uint8_t);
int nmea_verify_checksum(const char *);
int ignition_off(struct uart_device_struct);
int ignition_on(struct uart_device_struct);
int gps_data_processing(char *read_data, struct gps_data_struct *gps_data);
int initialize_gps_module(struct uart_device_struct gps_device);
char *dop_accuracy_string(double hdop);

#endif