#include "CppUTest/CommandLineTestRunner.h"
#include "CppUTestExt/MockSupportPlugin.h"
#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"
#include <string.h>
#include <stdio.h>
#include <stdint.h>

extern "C"
{
#include "./../src/serial_interface/serial_config.h"
#include "./../src/gps_module/gps_module.h"
#include "./../src/utils/common_utils.h"
#include "./../src/main.h"
}

TEST_GROUP(GPSTestGroup)
{
    void setup(){};
    void teardown(){};
};

TEST(GPSTestGroup, getLatLongTest)
{
    /*arrange*/
    double latitude = 1234.5678;

    /*act*/
    get_lat_log(&latitude);

    /*assert*/
    DOUBLES_EQUAL(12.576130, latitude, 0.0001);
}

TEST(GPSTestGroup, getDopsTest)
{
    /*arrange*/
    char *gsa_data = NULL;
    char *nmea_data = (char *)",A,3,26,27,09,02,28,17,12,,,,,,1.87,1.13,1.48*07";

    /*act*/
    get_dops(&gsa_data, nmea_data);

    /*assert*/
    STRCMP_EQUAL(",1.87,1.13,1.48*07", gsa_data);
}

TEST(GPSTestGroup, getGpsLatLongDataTest)
{
    /*arrange*/
    char *nmea_data = (char *)"$GNGGA,071938.00,1837.84498,N,07352.30812,E,2,08,3.41,621.3,M,-67.7,M,,0000*69";
    struct gps_data_struct gps_data;

    /*act*/
    get_gps_data(nmea_data, &gps_data);

    /*assert*/
    DOUBLES_EQUAL(18.63075, gps_data.latitude, 0.0001);
    DOUBLES_EQUAL(73.8718, gps_data.longitude, 0.0001);
}

TEST(GPSTestGroup, getGpsDopDataTest)
{
    /*arrange*/
    char *nmea_data = (char *)"$GNGSA,A,3,26,27,09,02,28,17,12,,,,,,1.87,1.13,1.48*07";
    struct gps_data_struct gps_data;

    /*act*/
    get_gps_data(nmea_data, &gps_data);

    /*assert*/
    DOUBLES_EQUAL(1.87, gps_data.pdop, 0.0001);
    DOUBLES_EQUAL(1.13, gps_data.hdop, 0.0001);
    DOUBLES_EQUAL(1.48, gps_data.vdop, 0.0001);
}

TEST(GPSTestGroup, getGprmcSpeedTest)
{

    /*arrange*/
    char *nmea_data = (char *)"$GPRMC,220516,A,5133.82,N,00042.24,W,173.8,231.8,130694,004.2,W*70";
    uint8_t position = 7;
    char *rmc_data = NULL;

    /*act*/
    get_gps_param_by_position(&rmc_data, nmea_data, position);

    /*assert*/
    STRCMP_EQUAL("173.8,231.8,130694,004.2,W*70", rmc_data);
}

TEST(GPSTestGroup, getGpvtgSpeedTest)
{

    /*arrange*/
    char *nmea_data = (char *)"$GPVTG,054.7,T,034.4,M,005.5,N,010.2,K";
    uint8_t position = 7;
    char *vtg_data = NULL;

    /*act*/
    get_gps_param_by_position(&vtg_data, nmea_data, position);

    /*assert*/
    STRCMP_EQUAL("010.2,K", vtg_data);
}

/* Test lat long value with - sign */
TEST(GPSTestGroup, getGpsLatLongDataWithSignTest)
{
    /*arrange*/
    char *nmea_data = (char *)"$GNGGA,071938.00,1837.84498,S,07352.30812,W,2,08,3.41,621.3,M,-67.7,M,,0000*69";
    struct gps_data_struct gps_data;

    /*act*/
    get_gps_data(nmea_data, &gps_data);

    /*assert*/

    DOUBLES_EQUAL(-18.63075, gps_data.latitude, 0.0001);
    DOUBLES_EQUAL(-73.8718, gps_data.longitude, 0.0001);
}

/* Test get_gps_data for invalid quality */
TEST(GPSTestGroup, getGpsLatLongDataWithInvalidSignal)
{
    /*arrange*/
    char *nmea_data = (char *)"$GLGGA,053137.00,1731.98832,N,07830.46500,E,1,05,30.42,414.3,M,-73.9,M,,*4A";
    char *nmea_data2 = (char *)"$GLGGA,053137.00,1731.98832,N,07830.46500,E,1,05,99.99,414.3,M,-73.9,M,,*4A";
    char *nmea_data3 = (char *)"$GNGSA,A,3,26,27,09,02,28,17,12,,,,,,30.87,30.13,30.48*07";
    char *nmea_data4 = (char *)"$GNGSA,A,3,26,27,09,02,28,17,12,,,,,,99.99,99.99,99.99*07";
    struct gps_data_struct gps_data;

    /*act*/
    int result = get_gps_data(nmea_data, &gps_data);
    int result2 = get_gps_data(nmea_data2, &gps_data);
    int result3 = get_gps_data(nmea_data3, &gps_data);
    int result4 = get_gps_data(nmea_data4, &gps_data);

    /*assert*/

    CHECK_EQUAL(923, result);
    CHECK_EQUAL(915, result2);
    CHECK_EQUAL(923, result3);
    CHECK_EQUAL(915, result4);
}

