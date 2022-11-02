#include "CppUTest/CommandLineTestRunner.h"
#include "CppUTestExt/MockSupportPlugin.h"
#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

extern "C"
{
#include "./../src/serial_interface/serial_config.h"
#include "./../src/client_controller/client_controller.h"
#include "./../src/cloud_server/cloud_server.h"
#include "./../src/main.h"
}

TEST_GROUP(ClientControllerTestGroup)
{
    void setup(){};
    void teardown()
    {
        mock().checkExpectations();
        mock().clear();
        mock().removeAllComparatorsAndCopiers();
    }
};

TEST(ClientControllerTestGroup, getClientControllerDataTest)
{
    /*arrange*/
    char *read_data = (char *)"$STMC,1,70.5,1,12,13,14,#";
    struct client_controller_data_struct client_controller_data;

    /*act*/
    get_client_controller_data(read_data, &client_controller_data);

    /*assert*/
    DOUBLES_EQUAL(70.5, client_controller_data.voltage, 0.0001);
    CHECK_EQUAL(1, client_controller_data.pto);
    CHECK_EQUAL(1, client_controller_data.motion);
    CHECK_EQUAL(12, client_controller_data.acc_x);
    CHECK_EQUAL(13, client_controller_data.acc_y);
    CHECK_EQUAL(14, client_controller_data.acc_z);
}

TEST(ClientControllerTestGroup, getClientControllerDataTestWithInvalidData)
{
    /*arrange*/
    char *read_data = (char *)"$STMC,,,,,,,#";
    struct client_controller_data_struct client_controller_data;

    /*act*/
    get_client_controller_data(read_data, &client_controller_data);

    /*assert*/
    DOUBLES_EQUAL(0.0, client_controller_data.voltage, 0.0001);
    CHECK_EQUAL(0, client_controller_data.pto);
    CHECK_EQUAL(0, client_controller_data.motion);
    CHECK_EQUAL(0, client_controller_data.acc_x);
    CHECK_EQUAL(0, client_controller_data.acc_y);
    CHECK_EQUAL(0, client_controller_data.acc_z);
}

TEST(ClientControllerTestGroup, stm32VerifyChecksumInValidTest)
{
    /*arrange*/
    char *sentence = (char *)"$STMC,1,70.5,1,12,13,14,#";
    char *sentence2 = (char *)"$#";
    char *sentence3 = (char *)"STMC,1,70.5,1,12,13,14,#3D";
    char *sentence4 = (char *)"$STMC,1,70.5,1,12,13,14,#31";

    /*act*/
    int result = verify_stm32_checksum(sentence);
    int result2 = verify_stm32_checksum(sentence2);
    int result3 = verify_stm32_checksum(sentence3);
    int result4 = verify_stm32_checksum(sentence4);

    /*assert*/
    CHECK_EQUAL(1002, result);
    CHECK_EQUAL(1002, result2);
    CHECK_EQUAL(1002, result3);
    CHECK_EQUAL(1002, result4);
}

TEST(ClientControllerTestGroup, stm32VerifyChecksumValidTest)
{
    /*arrange*/
    char *sentence = (char *)"$STMC,1,70.5,1,12,13,14,#3D";

    /*act*/
    int result = verify_stm32_checksum(sentence);

    /*assert*/
    CHECK_EQUAL(0, result);
}

TEST(ClientControllerTestGroup, stm32ReadThreadTest)
{
    /*arrange*/
    struct arg_struct client_controller_args;
    struct cloud_data_struct cloud_data; // = (struct cloud_data_struct)malloc(sizeof(struct cloud_data_struct));

    struct uart_device_struct client_controller_device;
    char *device_path = (char *)"/dev/ttyACM0";
    uart_setup(&client_controller_device, device_path, B115200, true);

    client_controller_args.uart_device = client_controller_device;
    client_controller_args.cloud_data = &cloud_data;
    client_controller_args.cloud_data->client_controller_data.mode = 0;

    /*act*/
    read_from_client_controller(&client_controller_args);
    uart_setup(&client_controller_device, device_path, B115200, true);
    read_from_client_controller(&client_controller_args);
    uart_setup(&client_controller_device, device_path, B115200, true);
    read_from_client_controller(&client_controller_args);
    uart_setup(&client_controller_device, device_path, B115200, true);
    read_from_client_controller(&client_controller_args);

    /*assert*/
    CHECK(cloud_data.client_controller_data.voltage > 0);
}
