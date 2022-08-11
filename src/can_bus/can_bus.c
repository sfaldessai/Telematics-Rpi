/*
 * created at 2022-07-29 14:30.
 *
 * Company: HashedIn By Deloitte.
 * Copyright (C) 2022 HashedIn By Deloitte
 */

#include <stdio.h>
#include "can_bus.h"
#include "../logger/logger.h"

#define CAN_LOG_MODULE_ID 4

void *read_from_can(void *arg)
{
    /* TODO */
    logger_info(CAN_LOG_MODULE_ID, "%p", arg);
    return 0;
}