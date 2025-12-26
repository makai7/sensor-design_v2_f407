/**
 ******************************************************************************
 * @file    servo_driver.c
 * @brief   Servo motor driver implementation for SG90 using TIM4 PWM
 * @author  Generated for STM32F407 Project
 ******************************************************************************
 */

#include "servo_driver.h"
#include "tim.h"

/**
 * @brief  Initialize servo motors (start PWM channels)
 * @param  None
 * @retval None
 */
void Servo_Init(void)
{
    /* Start PWM on both channels */
    HAL_TIM_PWM_Start(&htim4, SERVO_PAN_CHANNEL);
    HAL_TIM_PWM_Start(&htim4, SERVO_TILT_CHANNEL);

    /* Set both servos to middle position (90 degrees) */
    Servo_SetAngle(SERVO_PAN_CHANNEL, 90.0f);
    Servo_SetAngle(SERVO_TILT_CHANNEL, 90.0f);
}

/**
 * @brief  Convert angle to PWM pulse width (pure calculation for testing)
 * @param  angle: Target angle in degrees (0-180)
 * @retval Pulse width in timer ticks (500-2500)
 */
uint16_t Servo_AngleToPulse(float angle)
{
    /* Clamp angle to valid range */
    if (angle < 0.0f) angle = 0.0f;
    if (angle > 180.0f) angle = 180.0f;

    /* Convert angle to pulse width
     * Formula: pulse = MIN_PULSE + (angle / 180) * (MAX_PULSE - MIN_PULSE)
     * Example: 90° -> 500 + (90/180) * 2000 = 500 + 1000 = 1500
     */
    return SERVO_MIN_PULSE + (uint16_t)((angle / 180.0f) * (SERVO_MAX_PULSE - SERVO_MIN_PULSE));
}

/**
 * @brief  Set servo angle (0-180 degrees)
 * @param  channel: TIM_CHANNEL_1 (Pan) or TIM_CHANNEL_2 (Tilt)
 * @param  angle: Target angle in degrees (0-180)
 * @retval None
 */
void Servo_SetAngle(uint32_t channel, float angle)
{
    uint16_t pulse = Servo_AngleToPulse(angle);

    /* Set PWM pulse width */
    Servo_SetPulse(channel, pulse);
}

/**
 * @brief  Set servo pulse width directly (in timer ticks)
 * @param  channel: TIM_CHANNEL_1 (Pan) or TIM_CHANNEL_2 (Tilt)
 * @param  pulse: Pulse width in timer ticks (500-2500)
 * @retval None
 */
void Servo_SetPulse(uint32_t channel, uint16_t pulse)
{
    /* Clamp pulse to safe range */
    if (pulse < SERVO_MIN_PULSE) pulse = SERVO_MIN_PULSE;
    if (pulse > SERVO_MAX_PULSE) pulse = SERVO_MAX_PULSE;

    /* Update PWM compare value */
    __HAL_TIM_SET_COMPARE(&htim4, channel, pulse);
}
