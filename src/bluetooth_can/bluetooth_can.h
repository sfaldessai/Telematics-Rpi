/*
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

#define ELM_TEST "ATZ"
#define SET_AUTO_PROTOCOL_SELECT "AT SP 0"
#define ELM_VERSION_RESPONSE "ELM327 v1.5"
#define OK "OK"
#define SERVICE_SHOW_CURRENT_DATA "01"
#define SUPPORTED_PID "00"
#define SPEED_PID "0D"
#define RPM_PID "0C"
#define TEMPERATURE_PID "05"
#define VIN_PID "02"

int str2uuid( const char *, uuid_t * ); 
int setup_bluetooth_connection(int *,char *,char *);
void ble_close_socket(int *);
int ble_can_request(int , char *);
int ble_can_response(int , char *);
void *read_from_ble_can(void *);
int initialize_ELM(void);
int get_pid_response_by_request(char *,char * );
int get_supported_pids(void);



#endif