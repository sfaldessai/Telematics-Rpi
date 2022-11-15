/*
 * created at 2022-11-04 07:33.
 *
 * Company: HashedIn By Deloitte.
 * Copyright (C) 2022 HashedIn By Deloitte
 */

#include "bluetooth_can.h"
#include "../main.h"
#include "../logger/logger.h"

/*
 * Name : str2uuid
 *
 * Descriptoin: This function converts uuid string to uuid format
 *
 * Input parameters:
 *                  char *uuid_str : uuid in string format
 * 					uuid_t *uuid : uuid format response
 *
 * Output parameters: int return uuid validation station
 */


int str2uuid(const char *uuid_str, uuid_t *uuid)
{
    uint32_t uuid_int[4];
    char *endptr;

    if (strlen(uuid_str) == 36)
    {
        /* Parse uuid128 standard format: 12345678-9012-3456-7890-123456789012 */
        char buf[9] = {0};

        if (uuid_str[8] != '-' && uuid_str[13] != '-' &&
            uuid_str[18] != '-' && uuid_str[23] != '-')
        {
            return 0;
        }
        /* first 8-bytes */
        strncpy(buf, uuid_str, 8);
        uuid_int[0] = htonl(strtoul(buf, &endptr, 16));
        if (endptr != buf + 8)
            return 0;

        /* second 8-bytes */
        strncpy(buf, uuid_str + 9, 4);
        strncpy(buf + 4, uuid_str + 14, 4);
        uuid_int[1] = htonl(strtoul(buf, &endptr, 16));
        if (endptr != buf + 8)
            return 0;

        /* third 8-bytes */
        strncpy(buf, uuid_str + 19, 4);
        strncpy(buf + 4, uuid_str + 24, 4);
        uuid_int[2] = htonl(strtoul(buf, &endptr, 16));
        if (endptr != buf + 8)
            return 0;

        /* fourth 8-bytes */
        strncpy(buf, uuid_str + 28, 8);
        uuid_int[3] = htonl(strtoul(buf, &endptr, 16));
        if (endptr != buf + 8)
            return 0;

        if (uuid != NULL)
            sdp_uuid128_create(uuid, uuid_int);
    }
    else if (strlen(uuid_str) == 8)
    {
        /* 32-bit reserved UUID */
        uint32_t i = strtoul(uuid_str, &endptr, 16);
        if (endptr != uuid_str + 8)
            return 0;
        if (uuid != NULL)
            sdp_uuid32_create(uuid, i);
    }
    else if (strlen(uuid_str) == 4)
    {
        /* 16-bit reserved UUID */
        int i = strtol(uuid_str, &endptr, 16);
        if (endptr != uuid_str + 4)
            return 0;
        if (uuid != NULL)
            sdp_uuid16_create(uuid, i);
    }
    else
    {
        return 0;
    }

    return 1;
}

/*
 * Name : setup_bluetooth_connection
 *
 * Descriptoin: This function searches for specified bluetooth device and connects to it
 *
 * Input parameters:
 *                  int sockfd: socket file descriptor
 * 					char * uuid:  uuid of bluetooth device
 *                  char * blename : name of bluetooth device
 *
 * Output parameters: int return connection status
 */

