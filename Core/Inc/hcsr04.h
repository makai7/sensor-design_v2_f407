/**
 ******************************************************************************
 * @file    hcsr04.h
 * @brief   HC-SR04 ultrasonic sensor driver using TIM3 Input Capture (Non-blocking)
 * @author  Generated for STM32F407 Project
 ******************************************************************************
 */

#ifndef __HCSR04_H
#define __HCSR04_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"

/* HC-SR04 GPIO Pin Definitions */
#define HCSR04_TRIG_PORT   GPIOB
#define HCSR04_TRIG_PIN    GPIO_PIN_0

/* Measurement status */
typedef enum {
    HCSR04_IDLE = 0,
    HCSR04_MEASURING,
    HCSR04_READY,
    HCSR04_TIMEOUT
} HCSR04_Status_t;

/* Function prototypes */
void HCSR04_Init(void);
void HCSR04_Trigger(void);
float HCSR04_GetDistance(void);
HCSR04_Status_t HCSR04_GetStatus(void);

/* Interrupt callback (to be called from stm32f4xx_it.c) */
void HCSR04_CaptureCallback(void);

/* Pure calculation function for unit testing */
float HCSR04_PulseToDistance(uint32_t pulseWidth_us);

#ifdef __cplusplus
}
#endif

#endif /* __HCSR04_H */
