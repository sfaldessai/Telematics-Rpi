/*
 * created at 2022-07-27 12:00.
 *
 * Company: HashedIn By Deloitte.
 * Copyright (C) 2022 HashedIn By Deloitte
 */

#include <stdio.h>
#include <unistd.h>
#include <sqlite3.h>
#include "db_handler.h"
#include "../logger/logger.h"

#define SQL_CREATE_TABLE "CREATE TABLE IF NOT EXISTS TELEMATICS(ID INTEGER PRIMARY KEY AUTOINCREMENT, creation_time TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP, Latitude FLOAT, LatSign Text, Longitude FLOAT, LongSign Text, PDOP FLOAT, HDOP FLOAT, VDOP FLOAT,Serial TEXT, VIN TEXT, Speed INT, Dist_Travelled FLOAT, Idle_time FLOAT, Veh_in_Service FLOAT,Motion INT, Voltage FLOAT, PTO INT,AccX FLOAT, AccY FLOAT, AccZ FLOAT,RPM INT, Temperature FLOAT);"

void initialize_db(void)
{
    sqlite3 *db = NULL;
    int rc = db_setup(db);
    if (rc)
    {
        logger_error(DB_LOG_MODULE_ID, "Cannot open database: %s\n", sqlite3_errmsg(db));
    }
}

int db_setup(sqlite3 *db)
{
    char *err_msg = 0;

    int rc = sqlite3_open("test.db", &db);

    if (rc != SQLITE_OK)
    {
        logger_error(DB_LOG_MODULE_ID, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);

        return 1;
    }
    else
    {
        int result;
        char *sql = SQL_CREATE_TABLE;
        result = sqlite3_exec(db, sql, 0, 0, &err_msg);

        if (result != SQLITE_OK)
        {
            logger_error(DB_LOG_MODULE_ID, "SQL error: %s\n", err_msg);
            sqlite3_free(err_msg);
            sqlite3_close(db);
            return 1;
        }
    }
    sqlite3_close(db);
    return rc;
}

int insert_telematics_data(struct cloud_data_struct *cloud_data)
{
    char *err_msg = 0;
    sqlite3 *db;
    int result;
    char sql[1024];
    sprintf(sql, "INSERT INTO Telematics (Latitude,LatSign,Longitude,LongSign,PDOP,HDOP,VDOP,Serial,VIN,Speed,Dist_Travelled,Idle_time,Veh_in_Service,Motion,Voltage,PTO,AccX,AccY,AccZ,RPM,Temperature) VALUES (%f,'%c',%f,'%c',%f,%f,%f,'%s','%s',%d,'%f','%f','%f',%d,%f,%d,%f,%f,%f,%d,%f)", cloud_data->gps_data.latitude, cloud_data->gps_data.lat_cardinal_sign, cloud_data->gps_data.longitude, cloud_data->gps_data.long_cardinal_sign,
            cloud_data->gps_data.pdop, cloud_data->gps_data.hdop, cloud_data->gps_data.vdop, "12345", "abcdef", 85,180.00, 27.00, 86.00, cloud_data->client_controller_data.motion, cloud_data->client_controller_data.voltage, cloud_data->client_controller_data.pto,0.00,0.00,0.00,0,0.00);
    int rc = sqlite3_open("test.db", &db);
    if (rc != SQLITE_OK)
    {
        logger_error(DB_LOG_MODULE_ID, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);

        return 1;
    }
    result = sqlite3_exec(db, sql, 0, 0, &err_msg);

    if (result != SQLITE_OK)
    {
        logger_error(DB_LOG_MODULE_ID, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(db);
        return 1;
    }
    else
    {
        logger_info(CLOUD_LOG_MODULE_ID, "Data inserted successfully");
    }

    sqlite3_close(db);
    return 0;
}