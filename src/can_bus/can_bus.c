/*
 * created at 2022-07-29 14:30.
 *
 * Company: HashedIn By Deloitte.
 * Copyright (C) 2022 HashedIn By Deloitte
 */

#include <stdio.h>
#include "can_bus.h"

void *read_from_can(void *arg)
{   
    struct can_data_Struct v_in;
    unsigned int l_data = ;
    unsigned int l_pid = ;
    if(l_pid==0x02)
    {
        v_in.pid=l_pid;
        v_in.data=l_data;
    }

}

//for every 1000ms read vehicle speed data
void *read_canRx_1000ms(void *arg)
{
    struct can_data_Struct v_speed;
    unsigned int l_data ;
    unsigned int l_pid ;
    while(1)
    {
        l_pid = ;
        l_data = ;
        sleep(1);
        if(l_pid==0x0d)
        {
            v_speed.pid=l_pid;
            v_speed.data=l_data;
        }
    }
    
}


//for every 30ms read PID 0x00
void *read_canRx_30000ms(void *arg)
{
    struct can_data_Struct poll_data;
    unsigned int l_data;
    unsigned int l_pid;
    while(1)
    {
        l_pid = ;
        l_data = ;
        sleep(30);
        if(l_pid==0x00)
        {
            poll_data.pid=l_pid;
            poll_data.data=l_data;
        }
    }
    
}

//for every 30ms read PID 0x00
void *read_canAble(void *arg)
{
    read_from_can();
    read_canRx_1000ms();
    read_canRx_30000ms();
}
