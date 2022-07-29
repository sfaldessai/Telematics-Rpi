/*
 * created at 2022-07-29 13:37.
 *
 * Company: HashedIn By Deloitte.
 * Copyright (C) 2022 HashedIn By Deloitte
 */

#ifndef GPS_H_
#define GPS_H_

#define DEBUG

struct gps_data_struct
{
	float latitude;
	float longitude;
	char *gps_time;
	char lat_cardinal_sign;
	char long_cardinal_sign;
	float pdop;
	float vdop;
	float hdop;
};

void *read_from_gps(void *); /* pthread to handle gps read */

#endif