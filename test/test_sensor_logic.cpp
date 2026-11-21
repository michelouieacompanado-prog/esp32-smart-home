// Unity native unit tests for sensor conversion logic.

#include <unity.h>
#include "sensor/logic.hpp"

void setUp() {
}

void tearDown() {
}

void testLuxToLightIntensityZero() {
    TEST_ASSERT_EQUAL_UINT8(0, app::sensor::luxToLightIntensity(0, 2048));
}

void testLuxToLightIntensityMidpoint() {
    TEST_ASSERT_EQUAL_UINT8(50, app::sensor::luxToLightIntensity(1024, 2048));
}

void testLuxToLightIntensityFullScale() {
    TEST_ASSERT_EQUAL_UINT8(100, app::sensor::luxToLightIntensity(2048, 2048));
}

void testLuxToLightIntensitySaturation() {
    TEST_ASSERT_EQUAL_UINT8(100, app::sensor::luxToLightIntensity(3000, 2048));
    TEST_ASSERT_EQUAL_UINT8(100, app::sensor::luxToLightIntensity(65535, 2048));
}

void testLuxToLightIntensityQuarterScale() {
    TEST_ASSERT_EQUAL_UINT8(25, app::sensor::luxToLightIntensity(512, 2048));
}

void testLuxToLightIntensityCustomMaxLux() {
    TEST_ASSERT_EQUAL_UINT8(50, app::sensor::luxToLightIntensity(500, 1000));
    TEST_ASSERT_EQUAL_UINT8(100, app::sensor::luxToLightIntensity(1000, 1000));
    TEST_ASSERT_EQUAL_UINT8(100, app::sensor::luxToLightIntensity(1500, 1000));
}

void testLuxToLightIntensityZeroMaxLuxProtection() {
    TEST_ASSERT_EQUAL_UINT8(0, app::sensor::luxToLightIntensity(100, 0));
}

int main(int argc, char** argv) {
    UNITY_BEGIN();
    RUN_TEST(testLuxToLightIntensityZero);
    RUN_TEST(testLuxToLightIntensityMidpoint);
    RUN_TEST(testLuxToLightIntensityFullScale);
    RUN_TEST(testLuxToLightIntensitySaturation);
    RUN_TEST(testLuxToLightIntensityQuarterScale);
    RUN_TEST(testLuxToLightIntensityCustomMaxLux);
    RUN_TEST(testLuxToLightIntensityZeroMaxLuxProtection);
    return UNITY_END();
}
