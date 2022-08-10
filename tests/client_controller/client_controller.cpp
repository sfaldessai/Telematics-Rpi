#include "CppUTest/CommandLineTestRunner.h"
#include "CppUTestExt/MockSupportPlugin.h"
#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

extern "C"
{
#include "./../src/serial_interface/serial_config.h"
#include "./../src/client_controller/client_controller.h"
}

TEST_GROUP(ClientControllerTestGroup)
{
    void setup(){};
    void teardown(){};
};

TEST(ClientControllerTestGroup, getClientControllerDataTest)
{
    /*arrange*/
    char *read_data = (char *)"$STMC,1,70.5,1,#";
    struct client_controller_data_struct client_controller_data;

    /*act*/
    get_client_controller_data(read_data, &client_controller_data);

    /*assert*/
    DOUBLES_EQUAL(70.5, client_controller_data.voltage, 0.0001);
    CHECK_EQUAL(1, client_controller_data.pto);
    CHECK_EQUAL(1, client_controller_data.motion);
}

TEST(ClientControllerTestGroup, getClientControllerDataTestWithInvalidData)
{
    /*arrange*/
    char *read_data = (char *)"$STMC,,,,#";
    struct client_controller_data_struct client_controller_data;

    /*act*/
    get_client_controller_data(read_data, &client_controller_data);

    /*assert*/
    DOUBLES_EQUAL(0.0, client_controller_data.voltage, 0.0001);
    CHECK_EQUAL(0, client_controller_data.pto);
    CHECK_EQUAL(0, client_controller_data.motion);
}