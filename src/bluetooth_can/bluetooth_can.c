/*
 * created at 2022-11-04 07:33.
 *
 * Company: HashedIn By Deloitte.
 * Copyright (C) 2022 HashedIn By Deloitte
 */

#include "../main.h"

int blesockfd = 0;
sdp_session_t session;
sdp_record_t rec;
char response_buf[80] = {0};
enum can_state flag;
void *read_from_ble_can(void *arg)
{
    struct cloud_data_struct *cloud_data = (struct cloud_data_struct *)arg;
    int init_status;
    if (setup_bluetooth_connection(&blesockfd, &session, &rec, BLE_DEVICE_SERIAL_PROFILE_UUID, BLE_DEVICE_NAME) == 0)
    {
        flag = SEND;
        init_status = initialize_ELM();
        if (init_status == 0)
        {
            get_supported_pids(cloud_data);
            get_vin(cloud_data);
            while (1)
            {
                get_speed(cloud_data);
                get_rpm(cloud_data);
                get_temperature(cloud_data);
                get_battery(cloud_data);
                sleep(1);
            }
            ble_close_socket(&blesockfd);
            sdp_record_free(&rec);
            sdp_close(&session);
        }
    }
    return NULL;
}

int initialize_ELM(void)
{
    int rc;
    rc = get_pid_response_by_request(ECHO_OFF, response_buf, sizeof(ECHO_OFF));
    if (rc < 0)
    {
        logger_error(BLE_CAN_MODULE_ID, "ECHO_OFF write error %s\n", __func__);
        return -1;
    }
    // if (strcmp(response_buf, OK) != 0)
    // {
    //     logger_error(BLE_CAN_MODULE_ID, "ECHO_OFF read error %s\n", __func__);
    // }
    return 0;
}

int get_pid_response_by_request(char *request, char *response, int req_len)
{
    if (flag == SEND)
    {
        int rc = ble_can_request(blesockfd, request, req_len);
        if (rc != req_len)
        {
            logger_error(BLE_CAN_MODULE_ID, "Request : %s write error %s\n", request, __func__);
            return -1;
        }
        get_response(response);
        while (strstr(response, SEARCHING))
        {
            memset(response, 0, 80);
            get_response(response);
        }
        if (flag == RECEIVED)
        {
            flag = SEND;
            return 1;
        }
    }
    return -1;
}

void get_response(char *response)
{
    char read_data;
    int i = 0;
    int rc;
    do
    {
        flag = INPROGRESS;
        rc = ble_can_response_byte(blesockfd, &read_data, BYTE_SIZE);
        if (rc > 0)
        {
            response[i] = read_data;
            i++;
        }
    } while (read_data != DELIMITER);
    response[i] = '\0';
    flag = RECEIVED;
}
void get_vin_response(char *response)
{
    char read_data;
    int i = 0;
    int rc;
    do
    {
        rc = ble_can_response_byte(blesockfd, &read_data, BYTE_SIZE);
        if (rc > 0 && read_data!='\n' && read_data!='\r')
        {
            response[i] = read_data;
            i++;
        }
    } while (read_data!=DELIMITER);
    response[--i] = '\0'; 
    
}

