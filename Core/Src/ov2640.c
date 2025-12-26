/**
 ******************************************************************************
 * @file    ov2640.c
 * @brief   OV2640 Camera Driver Implementation
 * @author  Generated for STM32F407 Project
 *
 * @note    Hardware connections:
 *          - SCCB (I2C2): PB10 (SCL), PB11 (SDA)
 *          - DCMI: 8-bit parallel data + HSYNC/VSYNC/PCLK
 *          - RESET: PC10 (Active Low)
 *          - PWDN: PC12 (Active High)
 ******************************************************************************
 */

#include "ov2640.h"
#include "ov2640_regs.h"
#include "i2c.h"
#include "dcmi.h"

/* Private defines */
#define SCCB_TIMEOUT    100   // SCCB timeout in ms

/* Private variables */
static OV2640_Format_t currentFormat = OV2640_FORMAT_JPEG_QQVGA;

/**
 * @brief  Write a register via SCCB (I2C)
 * @param  reg: Register address
 * @param  data: Data to write
 * @retval OV2640_Status_t
 */
OV2640_Status_t OV2640_WriteReg(uint8_t reg, uint8_t data)
{
    uint8_t buf[2] = {reg, data};

    if (HAL_I2C_Master_Transmit(&hi2c2, OV2640_SCCB_ADDR, buf, 2, SCCB_TIMEOUT) != HAL_OK) {
        return OV2640_ERROR;
    }

    return OV2640_OK;
}

/**
 * @brief  Read a register via SCCB (I2C)
 * @param  reg: Register address
 * @param  data: Pointer to store read data
 * @retval OV2640_Status_t
 */
OV2640_Status_t OV2640_ReadReg(uint8_t reg, uint8_t *data)
{
    if (HAL_I2C_Master_Transmit(&hi2c2, OV2640_SCCB_ADDR, &reg, 1, SCCB_TIMEOUT) != HAL_OK) {
        return OV2640_ERROR;
    }

    if (HAL_I2C_Master_Receive(&hi2c2, OV2640_SCCB_ADDR, data, 1, SCCB_TIMEOUT) != HAL_OK) {
        return OV2640_ERROR;
    }

    return OV2640_OK;
}

/**
 * @brief  Read OV2640 Chip ID
 * @param  id: Pointer to store chip ID (should be 0x2642)
 * @retval OV2640_Status_t
 */
OV2640_Status_t OV2640_ReadID(uint16_t *id)
{
    uint8_t idh, idl;

    /* Select sensor register bank */
    if (OV2640_WriteReg(0xFF, 0x01) != OV2640_OK) {
        return OV2640_ERROR;
    }

    /* Read chip ID */
    if (OV2640_ReadReg(OV2640_CHIPIDH, &idh) != OV2640_OK) {
        return OV2640_ERROR;
    }

    if (OV2640_ReadReg(OV2640_CHIPIDL, &idl) != OV2640_OK) {
        return OV2640_ERROR;
    }

    *id = (idh << 8) | idl;

    return (*id == 0x2642) ? OV2640_OK : OV2640_ID_ERROR;
}

/**
 * @brief  Hardware reset OV2640
 * @param  None
 * @retval None
 */
static void OV2640_HardwareReset(void)
{
    /* Power down disable (PWDN = Low) */
    HAL_GPIO_WritePin(OV2640_PWDN_PORT, OV2640_PWDN_PIN, GPIO_PIN_RESET);
    HAL_Delay(10);

    /* Hardware reset (RESET = Low -> High) */
    HAL_GPIO_WritePin(OV2640_RESET_PORT, OV2640_RESET_PIN, GPIO_PIN_RESET);
    HAL_Delay(10);
    HAL_GPIO_WritePin(OV2640_RESET_PORT, OV2640_RESET_PIN, GPIO_PIN_SET);
    HAL_Delay(50);
}

/**
 * @brief  Write register array to OV2640
 * @param  regs: Pointer to register array
 * @param  size: Number of registers to write
 * @retval OV2640_Status_t
 */
static OV2640_Status_t OV2640_WriteArray(const OV2640_Reg_t *regs, uint16_t size)
{
    for (uint16_t i = 0; i < size; i++) {
        if (OV2640_WriteReg(regs[i].reg, regs[i].val) != OV2640_OK) {
            return OV2640_ERROR;
        }
        HAL_Delay(1); // Small delay between writes
    }

    return OV2640_OK;
}

/**
 * @brief  Initialize OV2640 camera
 * @param  format: Image format (JPEG QQVGA or QVGA)
 * @retval OV2640_Status_t
 */
OV2640_Status_t OV2640_Init(OV2640_Format_t format)
{
    uint16_t chipID;

    currentFormat = format;

    /* Hardware reset */
    OV2640_HardwareReset();

    /* Verify chip ID */
    if (OV2640_ReadID(&chipID) != OV2640_OK) {
        return OV2640_ID_ERROR;
    }

    /* Load SVGA initialization sequence */
    if (OV2640_WriteArray(OV2640_SVGA_Init, OV2640_SVGA_INIT_SIZE) != OV2640_OK) {
        return OV2640_ERROR;
    }

    /* Load JPEG mode configuration */
    if (OV2640_WriteArray(OV2640_JPEG_Init, OV2640_JPEG_INIT_SIZE) != OV2640_OK) {
        return OV2640_ERROR;
    }

    /* Load resolution-specific configuration */
    if (format == OV2640_FORMAT_JPEG_QQVGA) {
        if (OV2640_WriteArray(OV2640_JPEG_QQVGA, OV2640_JPEG_QQVGA_SIZE) != OV2640_OK) {
            return OV2640_ERROR;
        }
    } else if (format == OV2640_FORMAT_JPEG_QVGA) {
        if (OV2640_WriteArray(OV2640_JPEG_QVGA, OV2640_JPEG_QVGA_SIZE) != OV2640_OK) {
            return OV2640_ERROR;
        }
    }

    /* Enable JPEG mode in DCMI (if not already enabled) */
    hdcmi.Init.JPEGMode = DCMI_JPEG_ENABLE;
    if (HAL_DCMI_Init(&hdcmi) != HAL_OK) {
        return OV2640_ERROR;
    }

    return OV2640_OK;
}

/**
 * @brief  Start DCMI capture to memory buffer
 * @param  buffer: Pointer to image buffer
 * @param  buffer_size: Size of buffer in bytes
 * @retval OV2640_Status_t
 * @note   Buffer must be 32-bit aligned for DMA
 */
OV2640_Status_t OV2640_StartCapture(uint8_t *buffer, uint32_t buffer_size)
{
    /* Start DCMI DMA capture
     * buffer_size must be in words (divide by 4) for HAL API
     */
    if (HAL_DCMI_Start_DMA(&hdcmi, DCMI_MODE_SNAPSHOT, (uint32_t)buffer, buffer_size / 4) != HAL_OK) {
        return OV2640_ERROR;
    }

    return OV2640_OK;
}

/**
 * @brief  Stop DCMI capture
 * @param  None
 * @retval OV2640_Status_t
 */
OV2640_Status_t OV2640_StopCapture(void)
{
    if (HAL_DCMI_Stop(&hdcmi) != HAL_OK) {
        return OV2640_ERROR;
    }

    return OV2640_OK;
}
