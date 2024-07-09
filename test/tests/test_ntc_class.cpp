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

    TEST_ASSERT_EQUAL_FLOAT(212.166, ntc.AdcToTempC(297));
    TEST_ASSERT_EQUAL_FLOAT(212.833, ntc.AdcToTempC(293));
    TEST_ASSERT_EQUAL_FLOAT(117.5, ntc.AdcToTempC(1775));
    TEST_ASSERT_EQUAL_FLOAT(20.1666, ntc.AdcToTempC(3958));
    TEST_ASSERT_EQUAL_FLOAT(20.5, ntc.AdcToTempC(3956));

}

void test_temp_adc_outside_of_table(void) {
    NTC ntc;

    TEST_ASSERT_FLOAT_IS_NAN(ntc.AdcToTempC(69));
    TEST_ASSERT_FLOAT_IS_NAN(ntc.AdcToTempC(60));
    TEST_ASSERT_FLOAT_IS_NAN(ntc.AdcToTempC(61));

    TEST_ASSERT_FLOAT_IS_NAN(ntc.AdcToTempC(4087));
    TEST_ASSERT_FLOAT_IS_NAN(ntc.AdcToTempC(4095));
}


void runAllNtcClassTests() {
    RUN_TEST(test_temp_exact_find);
    RUN_TEST(test_temp_inexact_find);
    RUN_TEST(test_temp_adc_outside_of_table);
}