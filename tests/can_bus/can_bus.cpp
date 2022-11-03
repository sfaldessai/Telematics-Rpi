#include "CppUTest/CommandLineTestRunner.h"
#include "CppUTestExt/MockSupportPlugin.h"
#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"
#include <stdbool.h>

extern "C"
{
#include "./../src/can_bus/can_bus.h"
#include "./../src/cloud_server/cloud_server.h"
}

TEST_GROUP(CanBusTestGroup)
{
    void setup(){};
    void teardown(){};
};

TEST(CanBusTestGroup, validateVinValidTest)
{
    /*arrange*/
    char *vin = (char *)"5YJSA1DG9DFP14705";

    /*act*/
    bool result = validate_vin(vin);

    /*assert*/
    CHECK_EQUAL(1, result);
}

TEST(CanBusTestGroup, validateVinInValidTest)
{
    /*arrange*/
    char *vin = (char *)"5YJSA3DG9HFP14703";

    /*act*/
    bool result = validate_vin(vin);

    /*assert*/
    CHECK_EQUAL(0, result);
}

TEST(CanBusTestGroup, validateVinLenInValidTest)
{
    /*arrange*/
    char *vin = (char *)"5YJSA3DG9";

    /*act*/
    bool result = validate_vin(vin);

    /*assert*/
    CHECK_EQUAL(0, result);
}

TEST(CanBusTestGroup, getManufacturerDetailValidTest)
{
    /*arrange*/
    uint8_t *wmi = (uint8_t *)"5YJ";

    /*act*/
    char *vehicle_detail = get_manufacturer_detail(wmi);

    /*assert*/
    STRCMP_EQUAL("Tesla Motors", vehicle_detail);
}

TEST(CanBusTestGroup, getManufacturerDetailInValidTest)
{
    /*arrange*/
    uint8_t *wmi = (uint8_t *)"5YO";

    /*act*/
    char *vehicle_detail = get_manufacturer_detail(wmi);

    /*assert*/
    STRCMP_EQUAL(NULL, vehicle_detail);
}

TEST(CanBusTestGroup, readCaTemperaturePidTest)
{
    /*arrange*/
    struct cloud_data_struct cloud_data; // = (struct cloud_data_struct)malloc(sizeof(struct cloud_data_struct));
    cloud_data.can_data.mode = 0;
    pthread_t read_can_supported_thread, read_can_speed_thread, read_can_vin_thread, read_can_rpm_thread, read_can_temperature_thread;

    /*act*/
    read_from_can(&cloud_data, &read_can_supported_thread, &read_can_speed_thread, &read_can_vin_thread, &read_can_rpm_thread, &read_can_temperature_thread);

    /*assert*/
    CHECK(cloud_data.can_data.speed > 0);
}