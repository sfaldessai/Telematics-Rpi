/*
 * created at 2022-07-29 14:00.
 *
 * Company: HashedIn By Deloitte.
 * Copyright (C) 2022 HashedIn By Deloitte
 */

#include "../gps_module/gps_module.h"
#include "../client_controller/client_controller.h"

#ifndef DB_HANDLER_H_
#define DB_HANDLER_H_

#define DEBUG
struct cloud_data_struct* cloud_data;
int db_setup();

void initialize_db();

void insert_telematics_data(struct cloud_data_struct* inCloud_data);

#endif