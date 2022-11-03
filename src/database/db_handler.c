/*
 * created at 2022-07-27 12:00.
 *
 * Company: HashedIn By Deloitte.
 * Copyright (C) 2022 HashedIn By Deloitte
 */

#include <stdio.h>
#include <unistd.h>
#include <sqlite3.h>
#include <string.h>
#include "db_handler.h"
#include "../logger/logger.h"

/*
 * Name : initialize_db
 * Descriptoin: The initialize_db function is for creating a Telematic db and table if not exits
 * Input parameters: void
 * Output parameters: int: returning sqlite success or error code
 */
int initialize_db()
{
    sqlite3 *db = NULL;
    char *err_msg = 0;

    int rc = sqlite3_open(TELEMATICS_DB_PATH, &db);

    if (rc != SQLITE_OK)
    {
        logger_error(DB_LOG_MODULE_ID, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);

        return rc;
    }
    else
    {
        char *sql = SQL_CREATE_TABLE;
        rc = sqlite3_exec(db, sql, 0, 0, &err_msg);

        if (rc != SQLITE_OK)
        {
            logger_error(DB_LOG_MODULE_ID, "SQL error: %s:%d\n", err_msg, rc);
            sqlite3_free(err_msg);
            sqlite3_close(db);
            return rc;
        }
    }

    sqlite3_close(db);
    return 0;
}

/*
 * Name : insert_telematics_data
 * Descriptoin: The insert_telematics_data function is for inserting CAN, STM32, GPS, & other consolidated data into a database.
 * Input parameters: struct cloud_data_struct: which contains CAN, STM32, GPS, & other consolidated data.
 * Output parameters: int: returning sqlite success or error code
 */
int insert_telematics_data(struct cloud_data_struct *cloud_data)
{
    sqlite3_stmt *stmt;

    char *err_msg = NULL;
    sqlite3 *db;
    int rc = 1;
    char sql[QUERY_MAX_LEN];

    char supported_pids[CAN_PID_LENGTH + 1];
    size_t i = 0;

    /* Prepare Query String START */
    for (i = 0; i < CAN_PID_LENGTH; i++)
    {
        sprintf(&supported_pids[i], "%d", cloud_data->can_data.supported_pids[i]);
    }
    supported_pids[i] = '\0';

    sprintf(sql, DB_QUERY, cloud_data->gps_data.latitude,
            cloud_data->gps_data.longitude,
            cloud_data->gps_data.pdop, cloud_data->gps_data.hdop, cloud_data->gps_data.vdop,
            cloud_data->mac_address, cloud_data->can_data.vin, cloud_data->can_data.vehicle_type, cloud_data->can_data.speed,
            supported_pids, 180.00, cloud_data->idle_time_secs, 86, cloud_data->client_controller_data.motion,
            cloud_data->client_controller_data.voltage, cloud_data->client_controller_data.pto,
            cloud_data->client_controller_data.acc_x, cloud_data->client_controller_data.acc_y, cloud_data->client_controller_data.acc_x,
            cloud_data->can_data.rpm, cloud_data->can_data.temperature);

    logger_info(DB_LOG_MODULE_ID, "SQL QUERY: %s\n", sql);
    /* Prepare Query String END */

    /* Open DB */
    rc = sqlite3_open(TELEMATICS_DB_PATH, &db);

    if (rc != SQLITE_OK)
    {
        logger_error(DB_LOG_MODULE_ID, "Cannot open database: %s:%d\n", sqlite3_errmsg(db), rc);
        sqlite3_close(db);
        return rc;
    }

    /* The below setting is to speed up data insertion */
    rc = sqlite3_exec(db, "PRAGMA synchronous = OFF", 0, 0, &err_msg);

    if (rc != SQLITE_OK)
    {
        logger_error(DB_LOG_MODULE_ID, "Cannot open database: %s:%d\n", err_msg, rc);
        sqlite3_free(err_msg);
        sqlite3_close(db);
        return rc;
    }

    rc = sqlite3_prepare_v2(db, sql, (int)strlen(sql), &stmt, NULL);

    if (rc != SQLITE_OK)
    {
        logger_error(DB_LOG_MODULE_ID, "Cannot open database: %s:%d\n", sqlite3_errmsg(db), rc);
        sqlite3_close(db);
        return rc;
    }

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE)
    {
        logger_error(DB_LOG_MODULE_ID, "SQL Prepare Statement error: %s:%d\n", sqlite3_errmsg(db), rc);
        sqlite3_close(db);
        return rc;
    }
    else
    {
        logger_info(DB_LOG_MODULE_ID, "Data inserted successfully");
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);

    return 0;
}

/*
 * Name : get_single_column_value
 *
 * Descriptoin: The get_single_column_value function is for fetching single column namd and returning string value.
 *
 * Input parameters: char *column_name: column name
 *                   char *sort_by: sort option
 *                   uint8_t *return_value: to update retuned column value and return
 *
 * Output parameters: int: returning sqlite success or error code
 */
int get_single_column_value(char *column_name, char *sort_by, uint8_t *return_value)
{
    sqlite3 *db;
    sqlite3_stmt *res;
    char sql[QUERY_MAX_LEN];

    int rc = sqlite3_open(TELEMATICS_DB_PATH, &db);

    if (rc != SQLITE_OK)
    {
        logger_error(DB_LOG_MODULE_ID, "Cannot open database: %s:%d\n", sqlite3_errmsg(db), rc);
        sqlite3_close(db);
        return rc;
    }

    sprintf(sql, "SELECT %s FROM %s order by %s %s limit 1", column_name, TELEMATICS, creation_time, sort_by);

    logger_info(DB_LOG_MODULE_ID, "SQL QUERY: %s\n", sql);

    rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);

    if (rc == SQLITE_OK)
    {
        sqlite3_bind_int(res, 1, 1);
    }
    else
    {
        fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
    }

    int step = sqlite3_step(res);

    if (step == SQLITE_ROW)
    {
        strncpy((char *)return_value, (char *)sqlite3_column_text(res, 0), COLUMN_VALUE_MAX_LEN);
    }

    sqlite3_finalize(res);
    sqlite3_close(db);

    return 0;
}