/* Test NMEA sentence extraction with different prefix */
TEST(GPSTestGroup, differentPrefixTest)
{
    /*arrange*/
    char *nmea_data = (char *)"$GPGGA,071938.00,1837.84498,S,07352.30812,W,2,08,3.41,621.3,M,-67.7,M,,0000*78";
    char *nmea_data_1 = (char *)"$GLGGA,071938.00,1837.84498,S,07352.30812,W,2,08,3.41,621.3,M,-67.7,M,,0000*64";
    char *nmea_data_2 = (char *)"$GNGGA,071938.00,1837.84498,S,07352.30812,W,2,08,3.41,621.3,M,-67.7,M,,0000*66";
    struct gps_data_struct gps_data, gps_data_1, gps_data_2;

    /*act*/

    int result_1 = verify_checksum(nmea_data, 2, '$', '*');
    int result_2 = verify_checksum(nmea_data_1, 2, '$', '*');
    int result_3 = verify_checksum(nmea_data_2, 2, '$', '*');

    if (result_1 == 0)
    {
        get_gps_data(nmea_data, &gps_data);
    }
    if (result_2 == 0)
    {
        get_gps_data(nmea_data_1, &gps_data_1);
    }
    if (result_3 == 0)
    {
        get_gps_data(nmea_data_2, &gps_data_2);
    }

    /*assert*/
    DOUBLES_EQUAL(-18.63075, gps_data.latitude, 0.0001);
    DOUBLES_EQUAL(-73.8718, gps_data.longitude, 0.0001);

    DOUBLES_EQUAL(-18.63075, gps_data_1.latitude, 0.0001);
    DOUBLES_EQUAL(-73.8718, gps_data_1.longitude, 0.0001);

    DOUBLES_EQUAL(-18.63075, gps_data_2.latitude, 0.0001);
    DOUBLES_EQUAL(-73.8718, gps_data_2.longitude, 0.0001);
}

/* Test ignition off with invalid fd passed */
TEST(GPSTestGroup, ignition_offWithInvalidFDTest)
{
    /*arrange*/
    struct uart_device_struct device;
    device.file_name = (char *)"filename";
    device.baud_rate = 9600;
    device.fd = 0;

    /*act*/
    int result = ignition_off(device);

    /*assert*/
    CHECK_EQUAL(912, result);
}

/* Test ignition off with valid fd passed */
TEST(GPSTestGroup, ignition_offWithValidFDTest)
{
    /*arrange*/
    struct uart_device_struct gps_device;
    char *device_path = (char *)"/dev/ttyUSB0";
    uart_setup(&gps_device, device_path, B9600, true);

    /*act*/
    int result = ignition_off(gps_device);
    uart_stop(&gps_device);

    /*assert*/
    CHECK_EQUAL(0, result);
}

/* Test ignition on with invalid fd passed */
TEST(GPSTestGroup, ignition_onWithInvalidFDTest)
{
    /*arrange*/
    struct uart_device_struct device;
    device.file_name = (char *)"filename";
    device.baud_rate = 9600;
    device.fd = 0;

    /*act*/
    int result = ignition_on(device);

    /*assert*/
    CHECK_EQUAL(912, result);
}

/* Test ignition on with valid fd passed */
TEST(GPSTestGroup, ignition_onWithValidFDTest)
{
    /*arrange*/
    struct uart_device_struct gps_device;
    char *device_path = (char *)"/dev/ttyUSB0";
    uart_setup(&gps_device, device_path, B9600, true);

    /*act*/
    int result = ignition_on(gps_device);
    uart_stop(&gps_device);

    /*assert*/
    CHECK_EQUAL(0, result);
}

/* Test gps_data_processing function */
TEST(GPSTestGroup, gps_data_processingTest)
{
    /*arrange*/
    char *read_data1 = (char *)"$GPGGA,053137.00,1731.98832,N,07830.46500,E,1,05,12.42,414.3,M,-73.9,M,,*4A";
    char *read_data2 = (char *)"$GNGGA,071938.00,1837.84498,S,07352.30812,W,2,08,3.41,621.3,M,-67.7,M,,0000*69";
    struct gps_data_struct gps_data;

    /*act*/
    int result1 = gps_data_processing(read_data1, &gps_data);
    int result2 = gps_data_processing(read_data2, &gps_data);

    /*assert*/
    CHECK_EQUAL(0, result1);
    CHECK_EQUAL(903, result2);
}

/* Test initialize_gps_module function */
TEST(GPSTestGroup, initialize_gps_moduleTest)
{
    /*arrange*/
    struct uart_device_struct gps_device;
    char *device_path = (char *)"/dev/ttyUSB0";
    uart_setup(&gps_device, device_path, B9600, true);

    /*act*/
    int result = initialize_gps_module(gps_device);
    uart_stop(&gps_device);

    /*assert*/
    CHECK_EQUAL(0, result);
}
/* Test dop_accuracy_string function */
TEST(GPSTestGroup, dop_accuracy_stringTest)
{
    /*arrange*/
    double hdop = 0.6;
    double hdop2 = 1.5;
    double hdop3 = 3;
    double hdop4 = 6;
    double hdop5 = 12;
    double hdop6 = 22;

    /*act*/
    char *dop_accuracy = dop_accuracy_string(hdop);
    char *dop_accuracy2 = dop_accuracy_string(hdop2);
    char *dop_accuracy3 = dop_accuracy_string(hdop3);
    char *dop_accuracy4 = dop_accuracy_string(hdop4);
    char *dop_accuracy5 = dop_accuracy_string(hdop5);
    char *dop_accuracy6 = dop_accuracy_string(hdop6);

    /*assert*/
    STRCMP_EQUAL("IDEAL", dop_accuracy);
    STRCMP_EQUAL("EXCELLENT", dop_accuracy2);
    STRCMP_EQUAL("GOOD", dop_accuracy3);
    STRCMP_EQUAL("MODERATE", dop_accuracy4);
    STRCMP_EQUAL("FAIR", dop_accuracy5);
    STRCMP_EQUAL("POOR", dop_accuracy6);
}
