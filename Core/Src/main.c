/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dcmi.h"
#include "dma.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include "servo_driver.h"
#include "hcsr04.h"
#include "ov2640.h"

#ifdef ENABLE_UNIT_TESTS
#include "test_suite.h"
#endif

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Image buffer for OV2640 (aligned for DMA) */
#define IMAGE_BUFFER_SIZE   (10 * 1024)  // 10KB buffer for JPEG
__attribute__((aligned(4))) uint8_t imageBuffer[IMAGE_BUFFER_SIZE];

/* Scan parameters */
float currentPanAngle = 0.0f;    // Current horizontal angle
float currentTiltAngle = 90.0f;  // Current vertical angle (fixed)
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_DCMI_Init();
  MX_I2C2_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */

  /* ========================================
   * Application Initialization
   * ======================================== */

  printf("\r\n");
  printf("========================================\r\n");
  printf(" STM32F407 Smart Gimbal System\r\n");
  printf(" Firmware Version: 1.0\r\n");
  printf("========================================\r\n\r\n");

#ifdef ENABLE_UNIT_TESTS
  /* Run Unit Tests First */
  Run_All_Tests();
#endif

  /* 1. Initialize Servo Motors */
  printf("[INIT] Initializing servos...\r\n");
  Servo_Init();
  printf("[OK] Servos initialized\r\n");

  /* 2. Initialize HC-SR04 Ultrasonic Sensor */
  printf("[INIT] Initializing ultrasonic sensor...\r\n");
  HCSR04_Init();
  printf("[OK] Ultrasonic sensor initialized\r\n");

  /* 3. Initialize OV2640 Camera */
  printf("[INIT] Initializing OV2640 camera...\r\n");
  OV2640_Status_t camStatus = OV2640_Init(OV2640_FORMAT_JPEG_QQVGA);
  if (camStatus == OV2640_OK) {
      printf("[OK] OV2640 initialized (JPEG QQVGA 160x120)\r\n");
  } else {
      printf("[ERROR] OV2640 init failed (code: %d)\r\n", camStatus);
  }

  printf("\r\n[SYSTEM READY]\r\n\r\n");

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

    /* ========================================
     * Main Scanning Loop
     * ======================================== */

    /* Step 1: Move gimbal to new position */
    Servo_SetAngle(SERVO_PAN_CHANNEL, currentPanAngle);
    Servo_SetAngle(SERVO_TILT_CHANNEL, currentTiltAngle);
    HAL_Delay(300);  // Wait for servo to stabilize

    /* Step 2: Trigger ultrasonic distance measurement */
    HCSR04_Trigger();

    /* Wait for measurement to complete (non-blocking check) */
    uint32_t timeout = 0;
    while (HCSR04_GetStatus() == HCSR04_MEASURING && timeout < 10000) {
        timeout++;
    }

    /* Step 3: Read distance */
    float distance = 0.0f;
    if (HCSR04_GetStatus() == HCSR04_READY) {
        distance = HCSR04_GetDistance();
    }

    /* Step 4: Print telemetry data */
    printf("Pan: %.1f deg | Tilt: %.1f deg | Distance: %.1f cm\r\n",
           currentPanAngle, currentTiltAngle, distance);

    /* Step 5: (Optional) Trigger camera capture
     * Uncomment the following lines to enable camera capture
     */
    /*
    if (camStatus == OV2640_OK) {
        memset(imageBuffer, 0, IMAGE_BUFFER_SIZE);
        if (OV2640_StartCapture(imageBuffer, IMAGE_BUFFER_SIZE) == OV2640_OK) {
            HAL_Delay(100);  // Wait for capture
            OV2640_StopCapture();
            printf("  [Camera] Image captured\r\n");
        }
    }
    */

    /* Step 6: Update pan angle for next scan (0 to 180 degrees) */
    currentPanAngle += 30.0f;  // Increment by 30 degrees
    if (currentPanAngle > 180.0f) {
        currentPanAngle = 0.0f;
        printf("\r\n--- Scan cycle complete, restarting ---\r\n\r\n");
    }

    /* Delay before next measurement */
    HAL_Delay(500);
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
