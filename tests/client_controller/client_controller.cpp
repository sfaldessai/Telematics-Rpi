#include "CppUTest/CommandLineTestRunner.h"
#include "CppUTestExt/MockSupportPlugin.h"
#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"
#include <string.h>
#include <stdio.h>

extern "C"
{
#include "./../src/serial_interface/serial_config.h"
#include "./../src/client_controller/client_controller.h"
}

TEST_GROUP(GPSTestGroup)
{
    void setup(){};
    void teardown(){};
};

TEST(GPSTestGroup, getget_clinet_controller_data)
{
    /*arrange*/
    double latitude = 1234.5678;

    /*act*/
    // get_clinet_controller_data(&latitude); 

    /*assert*/
    DOUBLES_EQUAL(12.576130, latitude, 0.0001);
}