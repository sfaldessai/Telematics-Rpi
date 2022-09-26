/*
 * created at 2022-07-29 13:37.
 *
 * Company: HashedIn By Deloitte.
 * Copyright (C) 2022 HashedIn By Deloitte
 */

#ifndef GPS_MODULE_H_
#define GPS_MODULE_H_

#include <stdint.h>

#define DEBUG

#define GPS_INVALID_CHECKSUM_ERROR 16
#define ASSIST_NOW_AUTONOMOUS_CMD_LEN 48
#define NMEA_SENTENCE_CMD_LEN 28
#define POWER_SAVE_MODE_CMD_LEN 10
#define GNSS_STOP_START_CMD_LEN 12
#define GNSS_DEFAULT_SETTING_CMD_LEN 12
#define SAVE_CONFIG_CMD_LEN 21

#define IGNITION_ON 1
#define IGNITION_OFF 0
#define VOLTAGE_THRESHOLD 13

/* UBX CMD */
#define HEADER_1 0xB5
#define HEADER_2 0x62
#define CLASS_ID 0x62

static const uint8_t save_configuration[SAVE_CONFIG_CMD_LEN] = {
	HEADER_1, HEADER_2,
	CLASS_ID,
	0x09,		// message id
	0x0D, 0x00, // payload length little-endian unsigned int
	0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};

static const uint8_t set_NMEA_sentence_on[NMEA_SENTENCE_CMD_LEN] = {
	HEADER_1, HEADER_2,
	CLASS_ID,
	0x00,		// message id
	0x14, 0x00, // payload length little-endian unsigned int
	0x01, 0x00,
	0x00, 0x00, 0xD0, 0x08, 0x00, 0x00, 0x80, 0x25, 0x00, 0x00, 0x07, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00};

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
	char lat_cardinal_sign;
	char long_cardinal_sign;
	double pdop;
	double vdop;
	double hdop;
	double speed;
};

void *read_from_gps(void *); /* pthread to handle gps read */

void get_lat_log(double *);
void get_dops(char **, char *);
void get_gps_data(char *, struct gps_data_struct *);
void get_gps_param_by_position(char **, char *, uint8_t);
uint8_t nmea_verify_checksum(const char *);

#endif