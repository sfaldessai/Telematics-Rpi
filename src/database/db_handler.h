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

int db_setup();

int initialize_db();

int insert_telematics_data(struct cloud_data_struct* inCloud_data);

#endif