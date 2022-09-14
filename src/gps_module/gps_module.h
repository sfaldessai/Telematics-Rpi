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
void get_gps_param_by_position(char **,char*,uint8_t);
uint8_t nmea_verify_checksum(const char *);

#endif