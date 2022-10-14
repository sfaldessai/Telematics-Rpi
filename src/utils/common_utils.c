/*
 * created at 2022-09-07 19:39.
 *
 * Company HashedIn By Deloitte
 * Copyright (C) 2022 HashedIn By Deloitte
 */

#include <stdio.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string.h>
#include <math.h>
#include <linux/if.h>
#include "common_utils.h"
#include "../logger/logger.h"

/*
 * Name : get_master_mac_address
 * Descriptoin: The get_master_mac_address function is for fetching Raspberry Pi Ethernet MAC Address.
 * Input parameters:
 *                  uint8_t *mac_address : unsigned char array to Update the 12 character Master MAC address to cloud struct
 *                                         Sample MAC Address = b827ebe55413
 * Output parameters: void
 */
void get_master_mac_address(uint8_t *mac_address)
{
    struct ifreq ifr;
    int sock;
    char *ifname = NULL;

    /* for Ethernet */
    ifname = ETHERNET;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    strcpy(ifr.ifr_name, ifname);
    ifr.ifr_addr.sa_family = AF_INET;

    if (ioctl(sock, SIOCGIFHWADDR, &ifr) < 0)
    {
        logger_error(MAIN_LOG_MODULE_ID, "Error: failed to fetch MAC Address - %s\r\n", __func__);
    }

    memcpy(mac_address, ifr.ifr_hwaddr.sa_data, MAC_ADDRESS_HEX_LEN);
    sprintf((char *)mac_address, "%.2x%.2x%.2x%.2x%.2x%.2x",
            ifr.ifr_hwaddr.sa_data[0], ifr.ifr_hwaddr.sa_data[1], ifr.ifr_hwaddr.sa_data[2],
            ifr.ifr_hwaddr.sa_data[3], ifr.ifr_hwaddr.sa_data[4], ifr.ifr_hwaddr.sa_data[5]);

    logger_info(MAIN_LOG_MODULE_ID, "Master MAC Address: %s\n", mac_address);

    close(sock);
}

/*
 * Name : hex_to_decimal
 *
 * Description: The hex_to_decimal function is for converting hex into decimal value.
 *
 * Input parameters:
 *					uint8_t *read_data reference type, input hex bytes.
 *
 * Output parameters: converted decimal value
 */
uint16_t hex_to_decimal(uint8_t *read_data)
{
    uint16_t decimal = 0;
    int val = 0;
    size_t len = strlen((char *)read_data);
    len = len - 1;

    for (size_t i = 0; read_data[i] != '\0'; i++)
    {
        if (read_data[i] >= '0' && read_data[i] <= '9')
        {
            val = read_data[i] - 48;
        }
        else if (read_data[i] >= 'a' && read_data[i] <= 'f')
        {
            val = read_data[i] - 97 + 10;
        }
        else if (read_data[i] >= 'A' && read_data[i] <= 'F')
        {
            val = read_data[i] - 65 + 10;
        }
        decimal = (uint16_t)(decimal + (val * pow(16, len)));
        len = len - 1;
    }
    return decimal;
}