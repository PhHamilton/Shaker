/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include "tof_handler.h"
#include "serial_handler.h"
#include "tb6600.h"
#include "trajectory_planner.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define DEBUG_TRANSMIT_TIME 1000 //ms
#define MINIMUM_DISTANCE_MM 100
#define MAXIMUM_DISTANCE_MM 3000
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
static system_state_t system_state = SYSTEM_IDLE;
volatile bool tof_measurement_ready = false;
serial_packet_t serial_pkt;
tof_handler_t th;
test_suite_t test_suite;
motion_state_t motion_state;

volatile int32_t n_repetitions_counter = 0;

uint32_t latest_transmission = 0;
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
  MX_I2C2_Init();
  MX_USART1_UART_Init();
  MX_TIM14_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */

  serial_initialize(&serial_pkt);
  serial_dma_start();

  if(tof_handler_init(&th) != TOF_OK)
  {
      Error_Handler();
  }
  tof_handler_start_measurement(TOF_MEASUREMENT_ASAP);

  TB6600_init();
  motion_init(&motion_state, 0.001f);

  test_suite.j_max = 500;
  test_suite.a_max = 20;
  test_suite.v_max = 10;

  motion_state.target_d = -50;
  motion_state.steps_per_mm = 400;
  motion_state.is_running = true;

  TB6600_enable();
  HAL_TIM_Base_Start_IT(&htim2);
  HAL_TIM_PWM_Start(&htim14, TIM_CHANNEL_1);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

    if(serial_message_available())
    {
        if(serial_parse(&serial_pkt) == SERIAL_OK)
        {
            switch(serial_pkt.cmd)
            {
                case SERIAL_IDENTIFY:
                {
                    uint8_t tx_buf[9];
                    tx_buf[0] = serial_pkt.cmd;

                    for(uint8_t i = 0; i < sizeof(SW_VERSION)/sizeof(SW_VERSION[0]); i++)
                    {
                        tx_buf[i+1] = SW_VERSION[i];
                    }

                    if(serial_send(tx_buf, sizeof(tx_buf)) != SERIAL_OK)
                    {
                        // Do something..?
                    }
                }
                break;
                case SERIAL_CONFIGURE_PARAM:
                {
                    system_state = SYSTEM_CALIBRATION;
                }
                break;
                case SERIAL_CONFIGURE_TEST_SUITE:
                {
                    if(serial_pkt.payload_size != 5)
                    {
                        break;
                    }

                    test_suite.j_max = serial_pkt.payload[0];
                    test_suite.a_max = serial_pkt.payload[1];
                    test_suite.v_max = serial_pkt.payload[2];
                    test_suite.d = (int8_t)serial_pkt.payload[3];
                    test_suite.n_repetitions = serial_pkt.payload[4];

                    serial_send_ack(serial_pkt.cmd, SERIAL_ACK);
                }
                break;
                case SERIAL_TEST_INIT:
                {
                    int16_t mid_pos = MINIMUM_DISTANCE_MM + (MAXIMUM_DISTANCE_MM - MINIMUM_DISTANCE_MM)/2;
                    int16_t start_pos = mid_pos - th.x_mm;

                    motion_state.target_d = start_pos;
                    motion_state.is_running = true;
                    serial_send_ack(serial_pkt.cmd, SERIAL_ACK);
                    HAL_TIM_Base_Start_IT(&htim2);
                    HAL_TIM_PWM_Start(&htim14, TIM_CHANNEL_1);
                }
                break;
                case SERIAL_START_TEST:
                {
                    system_state = SYSTEM_RUN;

                    serial_send_ack(serial_pkt.cmd, SERIAL_ACK);

                    TB6600_enable();
                    motion_state.target_d = test_suite.d;
                    motion_state.is_running = true;
                    HAL_TIM_Base_Start_IT(&htim2);
                    HAL_TIM_PWM_Start(&htim14, TIM_CHANNEL_1);
                }
                break;
                case SERIAL_STOP_TEST:
                {
                    system_state = SYSTEM_IDLE;

                    HAL_TIM_Base_Stop_IT(&htim2);
                    TB6600_disable();
                    motion_state.is_running = false;

                    serial_send_ack(serial_pkt.cmd, SERIAL_ACK);
                }
                case SERIAL_UNKNOWN:
                {

                }
                break;
                default:
                {

                }
                break;
            }
        }
        else
        {
            asm("nop");
            __HAL_UART_SEND_REQ(&huart1, UART_RXDATA_FLUSH_REQUEST);
        }
    }

    switch(system_state)
    {
        case SYSTEM_IDLE:
        {

        }
        break;
        case SYSTEM_CALIBRATION:
        {
            tof_calibration_state_t calibration_state = tof_handler_calibrate(&th);
            if(calibration_state == TOF_CALIBRATION_COMPLETED)
            {
                system_state = SYSTEM_IDLE;
            }
            else if(calibration_state == TOF_CALIBRATION_ERROR)
            {
                Error_Handler();
            }
        }
        break;
        case SYSTEM_RUN:
        {
            uint32_t current_tick = HAL_GetTick();

            if(current_tick - latest_transmission > DEBUG_TRANSMIT_TIME)
            {
                latest_transmission = current_tick;
                uint8_t tx_buf[3];

                tx_buf[0] = SERIAL_DATA;
                tx_buf[1] = (th.x_mm >> 8) & 0xFF;
                tx_buf[2] = th.x_mm & 0xFF;
                serial_send(tx_buf, sizeof(tx_buf));
            }
        }
        break;
        default:
        {
            Error_Handler();
        }
    }

    if(tof_measurement_ready)
    {
        tof_measurement_ready = false;
        tof_handler_get_latest_measurement(&th);

        if(th.x_mm <= MINIMUM_DISTANCE_MM || th.x_mm >= MAXIMUM_DISTANCE_MM)
        {
         // STOP ALL!!
         TB6600_disable();
         HAL_TIM_Base_Stop_IT(&htim2);
         motion_state.is_running = false;
         asm("nop");
        }
    }
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
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;
  RCC_OscInitStruct.PLL.PLLN = 8;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void HAL_GPIO_EXTI_Rising_Callback(uint16_t GPIO_Pin)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(GPIO_Pin);
}

void HAL_GPIO_EXTI_Falling_Callback(uint16_t GPIO_Pin)
{
  if(GPIO_Pin == VL0x_INT_Pin)
  {
      tof_measurement_ready = true;
      asm("nop");
  }
 }
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim == &htim2)
  {
      if(motion_state.is_running)
      {
        motion_update(&motion_state, &test_suite);
      }
      else
      {
        if(test_suite.n_repetitions == 255) //Inifinate repetitions
        {
            motion_state.is_running = true;
            motion_state.target_d = -test_suite.d;
            return;
        }

        n_repetitions_counter++;

        if(n_repetitions_counter >= test_suite.n_repetitions)
        {
            HAL_TIM_Base_Stop_IT(&htim2);
        }
        else
        {
            motion_state.is_running = true;
            motion_state.target_d = -test_suite.d;
        }
      }
  }
}
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

#ifdef  USE_FULL_ASSERT
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