void get_supported_pids(void *arg)
{
    struct cloud_data_struct *cloud_data = (struct cloud_data_struct *)arg;
    char query[5] = SERVICE_SHOW_CURRENT_DATA;
    char *temp;
    char hex_arr[10][10];
    int resp[10];
    uint8_t supported_binary_value[CAN_PID_LENGTH];
    int i = 0;
    strcat(query, BLE_SUPPORTED_PID);
    memset(response_buf, 0, 80);
    int rc = get_pid_response_by_request(query, response_buf, sizeof(query));
    if (rc)
    {
        temp = strtok(response_buf, EMPTY_SPACE);

        while (temp != NULL)
        {
            sprintf(hex_arr[i], "%s", temp);
            temp = strtok(NULL, EMPTY_SPACE);
            i++;
        }
        for (i = 0; i < 10; i++)
        {
            resp[i] = (int)strtol(hex_arr[i], NULL, 16);
        }
        if (strstr(hex_arr[1], BLE_SUPPORTED_PID))
        {
            ble_hex_to_binary(resp, supported_binary_value);
            for (i = 0; i < CAN_PID_LENGTH; i++)
            {
                cloud_data->can_data.supported_pids[i] = supported_binary_value[i];
            }
        }
        logger_info(BLE_CAN_MODULE_ID, "Response : %s \n", response_buf);
    }
}
void get_vin(void *arg)
{
    struct cloud_data_struct *cloud_data = (struct cloud_data_struct *)arg;
    char query[5] = GET_VEHICLE_DATA_MODE;
    char vin_buff[1024];
    char vin_lines[1024]={0};
    char *temp;
    int i = 0,j=0,k=0;
    char hex_arr[25][10];
    int vin_dec[17];
    char vin[18];
    uint8_t wmi[4];

    strcat(query, BLE_VIN_PID);
    int rc = ble_can_request(blesockfd, query, strlen(query));
    if (rc != strlen(query))
    {
        logger_error(BLE_CAN_MODULE_ID, "Request : %s write error %s\n", query, __func__);
    }
    memset(vin_buff, 0, 1024);
    get_vin_response(vin_buff);
     for(i=4;i<strlen(vin_buff)-1;i++){
         vin_lines[j]=vin_buff[i];
         j++;
     }
     temp = strtok(vin_lines, EMPTY_SPACE);
     i=0;
     while (temp != NULL)
     {
         sprintf(hex_arr[i], "%s", temp);
         temp = strtok(NULL, EMPTY_SPACE);
         i++;
     }
     for (j = 0; j < i; j++)
     {
         if (j == 0 || j==1 || j==2 || j==3 || j == 7 || j == 15)
         {
             continue;
         }
         else
         {
             vin_dec[k] = hex_to_decimal((uint8_t *)hex_arr[j]);
             char dec_ascii = vin_dec[k];
             vin[k]=dec_ascii;
             cloud_data->can_data.vin[k] = vin[k];
             k++;
         }
     }
     vin[k]='\0';
     for (i = 0; i < WMI_LEN; i++)
        {
        wmi[i] = vin[i];
        }
        wmi[i]='\0';

    char *vehicle_detail = get_manufacturer_detail(wmi);
    strncpy(cloud_data->can_data.vehicle_type, vehicle_detail, WMI_STRING_LEN - 1);
     logger_info(BLE_CAN_MODULE_ID, "FINAL VIN : %s \n", vin);
     logger_info(BLE_CAN_MODULE_ID,"Vehicle Type : %s \n",cloud_data->can_data.vehicle_type);
}

void get_speed(void *arg)
{
    struct cloud_data_struct *cloud_data = (struct cloud_data_struct *)arg;
    char query[5] = SERVICE_SHOW_CURRENT_DATA;
    char *temp;
    char hex_arr[10][10];
    int i = 0;
    strcat(query, BLE_SPEED_PID);
    memset(response_buf, 0, 80);
    int rc = get_pid_response_by_request(query, response_buf, sizeof(query));
    if (rc)
    {
        temp = strtok(response_buf, EMPTY_SPACE);

        while (temp != NULL)
        {
            sprintf(hex_arr[i], "%s", temp);
            temp = strtok(NULL, EMPTY_SPACE);
            i++;
        }
        if (strstr(hex_arr[1], BLE_SPEED_PID))
        {
            cloud_data->can_data.speed = (uint16_t)hex_to_decimal((uint8_t *)hex_arr[2]);
            logger_info(BLE_CAN_MODULE_ID, "SPEED: %d \r\n", cloud_data->can_data.speed);
        }
    }
}

void get_temperature(void *arg)
{
    struct cloud_data_struct *cloud_data = (struct cloud_data_struct *)arg;
    char query[5] = SERVICE_SHOW_CURRENT_DATA;
    char *temp;
    char hex_arr[10][10];
    int i = 0;
    strcat(query, BLE_TEMPERATURE_PID);
    memset(response_buf, 0, 80);
    int rc = get_pid_response_by_request(query, response_buf, sizeof(query));
    if (rc)
    {
        temp = strtok(response_buf, EMPTY_SPACE);

        while (temp != NULL)
        {
            sprintf(hex_arr[i], "%s", temp);
            temp = strtok(NULL, EMPTY_SPACE);
            i++;
        }

        if (strstr(hex_arr[1], BLE_TEMPERATURE_PID))
        {
            cloud_data->can_data.temperature = hex_to_decimal((uint8_t *)hex_arr[2]) - 40;
            logger_info(BLE_CAN_MODULE_ID, "Temperature : %d\r\n", cloud_data->can_data.temperature);
        }
    }
}

