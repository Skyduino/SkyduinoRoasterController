#include <unity.h>
#include "./tests/test_ntc_class.cpp"

int main(int argc, char **argv) {
    UNITY_BEGIN();
    runAllNtcClassTests();
    UNITY_END();
}