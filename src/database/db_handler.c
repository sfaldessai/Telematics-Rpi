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

int initialize_db()
{
    sqlite3 *db;
    int rc = db_setup(&db);
    if (rc)
    {
        logger_error(DB_LOG_MODULE_ID, "Cannot open database: %s\n", sqlite3_errmsg(db));
        return (0);
    }
    return rc;
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
        char* sql = "CREATE TABLE IF NOT EXISTS TELEMATICS(ID INTEGER PRIMARY KEY AUTOINCREMENT, creation_time TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP, Lat FLOAT, LatSign Text, Long FLOAT, LongSign Text, PDOP FLOAT, HDOP FLOAT, VDOP FLOAT,
            Serial TEXT, VIN TEXT, Speed INT, Dist_Travelled TEXT, Idle_time TEXT, Veh_in_Service TEXT,Motion INT, Voltage Text, PTO INT); "
            "INSERT INTO TELEMATICS (Lat,LatSign,Long,LongSign,PDOP, HDOP, VDOP,Serial,VIN,Speed,Dist_Travelled,Idle_time,Veh_in_Service,Motion,Voltage,PTO) VALUES ('12.9010, 'N', '97.0013, 'E', '2.95', '3.02', '2.15','12345', '12345', '45kmph', '1800km', '27mins', '86mins',1, '0.0000', 0);"
        result = sqlite3_exec(db, sql, 0, 0, &err_msg);

        if (result != SQLITE_OK)
        {
            fprintf(stderr, "SQL error: %s\n", err_msg);
            sqlite3_free(err_msg);
            sqlite3_close(db);
            return 1;
        }
    }
    return rc;
}

int insert_telematics_data(struct cloud_data_struct* inCloud_data)
{
    cloud_data = inCloud_data;
    char* err_msg = 0;
    sqlite3* db;
    int result;
    char* sql;
    snprintf(sql, 2048, "INSERT INTO Telematics VALUES (%f, %c,%f,%c,%f,%f,%f,%s,%s,%d,%s,%s,%s,%d,%s,%d)", cloud_data->gps_data.latitude, cloud_data->gps_data.lat_cardinal_sign, cloud_data->gps_data.longitude, cloud_data->gps_data.long_cardinal_sign,
        cloud_data->gps_data.pdop, cloud_data->gps_data.hdop, cloud_data->gps_data.vdop, '12345', cloud_data->can_data.vin, cloud_data->can_data.speed, '1800km', '27mins', '86mins', cloud_data->client_controller_data.motion, cloud_data->client_controller_data.voltage, cloud_data->client_controller_data.pto);
    printf("query is: %s", sql);
    int rc = sqlite3_open("test.db", &db);
    result = sqlite3_exec(db, sql, 0, 0, &err_msg);

    if (result != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(db);
        return 1;
    }
    else
    {
        logger_info(CLOUD_LOG_MODULE_ID, "Data inserted successfully");
    }

    sqlite3_close(db);
}