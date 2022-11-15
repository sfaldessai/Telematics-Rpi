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

#define MAX_READ_CHAR_SIZE 80 /* GPS at most, sends 80 or so chars per message string.*/
#define NMEA_END_CHAR '\n'
#define GPS_NMEA_SENTENCE_CHECKSUM_ERROR 903
#define CHECKSUM_SUCESS_CODE 0
#define CC_CHECKSUM_ERROR 1002

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

/*
 * Name : add_binary_values
 *
 * Descriptoin: The add_binary_values function is for extracting VIN data from 3 can frames and converting into string value.
 *
 * Input parameters:
 *                  uint8_t *supported_binary: reference type, appedning all 32 PIDs binary data.
 *					int *index: referenc type, updating index value for supported_binary array index.
 *					char *binary: hex byte binary value, appending to supported_binary.
 * Output parameters: void
 */
void add_binary_values(uint8_t *supported_binary, int *index, char *binary)
{
	for (size_t i = 0; i < 4; i++)
	{
		supported_binary[*index] = (binary[i] - '0');
		*index = *index + 1;
	}
}


/*
 * Name : verify_checksum
 * Descriptoin: The nmea_verify_checksum function is for verifying GPS or STM32 checksum.
 * Input parameters:
 *                  const char *sentence : NMEA or STM32 senetence
 *
 * Output parameters: uint8_t: return 1 for invalid and 0 for valid
 */
int verify_checksum(const char *sentence, int module_id, char start_char, char end_char)
{
    int checksum = 0;
    uint8_t gps_checksum_hex[8];

    if (strlen(sentence) > MAX_READ_CHAR_SIZE || strchr(sentence, end_char) == NULL || strchr(sentence, start_char) == NULL)
    {
        logger_info(module_id, "Invalid sentence: %s\n", __func__);
        if (module_id == CC_LOG_MODULE_ID)
        {
            return CC_CHECKSUM_ERROR;
        }
        else
        {
            return GPS_NMEA_SENTENCE_CHECKSUM_ERROR;
        }
    }
    while (end_char != *sentence && NMEA_END_CHAR != *sentence)
    {
        if (start_char == *sentence)
        {
            sentence = sentence + 1;
            continue;
        }
        if ('\0' == *sentence)
        {
            logger_info(module_id, "Invalid sentence: %s %s\n", __func__, sentence);
            if (module_id == CC_LOG_MODULE_ID)
            {
                return CC_CHECKSUM_ERROR;
            }
            else
            {
                return GPS_NMEA_SENTENCE_CHECKSUM_ERROR;
            }
        }
        checksum = checksum ^ (uint8_t)*sentence;
        sentence = sentence + 1;
    }
    sentence = sentence + 1;

    if (strlen(sentence) >= 2)
    {
        gps_checksum_hex[0] = sentence[0];
        gps_checksum_hex[1] = sentence[1];
        gps_checksum_hex[2] = '\0';
    }
    else
    {
        logger_info(module_id, "Invalid Checksum: %s\n", __func__);
        if (module_id == CC_LOG_MODULE_ID)
        {
            return CC_CHECKSUM_ERROR;
        }
        else
        {
            return GPS_NMEA_SENTENCE_CHECKSUM_ERROR;
        }
    }

    uint16_t gps_checksum_dec = hex_to_decimal(gps_checksum_hex);
    if (checksum == gps_checksum_dec)
    {
        logger_info(module_id, "VALID CHECKSUM VERIFIED: %d : %d\n", checksum, gps_checksum_dec);
        return CHECKSUM_SUCESS_CODE;
    }
    else
    {
        logger_info(module_id, "VALID CHECKSUM VERIFIED: %d : %d\n", checksum, gps_checksum_dec);

        if (module_id == CC_LOG_MODULE_ID)
        {
            return CC_CHECKSUM_ERROR;
        }
        else
        {
            return GPS_NMEA_SENTENCE_CHECKSUM_ERROR;
        }
    }
}

