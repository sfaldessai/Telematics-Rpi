#include "CppUTest/CommandLineTestRunner.h"
#include "CppUTestExt/MockSupportPlugin.h"
#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"
#include <string.h>
#include <stdio.h>

extern "C"
{
#include "./../src/serial_interface/serial_config.h"
#include "./../src/gps_module/gps_module.h"
}

TEST_GROUP(GPSTestGroup)
{
    void setup(){};
    void teardown()
    {
    }
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
    CHECK_EQUAL('N', gps_data.lat_cardinal_sign);
    CHECK_EQUAL('E', gps_data.long_cardinal_sign);
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