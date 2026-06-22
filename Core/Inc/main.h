/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32g0xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
typedef enum
{
    SYSTEM_IDLE,
    SYSTEM_CALIBRATION,
    SYSTEM_RUN
}system_state_t;
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define T_NRST_Pin GPIO_PIN_2
#define T_NRST_GPIO_Port GPIOF
#define UART_TX_Pin GPIO_PIN_2
#define UART_TX_GPIO_Port GPIOA
#define UART_RX_Pin GPIO_PIN_3
#define UART_RX_GPIO_Port GPIOA
#define VL0x_XSHUT_Pin GPIO_PIN_4
#define VL0x_XSHUT_GPIO_Port GPIOA
#define VL0x_INT_Pin GPIO_PIN_5
#define VL0x_INT_GPIO_Port GPIOA
#define VL0x_INT_EXTI_IRQn EXTI4_15_IRQn
#define TB6600_PUL_Pin GPIO_PIN_1
#define TB6600_PUL_GPIO_Port GPIOB
#define STATUS_LED_Pin GPIO_PIN_6
#define STATUS_LED_GPIO_Port GPIOC
#define T_JTMS_Pin GPIO_PIN_13
#define T_JTMS_GPIO_Port GPIOA
#define T_JTCK_Pin GPIO_PIN_14
#define T_JTCK_GPIO_Port GPIOA
#define TB6600_EN_Pin GPIO_PIN_6
#define TB6600_EN_GPIO_Port GPIOB
#define TB6600_DIR_Pin GPIO_PIN_7
#define TB6600_DIR_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */
#define SW_VERSION "v1.0.0"
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
