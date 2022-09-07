/*
 * created at 2022-09-07 19:39.
 *
 * Company HashedIn By Deloitte
 * Copyright (C) 2022 HashedIn By Deloitte
 */

#include <stdio.h>
#include <stdint.h>
#include <linux/if.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string.h>
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