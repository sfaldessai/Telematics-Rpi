#include "CppUTest/CommandLineTestRunner.h"
#include "CppUTestExt/MockSupportPlugin.h"
#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"
#include <string.h>

extern "C"
{
#include <stdio.h>
#include "../src/serial_interface/serial_config.h"
}

int uart_start(struct uart_device_struct* device, bool canonical)
{
    return mock().actualCall("uart_start").withParameterOfType("uartDeviceStruct", "device", (const void*)device).
        withParameter("canonical", canonical).returnIntValue();
}

int uart_writen(struct uart_device_struct* device, char* buf, size_t buf_len)
{
    return mock().actualCall("uart_writen").withParameterOfType("uartDeviceStruct", "device", (const void*)device).
        withParameter("buffer", buf).
        withParameter("buf_length", buf_len).returnIntValue();
}

int uart_writes(struct uart_device_struct* device, char* string)
{
    return mock().actualCall(__func__).withParameterOfType("uartDeviceStruct", "device", (const void*)device)
        .withParameter("string", string).returnIntValue();
}

int uart_reads(struct uart_device_struct* device, char* buf, size_t buf_len)
{
    return mock().actualCall(__func__).withParameterOfType("uartDeviceStruct", "device", (const void*)device).withParameter("buffer", buf).withParameter("buf_length", buf_len).returnIntValueOrDefault(-1);
}

void uart_stop(struct uart_device_struct* device)
{
    mock().actualCall(__func__).withParameterOfType("uartDeviceStruct", "device", (const void*)device);
}

TEST_GROUP(UartTestGroup)
{
    void setup() {};
    void teardown()
    {
        mock().checkExpectations();
        mock().clear();
        mock().removeAllComparatorsAndCopiers();
    }
};

TEST(UartTestGroup, uartStartTest)
{
    /*arrange*/
    struct uart_device_struct device;
    device.file_name = (char*)"filename";
    device.baud_rate = 9600;
    mock().expectOneCall("uart_start").
        withParameterOfType("uartDeviceStruct", "device", (void*)&device).
        withParameter("canonical", false).andReturnValue(0);

    /*act*/
    int fd = uart_start(&device, false);

    /*assert*/
    CHECK_EQUAL(0, fd);
}

TEST(UartTestGroup, uartwrittenTest)
{
    struct uart_device_struct device;
    device.file_name = (char*)"filename";
    device.baud_rate = 9600;
    char buf[] = "hello";
    size_t buf_len = strlen(buf);

    mock().expectOneCall("uart_writen").
        withParameterOfType("uartDeviceStruct", "device", (void*)&device).
        withParameter("buffer", buf).
        withParameter("buf_length", buf_len).andReturnValue(0);

    int write = uart_writen(&device, buf, buf_len);

    CHECK_EQUAL(0, write);
}

TEST(UartTestGroup, uartwriteTest)
{
    struct uart_device_struct device;
    device.file_name = (char*)"filename";
    device.baud_rate = 9600;
    char string[] = "hello";

    mock().expectOneCall("uart_writes").
        withParameterOfType("uartDeviceStruct", "device", (void*)&device)
        .withParameter("string", string).andReturnValue(0);

    int result = uart_writes(&device, string);

    CHECK_EQUAL(0, result);
}


TEST(UartTestGroup, uartreadsTest)
{
    struct uart_device_struct device;
    device.file_name = (char*)"filename";
    device.baud_rate = 9600;
    char buf[] = "hello";
    size_t buf_len = strlen(buf);

    mock().expectOneCall("uart_reads").
        withParameterOfType("uartDeviceStruct", "device", (void*)&device).
        withParameter("buffer", buf).
        withParameter("buf_length", buf_len).andReturnValue(0);

    int result = uart_reads(&device, buf, buf_len);

    CHECK_EQUAL(0, result);
}


TEST(UartTestGroup, uartstopTest)
{
    struct uart_device_struct device;
    device.file_name = (char*)"filename";
    device.baud_rate = 9600;

    mock().expectOneCall("uart_stop").
        withParameterOfType("uartDeviceStruct", "device", (void*)&device);

    uart_stop(&device);

    POINTERS_EQUAL(NULL, device.tty);
}