int setup_bluetooth_connection(int *sockfd, sdp_session_t *session, sdp_record_t *rec ,char *spuuid, char *blename)
{
    int i, err, sock, dev_id = -1;
    struct hci_dev_info dev_info;
    inquiry_info *info = NULL;
    int num_rsp, length, flags;
    char addr[19] = {0};
    char name[248] = {0};
    uuid_t uuid = {0};
    /* Change this to your apps UUID*/
    char *uuid_str = spuuid;
    uint32_t range = 0x0000ffff;
    sdp_list_t *response_list = NULL, *search_list, *attrid_list;
    int  loco_channel = -1, status;
    struct sockaddr_rc loc_addr = {0};

    (void)signal(SIGINT, SIG_DFL);

    dev_id = hci_get_route(NULL);
    if (dev_id < 0)
    {
        logger_error(BLE_CAN_MODULE_ID, "Error: No Bluetooth Adapter Available- %s\r\n", __func__);
        return -1;
    }

    if (hci_devinfo(dev_id, &dev_info) < 0)
    {
        logger_error(BLE_CAN_MODULE_ID, "Error: Can't get device info- %s\r\n", __func__);
        return -1;
    }

    sock = hci_open_dev(dev_id);
    if (sock < 0)
    {
        logger_error(BLE_CAN_MODULE_ID, "Error: HCI device open failed %s\r\n", __func__);
        free(info);
        return -1;
    }

    if (!str2uuid(uuid_str, &uuid))
    {
        logger_error(BLE_CAN_MODULE_ID, "Error: Invalid UUID %s\r\n", __func__);
        free(info);
        return -1;
    }

    logger_info(BLE_CAN_MODULE_ID, "Scanning ...\n");
    info = NULL;
    num_rsp = 0;
    flags = 0;
    length = 8; /* ~10 seconds */
    num_rsp = hci_inquiry(dev_id, length, num_rsp, NULL, &info, flags);
    if (num_rsp < 0)
    {
        logger_error(BLE_CAN_MODULE_ID, "Error: Inquiry failed %s\r\n", __func__);
    }

    logger_info(BLE_CAN_MODULE_ID, "No of resp %d\n", num_rsp);

    for (i = 0; i < num_rsp; i++)
    {
        sdp_session_t *session;
        int retries;
        int foundit, responses;
        ba2str(&(info + i)->bdaddr, addr);
        memset(name, 0, sizeof(name));
        if (hci_read_remote_name(sock, &(info + i)->bdaddr, sizeof(name),
                                 name, 0) < 0)
            strcpy(name, "[unknown]");
        printf("BLE NAME BEFORE CHECK : %s\r\n",name);
        if (!strcmp(name, blename))
        {
            logger_info(BLE_CAN_MODULE_ID, "Found %s  %s, searching for the the desired service on it now\n", addr, name);
            /* connect to the SDP server running on the remote machine */
        
            session = 0;
            retries = 0;
            while (!session)
            {
                session = sdp_connect(BDADDR_ANY, &(info + i)->bdaddr, SDP_RETRY_IF_BUSY);
                if (session)
                    break;
                if (errno == EALREADY && retries < 5)
                {
                    logger_error(BLE_CAN_MODULE_ID, "Retrying sdp connect- %s\r\n", __func__);
                    retries++;
                    sleep(1);
                    continue;
                }
                break;
            }
            if (session == NULL)
            {
                logger_error(BLE_CAN_MODULE_ID, "Error: Can't open session with the device %s\r\n", __func__);
                free(info);
                continue;
            }
            search_list = sdp_list_append(0, &uuid);
            attrid_list = sdp_list_append(0, &range);
            err = 0;
            err = sdp_service_search_attr_req(session, search_list, SDP_ATTR_REQ_RANGE, attrid_list, &response_list);
            logger_info(BLE_CAN_MODULE_ID,"errval:%d\r\n",err);
            sdp_list_t *r = response_list;
            //sdp_record_t *rec;
            /* go through each of the service records */
            foundit = 0;
            responses = 0;
            for (; r; r = r->next)
            {
                responses++;
                rec = (sdp_record_t *)r->data;
                sdp_list_t *proto_list;

                /* get a list of the protocol sequences */
                if (sdp_get_access_protos(rec, &proto_list) == 0)
                {
                    sdp_list_t *p = proto_list;

                    /* go through each protocol sequence */
                    for (; p; p = p->next)
                    {
                        sdp_list_t *pds = (sdp_list_t *)p->data;

                        /* go through each protocol list of the protocol sequence */
                        for (; pds; pds = pds->next)
                        {

                            /* check the protocol attributes */
                            sdp_data_t *d = (sdp_data_t *)pds->data;
                            int proto = 0;
                            for (; d; d = d->next)
                            {
                                switch (d->dtd)
                                {
                                case SDP_UUID16:
                                case SDP_UUID32:
                                case SDP_UUID128:
                                    proto = sdp_uuid_to_proto(&d->val.uuid);
                                    break;
                                case SDP_UINT8:
                                    if (proto == RFCOMM_UUID)
                                    {
                                        logger_info(BLE_CAN_MODULE_ID, "rfcomm channel: %d\n", d->val.int8);
                                        loco_channel = d->val.int8;
                                        foundit = 1;
                                    }
                                    break;
                                }
                            }
                        }
                        sdp_list_free((sdp_list_t *)p->data, 0);
                    }
                    sdp_list_free(proto_list, 0);
                }
                if (loco_channel > 0)
                    break;
            }
            logger_info(BLE_CAN_MODULE_ID, "No of Responses %d\n", responses);
            if (loco_channel > 0 && foundit == 1)
            {
                logger_info(BLE_CAN_MODULE_ID, "Found service on this device\n");
                *sockfd = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
                loc_addr.rc_family = AF_BLUETOOTH;
                loc_addr.rc_channel = loco_channel;
                loc_addr.rc_bdaddr = *(&(info + i)->bdaddr);
                
                status = connect(*sockfd, (struct sockaddr *)&loc_addr, sizeof(loc_addr));
                printf("BLE NAME : %s\r\n",name);
               
                printf("Status:%d\r\n",status);
                if (status < 0)
                {
                    logger_error(BLE_CAN_MODULE_ID, "Error: Socket Connection Failed %s\r\n", __func__);
                    return -1;
                }    

            }

        }
    }
    return 0;
}

/*
 * Name : ble_close_socket
 *
 * Descriptoin: This function closes the socket
 *
 * Input parameters:
 *                  int sockfd: socket file descriptor
 *
 * Output parameters: void
 */
void ble_close_socket(int *sockfd)
{
    if (*sockfd > 0 && close(*sockfd) < 0)
    {
        logger_error(BLE_CAN_MODULE_ID, "Error: Socket Close- %s\r\n", __func__);
    }
}

/*
 * Name : ble_can_request
 *
 * Descriptoin: This function sends can request to bluetooth socket
 *
 * Input parameters:
 *                  int sockfd: socket file descriptor
 *                  char *request : request string
 *
 * Output parameters: int size of sent bytes
 */
int ble_can_request(int sockfd, char *request,int request_len)
{
    int rc = write(sockfd, request, request_len);
    if (rc != request_len)
    {
        logger_error(BLE_CAN_MODULE_ID, "Error: Write failed- %s\r\n", __func__);
        return rc;
    }
    return rc;
}

/*
 * Name : ble_can_response
 *
 * Descriptoin: This function recieves  can response from bluetooth socket
 *
 * Input parameters:
 *                  int sockfd: socket file descriptor
 *                  char *response : response string
 *
 * Output parameters: int size of recieved bytes
 */
int ble_can_response(int sockfd, char *response)
{
    int nbytes = read(sockfd, response, 1024);    
    if (nbytes <= 0)
    {
        logger_error(BLE_CAN_MODULE_ID, "Error: Read failed- %s\r\n", __func__);
    }
    return nbytes;
}

int ble_can_response_byte(int sockfd, char *response,int len)
{
    int nbytes = read(sockfd, response, len);    
    if (nbytes <= 0)
    {
        logger_error(BLE_CAN_MODULE_ID, "Error: Read failed- %s\r\n", __func__);
    }
    return nbytes;
}

