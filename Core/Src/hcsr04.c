/**
 ******************************************************************************
 * @file    hcsr04.c
 * @brief   HC-SR04 ultrasonic sensor driver using TIM3 Input Capture (Non-blocking)
 * @author  Generated for STM32F407 Project
 *
 * @note    TIM3 configured as:
 *          - Prescaler: 84-1 -> 1MHz (1us resolution)
 *          - CH4 (PB1) configured as Input Capture
 *          - Trigger pin: PB0 (GPIO Output)
 ******************************************************************************
 */

#include "hcsr04.h"
#include "tim.h"
#include "main.h"

/* Private variables */
static volatile uint32_t capture1 = 0;
static volatile uint32_t capture2 = 0;
static volatile uint8_t captureFlag = 0;
static volatile HCSR04_Status_t measureStatus = HCSR04_IDLE;
static volatile float distance_cm = 0.0f;
static volatile uint32_t timeoutCounter = 0;

/* Timeout threshold (software timeout counter, not TIM3 ticks)
 * ~100ms timeout = 100000 iterations in typical polling loop
 */
#define HCSR04_SW_TIMEOUT  100000UL

/**
 * @brief  Initialize HC-SR04 sensor
 * @param  None
 * @retval None
 */
void HCSR04_Init(void)
{
    /* Ensure TRIG pin is LOW */
    HAL_GPIO_WritePin(HCSR04_TRIG_PORT, HCSR04_TRIG_PIN, GPIO_PIN_RESET);

    /* Start Input Capture with interrupt */
    HAL_TIM_IC_Start_IT(&htim3, TIM_CHANNEL_4);

    measureStatus = HCSR04_IDLE;
}

/**
 * @brief  Trigger a distance measurement
 * @param  None
 * @retval None
 * @note   Non-blocking, result available via HCSR04_GetStatus() and HCSR04_GetDistance()
 */
void HCSR04_Trigger(void)
{
    /* Reset state */
    captureFlag = 0;
    measureStatus = HCSR04_MEASURING;
    timeoutCounter = 0;

    /* Send 10us trigger pulse */
    HAL_GPIO_WritePin(HCSR04_TRIG_PORT, HCSR04_TRIG_PIN, GPIO_PIN_SET);

    /* Delay 10us (using basic loop, more accurate than HAL_Delay) */
    for (volatile uint32_t i = 0; i < 168; i++); // ~10us at 168MHz

    HAL_GPIO_WritePin(HCSR04_TRIG_PORT, HCSR04_TRIG_PIN, GPIO_PIN_RESET);
}

/**
 * @brief  Get current measurement status
 * @param  None
 * @retval HCSR04_Status_t
 */
HCSR04_Status_t HCSR04_GetStatus(void)
{
    /* Check for timeout */
    if (measureStatus == HCSR04_MEASURING) {
        timeoutCounter++;
        if (timeoutCounter > HCSR04_SW_TIMEOUT) {
            measureStatus = HCSR04_TIMEOUT;
            distance_cm = 0.0f;
        }
    }

    return measureStatus;
}

/**
 * @brief  Get measured distance (only valid when status is HCSR04_READY)
 * @param  None
 * @retval Distance in centimeters
 */
float HCSR04_GetDistance(void)
{
    return distance_cm;
}

/**
 * @brief  Convert pulse width to distance (pure calculation for testing)
 * @param  pulseWidth_us: Pulse width in microseconds
 * @retval Distance in centimeters
 * @note   Speed of sound = 340 m/s = 0.034 cm/us
 *         Distance = (Time * 0.034) / 2 = Time * 0.017
 */
float HCSR04_PulseToDistance(uint32_t pulseWidth_us)
{
    return pulseWidth_us * 0.017f;
}

/**
 * @brief  Input Capture callback handler
 * @param  None
 * @retval None
 * @note   This should be called from HAL_TIM_IC_CaptureCallback in stm32f4xx_it.c
 */
void HCSR04_CaptureCallback(void)
{
    if (captureFlag == 0) {
        /* First edge (rising) - start of echo pulse */
        capture1 = HAL_TIM_ReadCapturedValue(&htim3, TIM_CHANNEL_4);
        captureFlag = 1;

        /* Change polarity to falling edge */
        __HAL_TIM_SET_CAPTUREPOLARITY(&htim3, TIM_CHANNEL_4, TIM_INPUTCHANNELPOLARITY_FALLING);
    }
    else if (captureFlag == 1) {
        /* Second edge (falling) - end of echo pulse */
        capture2 = HAL_TIM_ReadCapturedValue(&htim3, TIM_CHANNEL_4);

        /* Calculate pulse duration (in microseconds) */
        uint32_t pulseWidth;
        if (capture2 >= capture1) {
            pulseWidth = capture2 - capture1;
        } else {
            /* Timer overflow occurred */
            pulseWidth = (0xFFFF - capture1) + capture2;
        }

        /* Convert to distance using the pure calculation function */
        distance_cm = HCSR04_PulseToDistance(pulseWidth);

        /* Mark measurement complete */
        measureStatus = HCSR04_READY;
        captureFlag = 0;

        /* Reset polarity to rising edge for next measurement */
        __HAL_TIM_SET_CAPTUREPOLARITY(&htim3, TIM_CHANNEL_4, TIM_INPUTCHANNELPOLARITY_RISING);
    }
}