void get_rpm(void *arg)
{
    struct cloud_data_struct *cloud_data = (struct cloud_data_struct *)arg;
    char read_rpm[8];
    char *temp;
    char hex_arr[10][10];
    int i = 0;
    char query[5] = SERVICE_SHOW_CURRENT_DATA;
    strcat(query, BLE_RPM_PID);
    memset(response_buf, 0, 80);
    int rc = get_pid_response_by_request(query, response_buf, sizeof(query));
    if (rc)
    {
        temp = strtok(response_buf, EMPTY_SPACE);

        while (temp != NULL)
        {
            sprintf(hex_arr[i], "%s", temp);
            temp = strtok(NULL, EMPTY_SPACE);
            i++;
        }

        if (strstr(hex_arr[1], BLE_RPM_PID))
        {
            strcat(read_rpm, hex_arr[2]);
            strcat(read_rpm, hex_arr[3]);
            // sprintf((char *)read_rpm, "%s%s", hex_arr[2], hex_arr[3]);
            cloud_data->can_data.rpm = (float)hex_to_decimal((uint8_t *)read_rpm) * 0.25;
            logger_info(BLE_CAN_MODULE_ID, "RPM: %f\r\n", cloud_data->can_data.rpm);
        }
    }
}

void get_battery(void *arg)
{
    struct cloud_data_struct *cloud_data = (struct cloud_data_struct *)arg;
    uint8_t read_battery[8];
    memset(response_buf, 0, 80);
    int rc = get_pid_response_by_request(BATTERY_CMD, response_buf, sizeof(BATTERY_CMD));
    if (rc)
    {
        sprintf((char *)read_battery, "%c%c%c%c", response_buf[0], response_buf[1], response_buf[2], response_buf[3]);
        cloud_data->can_data.battery = atof((char *)read_battery);
        logger_info(BLE_CAN_MODULE_ID, "Battery : %.2f", cloud_data->can_data.battery);
    }
}
/*
 * Name : ble_hex_to_binary
 *
 * Descriptoin: The hex_to_binary function is for extracting VIN data from 3 can frames and converting into string value.
 *
 * Input parameters:
 *                  struct can_frame supported_frame: can frame data for supported PIDs.
 *					uint8_t *supported_binary: referenc type, updating binary supported PIDs data from can frame data.
 *
 * Output parameters: void
 */
void ble_hex_to_binary(int *response, uint8_t *supported_binary)
{
    size_t i = 0;
    int index = 0;

    uint8_t supported_value[16];
    sprintf((char *)supported_value, "%X%X%X%X", response[2], response[3], response[4], response[5]);

    while (supported_value[i])
    {

        switch (supported_value[i])
        {
        case '0':
            add_binary_values(supported_binary, &index, "0000");
            break;
        case '1':
            add_binary_values(supported_binary, &index, "0001");
            break;
        case '2':
            add_binary_values(supported_binary, &index, "0010");
            break;
        case '3':
            add_binary_values(supported_binary, &index, "0011");
            break;
        case '4':
            add_binary_values(supported_binary, &index, "0100");
            break;
        case '5':
            add_binary_values(supported_binary, &index, "0101");
            break;
        case '6':
            add_binary_values(supported_binary, &index, "0110");
            break;
        case '7':
            add_binary_values(supported_binary, &index, "0111");
            break;
        case '8':
            add_binary_values(supported_binary, &index, "1000");
            break;
        case '9':
            add_binary_values(supported_binary, &index, "1001");
            break;
        case 'A':
        case 'a':
            add_binary_values(supported_binary, &index, "1010");
            break;
        case 'B':
        case 'b':
            add_binary_values(supported_binary, &index, "1011");
            break;
        case 'C':
        case 'c':
            add_binary_values(supported_binary, &index, "1100");
            break;
        case 'D':
        case 'd':
            add_binary_values(supported_binary, &index, "1101");
            break;
        case 'E':
        case 'e':
            add_binary_values(supported_binary, &index, "1110");
            break;
        case 'F':
        case 'f':
            add_binary_values(supported_binary, &index, "1111");
            break;
        default:
            break;
        }
        i++;
    }
}
