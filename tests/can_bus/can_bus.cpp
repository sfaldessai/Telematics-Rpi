#include "CppUTest/CommandLineTestRunner.h"
#include "CppUTestExt/MockSupportPlugin.h"
#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"
#include <stdbool.h>

extern "C"
{
#include "./../src/can_bus/can_bus.h"
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