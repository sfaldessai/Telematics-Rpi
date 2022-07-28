#include "CppUTest/CommandLineTestRunner.h"
#include "CppUTestExt/MockSupportPlugin.h"
#include "CppUTest/TestHarness.h"
#include "CppUTest/TestRegistry.h"
extern "C"
{
#include <stdio.h>
#include "src/uart.h"
}

/// Debug console
void DebugOut(const char *szTemplate, ...)
{
	va_list ap;

	va_start (ap, szTemplate);
	vprintf (szTemplate, ap);
	va_end (ap);
}

class UartDeviceStructTypeComparator : public MockNamedValueComparator
{
public:
    bool isEqual(const void *object1, const void *object2) override
    {
		DebugOut("\t Comparator - object1=%p, object2=%p\n", object1, object2 );

        // Casting here the void pointers to the type to compare
        const auto *uartDeviceObject1 = (const uart_device_struct *)object1;
        const auto *uartDeviceObject2 = (const uart_device_struct *)object2;
		DebugOut("\t Comparator - object1=%s, object2=%s\n", uartDeviceObject1->file_name, uartDeviceObject2->file_name );

        // Perform comparison, in this case, comparing x and y
        return ((uartDeviceObject1->file_name == uartDeviceObject2->file_name) && (uartDeviceObject1->baud_rate == uartDeviceObject2->baud_rate) && (uartDeviceObject1->fd == uartDeviceObject2->fd) && (uartDeviceObject1->tty == uartDeviceObject2->tty));
    }
    virtual SimpleString valueToString(const void *object)
    {   
        return StringFrom(object);
    }
};

int main(int ac, char **av)
{
    MockSupportPlugin mockPlugin;
    UartDeviceStructTypeComparator comparator;
    mockPlugin.installComparator("uartDeviceStruct", comparator);
    TestRegistry::getCurrentRegistry()->installPlugin(&mockPlugin);
    //return CommandLineTestRunner::RunAllTests(ac, av);
    return RUN_ALL_TESTS(ac, av);
}

TEST_GROUP(FirstTestGroup){
    void setup(){}

    void teardown(){}};

TEST(FirstTestGroup, FirstTest)
{
    // FAIL("Fail me!");
    // LONGS_EQUAL(0, uart_start());
}
