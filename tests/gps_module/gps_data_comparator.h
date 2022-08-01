#include "CppUTest/CommandLineTestRunner.h"
#include "CppUTestExt/MockSupportPlugin.h"
#include "CppUTest/TestHarness.h"
#include "../../src/gps_module/gps_module.h"

#ifndef GPS_DATA_COMPARATOR_H_
#define GPS_DATA_COMPARATOR_H_

#define DEBUG

class GpsDataTypeComparator : public MockNamedValueComparator
{
public:
    bool isEqual(const void *object1, const void *object2) override
    {

        /* Casting here the void pointers to the type to compare */
        const auto *gpsDataObject1 = (const gps_data_struct *)object1;
        const auto *gpsDataObject2 = (const gps_data_struct *)object2;

        /* Perform comparison */
        return ((gpsDataObject1->latitude == gpsDataObject2->latitude) &&
                (gpsDataObject1->longitude == gpsDataObject2->longitude) &&
                (gpsDataObject1->gps_time == gpsDataObject2->gps_time) &&
                (gpsDataObject1->lat_cardinal_sign == gpsDataObject2->lat_cardinal_sign) &&
                (gpsDataObject1->long_cardinal_sign == gpsDataObject2->long_cardinal_sign) &&
                (gpsDataObject1->pdop == gpsDataObject2->pdop) &&
                (gpsDataObject1->vdop == gpsDataObject2->vdop) &&
                (gpsDataObject1->hdop == gpsDataObject2->hdop) &&
                (gpsDataObject1->lat_cardinal_sign == gpsDataObject2->lat_cardinal_sign));
    }
    virtual SimpleString valueToString(const void *object)
    {
        return StringFrom(object);
    }
};

#endif