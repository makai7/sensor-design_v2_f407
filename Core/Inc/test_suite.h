/**
 ******************************************************************************
 * @file    test_suite.h
 * @brief   Embedded Unit Test Framework for STM32F407 Smart Gimbal System
 * @author  Generated for STM32F407 Project
 ******************************************************************************
 */

#ifndef __TEST_SUITE_H
#define __TEST_SUITE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/* Test result structure */
typedef struct {
    uint32_t total;
    uint32_t passed;
    uint32_t failed;
} TestResult_t;

/* Test assertion macro */
#define TEST_ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            Test_ReportFailure(__FILE__, __LINE__, message); \
            return false; \
        } \
    } while(0)

/* Test assertion with tolerance for floating point */
#define TEST_ASSERT_FLOAT_EQUAL(expected, actual, tolerance, message) \
    do { \
        float _diff = (expected) - (actual); \
        if (_diff < 0) _diff = -_diff; \
        if (_diff > (tolerance)) { \
            Test_ReportFailure(__FILE__, __LINE__, message); \
            return false; \
        } \
    } while(0)

/* Test assertion for integer equality */
#define TEST_ASSERT_EQUAL(expected, actual, message) \
    do { \
        if ((expected) != (actual)) { \
            Test_ReportFailure(__FILE__, __LINE__, message); \
            return false; \
        } \
    } while(0)

/* Function prototypes */
void Test_ReportFailure(const char* file, uint32_t line, const char* message);
void Run_All_Tests(void);

/* Individual test functions */
bool Test_Servo_AngleToPulse(void);
bool Test_HCSR04_PulseToDistance(void);

#ifdef __cplusplus
}
#endif

#endif /* __TEST_SUITE_H */
