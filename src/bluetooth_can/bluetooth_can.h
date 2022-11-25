/*"
 * created at 2022-11-04 07:33.
 *
 * Company: HashedIn By Deloitte.
 * Copyright (C) 2022 HashedIn By Deloitte
 */

#ifndef BLUETOOTH_CAN_H_
#define BLUETOOTH_CAN_H_

#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <bluetooth/sdp.h>
#include <bluetooth/sdp_lib.h>
#include <bluetooth/rfcomm.h>

#define ELM_TEST "atz"
#define SET_AUTO_PROTOCOL_SELECT "AT SP 0"
#define ELM_VERSION_RESPONSE "ELM327 v1.5"
#define OK "OK"
#define ECHO_OFF "AT E 0"
#define SERVICE_SHOW_CURRENT_DATA "01"
#define BATTERY_CMD "AT RV"
#define GET_VEHICLE_DATA_MODE "09"
#define BLE_SUPPORTED_PID "00"
#define BLE_SPEED_PID "0D"
#define BLE_RPM_PID "0C"
#define BLE_TEMPERATURE_PID "05"
#define BLE_VIN_PID "02"
#define BYTE_SIZE 1
#define DELIMITER '>'
#define EMPTY_SPACE " "
#define SEARCHING "SEARCHING"
#define NODATA "NO DATA"
#define ENDCHAR '\n'

enum can_state{SEND,INPROGRESS,RECEIVED};

int str2uuid( const char *, uuid_t * ); 
int setup_bluetooth_connection(int *,sdp_session_t *,sdp_record_t *,char *,char *);
void ble_close_socket(int *);
int ble_can_request(int , char *,int);
int ble_can_response(int , char *);
void *read_from_ble_can(void *);
int initialize_ELM(void);
int get_pid_response_by_request(char *,char * ,int);
void get_supported_pids(void *arg);
void ble_hex_to_binary(int*, uint8_t*);
void get_speed(void *arg);
void get_temperature(void *arg);
void get_vin(void *arg);
void get_rpm(void *arg);
void get_battery(void *arg);
int ble_can_response_byte(int, char *,int);
void get_response(char *);
void get_vin_response(char *);

#endif
