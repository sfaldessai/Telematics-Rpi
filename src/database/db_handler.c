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

void initialize_db() {
    sqlite3* db;
    int rc = db_setup(&db);
    if (rc)
    {
        logger_error(DB_LOG_MODULE_ID, "Cannot open database: %s\n", sqlite3_errmsg(db));
        return (0);
    }
}

int db_setup(sqlite3* db)
{
    char* err_msg = 0;

    int rc = sqlite3_open("test.db", &db);

    if (rc != SQLITE_OK)
    {
        logger_error(DB_LOG_MODULE_ID, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);

        return 1;
    }
    else {
        int result;
        char* sql = "DROP TABLE IF EXISTS Telematics;"
            "CREATE TABLE Telematics(Date DATETIME,Motion INT,Voltage Text,PTO INT,Lat TEXT,Long TEXT,DOP TEXT,Serial TEXT,VIN TEXT,Speed TEXT,Dist_Travelled, TEXT, Idle_time TEXT, Veh_in_Service TEXT);"
            "INSERT INTO Telematics VALUES (datetime("now"), 1, '0.0000', 0, '12.9010 N', '97.0013 E', '2.95', '12345', '12345', '45kmph', '1800km', '27mins', '86mins'); "
            "INSERT INTO Telematics VALUES (datetime("now"), 1, '0.0000', 0, '12.9010 N', '97.0013 E', '2.95', '12345', '12345', '45kmph', '1800km', '27mins', '86mins'); "
            "INSERT INTO Telematics VALUES (datetime("now"), 1, '0.0000', 0, '12.9010 N', '97.0013 E', '2.95', '12345', '12345', '45kmph', '1800km', '27mins', '86mins'); "
        result = sqlite3_exec(db, sql, 0, 0, &err_msg);
    }
    return rc;
}

void insert_telematics_data(struct cloud_data_struct* inCloud_data)
{
    cloud_data = inCloud_data;
    char* err_msg = 0;
    sqlite3* db;
    int result;
    char* sql = "INSERT INTO Telematics VALUES (datetime("now"), cloud_data->client_controller_data.motion, cloud_data->client_controller_data.voltage, cloud_data->client_controller_data.pto,
        cloud_data->gps_data.latitude, cloud_data->gps_data.longitude, cloud_data->gps_data.pdop , '12345', cloud_data->can_data.vin, cloud_data->can_data.speed, '1800km', '27mins', '86mins'); "

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