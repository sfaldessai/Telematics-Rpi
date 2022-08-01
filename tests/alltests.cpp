#include "CppUTest/CommandLineTestRunner.h"
#include "CppUTestExt/MockSupportPlugin.h"
#include "CppUTest/TestHarness.h"
#include "CppUTest/TestRegistry.h"
#include "UartDeviceStructTypeComparator.h"


int main(int ac, char** av)
{
    MockSupportPlugin mockPlugin;
    UartDeviceStructTypeComparator comparator;

    mockPlugin.installComparator("uartDeviceStruct", comparator);
    TestRegistry::getCurrentRegistry()->installPlugin(&mockPlugin);
    return CommandLineTestRunner::RunAllTests(ac, av);
}

TEST_GROUP(FirstTestGroup) {
    void setup() {} void teardown() {}
};