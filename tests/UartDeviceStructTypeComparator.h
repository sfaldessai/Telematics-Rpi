
extern "C"
{
#include <stdio.h>
#include "../src/serial_interface/serial_config.h"
}

class UartDeviceStructTypeComparator : public MockNamedValueComparator
{
public:
    bool isEqual(const void* object1, const void* object2) override
    {

        /* Casting here the void pointers to the type to compare */
        const auto* uartDeviceObject1 = (const uart_device_struct*)object1;
        const auto* uartDeviceObject2 = (const uart_device_struct*)object2;

        /* Perform comparison */
        return ((uartDeviceObject1->file_name == uartDeviceObject2->file_name)
            && (uartDeviceObject1->baud_rate == uartDeviceObject2->baud_rate) &&
            (uartDeviceObject1->fd == uartDeviceObject2->fd) &&
            (uartDeviceObject1->tty == uartDeviceObject2->tty));
    }
    virtual SimpleString valueToString(const void* object)
    {
        return StringFrom(object);
    }
};