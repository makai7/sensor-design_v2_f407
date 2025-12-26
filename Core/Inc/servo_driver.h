/**
 ******************************************************************************
 * @file    servo_driver.h
 * @brief   Servo motor driver for SG90 using TIM4 PWM
 * @author  Generated for STM32F407 Project
 ******************************************************************************
 */

#ifndef __SERVO_DRIVER_H
#define __SERVO_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"

/* Servo channel definitions */
#define SERVO_PAN_CHANNEL   TIM_CHANNEL_1   // PD12 - Horizontal
#define SERVO_TILT_CHANNEL  TIM_CHANNEL_2   // PD13 - Vertical

/* Servo PWM pulse width range (in timer ticks, Period = 20000 = 20ms)
 * Standard SG90: 0.5ms-2.5ms pulse width
 * 0 degree   -> 500 ticks  (0.5ms)
 * 90 degree  -> 1500 ticks (1.5ms)
 * 180 degree -> 2500 ticks (2.5ms)
 */
#define SERVO_MIN_PULSE    500    // 0 degree
#define SERVO_MAX_PULSE    2500   // 180 degree
#define SERVO_MID_PULSE    1500   // 90 degree

/* Function prototypes */
void Servo_Init(void);
void Servo_SetAngle(uint32_t channel, float angle);
void Servo_SetPulse(uint32_t channel, uint16_t pulse);

/* Pure calculation function for unit testing */
uint16_t Servo_AngleToPulse(float angle);

#ifdef __cplusplus
}
#endif

#endif /* __SERVO_DRIVER_H */
