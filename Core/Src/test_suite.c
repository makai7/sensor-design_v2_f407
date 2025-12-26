/**
 ******************************************************************************
 * @file    test_suite.c
 * @brief   Embedded Unit Test Framework Implementation
 * @author  Generated for STM32F407 Project
 ******************************************************************************
 */

#include "test_suite.h"
#include "servo_driver.h"
#include "hcsr04.h"
#include <stdio.h>
#include <math.h>

/* Private variables */
static TestResult_t testResult;

/**
 * @brief  Report test failure via UART
 * @param  file: Source file name
 * @param  line: Line number
 * @param  message: Failure message
 * @retval None
 */
void Test_ReportFailure(const char* file, uint32_t line, const char* message)
{
    printf("  [FAIL] %s:%lu - %s\r\n", file, line, message);
}

/**
 * @brief  Test servo angle to pulse conversion
 * @retval true if all tests pass, false otherwise
 */
bool Test_Servo_AngleToPulse(void)
{
    uint16_t result;

    /* Test 1: 0 degree should give 500 */
    result = Servo_AngleToPulse(0.0f);
    TEST_ASSERT_EQUAL(500, result, "0 degree should return 500");

    /* Test 2: 90 degree should give 1500 */
    result = Servo_AngleToPulse(90.0f);
    TEST_ASSERT_EQUAL(1500, result, "90 degree should return 1500");

    /* Test 3: 180 degree should give 2500 */
    result = Servo_AngleToPulse(180.0f);
    TEST_ASSERT_EQUAL(2500, result, "180 degree should return 2500");

    /* Test 4: 45 degree should give 1000 */
    result = Servo_AngleToPulse(45.0f);
    TEST_ASSERT_EQUAL(1000, result, "45 degree should return 1000");

    /* Test 5: 135 degree should give 2000 */
    result = Servo_AngleToPulse(135.0f);
    TEST_ASSERT_EQUAL(2000, result, "135 degree should return 2000");

    /* Test 6: Negative angle should clamp to 0 (500) */
    result = Servo_AngleToPulse(-10.0f);
    TEST_ASSERT_EQUAL(500, result, "Negative angle should clamp to 500");

    /* Test 7: Over 180 should clamp to 180 (2500) */
    result = Servo_AngleToPulse(200.0f);
    TEST_ASSERT_EQUAL(2500, result, "Over 180 should clamp to 2500");

    return true;
}

/**
 * @brief  Test ultrasonic pulse to distance conversion
 * @retval true if all tests pass, false otherwise
 */
bool Test_HCSR04_PulseToDistance(void)
{
    float result;

    /* Test 1: 1000us should give ~17cm
     * Formula: distance = pulseWidth * 0.017
     * 1000 * 0.017 = 17.0 cm
     */
    result = HCSR04_PulseToDistance(1000);
    TEST_ASSERT_FLOAT_EQUAL(17.0f, result, 0.1f, "1000us should return ~17cm");

    /* Test 2: 2000us should give ~34cm */
    result = HCSR04_PulseToDistance(2000);
    TEST_ASSERT_FLOAT_EQUAL(34.0f, result, 0.1f, "2000us should return ~34cm");

    /* Test 3: 5882us should give ~100cm (1m)
     * 5882 * 0.017 = 99.994 ≈ 100 cm
     */
    result = HCSR04_PulseToDistance(5882);
    TEST_ASSERT_FLOAT_EQUAL(100.0f, result, 0.5f, "5882us should return ~100cm");

    /* Test 4: 0us should give 0cm */
    result = HCSR04_PulseToDistance(0);
    TEST_ASSERT_FLOAT_EQUAL(0.0f, result, 0.01f, "0us should return 0cm");

    /* Test 5: 11764us should give ~200cm (2m)
     * 11764 * 0.017 = 199.988 ≈ 200 cm
     */
    result = HCSR04_PulseToDistance(11764);
    TEST_ASSERT_FLOAT_EQUAL(200.0f, result, 1.0f, "11764us should return ~200cm");

    /* Test 6: Small pulse (100us) should give ~1.7cm */
    result = HCSR04_PulseToDistance(100);
    TEST_ASSERT_FLOAT_EQUAL(1.7f, result, 0.1f, "100us should return ~1.7cm");

    return true;
}

/**
 * @brief  Run a single test and update results
 * @param  testFunc: Test function to run
 * @param  testName: Name of the test
 * @retval None
 */
static void Run_Single_Test(bool (*testFunc)(void), const char* testName)
{
    printf("Running: %s...\r\n", testName);

    testResult.total++;

    if (testFunc()) {
        testResult.passed++;
        printf("  [PASS] %s\r\n", testName);
    } else {
        testResult.failed++;
    }

    printf("\r\n");
}

/**
 * @brief  Run all unit tests
 * @retval None
 */
void Run_All_Tests(void)
{
    /* Initialize test results */
    testResult.total = 0;
    testResult.passed = 0;
    testResult.failed = 0;

    printf("\r\n");
    printf("========================================\r\n");
    printf(" EMBEDDED UNIT TEST SUITE\r\n");
    printf("========================================\r\n\r\n");

    /* Run all tests */
    Run_Single_Test(Test_Servo_AngleToPulse, "Servo Angle to Pulse Conversion");
    Run_Single_Test(Test_HCSR04_PulseToDistance, "HC-SR04 Pulse to Distance Conversion");

    /* Print test summary */
    printf("========================================\r\n");
    printf(" TEST SUMMARY\r\n");
    printf("========================================\r\n");
    printf("Total Tests:  %lu\r\n", testResult.total);
    printf("Passed:       %lu\r\n", testResult.passed);
    printf("Failed:       %lu\r\n", testResult.failed);

    if (testResult.failed == 0) {
        printf("\r\n*** ALL TESTS PASSED ***\r\n");
    } else {
        printf("\r\n*** %lu TEST(S) FAILED ***\r\n", testResult.failed);
    }

    printf("========================================\r\n\r\n");
}
