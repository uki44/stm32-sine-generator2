/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
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
#include "stm32l4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
int status;

extern float frequencies[10];
extern float voltages[10];
extern int time[10];
extern int processState;

#define Freq_Len 10
#define Volt_Len 10
#define Time_Len 10

#define EEPROM_ADDR 0x50
#define DIGIPOT_ADDR 0x2C
#define OLED_ADDR 0x3C

#define PAGE_NUM 16  //number of pages on the eeprom chip
#define PAGE_SIZE 16 // size of a single page in bytes on the eeprom

typedef enum states{STATE_INIT, STATE_DATA_SET, STATE_RUN, STATE_NEXT_DATA_SET,STATE_USE_DATA_EEPROM}state_t;
typedef enum dataState{NO_DATA,RECIEVED_FROM_USB,READ_FROM_MEMORY,WIP}data_state_t;
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

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define B1_Pin GPIO_PIN_13
#define B1_GPIO_Port GPIOC
#define MCO_Pin GPIO_PIN_0
#define MCO_GPIO_Port GPIOH
#define USART_TX_Pin GPIO_PIN_2
#define USART_TX_GPIO_Port GPIOA
#define USART_RX_Pin GPIO_PIN_3
#define USART_RX_GPIO_Port GPIOA
#define STATUS_LED_2_Pin GPIO_PIN_2
#define STATUS_LED_2_GPIO_Port GPIOB
#define LD4_Pin GPIO_PIN_13
#define LD4_GPIO_Port GPIOB
#define STATUS_LED_1_Pin GPIO_PIN_7
#define STATUS_LED_1_GPIO_Port GPIOC
#define TMS_Pin GPIO_PIN_13
#define TMS_GPIO_Port GPIOA
#define TCK_Pin GPIO_PIN_14
#define TCK_GPIO_Port GPIOA
#define SWO_Pin GPIO_PIN_3
#define SWO_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */
void debug_printf(const char *format, ...);
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
