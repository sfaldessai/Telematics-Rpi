/*
 * created at 2022-07-29 14:00.
 *
 * Company: HashedIn By Deloitte.
 * Copyright (C) 2022 HashedIn By Deloitte
 */

#ifndef DB_HANDLER_H_
#define DB_HANDLER_H_
#include "../cloud_server/cloud_server.h"

#define DEBUG

#define QUERY_MAX_LEN 1024
#define COLUMN_VALUE_MAX_LEN 16

/* Table Name */
#define TELEMATICS "TELEMATICS"

/* DB Attributes */
#define SORT_BY_DESC "DESC"
#define SORT_BY_ASC "ASC"

/* DB columns names */
#define creation_time "creation_time"
#define LATITUDE "Latitude"
#define LATITUDE_SIGN "LatSign"
#define LONGITUDE "Longitude"
#define LONGITUDE_SIGN "LongSign"
#define PDOP "PDOP"
#define HDOP "HDOP"
#define VDOP "VDOP"
#define SERIAL "Serial"
#define VIN "VIN"
#define SPEED "Speed"
#define SUPPORTED_PIDs "Supported_Pids"
#define DISTANCE_TRAVLLED "Dist_Travelled"
#define IDLE_TIME "Idle_time"
#define VEHICLE_IN_SERVICE "Veh_in_Service"
#define MOTION "Motion"
#define VOLTAGE "Voltage"
#define PTO "PTO"
#define ACC_X "AccX"
#define ACC_Y "AccY"
#define ACC_Z "AccZ"
#define RPM "RPM"
#define TEMPERATURE "Temperature"

#define TELEMATICS_DB_PATH "/usr/sbin/telematic.db"
#define SQL_CREATE_TABLE "CREATE TABLE IF NOT EXISTS TELEMATICS(ID INTEGER PRIMARY KEY AUTOINCREMENT, creation_time TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP, Latitude FLOAT, LatSign Text, Longitude FLOAT, LongSign Text, PDOP FLOAT, HDOP FLOAT, VDOP FLOAT,Serial TEXT, VIN TEXT, Speed INT, Supported_Pids TEXT, Dist_Travelled FLOAT, Idle_time FLOAT, Veh_in_Service FLOAT,Motion INT, Voltage FLOAT, PTO INT,AccX FLOAT, AccY FLOAT, AccZ FLOAT,RPM INT, Temperature FLOAT);"
#define DB_QUERY "INSERT INTO TELEMATICS (Latitude,LatSign,Longitude,LongSign,PDOP,HDOP,VDOP,Serial,VIN,Speed,Supported_Pids,Dist_Travelled,Idle_time,Veh_in_Service,Motion,Voltage,PTO,AccX,AccY,AccZ,RPM,Temperature) VALUES (%f,'%c',%f,'%c',%f,%f,%f,'%s','%s',%d, '%s','%f','%f','%f',%d,%f,%d,%f,%f,%f,%d,%f)"

int initialize_db(void);

int insert_telematics_data(struct cloud_data_struct* inCloud_data);

int get_single_column_value(char *, char *, uint8_t *);

#endif