/**
 ******************************************************************************
 * @file    ov2640.h
 * @brief   OV2640 Camera Driver using SCCB (I2C) and DCMI
 * @author  Generated for STM32F407 Project
 ******************************************************************************
 */

#ifndef __OV2640_H
#define __OV2640_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"

/* OV2640 Control Pins */
#define OV2640_RESET_PORT   GPIOC
#define OV2640_RESET_PIN    GPIO_PIN_10

#define OV2640_PWDN_PORT    GPIOC
#define OV2640_PWDN_PIN     GPIO_PIN_12

/* OV2640 Image Formats */
typedef enum {
    OV2640_FORMAT_JPEG_QQVGA = 0,  // 160x120 JPEG
    OV2640_FORMAT_JPEG_QVGA,       // 320x240 JPEG
} OV2640_Format_t;

/* OV2640 Status */
typedef enum {
    OV2640_OK = 0,
    OV2640_ERROR,
    OV2640_TIMEOUT,
    OV2640_ID_ERROR
} OV2640_Status_t;

/* Function Prototypes */
OV2640_Status_t OV2640_Init(OV2640_Format_t format);
OV2640_Status_t OV2640_ReadID(uint16_t *id);
OV2640_Status_t OV2640_StartCapture(uint8_t *buffer, uint32_t buffer_size);
OV2640_Status_t OV2640_StopCapture(void);

/* Low-level SCCB (I2C) functions */
OV2640_Status_t OV2640_WriteReg(uint8_t reg, uint8_t data);
OV2640_Status_t OV2640_ReadReg(uint8_t reg, uint8_t *data);

#ifdef __cplusplus
}
#endif

#endif /* __OV2640_H */
