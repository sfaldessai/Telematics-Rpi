#include "CppUTest/CommandLineTestRunner.h"
#include "CppUTestExt/MockSupportPlugin.h"
#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"
#include <string.h>

extern "C"
{
#include "./../src/utils/common_utils.h"
}

TEST_GROUP(UtilsTestGroup)
{
    void setup(){};
    void teardown(){};
};

TEST(UtilsTestGroup, getMacAddressTest)
{
    /*arrange*/
    uint8_t mac_address[12];

    /*act*/
    get_master_mac_address(mac_address);

    /*assert*/
    CHECK_EQUAL(12, strlen((char *)mac_address));
}
