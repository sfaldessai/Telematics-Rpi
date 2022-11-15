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
#define LONGITUDE "Longitude"
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
#define SQL_CREATE_TABLE "CREATE TABLE IF NOT EXISTS TELEMATICS(ID INTEGER PRIMARY KEY AUTOINCREMENT, creation_time DATE DEFAULT (datetime('now', 'localtime')), Latitude FLOAT, Longitude FLOAT, PDOP FLOAT, HDOP FLOAT, VDOP FLOAT,Serial TEXT, VIN TEXT, Vehicle_Type TEXT, Speed INT, Supported_Pids TEXT, Dist_Travelled FLOAT, Idle_time INT, Veh_in_Service INT,Motion INT, Voltage FLOAT, PTO INT,AccX INT, AccY INT, AccZ INT,RPM FLOAT, Temperature INT);"
#define DB_QUERY "INSERT INTO TELEMATICS (Latitude,Longitude,PDOP,HDOP,VDOP,Serial,VIN,Vehicle_Type,Speed,Supported_Pids,Dist_Travelled,Idle_time,Veh_in_Service,Motion,Voltage,PTO,AccX,AccY,AccZ,RPM,Temperature) VALUES (%f,%f,%f,%f,%f,'%s','%s','%s',%d, '%s','%f','%lld','%d',%d,%f,%d,%d,%d,%d,%f,%d)"

int initialize_db(void);

int insert_telematics_data(struct cloud_data_struct *inCloud_data);

int get_single_column_value(char *, char *, uint8_t *);

int get_last_two_lat_log(double* latitude, double* longitude);

#endif
