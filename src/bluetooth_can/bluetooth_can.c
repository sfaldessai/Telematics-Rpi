/*
 * created at 2022-11-04 07:33.
 *
 * Company: HashedIn By Deloitte.
 * Copyright (C) 2022 HashedIn By Deloitte
 */

#include "bluetooth_can.h"
#include "../main.h"

int sockfd = 0;
char response_buf[80]={0}; 
void *read_from_ble_can(void *arg)
{
    struct cloud_data_struct *cloud_data = (struct cloud_data_struct *)arg;    
    int init_status;
    if (setup_bluetooth_connection(&sockfd,BLE_DEVICE_SERIAL_PROFILE_UUID,BLE_DEVICE_NAME) == 0)
    {
        init_status=initialize_ELM();
        if(init_status==0){
            while(1){
            get_supported_pids();
            /*TBD : Verify data structure and add other pids*/
            }
        }        
    }
}

int initialize_ELM(void)
{
        
        int rc=ble_can_request(sockfd,ELM_TEST)
        if(rc!=sizeof(ELM_TEST)){
            logger_error(BLE_CAN_MODULE_ID,"ELM_TEST write error %s\n",__func__);
            return -1;
        }
        rc=ble_can_response(sockfd,&response_buf);
        if(strcmp(response_buf,ELM_VERSION_RESPONSE)!=0){
            logger_error(BLE_CAN_MODULE_ID,"error in version test %s\n",__func__);
            return -1
        }
        rc=ble_can_request(sockfd,SET_AUTO_PROTOCOL_SELECT);
        if(rc!=sizeof(SET_AUTO_PROTOCOL_SELECT)){
            logger_error(BLE_CAN_MODULE_ID,"SET_AUTO_PROTOCOL_SELECT write error %s\n",__func__);
            return -1;
        }
        rc=ble_can_response(sockfd,&response_buf);
        if(strcmp(response_buf,OK)!=0){
            logger_error(BLE_CAN_MODULE_ID,"error in protocol selection %s\n",__func__);
            return -1
        }
        return 0;
}

int get_pid_response_by_request(char *request,char * response)
{
        int rc=ble_can_request(sockfd,&request)
        if(rc!=sizeof(request)){
            logger_error(BLE_CAN_MODULE_ID,"Request : %s write error %s\n",*request,__func__);
            return -1;
        }
        rc=ble_can_response(sockfd,&response);
        if(strcmp(response,ELM_VERSION_RESPONSE)!=0){
            logger_error(BLE_CAN_MODULE_ID,"error in getting response for %s - %s\n",*request,__func__);
            return -1
        }
        return 0;
}

int get_supported_pids(void){
    char query[4]=SERVICE_SHOW_CURRENT_DATA;
    strcat(query,SUPPORTED_PID);
    memset(response_buf,0,80);
    int rc=get_pid_response_by_request(query,&response_buf);
    if(rc>0){
        logger_info(BLE_CAN_MODULE_ID,"Response : %s \n",response_buf);
    }
}

