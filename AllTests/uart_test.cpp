#include "CppUTest/CommandLineTestRunner.h"
#include "CppUTestExt/MockSupportPlugin.h"
#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"


extern "C"
{
#include <stdio.h>
#include "src/uart.h"
}

int uart_start(struct uart_device_struct *device, bool canonical)
{
    return mock().actualCall("uart_start").withParameterOfType("uartDeviceStruct", "device", &device).withParameter("canonical", canonical).returnIntValueOrDefault(-1);
}

int uart_writen(struct uart_device_struct *dev, char *buf, size_t buf_len)
{
    return mock().actualCall(__func__).withParameter("device", dev).withParameter("buffer", buf).withParameter("buf_length", buf_len).returnIntValueOrDefault(-1);
}

int uart_writes(struct uart_device_struct *dev, char *string)
{
    return mock().actualCall(__func__).withParameter("device", dev).withParameter("string", string).returnIntValueOrDefault(-1);
}

int uart_reads(struct uart_device_struct *dev, char *buf, size_t buf_len)
{
    return mock().actualCall(__func__).withParameter("device", dev).withParameter("buffer", buf).withParameter("buf_length", buf_len).returnIntValueOrDefault(-1);
}

void uart_stop(struct uart_device_struct *dev)
{
    mock().actualCall(__func__).withParameter("device", dev);
}

TEST_GROUP(UartTestGroup)
{
    void setup()
    {
        mock().strictOrder();
    };
    void teardown()
    {
        // Call check expectations here, and also clear all comparators after that
        mock().checkExpectations();
        mock().clear();
        mock().removeAllComparatorsAndCopiers();
    }
};

TEST(UartTestGroup, BlackBoxTest)
{
    /*arrange*/
    struct uart_device_struct device;
    device.file_name = (char*)"filename";
    device.baud_rate = 9600;
    mock().expectOneCall("uart_start").withParameterOfType("uartDeviceStruct", "device", &device).withParameter("canonical", false).andReturnValue(0);

    /*act*/
    int fd = uart_start(&device, false);
    
    /*assert*/
    CHECK_EQUAL(0, fd);
}