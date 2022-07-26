#include "CppUTest/CommandLineTestRunner.h"
#include "CppUTestExt/MockSupportPlugin.h"
#include "CppUTest/TestHarness.h"

int main(int ac, char** av)
{
        return CommandLineTestRunner::RunAllTests(ac, av);
}


TEST_GROUP(FirstTestGroup)
{
    void setup()
    {
    }

    void teardown()
    {
    }
};

TEST(FirstTestGroup, FirstTest)
{
   //FAIL("Fail me!");
   //LONGS_EQUAL(0, uart_start());

}

