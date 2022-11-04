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

TEST(UtilsTestGroup, NmeaVerifyChecksumValidTest)
{
    /*arrange*/
    char *sentence = (char *)"$GNRMC,,V,,,,,,,,,,N*4D";
    char *sentence_1 = (char *)"$GPGGA,053137.00,1731.98832,N,07830.46500,E,1,05,12.42,414.3,M,-73.9,M,,*4A";

    /*act*/
    int result = verify_checksum(sentence, 2, '$', '*');
    int result_1 = verify_checksum(sentence_1, 2, '$', '*');

    /*assert*/
    CHECK_EQUAL(0, result);
    CHECK_EQUAL(0, result_1);
}

TEST(UtilsTestGroup, NmeaVerifyChecksumInvalidTest)
{
    /*arrange*/
    char *sentence = (char *)"$GNRMC,,V,,,,,N*4D";
    char *nmea_data_1 = (char *)"$GLGGA,053137.00,1731.98832,N,07830.46500,E,1,05,12.42,414.3,M,-73.9,M,,*4A";
    char *nmea_data_2 = (char *)"$GNGGA,053137.00,1731.98832,N,07830.46500,E,1,05,12.42,414.3,M,-73.9,M,,*4A";

    /*act*/
    int result = verify_checksum(sentence, 2, '$', '*');
    int result_1 = verify_checksum(nmea_data_1, 2, '$', '*');
    int result_2 = verify_checksum(nmea_data_2, 2, '$', '*');

    /*assert*/
    CHECK_EQUAL(903, result);
    CHECK_EQUAL(903, result_1);
    CHECK_EQUAL(903, result_2);
}

TEST(UtilsTestGroup, NmeaVerifyChecksumNullTest)
{
    /*arrange*/
    char *sentence = (char *)"\0";
    char *sentence2 = (char *)"$GNRMC,,V,,,,,N*";

    /*act*/
    int result = verify_checksum(sentence, 2, '$', '*');
    int result2 = verify_checksum(sentence2, 2, '$', '*');

    /*assert*/
    CHECK_EQUAL(903, result);
    CHECK_EQUAL(903, result2);
}



/* Test Partial NMEA sentence */
TEST(UtilsTestGroup, partialSentenceTest)
{
    /*arrange*/
    char *nmea_data = (char *)"GNGGA,071938.00,1837.84498,S,07352.30812,W,2,08,3.41,621.3,M,-67.7,M,,000069";

    /*act*/
    int result = verify_checksum(nmea_data, 2, '$', '*');

    /*assert*/
    CHECK_EQUAL(903, result);
}

/* Test Invalid lat long NMEA sentence : test for missing 'dot' in lat long value */
TEST(UtilsTestGroup, invalidLatLongTest)
{
    /*arrange*/
    char *nmea_data = (char *)"$GPGGA,071938.00,183784498,S,0735230812,W,2,08,3.41,621.3,M,-67.7,M,,0000*69";

    /*act*/
    int result = verify_checksum(nmea_data, 2, '$', '*');

    /*assert*/
    CHECK_EQUAL(903, result);
}

/* Test 'Empty values */
TEST(UtilsTestGroup, emptyvaluegTest)
{
    /*arrange*/
    char *nmea_data = (char *)"$GPGGA,,,S,,W,2,08,3.41,621.3,M,-67.7,M,,0000*69";

    /*act*/
    int result = verify_checksum(nmea_data, 2, '$', '*');

    /*assert*/
    CHECK_EQUAL(903, result);
}



TEST(UtilsTestGroup, stm32VerifyChecksumInValidTest)
{
    /*arrange*/
    char *sentence = (char *)"$STMC,1,70.5,1,12,13,14,#";
    char *sentence2 = (char *)"$#";
    char *sentence3 = (char *)"STMC,1,70.5,1,12,13,14,#3D";
    char *sentence4 = (char *)"$STMC,1,70.5,1,12,13,14,#31";

    /*act*/
    int result = verify_checksum(sentence, 5, '$', '#');
    int result2 = verify_checksum(sentence2, 5, '$', '#');
    int result3 = verify_checksum(sentence3, 5, '$', '#');
    int result4 = verify_checksum(sentence4, 5, '$', '#');

    /*assert*/
    CHECK_EQUAL(1002, result);
    CHECK_EQUAL(1002, result2);
    CHECK_EQUAL(1002, result3);
    CHECK_EQUAL(1002, result4);
}

TEST(UtilsTestGroup, stm32VerifyChecksumValidTest)
{
    /*arrange*/
    char *sentence = (char *)"$STMC,1,70.5,1,12,13,14,#3D";

    /*act*/
    int result = verify_checksum(sentence, 5, '$', '#');

    /*assert*/
    CHECK_EQUAL(0, result);
}
