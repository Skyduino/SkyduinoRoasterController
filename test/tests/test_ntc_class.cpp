#include <../src/ntc.cpp>

void test_temp_exact_find(void) {
    NTC ntc;

    TEST_ASSERT_EQUAL_FLOAT(-30, ntc.AdcToTempC(4086));
    TEST_ASSERT_EQUAL_FLOAT(0, ntc.AdcToTempC(4044));
    TEST_ASSERT_EQUAL_FLOAT(209, ntc.AdcToTempC(315));
    TEST_ASSERT_EQUAL_FLOAT(300, ntc.AdcToTempC(70));
}

void test_temp_inexact_find(void) {
    NTC ntc;

    TEST_ASSERT_EQUAL_FLOAT(0, ntc.AdcToTempC(101));
}

void runAllNtcClassTests() {
    RUN_TEST(test_temp_exact_find);
    RUN_TEST(test_temp_inexact_find);
}