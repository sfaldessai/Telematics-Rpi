/*
 * created at 2022-09-07 19:39.
 *
 * Company HashedIn By Deloitte
 * Copyright (C) 2022 HashedIn By Deloitte
 */
#ifndef COMMON_UTILS_H_
#define COMMON_UTILS_H_

#define ETHERNET "eth0"
#define MAC_ADDRESS_HEX_LEN 6
#define PATH_BUF_SIZE 128

#define DEBUG

void get_master_mac_address(uint8_t *);

uint16_t hex_to_decimal(uint8_t *);
void add_binary_values(uint8_t *, int *, char *);
int verify_checksum(const char *sentence, int module_id, char, char);
char *get_device_path(char **device_name, int len);


#endif